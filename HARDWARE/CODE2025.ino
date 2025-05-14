#include <LiquidCrystal.h>

// LCD pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Sensor pins
constexpr uint8_t HUMIDITY_PIN = A3;
constexpr uint8_t SOIL_PIN     = A2;
constexpr uint8_t LIGHT_PIN    = A1;
constexpr uint8_t TEMP_PIN     = A0;

// Actuator pins
constexpr uint8_t PUMP_PIN    = 7;
constexpr uint8_t HEATER_PIN  = 5;
constexpr uint8_t LAMP_PIN    = 6;

// Thresholds
constexpr uint8_t SOIL_LOW     = 30;   // % below which we water
constexpr uint8_t LIGHT_LOW    = 20;   // % below which lamp is on
constexpr float   TEMP_LOW     = 18.0; // °C below which heater is on

// Timing
unsigned long lastSerialTime = 0;
const unsigned long SERIAL_INTERVAL = 500; // ms

void setup() {
  lcd.begin(20, 4);
  Serial.begin(9600);

  pinMode(PUMP_PIN,   OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(LAMP_PIN,   OUTPUT);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   System Online  ");
  delay(1000);
  lcd.clear();
}

void loop() {
  // Read sensors
  int rawHum   = analogRead(HUMIDITY_PIN);
  int rawSoil  = analogRead(SOIL_PIN);
  int rawLight = analogRead(LIGHT_PIN);
  int rawTemp  = analogRead(TEMP_PIN);

  // Convert to units
  uint8_t humPct   = map(rawHum,   0, 1023, 0, 100);
  uint8_t soilPct  = map(rawSoil,  0, 1023, 0, 100);
  uint8_t lightPct = map(rawLight, 0, 1023, 0, 100);
  float   tempC    = (rawTemp * 5.0 / 1023.0) * 100.0; // LM35

  // Actuator logic
  bool pumpOn   = soilPct  < SOIL_LOW;
  bool lampOn   = lightPct < LIGHT_LOW;
  bool heaterOn = tempC   < TEMP_LOW;

  digitalWrite(PUMP_PIN,   pumpOn   ? HIGH : LOW);
  digitalWrite(LAMP_PIN,   lampOn   ? HIGH : LOW);
  digitalWrite(HEATER_PIN, heaterOn ? HIGH : LOW);

  // LCD update
  lcd.setCursor(0, 0);
  lcd.print("H:"); lcd.print(humPct);   lcd.print("%  ");
  lcd.print("S:"); lcd.print(soilPct);  lcd.print("% ");

  lcd.setCursor(0, 1);
  lcd.print("L:"); lcd.print(lightPct); lcd.print("%  ");
  lcd.print("T:"); lcd.print(tempC,1); lcd.write(223); lcd.print("C ");

  lcd.setCursor(0, 2);
  lcd.print(pumpOn  ? "PUMP:ON " : "PUMP:OFF ");
  lcd.print(lampOn  ? "LAMP:ON " : "LAMP:OFF ");

  lcd.setCursor(0, 3);
  lcd.print(heaterOn? "HEAT:ON " : "HEAT:OFF ");
  lcd.print("      "); // clear rest of line

  // Decorated Serial output every SERIAL_INTERVAL
  unsigned long now = millis();
  if (now - lastSerialTime >= SERIAL_INTERVAL) {
    lastSerialTime = now;

    // Header
    Serial.println(F("================================================"));
    Serial.print   (F(">> TRANSMISSION @ "));
    Serial.print   (now / 1000);
    Serial.println(F("s since start"));
    Serial.println(F(">> SENDING SENSOR DATA TO DATABASE"));

    // Payload (JSON-like)
    Serial.println(F("{"));
    Serial.print   (F("  \"timestamp_s\": ")); Serial.print(now/1000); Serial.println(F(","));
    Serial.print   (F("  \"humidity_pct\": ")); Serial.print(humPct);   Serial.println(F(","));
    Serial.print   (F("  \"soil_pct\": "));     Serial.print(soilPct);  Serial.println(F(","));
    Serial.print   (F("  \"light_pct\": "));    Serial.print(lightPct); Serial.println(F(","));
    Serial.print   (F("  \"temperature_c\": "));Serial.print(tempC,1); Serial.println(F(""));
    Serial.println(F("}"));

    // Footer
    Serial.println(F(">> TRANSMISSION COMPLETE"));
    Serial.println(F("================================================"));
    Serial.println();
  }

  // No delay(); loop refreshes LCD continuously
}
