/*
 Name:		ExternalThermalSensor.ino
 Created:	9/25/2015 11:06:53 PM
 Author:	Tim Lampman
*/
#include <JeeLib.h>
#include <XBee.h>
#include <limits.h>
#include <AltSoftSerial.h>

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup the watchdog

// XBee variables
AltSoftSerial xbeeSerial;  // The software serial port for communicating with the Xbee (TX Pin 9, RX Pin 8)
XBee localRadio = XBee();  // The connection for the local coordinating radio

// XBee data codes
const uint8_t TEMPERATURE_CODE = 1;
const uint8_t LUMINOSITY_CODE = 2;
const uint8_t PRESSURE_CODE = 3;
const uint8_t HUMIDITY_CODE = 4;
const uint8_t POWER_CODE = 5;
const uint8_t LUX_CODE = 6;
const uint8_t HEATING_CODE = 7;
const uint8_t THERMOSTAT_CODE = 8;
const uint8_t TEMP_12BYTE_CODE = 9;

// Determine the maximum unsigned long
const unsigned long MAX_LONG = ULONG_MAX;

// Timing variables
const unsigned long TEMP_PERIOD = 600000;  // The period between temperature measurements (ms).

// Local pins
const int LOCAL_POWER_PIN = A0;	// The analog pin that measures the power supply

const int POWER_PIN = 13;		// The digital pin for powering the sensors and XBee

// Union for conversion of numbers to byte arrays
union FloatConverter {
	float f;
	uint8_t b[sizeof(float)];
};


// the setup function runs once when you press reset or power the board
void setup() {
	pinMode(POWER_PIN, OUTPUT);		// Set the power pin to digital output
	digitalWrite(POWER_PIN, LOW);	// Turn off the power
}

// the loop function runs over and over again until power down or reset
void loop() {
	// Turn on the power to the attached components
	digitalWrite(POWER_PIN, HIGH);

	// Power up the XBee
	xbeeSerial.begin(9600);
	localRadio.setSerial(xbeeSerial);

	// Read the power
	FloatConverter Power;
	int powerSignal = analogRead(LOCAL_POWER_PIN);
	Power.f = 3.7*powerSignal/1023.0;

	// Create the byte array to pass through the XBee
	size_t floatBytes = sizeof(float);
	uint8_t package[1 + floatBytes];
	package[0] = POWER_CODE;
	for (int i = 0; i < floatBytes; i++) {
		package[i + 1] = Power.b[i];
	}

	// Send the data package to the coordinator
	XBeeAddress64 address = XBeeAddress64(0x00000000, 0x00000000);
	ZBTxRequest zbTX = ZBTxRequest(address, package, sizeof(package));
	localRadio.send(zbTX);

/*	// Print message to the serial port
	Serial.print("Sent the following message (");
	Serial.print(Temperature.f);
	Serial.print(",");
	Serial.print(Power.f);
	Serial.print("): ");
	for (int i = 0; i < sizeof(package); i++) {
		if (i != 0) Serial.print("-");
		Serial.print(package[i], HEX);
	}
	Serial.println("");
*/

	// Turn off the power to the components and sleep
	xbeeSerial.end();	// Turn off the serial communication with the xbee
	digitalWrite(POWER_PIN, LOW);
	Sleepy::loseSomeTime(TEMP_PERIOD);
}
