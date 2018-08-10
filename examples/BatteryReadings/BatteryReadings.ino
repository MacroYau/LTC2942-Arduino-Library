/*
	LTC2942 Battery Readings Example

  Reads the accumulated charge, remaining capacity, voltage, and temperature of 
  a battery gauged by a LTC2942 device connected to an Arduino board via I2C.
	
	Copyright (c) 2018 Macro Yau

	https://github.com/MacroYau/LTC2942-Arduino-Library
*/

#include <LTC2942.h>

const unsigned int fullCapacity = 240; // Maximum value is 5500 mAh

LTC2942 gauge(50); // Takes R_SENSE value (in milliohms) as constructor argument, can be omitted if using LTC2942-1

void setup() {
  Serial.begin(9600);
  Serial.println("LTC2942 Battery Readings Example");
  Serial.println();

  Wire.begin();

  while (gauge.begin() == false) {
    Serial.println("Failed to detect LTC2942!");
    delay(5000);
  }

  gauge.setBatteryCapacity(fullCapacity);
  gauge.setBatteryToFull(); // Sets accumulated charge registers to the maximum value
  gauge.setADCMode(ADC_MODE_SLEEP); // In sleep mode, voltage and temperature measurements will only take place when requested
  gauge.startMeasurement();
}

void loop() {
  unsigned int raw = gauge.getRawAccumulatedCharge();
  Serial.print(F("Raw Accumulated Charge: "));
  Serial.println(raw, DEC);

  float capacity = gauge.getRemainingCapacity();
  Serial.print(F("Battery Capacity: "));
  Serial.print(capacity, 3);
  Serial.print(F(" / "));
  Serial.print(fullCapacity, DEC);
  Serial.println(F(" mAh"));

  float voltage = gauge.getVoltage();
  Serial.print(F("Voltage: "));
  Serial.print(voltage, 3);
  Serial.println(F(" V"));

  float temperature = gauge.getTemperature();
  Serial.print(F("Temperature: "));
  Serial.print(temperature, 2);
  Serial.println(F(" 'C"));

  Serial.println();

  delay(5000);
}
