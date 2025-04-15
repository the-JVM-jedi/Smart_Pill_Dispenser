#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Hardware Configuration
const int SERVO_PIN = 9;
const int BUZZER_PIN = 8;
const int LED_PIN = 7;
const int BUTTON_PIN = 4;

// Time Configuration
const int DOSE_TIMES[][2] = {{8, 0}, {8, 15}, {8, 22}}; // 8AM, 8:15AM, 8PM
const int DOSE_WINDOW_MINUTES = 5; // 5-minute window to take pill
const unsigned long DOSE_WINDOW_MS = DOSE_WINDOW_MINUTES * 60 * 1000; // Convert to milliseconds

// System State
struct {
  int hour = 7;       // Start at 7:55 AM (for testing)
  int minute = 55;
  bool doseTaken[3] = {false, false, false};
  bool doseMissed[3] = {false, false, false};
  bool pillDispensed = false;
  bool isAlertActive = false;
  int doseStartHour = 0;
  int doseStartMinute = 0;
} SystemState;

// Hardware Objects
Servo dispenser;
LiquidCrystal_I2C lcd(0x20, 16, 2);

void setup() {
  Serial.begin(9600);
  
  // Initialize Hardware
  dispenser.attach(SERVO_PIN);
  dispenser.write(0); // Lock initially
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  updateDisplay();
}

void loop() {
  static unsigned long lastUpdate = 0;
  
  // Simulate 1 minute per second (for testing)
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    incrementTime();
    checkDoseTime();
    checkMissedDose();
    updateDisplay();
  }

  handleButtonPress();
}

//=======================
// Core Logic Functions
//=======================

void incrementTime() {
  SystemState.minute++;
  if (SystemState.minute >= 60) {
    SystemState.minute = 0;
    SystemState.hour++;
    if (SystemState.hour >= 24) {
      SystemState.hour = 0;
      resetMidnight();
    }
  }
}

void checkDoseTime() {
  for (int i = 0; i < 3; i++) {
    if (SystemState.hour == DOSE_TIMES[i][0] && 
        SystemState.minute == DOSE_TIMES[i][1] && 
        !SystemState.doseTaken[i] && 
        !SystemState.doseMissed[i]) {
      dispensePill();
      startAlerts();
      SystemState.pillDispensed = true;
      SystemState.doseStartHour = SystemState.hour;
      SystemState.doseStartMinute = SystemState.minute;
      break;
    }
  }
}

void checkMissedDose() {
  if (!SystemState.pillDispensed) return;
  
  // Calculate minutes passed since dose was dispensed
  int minutesPassed;
  if (SystemState.hour == SystemState.doseStartHour) {
    minutesPassed = SystemState.minute - SystemState.doseStartMinute;
  } else {
    minutesPassed = (60 - SystemState.doseStartMinute) + SystemState.minute;
  }
  
  if (minutesPassed >= DOSE_WINDOW_MINUTES) {
    retrievePill();
    for (int i = 0; i < 3; i++) {
      if (SystemState.hour == DOSE_TIMES[i][0]) {
        SystemState.doseMissed[i] = true;
        break;
      }
    }
    SystemState.pillDispensed = false;
  }
}

void resetMidnight() {
  for (int i = 0; i < 3; i++) {
    SystemState.doseTaken[i] = false;
    SystemState.doseMissed[i] = false;
  }
}

//=======================
// Hardware Interactions
//=======================

void dispensePill() {
  lcd.setCursor(0, 1);
  lcd.print("Dispensing...  ");
  dispenser.write(90); // Open
  delay(1000);
  dispenser.write(0);  // Close and lock
}

void retrievePill() {
  // Only retrieve if we haven't already
  if (SystemState.isAlertActive) {
    lcd.setCursor(0, 1);
    lcd.print("Retrieving...  ");
    dispenser.write(180); // Rotate opposite direction to "retrieve"
    delay(1000);
    dispenser.write(0);   // Return to locked position
    stopAlerts();
  }
}

void startAlerts() {
  tone(BUZZER_PIN, 1000);
  digitalWrite(LED_PIN, HIGH);
  SystemState.isAlertActive = true;
}

void stopAlerts() {
  noTone(BUZZER_PIN);
  digitalWrite(LED_PIN, LOW);
  SystemState.isAlertActive = false;
}

void handleButtonPress() {
  if (digitalRead(BUTTON_PIN) == LOW && SystemState.isAlertActive) {
    stopAlerts();
    markDoseTaken();
    lcd.setCursor(0, 1);
    lcd.print("Pill Taken!    ");
    delay(2000);
    SystemState.pillDispensed = false;
  }
}

void markDoseTaken() {
  for (int i = 0; i < 3; i++) {
    if (SystemState.hour == DOSE_TIMES[i][0]) {
      SystemState.doseTaken[i] = true;
      break;
    }
  }
}

//=======================
// Display Functions
//=======================

void updateDisplay() {
  // Show current time
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  if (SystemState.hour < 10) lcd.print("0");
  lcd.print(SystemState.hour);
  lcd.print(":");
  if (SystemState.minute < 10) lcd.print("0");
  lcd.print(SystemState.minute);

  // Show system status
  lcd.setCursor(0, 1);
  if (SystemState.isAlertActive) {
    lcd.print("ALERT: Take Pill!");
  } 
  else if (SystemState.doseMissed[0] || SystemState.doseMissed[1] || SystemState.doseMissed[2]) {
    lcd.print("Dose Missed!   ");
  } 
  else if (SystemState.pillDispensed) {
    lcd.print("Ready          ");
  } 
  else {
    lcd.print("Waiting        ");
  }
}