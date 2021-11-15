/*
	Arduino Library for LTC2941 and LTC2942
	
	Copyright (c) 2020 Macro Yau

	https://github.com/MacroYau/LTC2942-Arduino-Library
*/

#include "LTC2942.h"

LTC2942::LTC2942(uint8_t rSense) {
	_rSense = rSense;
	_prescalerM = 0xFF;
	_batteryCapacity = 5500; // Default value when M = 128
}

bool LTC2942::begin(TwoWire &wirePort) {
	// Wire.begin() should be called in the application code in advance
	_i2cPort = &wirePort;

	// Checks device ID
	uint8_t chipID = getStatus() >> A_CHIP_ID_OFFSET;
	if (chipID == CHIP_ID_LTC2942) {
		_chipModel = 42;
	} else if (chipID == CHIP_ID_LTC2941) {
		_chipModel = 41;
	} else {
		return false;
	}

	return true;
}

uint16_t LTC2942::getChipModel() {
	return 2900 + _chipModel;
}

void LTC2942::startMeasurement() {
	uint8_t value = readByteFromRegister(REG_B_CONTROL);
	value &= SHUTDOWN_MASK;
	writeByteToRegister(REG_B_CONTROL, value);
}

void LTC2942::stopMeasurement() {
	uint8_t value = readByteFromRegister(REG_B_CONTROL);
	value |= 1;
	writeByteToRegister(REG_B_CONTROL, value);
}

uint8_t LTC2942::getStatus() {
	return readByteFromRegister(REG_A_STATUS);
}

uint16_t LTC2942::getRawAccumulatedCharge() {
	if (_prescalerM == 0xFF) {
		// Needs to obtain M from register B
		uint8_t value = readByteFromRegister(REG_B_CONTROL);
		value &= ~PRESCALER_M_MASK;
		_prescalerM = value >> B_PRESCALER_M_OFFSET;
		_prescalerM = 1 << _prescalerM;
	}
	uint16_t acr = readWordFromRegisters(REG_C_ACC_CHG_MSB);
	return acr;
}

float LTC2942::getRemainingCapacity() {
	uint16_t acr = getRawAccumulatedCharge();
	float fullRange = 65535 * ((float) _prescalerM / 128) * 0.085 * ((float) 50 / _rSense);
	float offset = fullRange - _batteryCapacity;
	return (acr * ((float) _prescalerM / 128) * 0.085 * ((float) 50 / _rSense)) - offset; // mAh
}

float LTC2942::getVoltage(bool oneShot) {
	if (_chipModel != 42) {
		return -1;
	}

	if (oneShot) {
		setADCMode(ADC_MODE_MANUAL_VOLTAGE);
		delay(10);
	}
	uint16_t value = readWordFromRegisters(REG_I_VOLTAGE_MSB);
	return 6 * ((float) value / 65535); // V
}

float LTC2942::getTemperature(bool oneShot) {
	if (_chipModel != 42) {
		return -1;
	}

	if (oneShot) {
		setADCMode(ADC_MODE_MANUAL_TEMP);
		delay(10);
	}
	uint16_t value = readWordFromRegisters(REG_M_TEMP_MSB);
	return (600 * ((float) value / 65535)) - 273; // Celsius
}

void LTC2942::setADCMode(uint8_t mode) {
	if (_chipModel != 42) {
		return;
	}

	if (mode > 0b11) {
		return;
	}

	uint8_t value = readByteFromRegister(REG_B_CONTROL);
	value &= ADC_MODE_MASK;
	value |= (mode << B_ADC_MODE_OFFSET);
	writeByteToRegister(REG_B_CONTROL, value);
}

void LTC2942::setPrescalerM(uint8_t m) {
	if (m < 1 || m > 128) {
		return;
	}

	// Updates instance variable to avoid unnecessary access to register
	_prescalerM = m;
	m = findExponentOfPowerOfTwo(m);

	uint8_t value = readByteFromRegister(REG_B_CONTROL);
	value &= PRESCALER_M_MASK;
	value |= (m << B_PRESCALER_M_OFFSET);
	writeByteToRegister(REG_B_CONTROL, value);
}

