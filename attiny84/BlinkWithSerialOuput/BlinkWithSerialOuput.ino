#include <SoftwareSerial.h>
SoftwareSerial mySerial(9, 10);

int led = 0;

void setup() {                
  mySerial.begin(9600);
  pinMode(led, OUTPUT);     
}

void loop() {
  mySerial.println(1);
  digitalWrite(led, HIGH);
  delay(1000);      
  
  mySerial.println(0);
  digitalWrite(led, LOW);
  delay(1000);
}
