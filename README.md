# 🎯 Posture Corrector Device

A wearable device that uses real-time sensor feedback and computer vision to promote proper posture.

---

## 🧠 Overview

This device monitors upper back and neck posture using an **MPU-6050** sensor and a custom **OpenCV** model. Upon detecting poor posture, it emits an audible alert via a buzzer to notify the user.

---

## 🔧 First Iteration (Prototype)

- Built using an **Arduino Uno R3**, **MPU-6050**, and a **breadboard**.
- Detected angular deviations in back posture and triggered a buzzer during slouching.

---

## 🚀 Second Iteration (Refined)

- Upgraded to an **ESP32** microcontroller for wireless communication and improved performance.
- Integrated an **OpenCV + MediaPipe** model to track **neck posture** using webcam input and calculate head tilt angles.
- Real-time communication between OpenCV and ESP32 triggers the buzzer when poor neck posture is detected.
- Designed a **3D-printed wearable enclosure** using **SolidWorks** to house the electronics.
- 📂 Computer vision source code: [OpenCV Posture Model](https://github.com/james-yu2005/posture_corrector_code)

---

## 🛠️ Hardware Improvements

- Designed a **custom PCB** to replace the breadboard, enhancing compactness, reliability, and ease of assembly.
- A new **SolidWorks 3D enclosure** is in development for the updated hardware version.

---

## 🎥 Demo Videos

Explore both iterations in action:

- ▶️ **ESP32 + OpenCV-based Posture Corrector (2nd Iteration)**  
- ▶️ **Arduino-based Posture Corrector (1st Iteration)**

📁 [Watch the Demo Videos on Google Drive](https://drive.google.com/drive/folders/1XhhxoisGqGbtTAvJfZo68UkWgdPwPm_9?usp=drive_link)
