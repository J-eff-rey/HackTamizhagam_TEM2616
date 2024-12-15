#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>  // Include the Servo library

// Define GPIO pins for the LEDs and Servo
#define RED_LED_PIN 14    // D5 corresponds to GPIO14
#define GREEN_LED_PIN 12  // D6 corresponds to GPIO12
#define SERVO_PIN 2       // D4 corresponds to GPIO2

// WiFi credentials
const char* ssid = "Mario 2 4";
const char* password = "06010519";

// Create a web server on port 80
ESP8266WebServer server(80);

// Servo object
Servo myServo;

// Variables to control servo movement
bool motorRunning = true;  // Flag to check if the motor is running
int servoAngle = 0;        // Current angle of the servo
int increment = 1;         // Angle increment (1 degree per iteration)

// Function to handle the green LED ON command
void handleGreenOn() {
  digitalWrite(GREEN_LED_PIN, HIGH);  // Turn on green LED
  digitalWrite(RED_LED_PIN, LOW);     // Turn off red LED
  motorRunning = true;                // Resume the motor movement
  server.send(200, "text/plain", "Green LED ON and Motor Running");
}

// Function to handle the red LED ON command and stop the motor
void handleRedOn() {
  digitalWrite(RED_LED_PIN, HIGH);   // Turn on red LED
  digitalWrite(GREEN_LED_PIN, LOW);  // Turn off green LED
  motorRunning = false;              // Stop the motor
  myServo.write(0);                  // Move servo to 0 degrees (stop position)
  server.send(200, "text/plain", "Red LED ON and Motor Stopped");
}

void setup() {
  // Set up the LED pins as outputs
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  // Turn off all LEDs initially
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  // Set up the servo motor
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Start with the servo at 0 degrees

  // Start the serial communication
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Set up the server routes
  server.on("/green_on", handleGreenOn);
  server.on("/red_on", handleRedOn);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Handle incoming client requests
  server.handleClient();

  // Continuous back-and-forth servo movement when motorRunning is true
  if (motorRunning) {
    myServo.write(servoAngle);  // Move servo to the current angle
    delay(20);                  // Small delay for smooth movement

    // Update the servo angle for continuous movement between 0 and 90 degrees
    servoAngle += increment;
    if (servoAngle >= 90 || servoAngle <= 0) {
      increment = -increment;  // Reverse the direction of movement
    }
  }
}