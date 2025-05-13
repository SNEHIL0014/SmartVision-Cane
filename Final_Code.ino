#include <Wire.h>
#include <SoftwareSerial.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>

// Function prototypes
bool waitForModule(unsigned long timeout = 10000);
bool sendATCommand(String cmd, String expectedResponse, unsigned long timeout);
bool sendAlert(String number, String message);
bool waitForResponse(const char* expectedResponse, unsigned long timeout);
void printSensorStatus();
void checkFall();
void checkSOS();
void triggerAlert(String reason);
String getGPSLocation();
void handleWebClient();
void sendMapPage(WiFiClient& client);

// GSM Configuration
MPU6050 mpu;
SoftwareSerial gsmSerial(D5, D6);  // TX, RX
const int buttonPin = D3;
bool fallDetected = false;
unsigned long lastFallTime = 0;
const unsigned long debounceInterval = 10000;

// GPS Configuration
#define gpsRxPin D7
#define gpsTxPin D8
SoftwareSerial neo6m(gpsRxPin, gpsTxPin);
TinyGPSPlus gps;

// WiFi Configuration
const char* ssid = "sleep1";
const char* password = "sorry1234@";
String GMAP_API_KEY = "AIzaSyAoIb3Fr3bCXHbhwSOWfyIwlImLMOozTpY";
WiFiServer server(80);

// System Status Variables
unsigned long lastStatusUpdate = 0;
const long statusInterval = 1000;
String lastSMSStatus = "No messages sent";
int smsRetryCount = 0;

void setup() {
  // Initialize serial communications
  Serial.begin(115200);
  gsmSerial.begin(9600);
  neo6m.begin(9600);

  // Initialize MPU6050
  Wire.begin(D2, D1);  // SDA, SCL
  mpu.initialize();
  
  // Verify MPU connection
  if(mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
  }

  // Initialize SOS button
  pinMode(buttonPin, INPUT_PULLUP);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
  server.begin();

  // Initialize GSM module
  Serial.println("Initializing GSM...");
  if(!initializeGSM()) {
    Serial.println("GSM initialization failed!");
  }

  // Initial calibration delay
  Serial.println("System Calibration - Keep device stable for 5 seconds");
  delay(5000);
}

bool initializeGSM() {
  if (!waitForModule(5000)) return false;
  
  if (!sendATCommand("AT+CMGF=1", "OK", 2000)) return false;
  if (!sendATCommand("AT+CSCS=\"GSM\"", "OK", 2000)) return false;
  if (!sendATCommand("AT+CNMI=2,2,0,0,0", "OK", 2000)) return false;
  
  Serial.println("GSM initialized successfully");
  return true;
}

void loop() {
  // GPS Data Handling
  while (neo6m.available()) {
    gps.encode(neo6m.read());
  }

  // Safety Features
  checkFall();
  checkSOS();

  // Web Server Handling
  handleWebClient();

  // System Status Monitoring
  if (millis() - lastStatusUpdate >= statusInterval) {
    printSensorStatus();
    lastStatusUpdate = millis();
  }
}

// Rest of the functions remain the same as previous corrected version
// [checkFall(), checkSOS(), triggerAlert(), getGPSLocation(), 
//  handleWebClient(), sendMapPage(), waitForModule(), sendATCommand(), 
//  waitForResponse(), sendAlert(), printSensorStatus()]

void checkFall() {
  static unsigned long lastSample = 0;
  if (millis() - lastSample < 100) return;
  lastSample = millis();

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float accelMagnitude = sqrt(ax*ax + ay*ay + az*az) / 16384.0;
  float gyroX = gx / 131.0;
  float gyroY = gy / 131.0;
  float gyroZ = gz / 131.0;
  float gyroMagnitude = sqrt(gyroX*gyroX + gyroY*gyroY + gyroZ*gyroZ);

  bool accelerationTrigger = (accelMagnitude < 0.5 || accelMagnitude > 2.5);
  bool gyroTrigger = (gyroMagnitude > 300) || 
                    (abs(gyroX) > 200 || abs(gyroY) > 200 || abs(gyroZ) > 200);

  if (accelerationTrigger && gyroTrigger) {
    if (!fallDetected && millis() - lastFallTime > debounceInterval) {
      Serial.println("Fall Detected!");
      triggerAlert("Fall Detected!");
      fallDetected = true;
      lastFallTime = millis();
      
      Serial.print("Accel Magnitude: "); Serial.println(accelMagnitude);
      Serial.print("Gyro Values (X/Y/Z): ");
      Serial.print(gyroX); Serial.print("°/s, ");
      Serial.print(gyroY); Serial.print("°/s, ");
      Serial.print(gyroZ); Serial.println("°/s");
    }
  } else {
    fallDetected = false;
  }
}

void checkSOS() {
  if (digitalRead(buttonPin) == LOW) {
    Serial.println("SOS Button Pressed!");
    triggerAlert("SOS Button Activated!");
    delay(3000);
  }
}

void triggerAlert(String reason) {
  String message = reason + " Need help immediately.";
  String location = getGPSLocation();
  
  if(!sendAlert("+918290736156", message + location)) {
    Serial.println("Failed to send primary alert!");
  }
  sendAlert("+919102214625", "Emergency! " + reason + " [Police]" + location);
  sendAlert("+916207828991", "Emergency! " + reason + " [Ambulance]" + location);
}

