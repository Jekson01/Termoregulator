/*
 * TM1637.cpp
 *
 *  Created on: 7 окт. 2017 г.
 *      Author: jekson
 */

#include "TM1637.h"
/*
static uint8_t charTab[] = {0xEB,0x09,0xF1,0xB9,
                           0x1B,0xBA,0xFA,0x89,
                           0xFB,0xBB,0xDB,0x7A,
                           0xE2,0x79,0xF2,0xD2};//0~9,A,b,C,d,E,F
*/

void TM1637::start() {
	setClk(HIGH);
	setDio(HIGH);
	delayMicroseconds(2);
	setDio(LOW);
}

void TM1637::stop() {
	setClk(LOW);
	delayMicroseconds(2);
	setDio(LOW);
	delayMicroseconds(2);
	setClk(HIGH);
	delayMicroseconds(2);
	setDio(HIGH);
}

void TM1637::ask() {
	setClk(LOW);
	setDio(HIGH);
	delayMicroseconds(5);
	while(getDio());
	setClk(HIGH);
	delayMicroseconds(2);
	setClk(LOW);
}

TM1637::TM1637() {
	// TODO Auto-generated constructor stub
}

void TM1637::writeByte(uint8_t data) {
	for (uint8_t i = 0; i < 8; i++){
		setClk(LOW);
		if (data & 0x01){
			setDio(HIGH);
		}
		else {
			setDio(LOW);
		}
		delayMicroseconds(3);
		data = data >> 1;
		setClk(HIGH);
		delayMicroseconds(3);
	}
}

void TM1637::update() {
	start();
	writeByte(WRITE_DATA);
	ask();
	stop();
	start();
	writeByte(0xC0);
	ask();
	for(uint8_t i = 0; i < DGT_COUNT; i++){
		uint8_t data = charTable[digits[i]];
		if ((points & (1 << i)) != 0)
			data |= (1 << pointPin);
		writeByte(data);
		ask();
	}
	stop();
}

uint8_t TM1637::scanKey() {
	uint8_t keyCode = 0;
	start();
	writeByte(READ_KEY);
	ask();
	setDio (HIGH);
	for (uint8_t i = 0; i < 8; i++) {
		setClk (LOW);
		delayMicroseconds(30);
		setClk(HIGH);
		if (getDio()) {
			keyCode |= (1 << i);
		}

		delayMicroseconds(30);
	}
	ask();
	stop();
	return keyCode;
}

void TM1637::setBright(uint8_t br) {
	bright = br % MAX_BRIGHT;
	setDisplayControl();
}

void TM1637::on() {
	showOn = true;
	setDisplayControl();
}

void TM1637::setDisplayControl() {
	uint8_t control = CTRL | bright;
	if (showOn) control |= (1 << 3);
	start();
	writeByte(control);
	ask();
	stop();
}

void TM1637::off() {
	showOn = false;
	setDisplayControl();
}

void TM1637::printChar(uint8_t pos, uint8_t data) {
	digits[pos] = data;
}

void TM1637::setPoint(uint8_t pos) {
	points = pos;
}

void TM1637::setClk(uint8_t state) {
	if (state) {
		pinMode(clkPin, INPUT);
	} else {
		pinMode(clkPin, OUTPUT);
		digitalWrite(clkPin, LOW);
	}
}

void TM1637::setDio(uint8_t state) {
	if (state) {
		pinMode(dioPin, INPUT);
	} else {
		pinMode(dioPin, OUTPUT);
		digitalWrite(dioPin, LOW);
	}
}

void TM1637::initialize(uint16_t clk, uint16_t dio, char* data,	uint8_t pointPin) {
	clkPin = clk;
	dioPin = dio;
	bright = DEF_BRIGHT;
	showOn = true;
	charTable = data;
	this->pointPin = pointPin;
	setDisplayControl();
}

bool TM1637::getDio() {
	if (digitalRead(dioPin))
		return true;
	return false;
}

void TM1637::print(uint16_t data) {
	digits[2] = data % 10;
	data /= 10;
	digits[1] = data % 10;
	data /= 10;
	digits[0] = data % 10;
}
