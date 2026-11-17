// ==========================
// PIN DEFINITIONS
// ==========================
const int PIN_PWM  = 9;
const int PIN_VOLT = A0;
const int PIN_CURR = A1;

// ==========================
// HARDWARE CONSTANTS
// ==========================
// Voltage divider: 10k + 4k
const float R1 = 10000.0;
const float R2 = 4000.0;

// ACS712-5A
const float ACS_OFFSET = 1.5;     // calibrate if needed
const float ACS_SENS   = 0.185;   // V/A

// ==========================
// CHARGER LIMITS
// ==========================
const float CC_SET = 1.0;         // 1 A charge current
const float CV_SET = 12.6;        // 12 V battery full voltage

// PWM safety limit (buck: 12.6 / 24 ≈ 0.52)
const int PWM_MAX = 140;

// ==========================
// PI CONTROLLER GAINS
// ==========================
float Kp = 0.3;
float Ki = 0.05;

float integral = 0;

// ==========================
// VARIABLES
// ==========================
float Vbat = 0;
float Ibat = 0;
int pwmValue = 0;

// ==========================
// SETUP
// ==========================
void setup() {
  pinMode(PIN_PWM, OUTPUT);
  Serial.begin(9600);
}

// ==========================
// LOOP
// ==========================
void loop() {

  // ---------------------------------
  // 1. READ BATTERY VOLTAGE
  // ---------------------------------
  float Vadc = analogRead(PIN_VOLT) * (5.0 / 1023.0);
  Vbat = Vadc * ((R1 + R2) / R2);

  // ---------------------------------
  // 2. READ BATTERY CURRENT
  // ---------------------------------
  float Vcur = analogRead(PIN_CURR) * (5.0 / 1023.0);
  Ibat = (Vcur - ACS_OFFSET) / ACS_SENS;
  if (Ibat < 0) Ibat = 0;

  // ---------------------------------
  // 3. CC → CV MODE SELECTION
  // ---------------------------------
  float error;
  String mode;

  if (Vbat < (0.97 * CV_SET)) {
    // ===== CONSTANT CURRENT MODE =====
    error = CC_SET - Ibat;
    mode = "CC";
  } else {
    // ===== CONSTANT VOLTAGE MODE =====
    error = CV_SET - Vbat;
    mode = "CV";
  }

  // ---------------------------------
  // 4. PI CONTROLLER
  // ---------------------------------
  integral += error;
  float control = Kp * error + Ki * integral;

  pwmValue += control;
  pwmValue = constrain(pwmValue, 0, PWM_MAX);

  analogWrite(PIN_PWM, pwmValue);

  // ---------------------------------
  // 5. SERIAL MONITOR
  // ---------------------------------
  Serial.print("V=");
  Serial.print(Vbat, 2);
  Serial.print(" V  I=");
  Serial.print(Ibat, 2);
  Serial.print(" A  Mode=");
  Serial.print(mode);
  Serial.print("  PWM=");
  Serial.println(pwmValue);

  delay(10);
}
