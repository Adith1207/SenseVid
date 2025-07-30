# 🎥 SenseVid (Updated for OpenCV 4.x)

An updated version of the [SenseVid](http://w3.cran.univ-lorraine.fr/perso/moufida.maimour/SenseVid/sensevid.html) tool, modernized to work with **OpenCV 4.x** and current Linux environments. SenseVid is a video transmission and evaluation tool for **Wireless Sensor Networks (WSNs)**, offering functionality similar to **EvalVid**, but tailored specifically for video-based WSN protocol assessment.

---

## 📌 Why This Repo?

The original SenseVid codebase was designed for **OpenCV 2.x**, which is now obsolete. This repository:

- Updates all legacy OpenCV references to be compatible with **OpenCV 4.x**
- Enables modern compilation using **CMake 3.x** and **GCC 13+**
- Ensures smooth operation on up-to-date Linux distributions

---

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
