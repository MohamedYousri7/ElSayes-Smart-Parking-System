# 🚗 Smart Parking System (AI + IoT)

![Python](https://img.shields.io/badge/Python-3.8%2B-blue)
![YOLOv8](https://img.shields.io/badge/YOLOv8-00FFFF?style=flat&logo=ultrasignup&logoColor=black)
![OpenCV](https://img.shields.io/badge/OpenCV-5C3EE8?style=flat&logo=opencv&logoColor=white)
![IoT](https://img.shields.io/badge/IoT-ESP32-green)
![Flutter](https://img.shields.io/badge/Flutter-02569B?style=flat&logo=flutter&logoColor=white)
![Supabase](https://img.shields.io/badge/Supabase-3ECF8E?style=flat&logo=supabase&logoColor=white)

An AI-powered Smart Parking System that integrates Computer Vision, IoT devices, and Cloud services to automate vehicle entry and exit using License Plate Recognition (LPR).

This project was developed as a **Graduation Project** showcasing the combination of Artificial Intelligence, IoT, Web, and Mobile Applications for solving real-world parking congestion challenges.

## 📖 Problem Statement

Parking management is a significant challenge in crowded urban areas. Manual ticketing systems waste time, increase traffic congestion, and require constant human intervention.

**Our Solution →** An automated system that detects vehicles, recognizes their license plates using YOLOv8 + OCR, verifies entry in the database, and manages gate access seamlessly without human intervention.

## 💡 Features

- **🔍 AI-based License Plate Recognition** using YOLOv8 + OCR
- **📡 IoT Hardware Integration** (ESP32, IR sensors, LEDs, Servo motors) for gate automation
- **☁️ Cloud Integration** with Supabase for real-time synchronization
- **📱 Mobile & Web Applications** (Flutter + Web Dashboard) for monitoring and management
- **📊 Database Management** for users, vehicles, and parking slots
- **⚡ Fast Response**: Real-time detection & verification in < 2 seconds
- **🔐 Security**: Automated access control and vehicle verification

## ⚙️ Tech Stack

### **AI/Computer Vision**
![YOLOv8](https://img.shields.io/badge/YOLOv8-00FFFF?style=flat&logo=ultrasignup&logoColor=black)
![OpenCV](https://img.shields.io/badge/OpenCV-5C3EE8?style=flat&logo=opencv&logoColor=white)
![Tesseract](https://img.shields.io/badge/Tesseract-000000?style=flat&logo=tesseract&logoColor=white)
![Roboflow](https://img.shields.io/badge/Roboflow-FF6B6B?style=flat&logo=roboflow&logoColor=white)

### **IoT & Hardware**
![ESP32](https://img.shields.io/badge/ESP32-000000?style=flat&logo=espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-00979D?style=flat&logo=arduino&logoColor=white)
![IoT](https://img.shields.io/badge/IoT-Sensors-green)

### **Backend & Cloud**
![Python](https://img.shields.io/badge/Python-3776AB?style=flat&logo=python&logoColor=white)
![FastAPI](https://img.shields.io/badge/FastAPI-009688?style=flat&logo=fastapi&logoColor=white)
![Supabase](https://img.shields.io/badge/Supabase-3ECF8E?style=flat&logo=supabase&logoColor=white)

### **Frontend & Mobile**
![Flutter](https://img.shields.io/badge/Flutter-02569B?style=flat&logo=flutter&logoColor=white)
![Dart](https://img.shields.io/badge/Dart-0175C2?style=flat&logo=dart&logoColor=white)
![Web Dashboard](https://img.shields.io/badge/Web_Dashboard-React-blue)

## 🎯 My Role: AI & Computer Vision Lead

**End-to-End Responsibility for AI/Computer Vision Components:**

### 🔬 Research & Development
- **Model Selection**: Evaluated and selected YOLOv8 for optimal license plate detection
- **Dataset Curation**: Collected and annotated 2,500+ Egyptian license plate images
- **Algorithm Optimization**: Fine-tuned detection parameters for real-time performance

### 🛠️ Implementation
```python
# End-to-End LPR Pipeline I Developed
1. Vehicle Detection → YOLOv8 model training & optimization
2. License Plate Localization → Bounding box detection with 99.5% mAP
3. Image Preprocessing → Noise reduction, contrast enhancement
4. Character Segmentation → Individual character isolation
5. OCR Processing → Roboflow + custom Arabic character recognition
6. Post-processing → Right-to-left Arabic text interpretation
