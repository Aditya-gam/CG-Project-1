# Ray Tracer - CS 230 Computer Graphics (Winter 2025)

## 📌 Project Overview
This project is a **basic ray tracer** built as part of the **CS 230 - Computer Graphics** course. The ray tracer simulates how light interacts with objects to render realistic images. Over the course of the quarter, additional functionalities will be incrementally added to enhance its capabilities.

### 🌟 Features
- **Scene Parsing**: Reads scene descriptions from text files.
- **Ray Casting**: Computes intersections between rays and objects.
- **Debugging Tools**: Includes pixel traces to inspect individual pixel computations.
- **Automated Grading**: Provided grading script evaluates correctness against predefined test cases.

---

## 📁 Directory Structure

```
CG-Project-1/
│── .trunk/               # Version control system directory
│── .vscode/              # VS Code workspace settings
│── rt/                   # Contains source code and test cases
│── .gitignore            # Git ignore file
│── LICENSE               # License information
│── README.md             # Project documentation
```

---

## 🔧 Installation and Setup

### ✅ Prerequisites
Ensure you have the following dependencies installed:

- **C++ Compiler**: (`g++` for Linux/macOS, `MinGW` or `wsl` for Windows)
- **Python 3**: Required for the grading script.
- **SCons**: The build system used to compile the project.
- **libpng**: Required for image output.

### 🛠 Installation Steps

#### **Linux/macOS**
```sh
# Install dependencies
sudo apt-get install g++ python3 scons libpng-dev  # Ubuntu/Debian
brew install scons libpng                          # macOS (Homebrew)

# Clone the repository
git clone https://github.com/Aditya-gam/CG-Project-1.git
cd CG-Project-1
```

#### **Windows (Using WSL)**
1. Install **Ubuntu WSL** from Microsoft Store.
2. Open a WSL terminal and run:
   ```sh
   sudo apt-get install g++ python3 scons libpng-dev
   ```

---

## 🚀 Compilation
To build the project, simply run:
```sh
scons
```
If compilation is successful, an executable named `ray_tracer` will be generated.

---

## 🎯 Running the Ray Tracer

To render a scene:
```sh
./ray_tracer -i a/00.txt
```
This generates an output image `output.png` in the working directory.

### **Additional Options**
- **Compare with the expected solution**:
  ```sh
  ./ray_tracer -i a/00.txt -s a/00.png
  ```
- **Debug a specific pixel**:
  ```sh
  ./ray_tracer -i a/00.txt -x 350 -y 240
  ```
  The debugged pixel will be highlighted in green.

- **Change acceleration structure resolution**:
  ```sh
  ./ray_tracer -i a/00.txt -z 50
  ```

---

## ✅ Running the Grading Script
To check your progress:
```sh
python3 grading-script.py a
```
This script runs your implementation against test cases and reports discrepancies.

---

## 🛠 Development Guidelines
- **Understand the Codebase**: Read comments and documentation before modifying code.
- **Follow the TODOs**: Functions marked with `TODO` indicate work that needs to be completed.
- **Use Debugging Tools**: The `Pixel_Print` function in `misc.h` helps debug specific pixel computations.

---

## 📤 Submission Instructions
- **Package your submission**:
  ```sh
  zip hw-2.zip rt/*.h rt/*.cpp
  ```
- **If working in a team**, include both names in source file comments.

---

## 🔮 Future Enhancements
- Implement **shading models** (Phong, Blinn-Phong).
- Add **reflections and refractions** for realistic rendering.
- Improve **acceleration structures** to speed up ray tracing.

---

## 📜 Acknowledgments
This project is part of **CS 230 - Computer Graphics** at UC Riverside. Some test cases and debugging tools were provided as part of the course.

---

For any issues, refer to your course instructor or consult the project documentation.
