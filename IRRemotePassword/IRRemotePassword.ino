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
  if(accessGranted) {
    lcd.setCursor(0, 1);
    lcd.print("Welcome user!");

    float distance = getDistance();
    Serial.println(distance);

    if(distance <= 2.00) {
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

    delay(75);
  } else {
    digitalWrite(onPin, LOW);
    digitalWrite(offPin, HIGH);

    if (IrReceiver.decode()) {
      hexCode = IrReceiver.decodedIRData.decodedRawData;

      if(decodeHexCode(hexCode) == -1) {
        isReset = true;
        inputIndex = 0;
        lcd.clear();
        lcd.print("New 6-digit PIN:");
      }

      if (hexCode != 0) {
        int digit = decodeHexCode(hexCode);

        if(isReset) {
          if(digit == -2) {
            lcd.clear();
            lcd.print("Canceled...");
            inputIndex = 0;
            isReset = false;
            delay(2000);
            lcd.clear();
            lcd.print("6-digit PIN:");
          }

          if(digit == -5 && inputIndex > 0) {
            inputIndex--;
            lcd.setCursor(inputIndex, 1);
            lcd.print(" ");
            newPassword[inputIndex] = 0;
          }

          if(digit >= 0 && digit <= 9) {
            if(inputIndex < passwordLength) {
              lcd.setCursor(inputIndex, 1);
              lcd.print(digit);
              newPassword[inputIndex++] = hexCode;
            }
          }

          if(digit == -7 && inputIndex == passwordLength) {
            lcd.clear();
            lcd.print("New Password!");
            changePassword();
            inputIndex = 0;
            isReset = false;
            tries = 0;
            delay(2000);
            lcd.clear();
            lcd.print("6-digit PIN:");
          }
        } else {
          if(digit == -5 && inputIndex > 0) {
            inputIndex--;
            lcd.setCursor(inputIndex, 1);
            lcd.print(" ");
            input[inputIndex] = 0;
          }

          if(digit >= 0 && digit <= 9) {
            if(inputIndex < passwordLength) {
              lcd.setCursor(inputIndex, 1);
              lcd.print(digit);
              input[inputIndex++] = hexCode;
            }
          }

          if(digit == -7 && inputIndex == passwordLength) {
            if(checkPassword()) {
              lcd.clear();
              lcd.print("Access granted!");
              digitalWrite(onPin, HIGH);
              digitalWrite(offPin, LOW);
              accessGranted = true;
              tries = 0;
              digitalWrite(buzzerPin, HIGH);
              delay(2000);
              digitalWrite(buzzerPin, LOW);
              unlock();
            } else {
              tries++;
              lcd.clear();
              lcd.print("Access denied...");
              digitalWrite(denyPin, HIGH);
              delay(2000);
              lcd.clear();
              lcd.print("6-digit PIN:");
              digitalWrite(denyPin, LOW);
            }

            inputIndex = 0;
          }
        }
      }

      IrReceiver.resume();
    }

    if(tries == 3) {
      tries = 0;
      lcd.clear();
      lcd.print("Retry after:");
      lcd.setCursor(0, 1);
      lcd.print("1 minute");
      delay(60000);
      lcd.clear();
      lcd.print("6-digit PIN:");
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