#include <Seeed_Arduino_SSCMA.h>
#include <math.h>  // For sqrt function

#ifdef ESP32
#include <HardwareSerial.h>
HardwareSerial atSerial(0);  // Define Serial for ESP32C3
#endif

SSCMA AI;

// Define pins for hand detection and safety breach
const int detectionPin = D0;
const int breachPin = D10;

// Define the safety circle (center coordinates and radius)
const int circleX = 250;
const int circleY = 250;
const int radius = 50;

// Function to check if the hand's bounding box touches or crosses the safety circle
bool isHandTouchingCircle(int x_min, int y_min, int x_max, int y_max, int circle_center_x, int circle_center_y, int circle_radius) {
    // Check if any corner of the bounding box is inside or on the circumference of the safety circle
    int corners[4][2] = {{x_min, y_min}, {x_max, y_min}, {x_min, y_max}, {x_max, y_max}};
    for (int i = 0; i < 4; i++) {
        int x = corners[i][0];
        int y = corners[i][1];
        float distance = sqrt(pow(x - circle_center_x, 2) + pow(y - circle_center_y, 2));
        if (distance <= circle_radius) {
            return true;  // Corner is inside the circle
        }
    }

    // Check if any side of the bounding box is crossing the safety circle
    if ((x_min <= circle_center_x && circle_center_x <= x_max) &&
        (abs(circle_center_y - y_min) <= circle_radius || abs(circle_center_y - y_max) <= circle_radius)) {
        return true;  // Side crosses the circle horizontally
    }
    if ((y_min <= circle_center_y && circle_center_y <= y_max) &&
        (abs(circle_center_x - x_min) <= circle_radius || abs(circle_center_x - x_max) <= circle_radius)) {
        return true;  // Side crosses the circle vertically
    }

    // Check if the center of the bounding box is inside the safety circle
    int center_x = (x_min + x_max) / 2;
    int center_y = (y_min + y_max) / 2;
    float distance_to_center = sqrt(pow(center_x - circle_center_x, 2) + pow(center_y - circle_center_y, 2));
    if (distance_to_center <= circle_radius) {
        return true;  // Bounding box center is inside the circle
    }

    return false;  // No part of the bounding box is inside or touching the circle
}

void setup() {
    Serial.begin(115200);
    atSerial.begin(921600);  // For communication with AI module
    AI.begin(&atSerial, D3);  // Initialize SSCMA on ESP32C3

    // Set detection and breach pins as output
    pinMode(detectionPin, OUTPUT);
    pinMode(breachPin, OUTPUT);

    // Turn off both LEDs initially
    digitalWrite(detectionPin, LOW);
    digitalWrite(breachPin, LOW);
}

void loop() {
    if (!AI.invoke(1, false, false)) {  // Perform AI inference
        bool handDetected = false;
        bool safetyBreached = false;

        // Process detected boxes
        for (int i = 0; i < AI.boxes().size(); i++) {
            int x_min = AI.boxes()[i].x;
            int y_min = AI.boxes()[i].y;
            int w = AI.boxes()[i].w;
            int h = AI.boxes()[i].h;
            int x_max = x_min + w;
            int y_max = y_min + h;

            // Hand detected
            handDetected = true;

            // Check if the hand crosses the safety boundary
            if (isHandTouchingCircle(x_min, y_min, x_max, y_max, circleX, circleY, radius)) {
                safetyBreached = true;
            }

            // Display detection details over Serial
            Serial.println("Object Detected:");
            Serial.print("Box: ");
            Serial.print("x_min=");
            Serial.print(x_min);
            Serial.print(", y_min=");
            Serial.print(y_min);
            Serial.print(", x_max=");
            Serial.print(x_max);
            Serial.print(", y_max=");
            Serial.println(y_max);

            if (safetyBreached) {
                Serial.println("Safety Breach Detected!");
            }
        }

        // Update the D0 and D1 pins based on detection results
        digitalWrite(detectionPin, handDetected ? HIGH : LOW);  // Turn on D0 if hand is detected
        digitalWrite(breachPin, safetyBreached ? HIGH : LOW);  // Turn on D1 if breach occurs
    } else {
        Serial.println("Detection failed");
    }

    delay(100);  // Small delay for smoother processing
}
