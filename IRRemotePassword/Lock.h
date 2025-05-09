extern Servo servo;

void lock() {
  servo.write(90);
  delay(100);
}

void unlock() {
  servo.write(0);
  delay(100);
}