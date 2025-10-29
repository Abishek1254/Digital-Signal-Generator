#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <GL/freeglut.h>
using namespace std;

// ------------------ Global State ------------------
vector<pair<float, float>> waveformPoints;
float voltageScale = 1.0f;
bool animate = true;
size_t currentIndex = 0;
string encodingName = "";
int bitCount = 0;
float viewOffset = 0.0f;
float viewWidth = 15.0f;
float totalWidth = 0.0f;

// ------------------ PALINDROME O(n) ------------------
string longestPalindrome(const string& s) {
    if (s.empty()) return "";
    int n = s.size(), start = 0, maxLen = 1;
    for (int i = 0; i < n; ++i) {
        int l = i, r = i;
        while (l >= 0 && r < n && s[l] == s[r]) {
            if (r - l + 1 > maxLen) { start = l; maxLen = r - l + 1; }
            --l; ++r;
        }
        l = i; r = i + 1;
        while (l >= 0 && r < n && s[l] == s[r]) {
            if (r - l + 1 > maxLen) { start = l; maxLen = r - l + 1; }
            --l; ++r;
        }
    }
    return s.substr(start, maxLen);
}

// ------------------ Encoding Functions ------------------
string encodeNRZL(const string &data) { return data; }
string encodeNRZI(const string &data) {
    string encoded; char prev = '0';
    for (char bit : data) {
        if (bit == '1') prev = (prev == '0') ? '1' : '0';
        encoded += prev;
    }
    return encoded;
}
string encodeManchester(const string &data) {
    string encoded;
    for (char bit : data)
        encoded += (bit == '1') ? "10" : "01";
    return encoded;
}
string encodeDiffManchester(const string &data) {
    string encoded; char prev = '1';
    for (char bit : data) {
        if (bit == '0') {
            prev = (prev == '0') ? '1' : '0';
            encoded += prev;
            prev = (prev == '0') ? '1' : '0';
            encoded += prev;
        } else {
            encoded += prev;
            prev = (prev == '0') ? '1' : '0';
            encoded += prev;
        }
    }
    return encoded;
}
string encodeAMI(const string &data) {
    string encoded; char lastPulse = '-';
    for (char bit : data) {
        if (bit == '1') {
            lastPulse = (lastPulse == '+') ? '-' : '+';
            encoded += lastPulse;
        } else encoded += '0';
    }
    return encoded;
}
string scrambleB8ZS(const string &data) {
    string scrambled = data;
    for (size_t i = 0; i + 7 < scrambled.size(); i++) {
        if (scrambled.substr(i, 8) == "00000000") scrambled.replace(i, 8, "00010110");
    }
    return scrambled;
}
string scrambleHDB3(const string &data) { return data; }

// ------------ PCM/Delta Modulation ---------------
vector<int> encodePCM(const vector<double>& samples, int levels = 16) {
    vector<int> quantized;
    double minVal = *min_element(samples.begin(), samples.end());
    double maxVal = *max_element(samples.begin(), samples.end());
    for (double s : samples) {
        int q = round((s - minVal) / (maxVal-minVal) * (levels - 1));
        quantized.push_back(q);
    }
    return quantized;
}
string quantizedToBinary(const vector<int>& quantized, int bits = 4) {
    string bin;
    for (int q : quantized)
        for (int i = bits - 1; i >= 0; --i)
            bin += ((q & (1 << i)) ? '1' : '0');
    return bin;
}
string encodeDeltaModulation(const vector<double>& samples) {
    string encoded;
    double prev = 0.0, step = 1.0;
    for (double s : samples) {
        if (s >= prev) { encoded += '1'; prev += step; }
        else { encoded += '0'; prev -= step; }
    }
    return encoded;
}

