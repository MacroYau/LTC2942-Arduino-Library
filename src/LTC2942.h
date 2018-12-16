/*
	Arduino Library for LTC2942
	
	Copyright (c) 2018 Macro Yau

	https://github.com/MacroYau/LTC2942-Arduino-Library
*/

#ifndef LTC2942_H
#define LTC2942_H

#include "Arduino.h"
#include "Wire.h"

#define LTC2942_ADDRESS			0x64

/* Register Map */

#define REG_A_STATUS			0x00 // Status (R)
#define REG_B_CONTROL			0x01 // Control (R/W)
#define REG_C_ACC_CHG_MSB		0x02 // Accumulated Charge MSB (R/W)
#define REG_D_ACC_CHG_LSB		0x03 // Accumulated Charge LSB (R/W)
#define REG_E_CHG_THR_H_MSB		0x04 // Charge Threshold High MSB (R/W)
#define REG_F_CHG_THR_H_LSB		0x05 // Charge Threshold High LSB (R/W)
#define REG_G_CHG_THR_L_MSB		0x06 // Charge Threshold Low MSB (R/W)
#define REG_H_CHG_THR_L_LSB		0x07 // Charge Threshold Low LSB (R/W)
#define REG_I_VOLTAGE_MSB		0x08 // Voltage MSB (R)
#define REG_J_VOLTAGE_LSB		0x09 // Voltage LSB (R)
#define REG_K_VOLTAGE_THR_H		0x0A // Voltage Threshold High (R/W)
#define REG_L_VOLTAGE_THR_L		0x0B // Voltage Threshold Low (R/W)
#define REG_M_TEMP_MSB			0x0C // Temperature MSB (R)
#define REG_N_TEMP_LSB			0x0D // Temperature LSB (R)
#define REG_O_TEMP_THR_H		0x0E // Temperature Threshold High (R/W)
#define REG_P_TEMP_THR_L		0x0F // Temperature Threshold Low (R/W)

/* Status Register (A) */

#define A_CHIP_ID_OFFSET		7 // Default 0
#define CHIP_ID_LTC2942			0
#define CHIP_ID_LTC2941			1
// Bits below are cleared after register is read
#define A_ACC_CHG_OF_UF_OFFSET	5 // Default 0
#define A_TEMP_ALERT_OFFSET		4 // Default 0
#define A_CHG_ALERT_H_OFFSET	3 // Default 0
#define A_CHG_ALERT_L_OFFSET	2 // Default 0
#define A_VOLTAGE_ALERT_OFFSET	1 // Default 0
#define A_UV_LOCK_ALERT_OFFSET	0

/* Control Register (B) */

#define B_ADC_MODE_OFFSET		6 // Default [00]
#define ADC_MODE_MASK			0b00111111
#define ADC_MODE_AUTO			0b11
#define ADC_MODE_MANUAL_VOLTAGE	0b10
#define ADC_MODE_MANUAL_TEMP	0b01
#define ADC_MODE_SLEEP			0b00

#define B_PRESCALER_M_OFFSET	3 // Default [111], M = 2^(4 * B[5] + 2 * B[4] + B[3])
#define PRESCALER_M_MASK		0b11000111

#define B_ALCC_CONFIG_OFFSET	1 // Default [10]
#define ALCC_CONFIG_MASK		0b11111001
#define ALCC_MODE_ALERT			0b10
#define ALCC_MODE_CHG_COMPLETE	0b01
#define ALCC_MODE_DISABLED		0b00
#define ALCC_MODE_NOT_ALLOWED	0b11

#define SHUTDOWN_MASK			0b11111110

class LTC2942 {
	public:
		LTC2942(uint8_t rSense = 50);
		bool begin(TwoWire &wirePort = Wire);
		void startMeasurement();
		void stopMeasurement();
		uint8_t getStatus();
		uint16_t getRawAccumulatedCharge();
		float getRemainingCapacity();
		float getVoltage(bool oneShot = true);
		float getTemperature(bool oneShot = true);
		void setADCMode(uint8_t mode);
		void setPrescalerM(uint8_t m);
		void setBatteryCapacity(uint16_t mAh);
		void setBatteryToFull();
		void setRawAccumulatedCharge(uint16_t charge);
		void setChargeThresholds(uint16_t high, uint16_t low);
		void setVoltageThresholds(float high, float low);
		void setTemperatureThresholds(float high, float low);
		void configureALCC(uint8_t mode);
		uint8_t findExponentOfPowerOfTwo(uint8_t value);
		uint8_t roundUpToPowerOfTwo(uint8_t value);
		uint16_t readWordFromRegisters(uint8_t msbAddress);
		bool writeWordToRegisters(uint8_t msbAddress, uint16_t value);
		uint8_t readByteFromRegister(uint8_t address);
		bool writeByteToRegister(uint8_t address, uint8_t value);
	private:
		uint8_t _rSense;
		uint8_t _prescalerM;
		uint16_t _batteryCapacity;
		TwoWire *_i2cPort;
};

#endif
