/*
 * TM1637.h
 *
 *  Created on: 7 окт. 2017 г.
 *      Author: jekson
 */

#ifndef APP_TM1637_H_
#define APP_TM1637_H_

#include <inttypes.h>

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#include "pins_arduino.h"  // for digitalPinToBitMask, etc
#endif

#define CLK	5
#define DIO	4
#define POINT_NUM	2

#define DGT_COUNT	3

#define CLK_SET pinMode(CLK, INPUT)
#define CLK_CLR pinMode(CLK, OUTPUT);\
				digitalWrite(CLK, LOW)


#define DIO_SET pinMode(DIO, INPUT)
#define DIO_CLR	pinMode(DIO, OUTPUT);\
				digitalWrite(DIO, LOW);

#define DIO_READ	digitalRead(DIO)

#define MAX_BRIGHT	8
#define MIN_BRIGHT	0
#define DEF_BRIGHT	2

// 1, the data command set
#define WRITE_DATA	0x40
#define READ_KEY	0x42

// 2, set address command set
#define SET_DGT0	0xC0
#define SET_DGT1	0xC1
#define SET_DGT2	0xC2
#define SET_DGT3	0xC3
#define SET_DGT4	0xC4
#define SET_DGT5	0xC5

// 3, the display control
#define CTRL	0x80

#define POINT1	0x01
#define POINT2	0x02
#define POINT3	0x04
#define POINT4	0x08
#define POINT5	0x10
#define POINT6	0x20


class TM1637 {
	uint8_t bright;
	uint8_t points;
	bool showOn;
	uint8_t digits[DGT_COUNT];
	void start();
	void stop();
	void ask();
	void writeByte(uint8_t data);
	void setDisplayControl();
public:
	TM1637();
	void initialize();
	void update();
	uint8_t scanKey();
	void setBright(uint8_t br);
	void on();
	void off();
	void printChar(uint8_t pos, uint8_t data);
	void print(uint16_t data);
	void setPoint(uint8_t pos);
};

#endif /* APP_TM1637_H_ */