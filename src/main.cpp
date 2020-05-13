// main.cpp
//
// Runs a loop to measure heart rate and temperature for data logging and e-paper display.

#ifndef UNIT_TEST

// Use preprocessor directives to turn on and off various functions.
#ifdef USING_PLATFORMIO
#include <Arduino.h>
#endif
#include <String.h>
#ifndef NOHR
#include "MAX30105.h"
#include "BPMProcessor.h"
#endif
#ifndef NOEPD
#include "epd1in54.h"
#include "epdpaint.h"
#endif
#include "imagedata.h"
#include <Wire.h>
#ifndef NOTEMP
#include "Adafruit_MCP9808.h"
#endif
#include <SPI.h>
#ifndef NOSD
#include "PetitFS.h"
#endif

// ----------- State Variables -----------

enum State { idle, processing, beat, display };
State state = idle; // program state
struct stateData {
  float hr;
  float temp;
} sdata;

// ----------- Manual Switch -----------

// wire this pin to GND if you want to keep the main loop running
#define SWITCH PIND7

int onSwitch() {
  return !digitalRead(SWITCH);
}

// ----------- Temperature Sensor -----------

#ifndef NOTEMP
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
#endif

#ifndef NOEPD

// ----------- e-Paper Display -----------

#define COLORED     0
#define UNCOLORED   1

unsigned char image[512];
Paint paint(image, 0, 0);    // width should be the multiple of 8 
Epd epd;
#define offset_x 200-32
#define offset_y1 200-64+16
#define offset_y2 200-64

void drawDisplay() {
  int hrint = (int) sdata.hr;
  char hr10 = '0'+hrint/10;
  char hr1 = '0'+hrint%10;
  epd.SetFrameMemory(IMAGE_DATA);
  // epd.ClearFrameMemory(0xFF);
  paint.Clear(UNCOLORED);
  char hrStr[2] = {hr10,'\0'};
  paint.DrawNumStringAt(3,10,hrStr,&Font20Num,COLORED);
  epd.SetFrameMemory(paint.GetImage(), offset_x, offset_y1, paint.GetWidth(), paint.GetHeight());
  paint.Clear(UNCOLORED);
  hrStr[0] = hr1;
  paint.DrawNumStringAt(3,10,hrStr,&Font20Num,COLORED);
  epd.SetFrameMemory(paint.GetImage(), offset_x, offset_y2, paint.GetWidth(), paint.GetHeight());
  epd.DisplayFrame();
}

#endif

// ----------- Heart Rate Sensor -----------

#ifndef NOHR

MAX30105 particleSensor;
int sampCount = 0;

void startPulseSensor() {
  // initialize pulse sensor comms
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) //Use default I2C port, 400kHz speed
  {
    #ifdef DEBUG
    Serial.println("# MAX30105 was not found. Please check wiring/power. ");
    #endif
    while (1);
  }
  // initialize pulse sensor settings
  byte ledBrightness = 70; //Options: 0=Off to 255=50mA
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 400; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 69; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
  #ifdef DEBUG
  Serial.println("# MAX30105 started...");
  #endif
}

void stopPulseSensor() {
  // stop pulse sensor
  particleSensor.shutDown();
  #ifdef DEBUG
  Serial.println("# MAX30105 stopped.");
  #endif
}

#define SAMPLERATE 400
BPMProcessor bpm(SAMPLERATE);
int beatCount = 0;

#endif

// ----------- SD Card -----------

#ifndef NOSD

void errorHalt(const char * error) {
  Serial.println(error);
  while(1);
}

FATFS fs;
const char * path = "TEST.TXT";
UINT nr;

// init file system
void startSD() {
  if (pf_mount(&fs)) errorHalt("pf_mount");
  if (pf_open("TEST.TXT")) errorHalt("pf_open");
  pf_lseek(0);
}

// open file and write csv header
void openFile() {
  int success = pf_open("TEST.TXT");
   errorHalt("# File open failed.");
  #ifdef DEBUG
  if (!success) {
    errorHalt("# File open failed.");
  }
  #endif
  #ifdef NOTEMP
  writeFile("Time (us),Index,HR Signal,Latest Beat,Heart Rate (bpm)\n");
  #else
  writeFile("Time (us),Index,HR Signal,Temperature (C),Latest Beat,Heart Rate (bpm)\n");
  #endif
}

// helper functions to write to file

void writeFile(char * buff) {
  int i=0;
  char c = buff[i];
  while (c!='\0') {
    pf_write(buff+i,1,&nr);
    c = buff[++i];
  }
}

void writeFile(float f) {
  String s(f);
  unsigned char buff[30];
  s.getBytes(buff,s.length());
  int i=0;
  char c = buff[i];
  while (c!='\0') {
    pf_write(buff+i,1,&nr);
    c = buff[++i];
  }
}

void writeFile(long l) {
  String s(l);
  unsigned char buff[30];
  s.getBytes(buff,s.length());
  int i=0;
  char c = buff[i];
  while (c!='\0') {
    pf_write(buff+i,1,&nr);
    c = buff[++i];
  }
}

void writeFile(unsigned long l) {
  String s(l);
  unsigned char buff[30];
  s.getBytes(buff,s.length());
  int i=0;
  char c = buff[i];
  while (c!='\0') {
    pf_write(buff+i,1,&nr);
    c = buff[++i];
  }
}

