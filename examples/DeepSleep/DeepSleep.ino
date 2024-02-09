/**
 * @file      DeepSleep.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-11-16
 * @record    https://youtu.be/5G4COjtKsFU
 * T-A7608-S3 Deepsleep ~ 368uA
 */
#include "utilities.h"
#include <driver/gpio.h>

#define TINY_GSM_RX_BUFFER          1024 // Set RX buffer to 1Kb

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>

#define uS_TO_S_FACTOR      1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP       30          /* Time ESP32 will go to sleep (in seconds) */


#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

void setup()
{
    Serial.begin(115200); // Set console baud rate

    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    // Turn on DC boost to power on the modem
#ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
#endif


    pinMode(MODEM_DTR_PIN, OUTPUT);
    digitalWrite(MODEM_DTR_PIN, LOW);

    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_PWRKEY_PIN, HIGH);
    //Ton >= 100 <= 500
    delay(100);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);

    // Pull up DTR to put the modem into sleep
    pinMode(MODEM_DTR_PIN, OUTPUT);
    digitalWrite(MODEM_DTR_PIN, HIGH);

    // Delay sometime ...
    delay(10000);

    Serial.println("Check modem online .");
    while (!modem.testAT()) {
        Serial.print("."); delay(500);
    }
    Serial.println("Modem is online !");

    delay(5000);


    Serial.println("Enter modem power off!");

    if (modem.poweroff()) {
        Serial.println("Modem enter power off modem!");
    } else {
        Serial.println("modem power off failed!");
    }

    delay(5000);

    Serial.println("Check modem response .");
    while (modem.testAT()) {
        Serial.print("."); delay(500);
    }
    Serial.println("Modem is not respone ,modem has power off !");

    delay(5000);

#ifdef BOARD_POWERON_PIN
    // Turn on DC boost to power off the modem
    digitalWrite(BOARD_POWERON_PIN, LOW);
#endif

    Serial.println("Enter esp32 goto deepsleep!");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    delay(200);
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
}

void loop()
{
}

