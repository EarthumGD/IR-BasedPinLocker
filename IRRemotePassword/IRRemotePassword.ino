#include <IRremote.hpp>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "DefinePins.h"
#include "HexCodeDecoder.h"
#include "UltrasonicSensor.h"
#include "Lock.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;

unsigned long lastCommand = 0;
unsigned long hexCode;
bool isHolding = false;

// Initial PIN: 000000
unsigned long password[] = {3860463360, 3860463360, 3860463360, 3860463360, 3860463360, 3860463360};
unsigned long newPassword[6];
unsigned long input[6];
int passwordLength = 6;
int inputIndex = 0;
bool accessGranted = false;

bool isSensed = false;
bool isReset = false;
bool isAccessGrantedPrinted = true;
int tries = 0;

void setup() {
  // IR Receiver
  IrReceiver.begin(IRReceiverPin, ENABLE_LED_FEEDBACK);

  // Ultrasonic Sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Buzzer
  pinMode(buzzerPin, OUTPUT);

  // LED On/Off
  pinMode(onPin, OUTPUT);
  pinMode(offPin, OUTPUT);
  pinMode(denyPin, OUTPUT);

  // LCD1602
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("6-digit PIN:");

  // Servo Motor
  servo.attach(servoPin);
  servo.write(90);

  Serial.begin(9600);
}

void loop() {
  // Checks if it's unlocked; otherwise, it's locked
  if(accessGranted) {
    if(isAccessGrantedPrinted) {
      lcd.clear();
      lcd.print("Access granted!");
      lcd.setCursor(0, 1);
      lcd.print("Welcome user!");
      isAccessGrantedPrinted = false;
    }

    // Gets distance from ultrasonic sensor
    float distance = getDistance();
    Serial.println(distance);

    // Mainly used for resetting PIN number
    if(IrReceiver.decode()) {
      hexCode = IrReceiver.decodedIRData.decodedRawData;

      // Checks for long press
      if (hexCode != 0) {
        int digit = decodeHexCode(hexCode);

        // If input is * --> Reset mode
        if(digit == -1 && !isReset) {
          isReset = true;
          inputIndex = 0;
          lcd.clear();
          lcd.print("New 6-digit PIN:");
        }

        // Checks if the system is in reset mode
        if(isReset) {
          // If input is # --> Cancel reset
          if(digit == -2) {
            lcd.clear();
            lcd.print("Canceled...");
            inputIndex = 0;
            isReset = false;
            isAccessGrantedPrinted = true;
            delay(2000);
          }

          // If input is < (LEFT) --> Backspace
          if(digit == -5 && inputIndex > 0) {
            inputIndex--;
            lcd.setCursor(inputIndex, 1);
            lcd.print(" ");
            newPassword[inputIndex] = 0;
          }

          // Valid input
          if(digit >= 0 && digit <= 9) {
            if(inputIndex < passwordLength) {
              lcd.setCursor(inputIndex, 1);
              lcd.print("*");
              newPassword[inputIndex++] = hexCode;
            }
          }

          // If input is OK --> Confirm
          if(digit == -7 && inputIndex == passwordLength) {
            lcd.clear();
            lcd.print("New Password!");
            changePassword();
            inputIndex = 0;
            isReset = false;
            isAccessGrantedPrinted = true;
            delay(2000);
          }
        }
      }

      IrReceiver.resume();
    }

    // Stops if the system is in reset mode
    if(!isReset && (distance > 0 && distance <= 2.00)) {
      digitalWrite(buzzerPin, HIGH);
      lcd.clear();
      lcd.print("Locked");
      delay(2000);
      lock();
      digitalWrite(buzzerPin, LOW);
      accessGranted = false;
      lcd.clear();
      lcd.print("6-digit PIN:");
    }

    delay(50);
  } else {
    digitalWrite(onPin, LOW);
    digitalWrite(offPin, HIGH);

    // Mainly for pin input
    if (IrReceiver.decode()) {
      hexCode = IrReceiver.decodedIRData.decodedRawData;

      // Checks for long press
      if (hexCode != 0) {
        int digit = decodeHexCode(hexCode);

        // If input is < (LEFT) --> Backspace
        if(digit == -5 && inputIndex > 0) {
          inputIndex--;
          lcd.setCursor(inputIndex, 1);
          lcd.print(" ");
          input[inputIndex] = 0;
        }

        // Valid input
        if(digit >= 0 && digit <= 9) {
          if(inputIndex < passwordLength) {
            lcd.setCursor(inputIndex, 1);
            lcd.print("*");
            input[inputIndex++] = hexCode;
          }
        }

        // If input is OK --> Confirm
        if(digit == -7 && inputIndex == passwordLength) {
          // Checks if password is correct
          if(checkPassword()) {
            lcd.clear();
            lcd.print("Access granted!");
            digitalWrite(onPin, HIGH);
            digitalWrite(offPin, LOW);
            accessGranted = true;
            tries = 0;
            delay(2000);
            unlock();
          } else {
            tries++;
            lcd.clear();
            lcd.print("Access denied...");
            digitalWrite(denyPin, HIGH);
            digitalWrite(buzzerPin, HIGH);
            delay(2000);
            lcd.clear();
            lcd.print("6-digit PIN:");
            digitalWrite(denyPin, LOW);
            digitalWrite(buzzerPin, LOW);
          }

          inputIndex = 0;
        }
      }

      IrReceiver.resume();
    }

    // Checks if user tries three times
    // Waits for 60 seconds
    // Buzzer is on for the duration
    if(tries == 3) {
      tries = 0;
      digitalWrite(buzzerPin, HIGH);
      for(int i = 60; i > 0; i--) { 
        lcd.clear();
        lcd.print("Retry after:");
        if(i < 10) {
          lcd.setCursor(0, 1);
          lcd.print("0");
          lcd.setCursor(1, 1);
          lcd.print(i);
        } else {
          lcd.setCursor(0, 1);
          lcd.print(i);
        }
        delay(1000);
      }
      lcd.clear();
      lcd.print("6-digit PIN:");
      digitalWrite(buzzerPin, LOW);
    }
  }
}

bool checkPassword() {
  for (int i = 0; i < passwordLength; i++) {
    if (input[i] != password[i]) {
      return false;
    }
  }
  return true;
}

void changePassword() {
  for (int i = 0; i < passwordLength; i++) {
    password[i] = newPassword[i];
  }
}