void LTC2942::setBatteryCapacity(uint16_t mAh) {
	_batteryCapacity = mAh;
	float q = (float) mAh / 1000;
	uint8_t m = 23 * q;
	if (_rSense != 50) {
		m *= ((float) _rSense / 50);
	}
	if (m > 128) {
		m = 128;
	}
	m = roundUpToPowerOfTwo(m);
	setPrescalerM(m);
}

void LTC2942::setBatteryToFull() {
	writeWordToRegisters(REG_C_ACC_CHG_MSB, 0xFFFF);
}

void LTC2942::setRawAccumulatedCharge(uint16_t charge) {
	writeWordToRegisters(REG_C_ACC_CHG_MSB, charge);
}

void LTC2942::setChargeThresholds(uint16_t high, uint16_t low) {
	writeWordToRegisters(REG_E_CHG_THR_H_MSB, high);
	writeWordToRegisters(REG_G_CHG_THR_L_MSB, low);
}

void LTC2942::setVoltageThresholds(float high, float low) {
	if (_chipModel != 42) {
		return;
	}

	writeByteToRegister(REG_K_VOLTAGE_THR_H, (uint8_t) (high / 0.0234375));
	writeByteToRegister(REG_L_VOLTAGE_THR_L, (uint8_t) (low / 0.0234375));
}

void LTC2942::setTemperatureThresholds(float high, float low) {
	if (_chipModel != 42) {
		return;
	}

	writeByteToRegister(REG_M_TEMP_MSB, (uint8_t) ((high + 273) / 2.34375));
	writeByteToRegister(REG_N_TEMP_LSB, (uint8_t) ((low + 273) / 2.34375));
}

void LTC2942::configureALCC(uint8_t mode) {
	if (mode >= ALCC_MODE_NOT_ALLOWED) {
		return;
	}

	uint8_t value = readByteFromRegister(REG_B_CONTROL);
	value &= ALCC_CONFIG_MASK;
	value |= (mode << B_ALCC_CONFIG_OFFSET);
	writeByteToRegister(REG_B_CONTROL, value);
}

uint8_t LTC2942::findExponentOfPowerOfTwo(uint8_t value) {
	if (value > 64) {
		return 7;
	}

	for (uint8_t i = 0; i < 7; i++) {
		if ((value >> i) & 1) {
			return i;
		}
	}
}

uint8_t LTC2942::roundUpToPowerOfTwo(uint8_t value) {
	// Reference: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value++;
	return value;
}

uint16_t LTC2942::readWordFromRegisters(uint8_t msbAddress) {
	uint16_t value = 0;
	uint8_t msb = 0;
	uint8_t lsb = 0;

	_i2cPort->beginTransmission(LTC2942_ADDRESS);
	_i2cPort->write(msbAddress);
	_i2cPort->endTransmission(false);

	_i2cPort->requestFrom(LTC2942_ADDRESS, 2);
	msb = _i2cPort->read();
	lsb = _i2cPort->read();
	_i2cPort->endTransmission();
	value = (msb << 8) | lsb;

	return value;
}

bool LTC2942::writeWordToRegisters(uint8_t msbAddress, uint16_t value) {
	_i2cPort->beginTransmission(LTC2942_ADDRESS);
	_i2cPort->write(msbAddress);
	_i2cPort->write((uint8_t) (value >> 8));
	_i2cPort->write((uint8_t) value);
	return (_i2cPort->endTransmission() == 0);
}

uint8_t LTC2942::readByteFromRegister(uint8_t address) {
	uint8_t value = 0;

	_i2cPort->beginTransmission(LTC2942_ADDRESS);
	_i2cPort->write(address);
	_i2cPort->endTransmission(false);

	_i2cPort->requestFrom(LTC2942_ADDRESS, 1);
	value = _i2cPort->read();
	_i2cPort->endTransmission();

	return value;
}

bool LTC2942::writeByteToRegister(uint8_t address, uint8_t value) {
	_i2cPort->beginTransmission(LTC2942_ADDRESS);
	_i2cPort->write(address);
	_i2cPort->write(value);
	return (_i2cPort->endTransmission() == 0);
}