// --------- OpenGL Utility ---------
void renderBitmapString(float x, float y, void *font, const char *string) {
    glRasterPos2f(x, y);
    for (const char *c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}
void drawYAxisLabels(float maxV, float minV) {
    char buf[32];
    sprintf(buf, "+V (%.2f)", maxV);
    renderBitmapString(0.2,  0.85, GLUT_BITMAP_HELVETICA_18, buf);
    sprintf(buf, "0V (%.2f)", (maxV+minV)/2);
    renderBitmapString(0.2,  0.02, GLUT_BITMAP_HELVETICA_18, buf);
    sprintf(buf, "-V (%.2f)", minV);
    renderBitmapString(0.2, -0.93, GLUT_BITMAP_HELVETICA_18, buf);
}
void drawXAxisLabels(float totalW) {
    renderBitmapString(totalW-1.0f, -0.95, GLUT_BITMAP_HELVETICA_18, "Time →");
    renderBitmapString(totalW/2-2.5, -0.98, GLUT_BITMAP_HELVETICA_12, "Bit Number (Interval)");
}

// ----------- Display -------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glTranslatef(-viewOffset, 0.0f, 0.0f);

    // Y-grid
    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_LINES);
    for (float y = -1.0f; y <= 1.0f; y += 0.1f) {
        glVertex2f(0.0f, y); glVertex2f(totalWidth, y);
    }
    glEnd();

    // Dotted vertical bit interval lines
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xAAAA);
    glColor3f(0.4, 0.4, 0.4);
    glBegin(GL_LINES);
    for (int i = 0; i <= bitCount; ++i) {
        float x = i * 1.0;
        glVertex2f(x, -1.0f); glVertex2f(x, 1.0f);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // X and Y axes
    glColor3f(0.9f, 0.9f, 0.9f);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex2f(0.0f, -1.0f); glVertex2f(0.0f, 1.0f);     // y axis
    glVertex2f(0.0f, 0.0f); glVertex2f(totalWidth, 0.0f); // x axis
    glEnd();

    // Signal waveform
    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    size_t end = animate ? min(currentIndex, waveformPoints.size()) : waveformPoints.size();
    for (size_t i = 0; i < end; ++i)
        glVertex2f(waveformPoints[i].first, waveformPoints[i].second * voltageScale);
    glEnd();

    // Labels (Encoding name centered at the top middle of view)
    glColor3f(1.0f, 1.0f, 1.0f);
    drawYAxisLabels(0.5, -0.5);
    drawXAxisLabels(totalWidth);

    // Center encoding name in top middle
    float centerX = viewOffset + viewWidth / 2.0f - (encodingName.size() * 0.055f); // Small adjustment for centering text
    renderBitmapString(centerX, 0.92, GLUT_BITMAP_HELVETICA_18, encodingName.c_str());

    glPopMatrix();
    glutSwapBuffers();
}

// ----------- Timer / Input ------------
void timer(int value) {
    if (animate) {
        if (currentIndex < waveformPoints.size()) {
            currentIndex++;
            glutPostRedisplay();
            glutTimerFunc(25, timer, 0);
        }
    }
}
void keyboard(unsigned char key, int, int) {
    switch (key) {
        case '+': voltageScale += 0.1f; break;
        case '-': voltageScale = max(0.1f, voltageScale - 0.1f); break;
        case 'r': currentIndex = 0; animate = true; glutTimerFunc(0, timer, 0); break;
        case 'f': animate = false; currentIndex = waveformPoints.size(); break;
        case 27: exit(0);
    }
    glutPostRedisplay();
}
void specialKeys(int key, int, int) {
    // Scroll is only useful for very large signals. But all are visible by default now!
    float scrollSpeed = max(1.0f, viewWidth / 4.0f);
    if (key == GLUT_KEY_RIGHT) viewOffset = min(totalWidth - viewWidth, viewOffset + scrollSpeed);
    else if (key == GLUT_KEY_LEFT) viewOffset = max(0.0f, viewOffset - scrollSpeed);
    glutPostRedisplay();
}
void setupGL() {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, viewWidth, -1, 1);
}
void digitalStringToWaveform(const string &data) {
    waveformPoints.clear();
    // Special handling for Manchester or Differential Manchester
    bool isManchester = (encodingName.find("Manchester") != string::npos);

    if (!isManchester) {
        bitCount = data.size();
        float bitStep = 1.0f;
        totalWidth = bitCount * bitStep;
        viewWidth = totalWidth;

        auto getLevel = [](char ch) {
            if (ch == '0') return -0.5f;
            if (ch == '1' || ch == '+') return 0.5f;
            if (ch == '-') return -0.5f;
            return 0.0f;
        };
        float x = 0.0f;
        float y = getLevel(data[0]);
        waveformPoints.push_back({x, y});

        for (int i = 0; i < bitCount; ++i) {
            float x_next = x + bitStep;
            float y_now = getLevel(data[i]);
            waveformPoints.push_back({x_next, y_now});
            if (i + 1 < bitCount) {
                float y_next = getLevel(data[i + 1]);
                if (fabs(y_next - y_now) > 1e-6)
                    waveformPoints.push_back({x_next, y_next});
            }
            x = x_next;
        }
    } else {
        // Manchester/Diff Manchester handling (every bit = 2 encoded states)
        int logicBits = (int)(data.size() / 2);
        bitCount = logicBits;
        float bitStep = 1.0f;
        totalWidth = bitCount * bitStep;
        viewWidth = totalWidth;

        auto getLevel = [](char ch) {
            if (ch == '0') return -0.5f;
            if (ch == '1') return 0.5f;
            return 0.0f;
        };

        float x = 0.0f;
        float y = getLevel(data[0]);
        waveformPoints.push_back({x, y});

        for (int i = 0; i < logicBits; ++i) {
            float x_mid = x + bitStep / 2.0f;
            float x_next = x + bitStep;
            float y_left = getLevel(data[2 * i]);
            float y_right = getLevel(data[2 * i + 1]);

            waveformPoints.push_back({x_mid, y_left});
            if (fabs(y_right - y_left) > 1e-6)
                waveformPoints.push_back({x_mid, y_right});
            waveformPoints.push_back({x_next, y_right});
            if (i + 1 < logicBits && fabs(getLevel(data[2 * (i + 1)]) - y_right) > 1e-6)
                waveformPoints.push_back({x_next, getLevel(data[2 * (i + 1)])});

            x = x_next;
        }
    }
}

