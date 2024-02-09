/**
 * @file      GPS_BuiltIn.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-06-28
 * @note      SIM7670G does not have a built-in GPS module, regardless of whether there is a GPS antenna holder on the board.
 */
#include "utilities.h"

#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial


// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS


#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

void setup()
{
    Serial.begin(115200);
    // Turn on DC boost to power on the modem
#ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
#endif

    // Set modem reset pin ,reset modem
    pinMode(MODEM_RESET_PIN, OUTPUT);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL); delay(100);
    digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL); delay(2600);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);

    // Turn on modem
    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_PWRKEY_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);

    // Set modem baud
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    Serial.println("Start modem...");
    delay(3000);

    int retry = 0;
    while (!modem.testAT(1000)) {
        Serial.println(".");
        if (retry++ > 10) {
            digitalWrite(BOARD_PWRKEY_PIN, LOW);
            delay(100);
            digitalWrite(BOARD_PWRKEY_PIN, HIGH);
            delay(1000);
            digitalWrite(BOARD_PWRKEY_PIN, LOW);
            retry = 0;
        }
    }
    Serial.println();
    delay(200);

    Serial.println("Enabling GPS/GNSS/GLONASS");
    while (!modem.enableGPS(MODEM_GPS_ENABLE_GPIO)) {
        Serial.print(".");
    }
    Serial.println();
    Serial.println("GPS Enabled");
}

void loop()
{
    float lat2      = 0;
    float lon2      = 0;
    float speed2    = 0;
    float alt2      = 0;
    int   vsat2     = 0;
    int   usat2     = 0;
    float accuracy2 = 0;
    int   year2     = 0;
    int   month2    = 0;
    int   day2      = 0;
    int   hour2     = 0;
    int   min2      = 0;
    int   sec2      = 0;
    uint8_t    fixMode   = 0;
    for (int8_t i = 15; i; i--) {
        Serial.println("Requesting current GPS/GNSS/GLONASS location");
        if (modem.getGPS(&fixMode, &lat2, &lon2, &speed2, &alt2, &vsat2, &usat2, &accuracy2,
                         &year2, &month2, &day2, &hour2, &min2, &sec2)) {

            Serial.print("FixMode:"); Serial.println(fixMode);
            Serial.print("Latitude:"); Serial.print(lat2, 6); Serial.print("\tLongitude:"); Serial.println(lon2, 6);
            Serial.print("Speed:"); Serial.print(speed2); Serial.print("\tAltitude:"); Serial.println(alt2);
            Serial.print("Visible Satellites:"); Serial.print(vsat2); Serial.print("\tUsed Satellites:"); Serial.println(usat2);
            Serial.print("Accuracy:"); Serial.println(accuracy2);

            Serial.print("Year:"); Serial.print(year2);
            Serial.print("\tMonth:"); Serial.print(month2);
            Serial.print("\tDay:"); Serial.println(day2);

            Serial.print("Hour:"); Serial.print(hour2);
            Serial.print("\tMinute:"); Serial.print(min2);
            Serial.print("\tSecond:"); Serial.println(sec2);
            break;
        } else {
            Serial.println("Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
            delay(15000L);
        }
    }
    Serial.println("Retrieving GPS/GNSS/GLONASS location again as a string");
    String gps_raw = modem.getGPSraw();
    Serial.print("GPS/GNSS Based Location String:");
    Serial.println(gps_raw);
    Serial.println("Disabling GPS");

    modem.disableGPS();

    while (1) {
        delay(10);
    }
}

