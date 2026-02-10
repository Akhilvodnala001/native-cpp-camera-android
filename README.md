# Native C/C++ Camera Application (Android NDK)

A fully native **Android Camera application** built using **C/C++ (NDK)** and the **Camera2 NDK API**, targeting **AOSP Vanilla (Ice Cream)**.  
This project minimizes Java/Kotlin usage and demonstrates direct interaction with Android’s native camera stack.

---

## 📌 Features
- Native camera preview using `ANativeWindow`
- Still image capture (JPEG) via `AImageReader`
- Image saved directly to Android Gallery using MediaStore
- Camera lifecycle fully handled in C++
- Minimal Java layer (permissions + lifecycle only)
- Designed for AOSP / embedded Android environments

---

## 🛠 Technology Stack
- **Language:** C++ (NDK), minimal Kotlin
- **Camera API:** Camera2 NDK (`camera2ndk`)
- **Media:** Media NDK (`mediandk`)
- **UI:** XML + TextureView
- **Build System:** Gradle (Kotlin DSL) + CMake
- **Android:** minSdk 26, targetSdk 36

---

## 📐 Architecture Overview
UI (XML / TextureView)
↓
MainActivity (Kotlin)
↓ JNI
Native Layer (C++)
↓
Camera2 NDK → Camera HAL → Hardware



---

## ▶️ Build & Run
1. Open project in **Android Studio**
2. Grant **Camera permission**
3. Run on a real Android device (Camera HAL required)
4. Tap **PHOTO** to capture image

> Emulator is not recommended (no full camera HAL support)

---

## 📂 Documentation
Detailed project documentation is available here:  
📄 [`docs/Project_Documentation.pdf`](docs/Project_Documentation.pdf)

---

## 🚀 Future Enhancements
- Video recording using MediaCodec + MediaMuxer
- Front/Back camera switching
- Autofocus & exposure control
- System app integration for AOSP

---

## 👤 Author
**Akhil**  
Android NDK | Embedded Systems | OS Internals


