#include <SoftwareSerial.h>

#define echoPin 9     // Ultrasonic sensor ECHO pin connected to Arduino pin 9
#define trigPin 10    // Ultrasonic sensor TRIG pin connected to Arduino pin 10
#define GPS_TX 4      // GPS module TX connected to Arduino pin 4
#define GPS_RX 3      // GPS module RX connected to Arduino pin 3
#define binDepth_inch 48 // Bin height in inches (4 feet)

long duration;           
int distance_inch;       
int fillLevelPercent;    

SoftwareSerial gpsSerial(GPS_RX, GPS_TX); 
void setup() {
  // Set up pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  
  Serial.begin(9600);       // Serial monitor for output
  gpsSerial.begin(9600);    // GPS module baud rate
  
  // Initialize Wi-Fi connection (ESP8266)
  Serial.println("Initializing Wi-Fi...");
  initializeWiFi();
}

void loop() {
  // Measure garbage fill level
  fillLevelPercent = getFillLevel();
  Serial.print("Garbage Fill Level: ");
  Serial.print(fillLevelPercent);
  Serial.println("%");

  // Get GPS location
  String gpsData = getGPSData();
  Serial.print("GPS Location: ");
  Serial.println(gpsData);

  // Send data to the cloud
  sendDataToCloud(fillLevelPercent, gpsData);

  delay(1000); // Delay for one second before the next reading
}

// Function to measure fill level using ultrasonic sensor
int getFillLevel() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance_inch = duration * 0.0133 / 2;
  int fillLevel = (100 * (binDepth_inch - distance_inch)) / binDepth_inch;

  // Ensure fill level is between 0-100%
  fillLevel = constrain(fillLevel, 0, 100);
  return fillLevel;
}

// Function to read GPS data
String getGPSData() {
  String gpsData = "";
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    gpsData += c;
    if (c == '\n') break;  // Read until end of line
  }
  return gpsData.length() > 0 ? gpsData : "GPS data not available";
}

// Function to initialize Wi-Fi
void initializeWiFi() {
  Serial.begin(115200);       // Communication with ESP8266 Wi-Fi module
  Serial.println("AT");       // Basic AT command to test communication
  delay(1000);
  if (Serial.find("OK")) {
    Serial.println("Wi-Fi module ready");
  } else {
    Serial.println("Wi-Fi module not responding");
  }

  // Connect to Wi-Fi
  Serial.println("AT+CWMODE=1");           // Set Wi-Fi mode to station
  delay(1000);
  Serial.println("AT+CWJAP=\"yourSSID\",\"yourPASSWORD\""); // Replace with your network credentials
  delay(5000); // Time to connect to Wi-Fi
}

// Function to send data to the cloud
void sendDataToCloud(int fillLevel, String gpsData) {
  String server = "your.cloud.server.com"; // Replace with your cloud server address
  String data = "Garbage Fill Level: " + String(fillLevel) + "%, GPS: " + gpsData;

  // Start a TCP connection and send data
  Serial.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80"); // Start TCP connection
  delay(2000);
  Serial.println("AT+CIPSEND=" + String(data.length() + 4)); // Specify data length
  delay(2000);
  Serial.println(data); // Send actual data
  delay(2000);
  Serial.println("AT+CIPCLOSE"); // Close TCP connection
}

