#include <Arduino.h>

// Define LED control pins
#define LED1 16
#define LED2 17
#define LED3 18

// Define analog input pin (ADC0 = GPIO36 on ESP32)
#define ADC0 36

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 LED Control + Analog Read Test");

  // Set LED pins as outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // Set ADC pin as input (not strictly required, but for clarity)
  pinMode(ADC0, INPUT);
}

void loop() {
  // Basic LED pattern
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  delay(500);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, LOW);
  delay(500);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, HIGH);
  delay(500);

  // Read the analog voltage at ADC0 (GPIO36)
  int rawValue = analogRead(ADC0);

  // Convert to voltage (assuming 3.3 V reference)
  float voltage = (rawValue / 4095.0) * 3.3;

  // Print results
  Serial.print("ADC0 Raw: ");
  Serial.print(rawValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");
}