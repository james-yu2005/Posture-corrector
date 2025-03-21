import cv2
import mediapipe as mp
import numpy as np
import time
from playsound import playsound
import os
import math
import requests
import time

# Replace with your ESP32's IP address
esp32_ip = "ip-address"

# URLs to turn the buzzer on and off
buzzer_on_url = f"{esp32_ip}/buzzer/on"
buzzer_off_url = f"{esp32_ip}/buzzer/off"

# Initialize MediaPipe Pose and webcam
mp_pose = mp.solutions.pose
mp_drawing = mp.solutions.drawing_utils
pose = mp_pose.Pose(static_image_mode=False, min_detection_confidence=0.5, min_tracking_confidence=0.5)

# Function to turn the buzzer on
def turn_buzzer_on():
    try:
        response = requests.get(buzzer_on_url)
        if response.status_code == 200:
            print("Buzzer is ON")
        else:
            print(f"Failed to turn buzzer on. Status code: {response.status_code}")
    except Exception as e:
        print(f"Error: {e}")

# Function to turn the buzzer off
def turn_buzzer_off():
    try:
        response = requests.get(buzzer_off_url)
        if response.status_code == 200:
            print("Buzzer is OFF")
        else:
            print(f"Failed to turn buzzer off. Status code: {response.status_code}")
    except Exception as e:
        print(f"Error: {e}")

# Start webcam
cap = cv2.VideoCapture(0)

# Check if the webcam is accessible
if not cap.isOpened():
    print("Error: Could not access the camera.")
    exit()

# Initialize other necessary variables for posture analysis
calibration_frames = 0
calibration_shoulder_angles = []
calibration_neck_angles = []
is_calibrated = False
shoulder_threshold = 0
neck_threshold = 0
last_alert_time = 0
alert_cooldown = 3
sound_file = 'alert_sound.mp3'

def calculate_distance(p1, p2):
    return math.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2)

# Function to calculate the angle between three points
# b is middle
def calculate_angle(a, b, c):
    # Calculate the angle using the law of cosines or other geometry logic
    ab = calculate_distance(a, b)
    bc = calculate_distance(b, c)
    ac = calculate_distance(a, c)

    # Apply the Law of Cosines to find the angle at point b
    angle = math.acos(((ac ** 2) - (bc ** 2) - (ab ** 2)) / (-2 * bc * ab))

    # Convert the angle from radians to degrees (if needed)
    angle_degrees = math.degrees(angle)
    return angle_degrees

# Function to draw the calculated angle on the frame
def draw_angle(frame, point1, point2, point3, angle, color=(0, 255, 0)):
    # Draw lines between the points
    cv2.line(frame, point1, point2, color, 2)  # Line between point1 and point2
    cv2.line(frame, point2, point3, color, 2)  # Line between point2 and point3

    # Draw a circle at point2 (the vertex of the angle)
    cv2.circle(frame, point2, 5, color, -1)

    # Draw the angle text on the image
    font = cv2.FONT_HERSHEY_SIMPLEX
    cv2.putText(frame, f'{int(angle)}°', (point2[0] + 10, point2[1] - 10), font, 1, color, 2, cv2.LINE_AA)

# Main loop
try:
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            print("Failed to capture frame. Retrying...")
            continue

        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = pose.process(rgb_frame)

        if results.pose_landmarks:
            landmarks = results.pose_landmarks.landmark

            # STEP 2: Pose Detection
            # Extract key body landmarks
            left_shoulder = (int(landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].x * frame.shape[1]),
                             int(landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value].y * frame.shape[0]))
            right_shoulder = (int(landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER.value].x * frame.shape[1]),
                              int(landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER.value].y * frame.shape[0]))
            left_ear = (int(landmarks[mp_pose.PoseLandmark.LEFT_EAR.value].x * frame.shape[1]),
                        int(landmarks[mp_pose.PoseLandmark.LEFT_EAR.value].y * frame.shape[0]))
            right_ear = (int(landmarks[mp_pose.PoseLandmark.RIGHT_EAR.value].x * frame.shape[1]),
                         int(landmarks[mp_pose.PoseLandmark.RIGHT_EAR.value].y * frame.shape[0]))

            # STEP 3: Angle Calculation
            shoulder_angle = calculate_angle(left_shoulder, right_shoulder, (right_shoulder[0], 0))
            neck_angle = calculate_angle(left_ear, left_shoulder, (left_shoulder[0], 0))

            # STEP 1: Calibration
            if not is_calibrated and calibration_frames < 30:
                calibration_shoulder_angles.append(shoulder_angle)
                calibration_neck_angles.append(neck_angle)
                calibration_frames += 1
                cv2.putText(frame, f"Calibrating... {calibration_frames}/30", (10, 30), 
                            cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2, cv2.LINE_AA)
            elif not is_calibrated:
                shoulder_threshold = np.mean(calibration_shoulder_angles) - 10
                neck_threshold = np.mean(calibration_neck_angles) - 10
                is_calibrated = True
                print(f"Calibration complete. Shoulder threshold: {shoulder_threshold:.1f}, Neck threshold: {neck_threshold:.1f}")

            # Draw skeleton and angles
            mp_drawing.draw_landmarks(frame, results.pose_landmarks, mp_pose.POSE_CONNECTIONS)
            midpoint = ((left_shoulder[0] + right_shoulder[0]) // 2, (left_shoulder[1] + right_shoulder[1]) // 2)
            draw_angle(frame, left_shoulder, midpoint, (midpoint[0], 0), shoulder_angle, (255, 0, 0))
            draw_angle(frame, left_ear, left_shoulder, (left_shoulder[0], 0), neck_angle, (0, 255, 0))

            # STEP 4: Feedback
            if is_calibrated:
                current_time = time.time()
                if shoulder_angle < shoulder_threshold or neck_angle < neck_threshold:
                    turn_buzzer_on()
                    status = "Poor Posture"
                    color = (0, 0, 255)  # Red
                    if current_time - last_alert_time > alert_cooldown:
                        print("Poor posture detected! Please sit up straight.")
                        if os.path.exists(sound_file):
                            playsound(sound_file)
                        last_alert_time = current_time
                else:
                    status = "Good Posture"
                    turn_buzzer_off()
                    color = (0, 255, 0)  # Green

                cv2.putText(frame, status, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2, cv2.LINE_AA)
                cv2.putText(frame, f"Shoulder Angle: {shoulder_angle:.1f}/{shoulder_threshold:.1f}", (10, 60), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1, cv2.LINE_AA)
                cv2.putText(frame, f"Neck Angle: {neck_angle:.1f}/{neck_threshold:.1f}", (10, 90), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1, cv2.LINE_AA)

        # Display the frame
        cv2.imshow('Posture Corrector', frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("Program interrupted by user.")
finally:
    cap.release()
    cv2.destroyAllWindows()

