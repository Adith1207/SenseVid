# 🎥 SenseVid (Updated for OpenCV 4.x)

This repository contains an updated version of the [SenseVid](http://w3.cran.univ-lorraine.fr/perso/moufida.maimour/SenseVid/sensevid.html) tool, modified to work with modern OpenCV (4.x) versions. SenseVid is a a new video transmission and evaluation tool that follows the same principle of EvalVid while being specific to WSN. SenseVid can help network researchers to assess network protocols targeted to video applications in WSN.

## 📌 Why This Repo?

The original SenseVid source was last updated for OpenCV 2.x and uses outdated headers and constants. This version fixes those issues and ensures compatibility with OpenCV 4.x and modern Linux distributions.

## ✅ What's Changed

- Replaced old OpenCV headers with current ones.
- Replaced deprecated constants with modern equivalents (`cv::CAP_*`, etc.).
- Added missing includes like `<iostream>` and applied `using namespace std;` where appropriate.
- Minor code cleanups for compiler compatibility with g++ / cmake 3.x and OpenCV 4.x.

## 🧪 Compatibility

- ✅ Tested with OpenCV 4.12
- ✅ Compiles with GCC 13.3, CMake 3.30
- ✅ Works with Cooja integration for vision-based simulation in academic setups

## 📁 Original License

This project retains the original **Academic Public License**. You are free to use, modify, and distribute this code **for non-commercial academic and research purposes only**.

> The full license is included in the [`LICENSE`](./LICENSE_SENSEVID) file.

## 🛠️ Building the Project

```bash
cmake .
make
```
