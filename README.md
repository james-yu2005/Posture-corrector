# Posture Corrector Device

This is a wearable posture corrector that uses real-time sensor feedback to encourage proper posture.

## 🧠 Overview

The device monitors body and neck posture using an MPU-6050 sensor and computer vision. When poor posture is detected, it emits a buzzing sound to alert the user.

## 🔧 First Iteration

- Built using an **Arduino Uno R3**, **MPU-6050**, and a **breadboard**.
- Detected angular deviations in upper back posture and triggered a buzzer when slouching occurred.

## 🚀 Second Iteration

- Upgraded to an **ESP32** microcontroller for better performance and wireless capabilities.
- Integrated the MPU-6050 with an **OpenCV model** using **MediaPipe** to track **neck posture** via calculated angles.
- The OpenCV model runs on a connected device and sends posture data to the ESP32, which activates the buzzer if poor neck posture is detected.
- A **3D-printed enclosure** was designed in **SolidWorks** to house the components and make the device wearable.
- The computer vision code is available here: [OpenCV Posture Model](https://github.com/james-yu2005/posture_corrector_code)

## 🛠️ Hardware Improvements

- A **custom PCB** was designed to replace the breadboard setup, making the device more compact, reliable, and easier to assemble.
- A new **SolidWorks 3D enclosure** is currently being developed to house the updated PCB version.
