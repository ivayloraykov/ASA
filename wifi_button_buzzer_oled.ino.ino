#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AM2320.h>
#include <SoftwareSerial.h>
#include "dht.h"
#define dht_apin A0 //A0 pin DHT11 sensor
#define OLED_RESET 4 // 4 pin refresh oled display

//User defined variables
const String roomName = "Server room 1"; // the name of the room
int messageDelay = 10000; // time out delay in seconds befor next SMS 
#define numberCount 2 // numbers of phone
String recievers[numberCount] = { "+359883286555","+359884994335","+359884994333"}; // declarate phones which will receive sms
int rainDropSensorPin = A1; // Rain drop sensor pin

dht DHT;
SoftwareSerial mySerial(3, 2); //SIM800L Tx & Rx is connected to Arduino #3 & #2

const int BUTTON = 7; // Button sensor Pin
const int buzzer = 5; // Buzzer Pin
int smsTimeOut = 0;
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
  for (int i = 0; i < numberCount; i++)
  {
    mySerial.println("AT"); updateSerial(); //Once the handshake test is successful, it will back to OK
    mySerial.println("AT+CMGF=1");
    mySerial.println("AT+CMGS=\"" + recievers[i] + "\""); updateSerial();
    mySerial.print("Arduino works"); updateSerial(); //text content
    mySerial.write(26);
    Serial.println("AT+CMGS=\"" + recievers[i] + "\"");
    delay(3000);
  }
  
  pinMode(buzzer, OUTPUT);

}

void displayTempHumid()
{
  display.clearDisplay(); // clear display
  display.setTextColor(WHITE); // display color
  display.setTextSize(1); // display text size
  display.setCursor(0, 0); //cursor position

  display.print(roomName); // Show room name on the display
  display.setCursor(0, 10); // Position on the display

  display.print("Humidity: " + String(DHT.humidity, 2) + " %"); // Show humidity on the display
  display.setCursor(0, 20);

  display.print("Temperature: " + String(DHT.temperature, 2) + " C"); // Show temperature on the display
}

void sendSMStoNumbers(String msg) // function for sending SMS
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
  displayTempHumid(); // Show humidity and temperature on the display
  display.display();  
  DHT.read11(dht_apin); // read information from DHT11 sensor

  if (DHT.temperature > 28 && smsTimeOut <= 0) // If Temperature is over 28C you will receive sms
  {
    sendSMStoNumbers("Temperature is over 28C !!!");
    smsTimeOut = messageDelay;
  }
  else if (digitalRead(BUTTON) == HIGH) // If you push the button you will receive sms with Temperature and Humidity right now 
  {
    tone(buzzer, 1000); // Send 1KHz sound signal...
    delay(1000);
    noTone(buzzer);     // Stop sound...
    sendSMStoNumbers("Current humidity = " + String(DHT.humidity, 2) + "%" + "Current temperature = " + String(DHT.temperature, 2) + "C");
  }
  else if (analogRead(rainDropSensorPin) < 700 && smsTimeOut <= 0)// If Rain drop value (1024) is less then 700 you will receive sms
  {
    sendSMStoNumbers("Water sensor activated !!!");
    smsTimeOut = messageDelay;
  }
  else if (smsTimeOut > 0) // smsTimeOut will start
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