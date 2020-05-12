#include <unity.h>
// #include <iostream>
#include "HRFilter.h"
#include "BPMProcessor.h"
#include "test_data.h"

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

#define SAMPLERATE 400

BiQuad f(0.00208057, 0.00416113, 0.00208057, -1.86689228,  0.87521455);
HRFilter hrfilt;
BPMProcessor bpm(SAMPLERATE);

void test_function_biquad_init(void) {
    f.restart();
    float out = f.iter(10);
    TEST_ASSERT_EQUAL_FLOAT(10, out);
}

void test_function_biquad_iter(void) {
    f.restart();
    f.iter(10);
    f.iter(300);
    f.iter(230);
    float out = f.iter(90);
    TEST_ASSERT_EQUAL_FLOAT(16.36748, out);
}

void test_function_filter_iter(void) {
    hrfilt.iter(10);
    hrfilt.iter(100);
    float out = hrfilt.iter(12);
    TEST_ASSERT_EQUAL_FLOAT(0.0347432, out);
}

void test_function_processor_update(void) {
    bpm.start();
    bpm.update(10);
    TEST_ASSERT_EQUAL(1,test_sampleCount(bpm));
}

void test_function_processor_prior(void) {
    bpm.start();
    bpm.update(10);
    TEST_ASSERT_EQUAL_FLOAT(1,test_prior(bpm));
}

void test_function_processor_posterior(void) {
    bpm.start();
    for (int i=0; i<100; i++) {
        bpm.update(hr_data[i]);
    }
    TEST_ASSERT_EQUAL_FLOAT(-6776,test_posterior(bpm));
}

void test_function_processor_state0(void) {
    bpm.start();
    for (int i=0; i<10; i++) {
        bpm.update(hr_data[i]);
    }
    TEST_ASSERT_EQUAL(0,test_state(bpm));
}

void test_function_processor_state2(void) {
    bpm.start();
    for (int i=0; i<LEN; i++) {
        bpm.update(hr_data[i]);
    }
    TEST_ASSERT_EQUAL(2,test_state(bpm));
}

void test_function_processor_available(void) {
    bpm.start();
    for (int i=0; i<LEN; i++) {
        bpm.update(hr_data[i]);
    }
    TEST_ASSERT_EQUAL(1,bpm.available());
}

void test_function_processor_beat(void) {
    int beat = 0;
    bpm.start();
    for (int i=0; i<LEN; i++) {
        bpm.update(hr_data[i]);
        if (bpm.available()) {
            beat = test_beat(bpm);
            break;
        }
        // std::cout << test_state(bpm) << "," << test_lastBeat(bpm) << "," << test_beat(bpm) << "," << test_nextBeat(bpm) << "," << bpm.getBPM() << std::endl;
    }
    TEST_ASSERT_EQUAL(68,beat);
}

void process() {
    UNITY_BEGIN();
    RUN_TEST(test_function_biquad_init);
    RUN_TEST(test_function_biquad_iter);
    // RUN_TEST(test_function_filter_iter);
    RUN_TEST(test_function_processor_update);
    RUN_TEST(test_function_processor_prior);
    RUN_TEST(test_function_processor_posterior);
    RUN_TEST(test_function_processor_state0);
    RUN_TEST(test_function_processor_state2);
    RUN_TEST(test_function_processor_available);
    RUN_TEST(test_function_processor_beat);
    UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    process();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}

#else

int main( int argc, char **argv) {
    process();
    return 0;
}

#endif