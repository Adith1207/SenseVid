# Modifications to SenseVid Source Code

**Modified by:** I.Karthik Saiharsh
**Date:** July 10, 2025
**Purpose:** Update for compatibility with modern OpenCV (4.x) and general build fixes

---

## 🔧 Summary of Changes

### ✅ OpenCV Compatibility Fixes
- Replaced obsolete header files with modern OpenCV 4.x headers
- Replaced deprecated constants and enumerations
- Adjusted function calls to match updated OpenCV API syntax

### ✅ Compiler and Build Fixes
- Added missing `#include <iostream>` in source files that used `std::cout`, `std::cerr`, etc.
- Added `using namespace std;` where necessary to resolve standard namespace issues

### ✅ Build Environment
- Successfully compiled with:
  - OpenCV 4.12.0
  - GCC 13.3.0
  - CMake 3.30.5
  - Ubuntu 24.04 LTS

---

## 📘 License Notes

These modifications are distributed under the same **Academic Public License** as the original SenseVid software. See [`LICENSE`](./LICENSE_SENSEVID) for full terms. This modified version is intended strictly for **non-commercial academic and research purposes**.

---

## 📩 Contact

If you encounter any issues or have questions about the changes, feel free to open an issue or pull request.

