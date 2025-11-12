# 🎛️ Digital Signal Generator & Visualizer (C++ & OpenGL)

A **Digital Signal Visualization Tool** built using **C++** and **OpenGL (FreeGLUT)** that graphically represents various **digital line coding schemes** (like NRZ-L, NRZ-I, Manchester, AMI, etc.) and **analog modulation techniques** (like PCM and Delta Modulation).  
It includes real-time signal drawing, scaling, scrolling, animation, and automatic grid alignment.

---
## 👨‍💻 Contributors
- **Abishek Salaria** — *2023BITE024*  
- **Anant Gautam** — *2023BITE037*  
- **Ansh Jasrotia** — *2023BITE010*
---

## 🧠 Features

✅ Visualizes major **Digital Encoding Schemes**:
- NRZ-L (Non-Return-to-Zero Level)
- NRZ-I (Non-Return-to-Zero Inverted)
- Manchester Encoding
- Differential Manchester
- AMI (Alternate Mark Inversion)
- B8ZS / HDB3 Scrambling support

✅ Supports **Analog-to-Digital Modulations**:
- PCM (Pulse Code Modulation)
- Delta Modulation


✅ Automatic grid alignment  
✅ Dotted vertical clock lines per bit   
✅ Centered and labeled axes  
✅ Auto window resizing based on signal length  
✅ Displays longest palindrome substring in encoded stream

---

## 🧩 Encoding Logic Summary

| Encoding Type | Description |
|----------------|-------------|
| **NRZ-L** | 1 → High, 0 → Low |
| **NRZ-I** | Transition on 1, no transition on 0 |
| **Manchester** | 1 → High→Low, 0 → Low→High |
| **Differential Manchester** | Transition at start for 0, no transition for 1 |
| **AMI** | Alternate polarity for 1s, 0 stays at 0V |
| **B8ZS/HDB3** | Substitution scrambling for long zero sequences |

---


## 🧰 Requirements

- C++ compiler (MinGW / MSVC / GCC)
- [FreeGLUT](http://freeglut.sourceforge.net/)
- OpenGL libraries

---

## 🏗️ Project Structure

```bash
Signal_Generator/
│
├── include/
│   └── GL/
│       ├── freeglut.h
│       ├── freeglut_ext.h
│       ├── freeglut_std.h
│       └── glut.h
│
├── lib/
│   └── x64/
│       ├── libfreeglut.a
│       ├── libfreeglut_static.a
│       └── other OpenGL libs...
│
├── freeglut.dll
├── code.cpp
├── Signal_Generator.exe

```

## 🚀 How to Run the Project

### 1️⃣ Prerequisites
Make sure you have the following installed:
- **C++ Compiler** (GCC, MinGW, or MSVC)
- **FreeGLUT** and **OpenGL** libraries properly linked
- **GLUT include** and **lib** paths configured in your compiler

### 2️⃣ Compilation (Using g++)
If you're using **g++**, run this command inside the project folder:

```bash
g++ code.cpp -o Signal_Generator.exe -lfreeglut -lopengl32 -lglu32

```
After successful compilation, execute the program:
```bash
./Signal_Generator.exe
```

## References
- *ChatGPT*
- *Class Notes*

