#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AM2320.h>
#include <SoftwareSerial.h>
#include "dht.h"
#define dht_apin A0
#define OLED_RESET 4

//User defined variables
const String roomName = "Server room 1";
int smsTimeOut = 0;
#define numberCount 1
String recievers[numberCount] = { "+359883286555" };
int rainDropSensorPin = A1;

dht DHT;
SoftwareSerial mySerial(3, 2); //SIM800L Tx & Rx is connected to Arduino #3 & #2

const int BUTTON = 7; // Button sensor Pin
const int buzzer = 5; // Buzzer Pin
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_AM2320 am2320 = Adafruit_AM2320();


void setup()
{
  Serial.begin(9600);
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  am2320.begin();

  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  mySerial.begin(9600);

  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+359883286555\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  mySerial.print("Arduino works !!! "); //text content
  updateSerial();
  mySerial.write(26);
  Serial.println("DHT11 Humidity & temperature Sensor\n\n");
  pinMode(buzzer, OUTPUT);

}

void displayTempHumid()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print(roomName);
  display.setCursor(0, 10);

  display.print("Humidity: " + String(DHT.humidity, 2) + " %");
  display.setCursor(0, 20);

  display.print("Temperature: " + String(DHT.temperature, 2) + " C");
}

void sendSMStoNumbers(String msg)
{
  for (int i = 0; i < numberCount; i++)
  {
    mySerial.println("AT"); updateSerial(); //Once the handshake test is successful, it will back to OK
    mySerial.println("AT+CMGS=\"" + recievers[i] + "\""); updateSerial();
    mySerial.print(msg); updateSerial(); //text content
    mySerial.write(26);
    Serial.println("AT+CMGS=\"" + recievers[i] + "\"");
    delay(3000);
  }
}

void loop()
{
  displayTempHumid();
  display.display();  
  DHT.read11(dht_apin);

  Serial.print("Water sensor value: ");
  Serial.println(analogRead(rainDropSensorPin));

  if (DHT.temperature > 28 && smsTimeOut <= 0)
  {
    sendSMStoNumbers("Temperature is over 28C !!!");
    smsTimeOut = 10000;
  }
  else if (digitalRead(BUTTON) == HIGH) 
  {
    tone(buzzer, 1000); // Send 1KHz sound signal...
    delay(1000);
    noTone(buzzer);     // Stop sound...
    sendSMStoNumbers("Current humidity = " + String(DHT.humidity, 2) + "%" + "Current temperature = " + String(DHT.temperature, 2) + "C");
  }
  else if (analogRead(rainDropSensorPin) < 700 && smsTimeOut <= 0)
  {
    sendSMStoNumbers("Water sensor activated !!!");
    smsTimeOut = 10000;
  }
  else if (smsTimeOut > 0)
  {smsTimeOut --;}
}

void updateSerial()
{
  delay(500);
  while (Serial.available())
  {mySerial.write(Serial.read());} //Forward what Serial received to Software Serial Port

  while (mySerial.available())
  {Serial.write(mySerial.read());} //Forward what Software Serial received to Serial Port
}
