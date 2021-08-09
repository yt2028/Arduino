#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
 
HX711 scale;

void setup() {
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(510.0);                      // 調整する
  scale.set_offset(0.5);
  scale.tare();                // reset the scale to 0
  }

  void loop() {
    Serial.println(scale.get_units(10)+0.50, 1);
    delay(1);
  }
