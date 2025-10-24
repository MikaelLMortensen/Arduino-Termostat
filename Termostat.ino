#include <Arduino.h>
#include "DHT_Async.h"
#include <LiquidCrystal.h>

/* Uncomment according to your sensortype. */
//#define DHT_SENSOR_TYPE DHT_TYPE_11
//#define DHT_SENSOR_TYPE DHT_TYPE_21
#define DHT_SENSOR_TYPE DHT_TYPE_22

static const int DHT_SENSOR_PIN = A1;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // LCD Keypad Shield standard pins
bool odd = true;

/*
 * Initialize the serial port.
 */
void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("* DHT11 Sensor *");
  delay(2000);
  lcd.clear();
}

/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment(float *temperature, float *humidity) {
    static unsigned long measurement_timestamp = millis();

    /* Measure once every four seconds. */
    if (millis() - measurement_timestamp > 2000ul) {
        if (dht_sensor.measure(temperature, humidity)) {
            measurement_timestamp = millis();
            return (true);
        }
    }

    return (false);
}


/*
 * Main program loop.
 */
void loop() {
    float temperature;
    float humidity;

    /* Measure temperature and humidity.  If the functions returns
       true, then a measurement is available. */
    if (measure_environment(&temperature, &humidity)) {
        Serial.print("T = ");
        Serial.print(temperature, 1);
        Serial.print(" deg. C, H = ");
        Serial.print(humidity, 1);
        Serial.println("%");

        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temperature);
        lcd.print((char)223);  // °-symbolet
        if (odd == true) {
            lcd.print("C  *");     
        } else {
            lcd.print("C   ");     
        }

        lcd.setCursor(0, 1);
        lcd.print("Fugt: ");
        lcd.print(humidity);

        if (odd == true) {
            lcd.print("%    ");
        } else {
            lcd.print("%   *");
        }
        odd = !odd;

        int adc_key_in = analogRead(A0);
        Serial.print("Analog: ");
        Serial.print(adc_key_in);
        Serial.print(" - ");
        Serial.println(readKeypad());
    }
}

// Læs knapper fra LCD Keypad Shield
static String readKeypad() {
  int adc_key_in = analogRead(A0);
  if (adc_key_in == 0) return "RIGHT"; // UP
  if (adc_key_in < 150) return "UP"; // UP
  if (adc_key_in < 350) return "DOWN"; // UP
  if (adc_key_in < 500) return "LEFT"; // RIGHT
  if (adc_key_in < 750) return "SELECT"; // SELECT
  return ""; // Ingen knap trykket
}
