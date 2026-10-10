#include <Arduino.h>
#include "CANTProtocol.h"

CANTProtocol CAN(10, 2, 0X10);

void builder(unsigned char dataLength, byte* incomingData, unsigned long callbackID){
  int value = analogRead(A6);
  CAN.sendRequestResponse(value, callbackID);
}


void setup() {
  // put your setup code here, to run once:
  pinMode(A6, INPUT);
  Serial.begin(9600);

  CAN.registerRequest(0, builder);

  while(!CAN.begin()){
    delay(100);
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  CAN.execute();
}