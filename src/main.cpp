/*
 * SMART Bike LOCK
 * Author: Pasindu Akalpa
 * Date: 2022-10-22 18:00:00
 * Last Modified by: Pasindu Akalpa
 * Github: https://github.com/pAkalpa
 * Email: pasinduakalpa1998@gmail.com
 * Repository: https://github.com/pAkalpa/SMART_Bike_Lock
 */

// Importing Libraries
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Servo.h>

// Define IO Pins
#define lockedLEDPin 9
#define unlockedLEDPin 8
#define buzzerPin 10
#define lockButtonPin 12
#define tiltInputPin 7
#define fWheelLockPin 5
#define rWheelLockPin 6

// Define Objects
SoftwareSerial mySerial(2, 3);
Servo fWheelServo;
Servo rWheelServo;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Global Variables
bool locked = true;
int pos = 0;
uint8_t SERIAL_READ = 0;
uint8_t tiltCount = 0;
unsigned long lastTiltDuration;
unsigned long timeNow;
int id;
int detectId;
bool isMsgSent = false;

// ------- Serial Commands --------
const uint8_t ARM_SYS = 200;
const uint8_t DISARM_SYS = 201;

// ------- Serial Commands --------

void setup()
{
  finger.begin(57600);
  Serial.begin(9600);
  pinMode(lockedLEDPin, OUTPUT);
  pinMode(unlockedLEDPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(lockButtonPin, INPUT);
  fWheelServo.attach(fWheelLockPin);
  rWheelServo.attach(rWheelLockPin);
}

/*
 * Detect Fingerprints
 *
 * @return int -1 if failed, otherwise returns ID #
 */
int detectFingerprintID()
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
    return -1;

  return finger.fingerID;
}

/*
 * Control Lock State using Servos
 *
 * @param int state - 1 to unlock, 0 to lock
 */
void lockControl(bool state)
{
  if (state)
  {
    fWheelServo.write(0);
    rWheelServo.write(0);
  }
  else
  {
    fWheelServo.write(180);
    rWheelServo.write(180);
  }
}

/*
 * Read Commands from Serial using Bluetooth
 */
void serialRead()
{
  if (Serial.available() > 0)
  {
    SERIAL_READ = Serial.parseInt();
    Serial.println(SERIAL_READ);
    switch (SERIAL_READ)
    {
    case ARM_SYS:
      Serial.println("ARMING SYSTEM");
      detectId = -1;
      locked = true;
      break;
    case DISARM_SYS:
      Serial.println("DISARMING SYSTEM");
      detectId = 0;
      locked = false;
      break;
    default:
      break;
    }
  }
}

void buzzerTone1(unsigned int duration)
{
  timeNow = millis();
  while (millis() - timeNow < duration)
  {
    tone(buzzerPin, 1000, 100); // freq 1000 Hz,delay 100 ms
    delay(1000);
    tone(buzzerPin, 1000, 1000); // freq 1000 Hz,delay 1 sec
    delay(100);
  }
}

void buzzerTone2(unsigned int duration)
{
  timeNow = millis();
  while (millis() - timeNow < duration)
  {
    digitalWrite(buzzerPin, LOW);
    delay(60);
    digitalWrite(buzzerPin, HIGH);
    delay(500);
  }
}

void loop()
{
  digitalWrite(buzzerPin, HIGH);
  detectId = detectFingerprintID();
  lockControl(true);

  serialRead();
  lastTiltDuration = millis();
  if (detectId != -1)
  {
    locked = false;
    digitalWrite(lockedLEDPin, LOW);
    while (!locked)
    {
      serialRead();
      lockControl(false);
      digitalWrite(unlockedLEDPin, HIGH);
      uint8_t tempCount = tiltCount;

      if (digitalRead(tiltInputPin) == LOW)
      {
        tiltCount++;
        lastTiltDuration = millis();
      }

      if (tiltCount == tempCount)
      {
        if (millis() - lastTiltDuration >= 15000)
        {
          if (!isMsgSent)
          {
            Serial.println(500);
            isMsgSent = true;
          }
        }

        if (millis() - lastTiltDuration >= 20000)
        {
          buzzerTone2(2000);
          digitalWrite(buzzerPin, HIGH);
          locked = true;
          break;
        }
      }
      else
      {
        tiltCount = 0;
      }

      if (digitalRead(lockButtonPin) == HIGH)
      {
        locked = true;
        break;
      }
    }
  }
  else
  {
    if (digitalRead(tiltInputPin) == LOW)
    {
      buzzerTone1(5000);
      digitalWrite(buzzerPin, HIGH);
    }
    digitalWrite(lockedLEDPin, HIGH);
    digitalWrite(unlockedLEDPin, LOW);
    locked = true;
  }
}
