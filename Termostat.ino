#include <Arduino.h>
#include "DHT_Async.h"
#include <LiquidCrystal.h>
#include <IRremote.h>

/* Uncomment according to your sensortype. */
//#define DHT_SENSOR_TYPE DHT_TYPE_11
//#define DHT_SENSOR_TYPE DHT_TYPE_21
#define DHT_SENSOR_TYPE DHT_TYPE_22
#define LCD_BACKLIGHT 10

static const int DHT_SENSOR_PIN = A1;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // LCD Keypad Shield standard pins
bool odd = true;

float termostatHighTemp = 21;
float termostatLowTemp = 19;

static const unsigned long intervalInMs = 5000;
unsigned long keypress_timeout = 0;

String output1 = "";
String output2 = "";

const int irSenderPin = 3;
IRsend irsend(irSenderPin);

const int NOTHING = 0;
const int UP      = 1;
const int DOWN    = 2;
const int LEFT    = 3;
const int RIGHT   = 4;
const int SELECT  = 5;

bool termostatOn = false;
String status = "OFF";

/*
 * Initialize the serial port.
 */
void setup() {
  Serial.begin(115200);
//   pinMode(LCD_BACKLIGHT, OUTPUT);
//   digitalWrite(LCD_BACKLIGHT, HIGH);  // TÆND
//   delay(2000);
//   digitalWrite(LCD_BACKLIGHT, LOW);   // SLUK

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("** TERMOSTAT **");
//   for(int i=0;i<255;i++) {
//     analogWrite(LCD_BACKLIGHT, i);  // 50% lysstyrke (0-255)        
//     delay(20);
//   }
  delay(1000);
  //lcd.clear();

}

/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment(float *temperature, float *humidity) {
    static unsigned long next_measurement_timestamp = millis() + intervalInMs;

    /* Measure once every four seconds. */
    if (millis() > next_measurement_timestamp) {
        if (dht_sensor.measure(temperature, humidity)) {
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

    int keypress = readKeypad();

    if (keypress != NOTHING) {
        keypress_timeout = millis() +  5000;

        switch (keypress){
            case LEFT:
              termostatLowTemp = termostatLowTemp - 0.5;
            break;
            case RIGHT:
              termostatLowTemp = termostatLowTemp + 0.5;
            break;
            case UP:
              termostatHighTemp = termostatHighTemp + 0.5;
            break;
            case DOWN:
              termostatHighTemp = termostatHighTemp - 0.5;
            break;
            case SELECT:
            break;
        }

        if (termostatHighTemp <= termostatLowTemp + 1){
          termostatLowTemp = termostatHighTemp - 1;
        }

        output1 = "Min Temp: " + String(termostatLowTemp) + (char)223 + "C";
        output2 = "Max Temp: " + String(termostatHighTemp) + (char)223 + "C";

        lcd.setCursor(0, 0);
        lcd.print(output1);
        lcd.setCursor(0, 1);
        lcd.print(output2);
        delay(300);
    }

    /* Measure temperature and humidity.  If the functions returns
       true, then a measurement is available. */
    if (keypress_timeout < millis() && measure_environment(&temperature, &humidity)) {
        String star = " ";
        if (odd) {
            star = "*";
        }    
        odd = !odd;

        if (termostatOn) {
          if (temperature > termostatHighTemp) {
            termostatOn = false;
            status = "OFF";
          }
        } else {
          if (temperature < termostatLowTemp) {
              status = " ON";
              termostatOn = true;
          }
        }

        if (termostatOn) {
          irsend.sendNEC(0x20DF10EF, 32); // NEC-protokol,32-bit  => "Power"
        } else {
          irsend.sendNEC(0x20DF906F, 32); // NEC-protokol, 32-bit => "Mute"
        }

        output1 = "Temp: " + String(temperature) + (char)223 + "C  " + star;
        output2 = "Hum : " + String(humidity) + "% " +  status;

        Serial.println(output1);
        Serial.println(output2);

        lcd.setCursor(0, 0);
        lcd.print(output1);
        lcd.setCursor(0, 1);
        lcd.print(output2);
    }
}

// Læs knapper fra LCD Keypad Shield
static int readKeypad() {
  int adc_key_in = analogRead(A0);
  if (adc_key_in == 0) return RIGHT; 
  if (adc_key_in < 150) return UP; 
  if (adc_key_in < 350) return DOWN; 
  if (adc_key_in < 500) return LEFT; 
  if (adc_key_in < 750) return SELECT; 
  return 0; // Ingen knap trykket
}

// Format string function 
String format(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int size = vsnprintf(nullptr, 0, fmt, args) + 1;
  va_end(args);

  char *buffer = new char[size];
  va_start(args, fmt);
  vsnprintf(buffer, size, fmt, args);
  va_end(args);

  String result(buffer);
  delete[] buffer;
  return result;
}
