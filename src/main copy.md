#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("hello world");  // Initial message
}

void loop() {
  int potValue = analogRead(15);  // Read potentiometer from GPIO15
  Serial.print("Potentiometer value: ");
  Serial.println(potValue);
  delay(200);  // Adjust delay as needed
}
