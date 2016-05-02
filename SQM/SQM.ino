

#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define version 1.0
#define READBUTTON 3 // will be used as an interrupt to wake
#define LOWSCALE 1.0
#define MEDSCALE 25.0
#define HIGHSCALE 428.0
#define MAXSCALE 9876.0

Adafruit_TSL2591 tsl=Adafruit_TSL2591(2591);
double gainscale = MAXSCALE;
uint32_t luminosity;
uint16_t ir, full, visible;
double adjustedVisible, adjustedIR;
double mag;

void setup() {
	Serial.begin(115200);
	tsl.begin();
	tsl.setGain(TSL2591_GAIN_MAX);
	tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);
	display.begin(/*SSD1306_EXTERNALVCC*/SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
	pinMode(READBUTTON, INPUT);
	
	// Clear the buffer.
	display.clearDisplay();
	display.display();
	display.setTextColor(WHITE);
	display.setTextSize(1);
	activate();
}

void loop() {
	if (debounce(READBUTTON)) { // replace with interrupt wakeup
		activate();
	}
	
}

void activate() {
	String gainString = "Max gain";
	display.clearDisplay();
	display.display();
	display.setTextSize(1);
	delay(50); // display cooldown
	tsl.setGain(TSL2591_GAIN_HIGH);
	luminosity = tsl.getFullLuminosity();
	delay(50);
	luminosity = tsl.getFullLuminosity(); // Read twice so value can stabilize.
	ir = luminosity >> 16;
	full = luminosity & 0xFFFF;
	visible = full - ir;
	if (visible== 0xFFFF||ir==0xFFFF) {
		gainString = "High gain";
		tsl.setGain(TSL2591_GAIN_HIGH);
		gainscale = HIGHSCALE;
		luminosity = tsl.getFullLuminosity();
		delay(50);
		luminosity = tsl.getFullLuminosity();
		ir = luminosity >> 16;
		full = luminosity & 0xFFFF;
		visible = full - ir;
		if (visible == 0xFFFF || ir == 0xFFFF) { // look, dude. It's daylight at this point. Knock it off
			gainString = "Med gain";
			tsl.setGain(TSL2591_GAIN_MED);
			gainscale = MEDSCALE;
			luminosity = tsl.getFullLuminosity();
			delay(50);
			luminosity = tsl.getFullLuminosity();
			ir = luminosity >> 16;
			full = luminosity & 0xFFFF;
			visible = full - ir;
			if (visible == 0xFFFF || ir == 0xFFFF) { // ARE YOU ON THE SUN?
				gainString = "Low gain";
				tsl.setGain(TSL2591_GAIN_LOW);
				gainscale = LOWSCALE;
				luminosity = tsl.getFullLuminosity();
				delay(50);
				luminosity = tsl.getFullLuminosity();
				ir = luminosity >> 16;
				full = luminosity & 0xFFFF;
				visible = full - ir;

			}
		}
	}
	adjustedIR = (float)ir / gainscale;
	adjustedVisible = (float)visible / gainscale;
	mag = -1.085736205*log(.925925925 * pow(10,-5.)*adjustedVisible);
	Serial.print("I="); Serial.print(adjustedIR,6); Serial.print(",V="); Serial.print(adjustedVisible,6); Serial.print(",M="); Serial.println(mag);
	display.setCursor(0, 0);
	display.print("Light Polution Meter");
	display.setCursor(15, 8);
	display.print("v"); display.print(version); display.print(" - "); display.print(gainString);
	display.setCursor(0, 18);
	display.print("IR:  "); display.println(adjustedIR, 6);
	display.setCursor(0, 27);
	display.print("Vis: "); display.println(adjustedVisible, 6);
	display.setCursor(30, 36);
	display.print("Mag/sec^2");
	display.setTextSize(2);
	display.setCursor(30, 48);
	if (isinf(mag)) {
		mag = 25.0;
	}
	display.println(mag);
	display.display();	
}

bool debounce(uint8_t _pin) {
	uint8_t read = digitalRead(_pin);
	delay(50);
	return (digitalRead(_pin) == read && read == HIGH);
}