import cv2
import mediapipe as mp
import math
import winsound
import numpy as np

def detect_hands(frame, mp_hands):
    image_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = mp_hands.process(image_rgb)
    hands = []
    if results.multi_hand_landmarks:    
        for hand_landmarks in results.multi_hand_landmarks:
            x_min, y_min, x_max, y_max = 10000, 10000, 0, 0
            for landmark in hand_landmarks.landmark:
                x, y = int(landmark.x * frame.shape[1]), int(landmark.y * frame.shape[0])
                if x < x_min:
                    x_min = x
                if x > x_max:
                    x_max = x
                if y < y_min:
                    y_min = y
                if y > y_max:
                    y_max = y
            hands.append([x_min, y_min, x_max, y_max])
    return hands

def draw_hand_boxes(image, hands):
    for box in hands:
        x_min, y_min, x_max, y_max = box
        cv2.rectangle(image, (x_min, y_min), (x_max, y_max), (0, 255, 0), 4)

def draw_safety_circle(image, center, radius):  
    cv2.circle(image, center, radius, (0, 0, 255), 2)

def is_hand_touching_circle(box, circle_center, circle_radius):
    circle_center_x, circle_center_y = safety_circle_center
    circle_radius = safety_circle_radius

    # Check if any corner of the bounding box is inside or on the circumference of the safety circle
    for x, y in [(box[0], box[1]), (box[2], box[1]), (box[0], box[3]), (box[2], box[3])]:
        distance = math.sqrt((x - circle_center_x)**2 + (y - circle_center_y)**2)
        if distance <= circle_radius:
            return True

    # Check if any side of the bounding box is crossing the safety circle
    x_min, y_min, x_max, y_max = box
    if (x_min <= circle_center_x <= x_max) and (abs(circle_center_y - y_min) <= circle_radius or abs(circle_center_y - y_max) <= circle_radius):
        return True
    if (y_min <= circle_center_y <= y_max) and (abs(circle_center_x - x_min) <= circle_radius or abs(circle_center_x - x_max) <= circle_radius):
        return True

    # Check if the center of the bounding box is inside the safety circle
    center_x = (box[0] + box[2]) // 2
    center_y = (box[1] + box[3]) // 2
    distance_to_center = math.sqrt((center_x - circle_center_x)**2 + (center_y - circle_center_y)**2)
    if distance_to_center <= circle_radius:
        return True

    return False

# Initialize Mediapipe Hands
mp_hands = mp.solutions.hands.Hands(static_image_mode=False, max_num_hands=2, min_detection_confidence=0.8)

# Open video capture (change 0 to the appropriate camera index if needed)
cap = cv2.VideoCapture(1)

# Initialize safety circle parameters
safety_circle_center = (320, 170)  # (x, y) coordinates of the circle center
safety_circle_radius = 50  # Radius of the safety circle

cv2.namedWindow("Real-time Hand Detection",cv2.WINDOW_NORMAL)
hand_touching_circle = False  # Initialize hand_touching_circle as False
beep_playing = False  # Initialize beep_playing as False

while True:
    ret, frame = cap.read()
    if not ret:
        break
    
    overlay = np.zeros_like(frame)
    overlay[:] = (0, 0, 255)  # (B, G, R, alpha)
    
    # Detect hands
    hands = detect_hands(frame, mp_hands)

    # Draw rectangles around detected hands
    draw_hand_boxes(frame, hands)

    # Draw the predefined safety circle
    draw_safety_circle(frame, safety_circle_center, safety_circle_radius)

    hand_touching_circle_new = False  # Initialize the flag for this frame
    for box in hands:
        if is_hand_touching_circle(box, safety_circle_center, safety_circle_radius):
            print("Hand detected")
            hand_touching_circle_new = True
            # Combine the original frame and the transparent red overlay using bitwise operations
            frame = cv2.addWeighted(frame, 0.4, overlay, 0.6, 0)
            break

    # Start or stop beep based on hand status
    if hand_touching_circle_new and not hand_touching_circle and not beep_playing:
        winsound.Beep(1500, 50)  # Start beep
        beep_playing = True
    elif not hand_touching_circle_new and hand_touching_circle and beep_playing:
        # Stop beep
        winsound.Beep(1500, 50)  # Stop beep
        beep_playing = False

    hand_touching_circle = hand_touching_circle_new

    # Display text if the hand is inside the circle
    if hand_touching_circle:
        text = "Safety breach!"
        font_scale = 1
        font_thickness = 2
        text_size = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, font_scale, font_thickness)[0]
        # Calculate dynamic text position based on window size and text size
        text_position = ((frame.shape[1] - text_size[0]) // 2, frame.shape[0] // 2)
        cv2.putText(frame, text, text_position, cv2.FONT_HERSHEY_SIMPLEX, font_scale, (0, 255, 255), font_thickness)

    # Show the frame with hand detection and safety circle
    cv2.imshow("Real-time Hand Detection", frame)
    cv2.setWindowProperty("Real-time Hand Detection", cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)

    # Press 'q' to exit the loop and terminate the program
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release video capture and close all windows
cap.release()
cv2.destroyAllWindows()
