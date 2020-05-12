#include <unity.h>
#include <Arduino.h>
#include "SdSkinny.h"
// #include "SdFat.h"
// #include <SPI.h>

#define CHIPSELECT SS 

SdSkinny sd;
// SdFat sd;

void test_sd_begin(void) {
    bool result = sd.begin(CHIPSELECT, SD_SCK_MHZ(50));
    // Serial.println(sd.errorCode);
    TEST_ASSERT_EQUAL(true,result);
}

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);
    Serial.begin(9600);

    UNITY_BEGIN();
    RUN_TEST(test_sd_begin);
    UNITY_END();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}