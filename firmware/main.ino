/**
 * Smart Medicine Reminder & Dispenser
 * ESP32 Production Firmware
 * Author: Joel C. Abraham
 */

#include <WiFi.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include "config.h"

// Hardware Drivers
Servo carouselServo;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Scheduled Medicines Array (6 Compartments)
MedicineDose schedule[TOTAL_COMPARTMENTS] = {
    {1, "Aspirin (8AM)",     0, 8,  0, false, false},
    {2, "Vitamin C (1PM)",   1, 13, 0, false, false},
    {3, "Metformin (8PM)",   2, 20, 0, false, false},
    {4, "Calcium (8AM)",     3, 8,  0, false, false},
    {5, "BP Med (8PM)",      4, 20, 0, false, false},
    {6, "Multi-Vit (1PM)",   5, 13, 0, false, false}
};

int currentAngle = 0;
bool isAlertActive = false;
int activeDoseIndex = -1;

void setup() {
    Serial.begin(115200);
    Serial.println("\n[+] ESP32 Smart Medicine Dispenser Booting...");

    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_LED_STATUS, OUTPUT);
    pinMode(PIN_IR_SENSOR, INPUT);
    pinMode(PIN_BUTTON_ACK, INPUT_PULLUP);

    carouselServo.attach(PIN_SERVO_CAROUSEL);
    carouselServo.write(0);

    // Initialize OLED
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("[-] OLED Display Allocation Failed!");
    } else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,10);
        display.println("Smart Med Dispenser");
        display.println("Connecting WiFi...");
        display.display();
    }

    // Connect WiFi & Sync Time
    connectWiFi();
    configTime(19800, 0, "pool.ntp.org"); // IST GMT+5:30
}

void connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[+] WiFi Connected! IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\n[-] Offline Mode Active (Standalone RTC)");
    }
}

void loop() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("[-] Failed to obtain NTP time");
        delay(1000);
        return;
    }

    int currHour = timeinfo.tm_hour;
    int currMin  = timeinfo.tm_min;

    // Check Schedule
    for(int i = 0; i < TOTAL_COMPARTMENTS; i++) {
        if(schedule[i].hour == currHour && schedule[i].minute == currMin && !schedule[i].isDispensed) {
            triggerReminder(i);
        }
    }

    // Check Manual ACK Button
    if(isAlertActive && digitalRead(PIN_BUTTON_ACK) == LOW) {
        Serial.println("[+] User Acknowledged Reminder!");
        dispenseDose(activeDoseIndex);
    }

    updateDisplay(timeinfo);
    delay(500);
}

void triggerReminder(int index) {
    isAlertActive = true;
    activeDoseIndex = index;

    Serial.printf("[!] ALERT: Time for %s!\n", schedule[index].name);
    
    // Audible Buzzer Pulse
    for(int b = 0; b < 3; b++) {
        digitalWrite(PIN_BUZZER, HIGH);
        digitalWrite(PIN_LED_STATUS, HIGH);
        delay(200);
        digitalWrite(PIN_BUZZER, LOW);
        digitalWrite(PIN_LED_STATUS, LOW);
        delay(200);
    }
}

void dispenseDose(int index) {
    if(index < 0 || index >= TOTAL_COMPARTMENTS) return;

    int targetAngle = index * DEGREES_PER_SLOT;
    Serial.printf("[+] Rotating Carousel to %d degrees (Compartment %d)...\n", targetAngle, index);

    // Actuate Servo
    carouselServo.write(targetAngle);
    delay(1000);

    // IR Sensor Verification
    int sensorVal = digitalRead(PIN_IR_SENSOR);
    if(sensorVal == LOW) { // IR Beam Blocked = Dose Released
        Serial.println("[+] Sensor Confirmed: Pill Dispensed Successfully!");
        schedule[index].isDispensed = true;
        schedule[index].isAcknowledged = true;
    } else {
        Serial.println("[!] WARNING: IR Sensor did not detect pill release!");
        schedule[index].isDispensed = true; // Mark handled to prevent jam loops
    }

    isAlertActive = false;
    activeDoseIndex = -1;
    digitalWrite(PIN_BUZZER, LOW);
}

void updateDisplay(struct tm timeinfo) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf("TIME: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    display.println("---------------------");

    if(isAlertActive && activeDoseIndex != -1) {
        display.println(">> DOSE DUE! <<");
        display.println(schedule[activeDoseIndex].name);
        display.println("Press Button to Take!");
    } else {
        display.println("Status: READY");
        display.println("Next Dose: 8:00 PM");
        display.println("WiFi: Connected");
    }
    display.display();
}
