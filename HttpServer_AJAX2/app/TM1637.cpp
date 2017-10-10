/*
 * TM1637.cpp
 *
 *  Created on: 7 окт. 2017 г.
 *      Author: jekson
 */

#include "TM1637.h"

static uint8_t TubeTab[] = {0xEB,0x09,0xF1,0xB9,
                           0x1B,0xBA,0xFA,0x89,
                           0xFB,0xBB,0xDB,0x7A,
                           0xE2,0x79,0xF2,0xD2};//0~9,A,b,C,d,E,F

void TM1637::start() {
	CLK_SET;
	DIO_SET;
	delayMicroseconds(2);
	DIO_CLR;
}

void TM1637::stop() {
	CLK_CLR;
	delayMicroseconds(2);
	DIO_CLR;
	delayMicroseconds(2);
	CLK_SET;
	delayMicroseconds(2);
	DIO_SET;
}

void TM1637::ask() {
	CLK_CLR;
	DIO_SET;
	delayMicroseconds(5);
	while(DIO_READ);
	CLK_SET;
	delayMicroseconds(2);
	CLK_CLR;
}

TM1637::TM1637() {
	// TODO Auto-generated constructor stub
	bright = DEF_BRIGHT;
	showOn = true;
	setDisplayControl();
}

void TM1637::writeByte(uint8_t data) {
	for (uint8_t i = 0; i < 8; i++){
		CLK_CLR;
		if (data & 0x01){
			DIO_SET;
		}
		else {
			DIO_CLR;
		}
		delayMicroseconds(3);
		data = data >> 1;
		CLK_SET;
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
		uint8_t data = TubeTab[digits[i]];
		if ((points & (1 << i)) != 0)
			data |= (1 << POINT_NUM);
		writeByte(data);
		ask();
	}
	stop();
}

uint8_t TM1637::scanKey() {
	uint8_t key;
	start();
	writeByte(READ_KEY);
	ask();
	DIO_SET;
	for (uint8_t i = 0; i < 8; i++){
		CLK_CLR;
		delayMicroseconds(30);
		CLK_SET;
		if(DIO_READ != 0){
			key |= (1 << i);
		}

		delayMicroseconds(30);
	}
	ask();
	stop();
	return key;
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

void TM1637::initialize() {
}

void TM1637::off() {
	showOn = false;
	setDisplayControl();
}

void TM1637::printChar(uint8_t pos, uint8_t data) {
	digits[pos] = data;
}

void TM1637::print(uint16_t data) {
	for (uint8_t i = DGT_COUNT; i > 0; i--){
		digits[i - 1] = data % 10;
		data /= 10;
	}
}

void TM1637::setPoint(uint8_t pos) {
	points = pos;
}