// ----------- Main Logic ---------------
int main(int argc, char **argv) {
    string digitalData, encodedData;
    vector<double> analogSamples;
    bool isDigital = false;
    cout << "Digital or analog input? (1: Digital, 2: Analog): ";
    int inpType; cin >> inpType;

    if (inpType == 1) {
        isDigital = true;
        cout << "Enter digital bit stream (eg 011010...): ";
        cin >> digitalData;
    } else if (inpType == 2) {
        cout << "Enter number of analog samples: ";
        int n; cin >> n;
        analogSamples.resize(n);
        cout << "Enter samples (space separated): ";
        for (int i = 0; i < n; ++i) cin >> analogSamples[i];
    } else {
        cout << "Invalid input type.\n"; return 1;
    }

    // Encode
    if (isDigital) {
        cout << "Coding: 1 NRZ-L 2 NRZ-I 3 Manchester 4 DiffManch 5 AMI: ";
        int code; cin >> code;
        if (code == 1) { encodedData = encodeNRZL(digitalData); encodingName = "NRZ-L"; }
        else if (code == 2) { encodedData = encodeNRZI(digitalData); encodingName = "NRZ-I"; }
        else if (code == 3) { encodedData = encodeManchester(digitalData); encodingName = "Manchester"; }
        else if (code == 4) { encodedData = encodeDiffManchester(digitalData); encodingName = "Differential Manchester"; }
        else if (code == 5) {
            encodingName = "AMI";
            cout << "Scrambling? (1: Yes, 0: No): "; int scr; cin >> scr;
            encodedData = encodeAMI(digitalData);
            if (scr == 1) {
                cout << "Type: 1 B8ZS 2 HDB3: "; int st; cin >> st;
                if (st == 1) { encodedData = scrambleB8ZS(encodedData); encodingName += " + B8ZS"; }
                else { encodedData = scrambleHDB3(encodedData); encodingName += " + HDB3"; }
            }
        } else { cout << "Invalid choice\n"; return 1; }
    } else {
        cout << "Modulation: 1 PCM 2 Delta Modulation: ";
        int mod; cin >> mod;
        if (mod == 1) {
            vector<int> q = encodePCM(analogSamples);
            digitalData = quantizedToBinary(q);
        } else if (mod == 2) {
            digitalData = encodeDeltaModulation(analogSamples);
        } else { cout << "Invalid mod.\n"; return 1; }
        cout << "Coding: 1 NRZ-L 2 NRZ-I 3 Manchester 4 DiffManch 5 AMI: ";
        int code; cin >> code;
        if (code == 1) { encodedData = encodeNRZL(digitalData); encodingName = "NRZ-L"; }
        else if (code == 2) { encodedData = encodeNRZI(digitalData); encodingName = "NRZ-I"; }
        else if (code == 3) { encodedData = encodeManchester(digitalData); encodingName = "Manchester"; }
        else if (code == 4) { encodedData = encodeDiffManchester(digitalData); encodingName = "Differential Manchester"; }
        else if (code == 5) {
            encodingName = "AMI";
            cout << "Scrambling? (1: Yes, 0: No): "; int scr; cin >> scr;
            encodedData = encodeAMI(digitalData);
            if (scr == 1) {
                cout << "Type: 1 B8ZS 2 HDB3: "; int st; cin >> st;
                if (st == 1) { encodedData = scrambleB8ZS(encodedData); encodingName += " + B8ZS"; }
                else { encodedData = scrambleHDB3(encodedData); encodingName += " + HDB3"; }
            }
        } else { cout << "Invalid choice\n"; return 1; }
    }

    cout << "\nDigital Data: " << digitalData << endl;
    cout << "Encoded Stream: " << encodedData << endl;
    cout << "Longest Palindrome: " << longestPalindrome(encodedData) << endl;
    digitalStringToWaveform(encodedData);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize((int)max(1200.0f, totalWidth * 70), 500); // auto-width for huge signals!
    glutCreateWindow("Digital Signal Generator Visualization");
    setupGL();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}