// write one data line
void writeData() {
  writeFile(micros());
  writeFile(",");
  #ifndef NOHR
  writeFile(test_sampleCount(bpm));
  writeFile(",");
  writeFile(bpm.y);
  if (bpm.available()) {
    #ifndef NOTEMP
    writeFile(",");
    writeFile(tempsensor.readTempC());
    #endif
    writeFile(",");
    writeFile(test_lastBeat(bpm));
    writeFile(",");
    writeFile(bpm.getBPM());
  }
  #else
  #ifndef NOTEMP
  writeFile(",");
  writeFile(tempsensor.readTempC());
  #endif
  #endif
  writeFile("\n");
}

void closeFile() {
  // write out the end of the 512 byte block
  pf_write(0,0,&nr);
}

#endif

// ----------- Serial Debugging -----------

#ifdef DEBUG

void printOut() {
  #ifndef NOHR
  Serial.print(test_sampleCount(bpm));
  // Serial.print(",");
  // Serial.print(bpm.x);
  Serial.print(",");
  Serial.print(bpm.y);
  Serial.print(",");
  Serial.print(test_prior(bpm));
  Serial.print(",");
  Serial.print(test_posterior(bpm));
  if (bpm.available()) {
    Serial.print(",");
    Serial.print(test_lastBeat(bpm));
    Serial.print(",");
    Serial.print(bpm.minY);
    Serial.print(",");
    Serial.print(bpm.getBPM());
  }
  #else
  #ifndef NOTEMP
  Serial.print(tempsensor.readTempC());
  #endif
  delay(2);
  #endif
  Serial.println();
}

#endif

// ----------- State Functions -----------

void initPeriphs() {
  #ifndef NOTEMP
  if (!tempsensor.begin(0x18)) {
    #ifdef DEBUG
    Serial.println("# Couldn't find MCP9808! Check your connections and verify the address is correct.");
    #endif
    // while (1);
  }
  tempsensor.setResolution(3);
  #endif

  #ifndef NOSD
  startSD();
  #endif

  #ifndef NOEPD

  // EPD init
  if (epd.Init(lut_full_update) != 0) {
    #ifdef DEBUG
    Serial.println("# e-Paper init failed");
    #endif
    while (1);
  } else {
    #ifdef DEBUG
    Serial.println("# e-Paper init successful.");
    #endif
  }

  // Setup paint
  epd.SetFrameMemory(IMAGE_DATA);
  epd.DisplayFrame();
  // epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  // epd.DisplayFrame();
  paint.SetRotate(ROTATE_270);
  paint.SetWidth(32);
  paint.SetHeight(16);
  paint.Clear(UNCOLORED);
  
  #endif
}

void wake() {
  #ifndef NOSD
  openFile();
  #endif
  #ifndef NOHR
  startPulseSensor();
  bpm.start();
  #endif
  #ifndef NOTEMP
  tempsensor.wake();
  #endif
}

void sleep() {
  #ifndef NOSD
  closeFile();
  #endif
  #ifndef NOHR
  stopPulseSensor();
  #endif
  #ifndef NOTEMP
  tempsensor.shutdown_wake(1);
  #endif
}

void setup(void) {
  #ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);
  Serial.println("# Serial started...");
  #endif
  pinMode(SWITCH,INPUT_PULLUP);
  initPeriphs();
  #ifdef DEBUG
  Serial.println("# Starting program...");
  #endif
}

void loop(void) {
  switch (state) {
    case idle: {
      if (onSwitch()) {
        #ifdef DEBUG
        Serial.println("# Switch on...");
        #endif
        wake();
        #ifdef DEBUG
        Serial.println("# Collecting data...");
        #endif
        state = processing;
      }
      break;
    }
    case processing: {
      if (!onSwitch()) {
        #ifdef DEBUG
        Serial.println("# Switch off");
        #endif
        sleep();
        #ifdef DEBUG
        #ifndef NOHR
        Serial.print("# Data collection stopped at ");
        Serial.print(sampCount);
        Serial.println(" samples.");
        #endif
        #endif
        state = idle;
        break;
      }
      #ifndef NOHR
      particleSensor.check();
      while (particleSensor.available()) {
        bpm.update(particleSensor.getFIFOIR());
        #ifndef NOSD
        writeData();
        #endif
        #ifdef DEBUG
        printOut();
        #endif
        particleSensor.nextSample();
        if (bpm.available()) {
          bpm.next();
          state = beat;
          break;
        }
      }
      #else
      printOut();
      #endif
      break;
    }
    case beat: {
      #ifdef DEBUG
      #ifndef NOHR
      Serial.print("# BPM update: ");
      Serial.println(bpm.getBPM());
      #endif
      #endif
      #ifndef NOTEMP
      sdata.temp = 0.3*tempsensor.readTempC() + 0.7*sdata.temp; // low pass filter temp
      #endif
      #ifndef NOHR
      if (++beatCount>5) {
        #ifdef DEBUG
        Serial.println("# 5 beats counted...");
        #endif
        beatCount = 0;
        state = display;
      } else {
        state = processing;
      }
      #endif
      break;
    }
    case display: {
      #ifndef NOEPD
      #ifdef DEBUG
      Serial.print("# Displaying... ");
      #endif
      sdata.hr = bpm.getBPM();
      drawDisplay();
      #endif
      #ifdef DEBUG
      Serial.println("# Collecting data...");
      #endif
      state = processing;
      break;
    }
  }
}

#endif