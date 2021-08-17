/**
 * ATtiny85: Drain one (charged) LiPo cell to 3.85v for storage.
 */

// github.com/markfickett/arduinomorse
#include <morse.h>

// The load used to drain the battery. Arduino pin 1, ATtiny85 pin 6.
#define PIN_LOAD 1

// A voltage divider connects the battery to an analog input (1.1v reference),
// to sense battery voltage.
#define PIN_BATTERY_DIVIDED A1
// Measured resistance of 68k and 15k voltage divider resistors.
#define DIVIDER_HIGH 67.5
#define DIVIDER_LOW 14.88

// Actual "1.1v" internal reference value.
#define VOLTAGE_REF 1.113
/*
To calculate the actual internal reference value:
1.  Upload the sketch with measured DIVIDER_* values and VOLTAGE_REF=1.1.
2.  Measure actual battery voltage (with a voltmeter) and reported voltage
    (from Morse code on the status LED).
3.  Determine the correct VOLTAGE_REF using calculate_aref.py.
4.  Re-upload with corrected VOLTAGE_REF (and same DIVIDER_) values.
*/

// Storage voltage for LiPo battery cells.
// Keep the load on until the battery reaches this voltage.
#define VOLTAGE_TARGET 3.85
// If the voltage falls below 3.80, reduce status LED usage
// to avoid reducing the voltage further.
#define VOLTAGE_MARGIN 0.05

// Turn off the load to check the resting battery voltage every
// 5 minutes. Report the last-read battery voltage every 30s.
#define REPORT_INTERVAL_MS 30000
#define REPORTS_PER_CHECK 10
#define STABILIZATION_DELAY_MS 1000

#define PIN_STATUS_LED 0

LEDMorseSender voltageSender(PIN_STATUS_LED, 8.0f /* WPM */);

float batteryVoltage;
int checkCountdown;

void setup() {
  voltageSender.setup();
  pinMode(PIN_LOAD, OUTPUT);
  analogReference(INTERNAL);  // 1.1v internal ATtiny85 voltage reference.
  checkCountdown = 0;
}

void loop() {
  if (checkCountdown-- <= 0) {
    batteryVoltage = getBatteryVoltage();
    if (batteryVoltage > VOLTAGE_TARGET) {
      digitalWrite(PIN_LOAD, HIGH);
    }
    checkCountdown = REPORTS_PER_CHECK;
  }
  sendVoltage(batteryVoltage);
  delay(REPORT_INTERVAL_MS);
}

float getBatteryVoltage() {
  digitalWrite(PIN_LOAD, LOW);
  delay(STABILIZATION_DELAY_MS);
  int divided = analogRead(PIN_BATTERY_DIVIDED);
  float measuredVoltage = VOLTAGE_REF * (divided / 1024.0);
  return measuredVoltage * ((DIVIDER_HIGH + DIVIDER_LOW) / DIVIDER_LOW);
}

void sendVoltage(float batteryVoltage) {
  if (batteryVoltage <= VOLTAGE_TARGET - VOLTAGE_MARGIN) {
    // Just send one dit, to avoid running the battery down further.
    voltageSender.setMessage(String(" e "));
  } else {
    String voltageDigits(int(100 * batteryVoltage));
    voltageDigits = " " + voltageDigits + " ";
    voltageSender.setMessage(voltageDigits);
  }
  voltageSender.sendBlocking();
}
