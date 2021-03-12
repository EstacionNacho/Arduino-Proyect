#include <arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <time.h>
#include "uartLib/uart.hpp"
#include <string.h>
#include <stdio.h>

#define LATCH_DIO 4
#define CLK_DIO 7
#define DATA_DIO 8
#define BAUD 9600
#define MYUBRR  F_CPU/16/BAUD-1
#define LOW  0
#define HIGH 1
#define LSBFIRST 0
#define MSBFIRST 1

const unsigned char SEGMENT_MAP[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82,
		0xF8, 0X80, 0X90 };
const unsigned char SEGMENT_SELECT[] = { 0xF1, 0xF2, 0xF4, 0xF8 };

const int speed1 = 250;
const int speed2 = 120;
const int speed3 = 75;
int cont;

void WriteSegment(byte Segment, byte Value);
void WriteSegmentValue(byte Segment, byte Value);
unsigned char randomPos();
void countUP();
void delay();
void beep();

int main() {

	//LED AND PANEL
	DDRB |= (1 << DDB5) | (1 << DDB4) | (1 << DDB3) | (1 << DDB2)
			| /*PANEL D8*/(1 << DDB0);
	PORTB |= (1 << PORTB5) | (1 << PORTB4) | (1 << PORTB3) | (1 << PORTB2)
			| (1 << PORTB0);
	//BUTTONS
	DDRC &= ~(1 << DDC3) | ~(1 << DDC2) | ~(1 << DDC1);
	PORTC |= (1 << PC3) | (1 << PC2) | (1 << PC1);
	//BUZZER AND PANEL
	DDRD |= (1 << DDD3) | /* PANEL D4 D7 */(1 << DDD4) | (1 << DDD7);
	PORTD |= (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD7);

	int obj = 0;
	byte pos = 0xc0;
	cont = 0;
	int fail = 0;

	init_uart(MYUBRR);
	//WriteSegmentAdvanced(0, 0x9c);
	//WriteSegmentAdvanced(1, 0xa3);

	while (1) {
		//Check fails and light leds
		if (fail == 1)
			PORTB = 0b11111011;
		if (fail == 2)
			PORTB = 0b11110011;
		if (fail == 3)
			PORTB = 0b11100011;
		if (fail == 4) {
			//Losing the game
			PORTB = 0b11000011;
			beep();
			while ((PINC & 1 << PINC2)) {
				int aux = cont / 10;
				int aux2 = cont % 10;
				WriteSegment(2, aux);
				WriteSegment(3, aux2);
			}
			fail = 0;
			PORTB = 0b11111111;
			cont = 0;
			obj = 0;
			pos = 0xc0;
		}

		//Player guide
		WriteSegmentValue(3, 0xdd);
		WriteSegmentValue(3, 0xeb);
		//Square light and movement
		WriteSegmentValue(obj, pos);

		delay();

		//Check button press
		if (!(PINC & 1 << PINC1) && (obj == 3 && pos == 0x9c))
			countUP();
		if (!(PINC & 1 << PINC3) && (obj == 3 && pos == 0xa3))
			countUP();
		if ((!(PINC & 1 << PINC1) && (obj != 3 || pos != 0x9c))
				|| (!(PINC & 1 << PINC3) && (obj != 3 || pos != 0xa3)))
			fail++;

		//Position of squares control
		obj++;
		if (obj > 3) {
			obj = 0;
			pos = randomPos();
		}
		delay();
	}

}

//Control of speed
void delay() {
	if (cont <= 10)
		_delay_ms(speed1);
	if (cont > 10 && cont <= 20)
		_delay_ms(speed2);
	if (cont > 20)
		_delay_ms(speed3);
}

//Streak information
void countUP() {
	char contS[12];
	cont++;
	sprintf(contS, "%d", cont);
	USART_Transmit_StringNoLine(contS);
	USART_Transmit_String(" << Streak");
}

//Random generation of positions top or bottom
unsigned char randomPos() {
	int r = random(0, 2);
	if (r == 0)
		return 0x9c;
	if (r == 1)
		return 0xa3;
	return 0xbf;
}

void beep(){
	for(int i = 0; i < 10; i++){
	PORTD = 0b11110111;
	_delay_ms(50);
	PORTD = 0b11111111;
	_delay_ms(50);
	}
}

//Write 7 segment display
void WriteSegmentValue(byte Segment, byte Value) {
	digitalWrite(LATCH_DIO, LOW);
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, Value);
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[Segment]);
	digitalWrite(LATCH_DIO, HIGH);
}

void WriteSegment(byte Segment, byte Value) {
	digitalWrite(LATCH_DIO, LOW);
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_MAP[Value]);
	shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[Segment]);
	digitalWrite(LATCH_DIO, HIGH);
}
