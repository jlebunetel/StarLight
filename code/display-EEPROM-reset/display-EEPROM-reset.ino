#include <EEPROM.h>


void setup() {
  // put your setup code here, to run once:
  delay(2000);
  for (int address = 0; address < 14; address ++) {
    EEPROM.write(address, 0);
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
