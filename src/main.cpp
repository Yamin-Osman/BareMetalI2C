#include <Arduino.h>
#include <avr/io.h>




void setup() {
  // put your setup code here, to run once:
  DDRC |= (1 << PC7); // Set PC7 as output
}

void loop() {
  // put your main code here, to run repeatedly:
  PINC = (1 << PC7); // Toggle PC7
  delay(500); // Wait for 500 milliseconds
  PINC = (1 << PC7); // Toggle PC7 again
  delay(500); // Wait for 500 milliseconds  
}
