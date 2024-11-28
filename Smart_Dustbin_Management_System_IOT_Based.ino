#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <NewPing.h>
#include <Servo.h>

Servo myservo;  

String dustbin = "SmtDstBin1";


// Wi-Fi credentials
#define WIFI_SSID "44444444"
#define WIFI_PASSWORD "44444444"


#define API_KEY "AIzaSyAujt_zf6fCCLEPICef4_VAd7W4rSQshJE"
#define DATABASE_URL "byte4genodemcu-default-rtdb.firebaseio.com"



#define trigPin1 D0  
#define echoPin1 D1 

#define trigPin2 D5 
#define echoPin2 D6 

#define CO2_SENSOR_PIN A0  
int servoPin = D3;

long duration1, distance1;
long duration2, distance2;

LiquidCrystal_I2C lcd(0x27, 16, 2); 

int threshold = 500;       
int sensorValue = 0;      

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

String Lidstate = "close";        
String prevLidState = "close";   

void setup() {
  myservo.attach(servoPin);  
  Serial.begin(115200);  

  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign-up successful");
    signupOK = true;
  }
  else {
    Serial.printf("Sign-up failed: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  sensorValue = analogRead(CO2_SENSOR_PIN);
  // Convert sensor value to CO2 concentration (ppm)
  float co2Value = sensorValue * (5.0 / 1023.0) * 1000;

//  Serial.print("CO2 Value: ");
//  Serial.print(co2Value);
//  Serial.println(" ppm");

  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.setFloat(&fbdo, "/IistBiharData/" + dustbin + "/co2", co2Value)) {
//      Serial.println("Failed to update CO2 Level to Firebase");
    } else {
//      Serial.println("CO2 Level updated to Firebase successfully");
    }
  }

  
  distance1 = getDistance(trigPin1, echoPin1);
//  Serial.print("Distance 1: ");
//  Serial.print(distance1);
//  Serial.println(" cm");

  
  distance2 = getDistance(trigPin2, echoPin2);
  Serial.print("Distance 2: ");
  Serial.print(distance2);
  Serial.println(" cm");

 int fullnessPercentage;
fullnessPercentage = (1 - (distance2 / 50.0)) * 100;

  if (fullnessPercentage < 0) {
    fullnessPercentage = 0; 
  } else if (fullnessPercentage > 100) {
    fullnessPercentage = 100; 
  }


Serial.print("Dustbin Fullness: ");
Serial.print(fullnessPercentage);
Serial.println(" %");
  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.setInt(&fbdo, "/IistBiharData/" + dustbin + "/level", fullnessPercentage)) {
      Serial.println("Failed to update  Level to Firebase");
    } else {
      Serial.println("Level updated to Firebase successfully");
    }
  }

 
  if (distance1 <= 40) {
    
    myservo.write(180);
    Lidstate = "open";
    delay(1000);  
  } else {
   
    myservo.write(0);
    Lidstate = "close";
  }


  if (Lidstate != prevLidState) {
 
    if (Firebase.ready() && signupOK) {
      if (!Firebase.RTDB.setString(&fbdo, "/IistBiharData/" + dustbin + "/status", Lidstate)) {
//        Serial.println("Failed to update Lid state to Firebase");
      } else {
//        Serial.println("Lid state updated to Firebase successfully");
      }
    }
    prevLidState = Lidstate;  
  }

  delay(1000);  
}


long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  
  long duration = pulseIn(echoPin, HIGH);

  
  long distance = duration * 0.034 / 2;
  return distance;
}
