/**
 * Smart Medicine Reminder & Dispenser - Hardware Pinout & Configuration
 * Controller: ESP32 Dev Module
 */

#ifndef CONFIG_H
#define CONFIG_H

// WiFi & Cloud IoT Config
#define WIFI_SSID       "Your_WiFi_Network"
#define WIFI_PASSWORD   "Your_WiFi_Password"
#define BLYNK_AUTH_TOKEN "Your_Blynk_Auth_Token"

// Hardware Pinout Definitions
#define PIN_SERVO_CAROUSEL 18   // PWM Servo Motor output pin
#define PIN_BUZZER         19   // Audible Reminder Buzzer
#define PIN_IR_SENSOR      23   // IR Compartment / Pill Release Verification
#define PIN_BUTTON_ACK     4    // User Manual Acknowledgement Pushbutton
#define PIN_LED_STATUS     2    // Onboard Status LED

// OLED Display I2C Pins
#define OLED_SDA           21
#define OLED_SCL           22
#define SCREEN_WIDTH       128
#define SCREEN_HEIGHT      64

// Dispensing Carousel Calibration
#define TOTAL_COMPARTMENTS 6
#define DEGREES_PER_SLOT   (360 / TOTAL_COMPARTMENTS)
#define DISPENSE_SERVO_SPEED 15  // Delay per degree in ms

// Dose Events Struct
struct MedicineDose {
    int id;
    char name[32];
    int compartment;
    int hour;
    int minute;
    bool isDispensed;
    bool isAcknowledged;
};

#endif // CONFIG_H
