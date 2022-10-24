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
#define lockedLED 9
#define unlockedLED 8
#define buzzer 10
#define lockButton 12
#define tiltInput A5
#define fWheelLock 5
#define rWheelLock 6

// Define Objects
SoftwareSerial mySerial(2, 3);
Servo fWheelServo;
Servo rWheelServo;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Global Variables
bool locked = true;
int brightness = 0;
uint8_t fadeAmount = 5;
bool tilted = true;
int pos = 0;
uint8_t SERIAL_READ = 0;
unsigned long lastTileDuration;
int id;

// ------- Serial Commands --------
// RECEIVE DATA
uint8_t ARM_SYS = 5;
uint8_t UN_ARM_SYS = 10;
uint8_t ENROLL = 6;
uint8_t DELETE_ALL = 7;
uint8_t DELETE_SINGLE = 8;

// SEND DATA

// ------- Serial Commands --------

void setup()
{
  finger.begin(57600);
  Serial.begin(9600);
  pinMode(lockedLED, OUTPUT);
  pinMode(unlockedLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(lockButton, INPUT);
  fWheelServo.attach(fWheelLock);
  rWheelServo.attach(rWheelLock);
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
 * Enroll New Fingerprints
 *
 * @return int 1 if passed, otherwise returns -1
 */
uint8_t newFingerprintEnroll()
{

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

/*
 * Delete Specific Fingerprint
 *
 * @param int id - ID of fingerprint to delete
 * @return int 1 if passed, otherwise returns -1
 */
uint8_t deleteFingerprint(uint8_t id)
{
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK)
  {
    Serial.println("Deleted!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not delete in that location");
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
  }
  else
  {
    Serial.print("Unknown error: 0x");
    Serial.println(p, HEX);
  }

  return p;
}

/*
 * Delete Whole Fingerprint Database
 */
void deleteAllFingerprint()
{
  finger.emptyDatabase();
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
    for (pos = 180; pos >= 0; pos -= 1)
    { // goes from 180 degrees to 0 degrees
      fWheelServo.write(pos);
      rWheelServo.write(pos);
      delay(15);
    }
  }
  else
  {
    for (pos = 0; pos <= 180; pos += 1)
    { // goes from 0 degrees to 180 degrees
      fWheelServo.write(pos);
      rWheelServo.write(pos);
      delay(15);
    }
  }
}

/*
 * Read Commands from Serial using Bluetooth
 */
void serialRead()
{
  if (Serial.available() > 0)
  {
    SERIAL_READ = Serial.read();
  }
}

void loop()
{
  digitalWrite(buzzer, HIGH);
  int val = detectFingerprintID();
  if (val != -1)
  {
    locked = false;
    digitalWrite(lockedLED, LOW);
    while (!locked)
    {
      digitalWrite(unlockedLED, HIGH);
      Serial.println("Unlocked");
      delay(500);
      // if (digitalRead(tiltInput) == HIGH) {
      //   tilted = false;
      // } else {
      //   tilted = true;
      // }
      // Serial.println(digitalRead(tiltInput));
      // if (analogRead(tiltInput) <= 500) {
      //   lastTileDuration = millis();
      // }
      // Serial.println(millis() - lastTileDuration);
      // if (millis() - lastTileDuration >= 10000) {
      while (analogRead(tiltInput) >= 1010)
      {
        Serial.println("not tilted");
        delay(1000);
        digitalWrite(buzzer, LOW);
        delay(5000);
        digitalWrite(buzzer, HIGH);
        locked = true;
        break;
      }
      //   break;
      // }
      if (digitalRead(lockButton) == HIGH)
      {
        locked = true;
        break;
      }
    }
  }
  else
  {
    while (analogRead(tiltInput) <= 1000)
    {
      digitalWrite(buzzer, LOW);
      delay(5000);
    }
    Serial.println("Locked!");
    digitalWrite(lockedLED, HIGH);
    digitalWrite(unlockedLED, LOW);
    locked = true;
  }
  delay(50);
}