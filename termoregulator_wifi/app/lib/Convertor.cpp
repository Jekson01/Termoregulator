/*
 * Convertor.cpp
 *
 *  Created on: 30 окт. 2017 г.
 *      Author: r2d2
 */

#include "Convertor.h"

char hexToDec(char hex) {
	char dec = 0;
	switch (hex) {
	case '0':
		dec = 0;
		break;
	case '1':
		dec = 1;
		break;
	case '2':
		dec = 2;
		break;
	case '3':
		dec = 3;
		break;
	case '4':
		dec = 4;
		break;
	case '5':
		dec = 5;
		break;
	case '6':
		dec = 6;
		break;
	case '7':
		dec = 7;
		break;
	case '8':
		dec = 8;
		break;
	case '9':
		dec = 9;
		break;

	case 'A':
	case 'a':
		dec = 10;
		break;
	case 'B':
	case 'b':
		dec = 11;
		break;
	case 'C':
	case 'c':
		dec = 12;
		break;
	case 'D':
	case 'd':
		dec = 13;
		break;
	case 'E':
	case 'e':
		dec = 14;
		break;
	case 'F':
	case 'f':
		dec = 15;
	}
	return dec;
}

String urlToString(String url) {
	uint16_t length = url.length();
	String outStr = "";
	uint16_t pos = 0;
	for (uint16_t i = 0; i < length; i++) {
		if (url.charAt(i) == '%') {
			char arr[5];
			String s1, s2;
			arr[0] = url.charAt(i + 1);
			arr[1] = url.charAt(i + 2);
			arr[2] = url.charAt(i + 4);
			arr[3] = url.charAt(i + 5);
			arr[4] = '\0';
			i = i + 5;
			outStr.concat("    ");
			outStr.setCharAt(pos, charToHex(arr[0], arr[1]));
			outStr.setCharAt(pos + 1, charToHex(arr[2], arr[3]));
			pos += 2;
			outStr.trim();

		} else {
			pos++;
			if (url.charAt(i) == '+'){
				outStr.concat(' ');
			}else{
				outStr.concat(url.charAt(i));
			}
		}
	}
	return outStr;
}

uint8_t charToHex(uint8_t c1, uint8_t c2) {
	uint8_t out = 0;
	out = hexToDec(c1);
	out = out << 4;
	out |= hexToDec(c2);
	return out;
}