String getGPSLocation() {
  if (!gps.location.isValid()) return " Location: Unknown";
  return " Location: https://maps.google.com/?q=" + 
         String(gps.location.lat(), 6) + "," + 
         String(gps.location.lng(), 6);
}

void handleWebClient() {
  WiFiClient client = server.available();
  if (!client) return;

  String currentLine = "";
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n') {
        if (currentLine.length() == 0) {
          sendMapPage(client);
          break;
        }
        currentLine = "";
      } 
      else if (c != '\r') currentLine += c;
    }
  }
  client.stop();
}

void sendMapPage(WiFiClient& client) {
  String html = F("<!DOCTYPE html><html><head>");
  html += F("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  html += F("<style>#map{height:400px;width:100%;}</style>");
  html += F("</head><body><h1>Emergency Tracker</h1><div id='map'></div>");
  html += F("<script>var map, marker;");
  html += F("function initMap(){");
  html += F("map=new google.maps.Map(document.getElementById('map'),{");
  html += F("zoom:16,mapTypeId: google.maps.MapTypeId.ROADMAP,");
  
  if (gps.location.isValid()) {
    html += "center:{lat:" + String(gps.location.lat(),6) + 
            ",lng:" + String(gps.location.lng(),6) + "}";
  } else {
    html += F("center:{lat:0,lng:0}");
  }
  
  html += F("});} window.setInterval(updateMarker,5000);");
  html += F("function updateMarker(){");
  html += F("var pos=new google.maps.LatLng(");
  
  if (gps.location.isValid()) {
    html += String(gps.location.lat(),6) + "," + String(gps.location.lng(),6);
  } else {
    html += F("0,0");
  }
  
  html += F(");");
  html += F("if(!marker){marker=new google.maps.Marker({position:pos,map:map});}");
  html += F("else marker.setPosition(pos);}");
  html += F("</script>");
  html += F("<script async defer src='https://maps.googleapis.com/maps/api/js?key=");
  html += GMAP_API_KEY;
  html += F("&callback=initMap'></script></body></html>");

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.print(html);
}

bool waitForModule(unsigned long timeout) {
  unsigned long start = millis();
  Serial.print("Waiting for GSM module");
  
  while (millis() - start < timeout) {
    gsmSerial.println("AT");
    if (waitForResponse("OK", 1000)) {
      Serial.println("\nGSM Ready");
      return true;
    }
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("\nGSM module timeout!");
  return false;
}

bool sendATCommand(String cmd, String expectedResponse, unsigned long timeout) {
  gsmSerial.println(cmd);
  return waitForResponse(expectedResponse.c_str(), timeout);
}

bool waitForResponse(const char* expectedResponse, unsigned long timeout) {
  unsigned long start = millis();
  String response;
  
  while (millis() - start < timeout) {
    while (gsmSerial.available()) {
      char c = gsmSerial.read();
      response += c;
      if (response.indexOf(expectedResponse) != -1) {
        return true;
      }
    }
  }
  return false;
}

bool sendAlert(String number, String message) {
  const int maxRetries = 3;
  bool success = false;
  
  for (smsRetryCount = 0; smsRetryCount < maxRetries; smsRetryCount++) {
    gsmSerial.println("AT+CMGS=\"" + number + "\"");
    if (!waitForResponse(">", 2000)) {
      lastSMSStatus = "Failed to start SMS";
      continue;
    }

    gsmSerial.print(message);
    gsmSerial.write(26);
    
    if (waitForResponse("+CMGS:", 10000)) {
      success = true;
      lastSMSStatus = "Message sent to " + number;
      smsRetryCount = 0;
      break;
    }
    
    lastSMSStatus = "SMS failed (retry " + String(smsRetryCount+1) + "/" + String(maxRetries) + ")";
    delay(2000);
  }
  
  if (!success) {
    lastSMSStatus = "SMS failed after " + String(maxRetries) + " attempts";
  }
  
  return success;
}

void printSensorStatus() {
  Serial.println("\n===== System Status =====");
  Serial.print("WiFi: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  
  Serial.print("GSM: ");
  gsmSerial.println("AT");
  if(waitForResponse("OK", 1000)) {
    Serial.println("Ready");
  } else {
    Serial.println("Not Responding");
  }

  Serial.println("Last SMS Status: " + lastSMSStatus);
  Serial.print("SMS Retries: ");
  Serial.println(smsRetryCount);

  Serial.print("GPS Fix: ");
  Serial.println(gps.location.isValid() ? "Valid" : "No Fix");
  if(gps.location.isValid()) {
    Serial.print("Lat: "); Serial.println(gps.location.lat(), 6);
    Serial.print("Lng: "); Serial.println(gps.location.lng(), 6);
  }

  Serial.print("MPU6050: ");
  Serial.println(mpu.testConnection() ? "Connected" : "Disconnected");
  
  Serial.print("Last Fall: ");
  if(lastFallTime > 0) {
    Serial.print((millis() - lastFallTime)/1000);
    Serial.println(" seconds ago");
  } else {
    Serial.println("Never");
  }
  
  Serial.println("========================");
}