#include <arduino.h>
#include <avr/io.h>
#include <util/delay.h>

#define LATCH 4
#define CLK 7
#define DATA 8
#define LOW  0
#define HIGH 1
#define LSBFIRST 0
#define MSBFIRST 1

const unsigned char SEGMENT_MAP[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82,
		0xF8, 0X80, 0X90 };
const unsigned char SEGMENT_SELECT[] = { 0xF1, 0xF2, 0xF4, 0xF8 };

void WriteSegment(byte Segment, byte Value);
void WriteSegmentValue(byte Segment, byte Value);
void writePanel();
void restart();
void countdown();
void beep();

int PL3 = 0;
int PL2 = 0;
int PL1 = 0;
int PL0 = 0;

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
	DDRD |= (1 << DDD3) | /*PANEL D4 D7*/(1 << DDD4) | (1 << DDD7);
	PORTD |= (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD7);

	countdown();

	while (1) {

		writePanel();

		//cheat mode
		/*
		if (PL0 == 1 && PL1 == 0 && PL2 == 0 && PL3 == 0){
			_delay_ms(1000);
		}
		*/

		//Check if button is pressed
		if (!(PINC & 1 << PINC1)) {
			//Check values of timer
			if (PL0 == 1 && PL1 == 0 && PL2 == 0 && PL3 == 0) {
				WriteSegmentValue(0, 0xFF);
				beep();
				while ((PINC & 1 << PINC1)) {
					writePanel();
				}
				WriteSegmentValue(3, 0xFF);
				_delay_ms(1000);
				restart();
				countdown();
			} else {
				WriteSegmentValue(0, 0xFF);
				_delay_ms(3000);
				while ((PINC & 1 << PINC1)) {
					writePanel();
				}
				WriteSegmentValue(3, 0xFF);
				_delay_ms(1000);
				restart();
				countdown();
			}
		}
		//Timer control
		PL3++;
		if (PL3 > 9) {
			PL3 = 0;
			PL2++;
		}
		if (PL2 > 9) {
			PL2 = 0;
			PL1++;
		}
		if (PL1 > 9) {
			PL1 = 0;
			PL0++;
		}
		if (PL0 == 2) {
			while ((PINC & 1 << PINC1)) {
				WriteSegment(0, 0);
				WriteSegment(1, 0);
				WriteSegment(2, 0);
				WriteSegment(3, 0);
			}
			WriteSegmentValue(3, 0xFF);
			_delay_ms(1000);
			restart();
			countdown();
		}
	}
}

//Writes the time on the panel and controls delay
void writePanel() {
	WriteSegment(0, PL0);
	_delay_ms(2);
	WriteSegment(1, PL1);
	_delay_ms(2);
	WriteSegmentValue(1, 0x7F);
	_delay_ms(2);
	WriteSegment(2, PL2);
	_delay_ms(2);
	WriteSegment(3, PL3);
	_delay_ms(2);
}

void restart() {
	PL3 = 0;
	PL2 = 0;
	PL1 = 0;
	PL0 = 0;
}

void countdown() {
	WriteSegment(3, 3);
	_delay_ms(1000);
	WriteSegment(3, 2);
	_delay_ms(1000);
	WriteSegment(3, 1);
	_delay_ms(1000);
}

void beep(){
	for(int i = 0; i < 10; i++){
	PORTD = 0b11110111;
	_delay_ms(50);
	PORTD = 0b11111111;
	_delay_ms(50);
	}
}

//Writes on 7 segment display
void WriteSegmentValue(byte Segment, byte Value) {
	digitalWrite(LATCH, LOW);
	shiftOut(DATA, CLK, MSBFIRST, Value);
	shiftOut(DATA, CLK, MSBFIRST, SEGMENT_SELECT[Segment]);
	digitalWrite(LATCH, HIGH);
}

void WriteSegment(byte Segment, byte Value) {
	digitalWrite(LATCH, LOW);
	shiftOut(DATA, CLK, MSBFIRST, SEGMENT_MAP[Value]);
	shiftOut(DATA, CLK, MSBFIRST, SEGMENT_SELECT[Segment]);
	digitalWrite(LATCH, HIGH);
}

