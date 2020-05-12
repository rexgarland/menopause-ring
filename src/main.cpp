#ifndef UNIT_TEST

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
#include "PetitSerial.h"
#endif

#define SWITCH PIND7

#define SAMPLERATE 400

enum State { idle, processing, beat, display };
State state = idle; // program state


// Temperature
#ifndef NOTEMP
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
#endif


#ifndef NOEPD

// EPD

#define COLORED     0
#define UNCOLORED   1

unsigned char image[512];
Paint paint(image, 0, 0);    // width should be the multiple of 8 
Epd epd;
#define offset_x1 200-32
#define offset_x2 200-16
#define offset_y 200-64

void drawDisplay(float hr) {
  // update display string
  String s = String((int) hr);
  char hrStr[3];
  s.toCharArray(hrStr, 3);
  // paint digits onto display frame
  epd.SetFrameMemory(IMAGE_DATA);
  paint.Clear(UNCOLORED);
  paint.DrawNumAt(3,10,hrStr[0],&Font20Num,COLORED);
  epd.SetFrameMemory(paint.GetImage(), offset_x1, offset_y, paint.GetWidth(), paint.GetHeight());
  paint.Clear(UNCOLORED);
  paint.DrawNumAt(3,10,hrStr[1],&Font20Num,COLORED);
  epd.SetFrameMemory(paint.GetImage(), offset_x2, offset_y, paint.GetWidth(), paint.GetHeight());
  // push to display
  epd.DisplayFrame();
}

#endif

// Pulse sensor declarations

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
  int sampleRate = SAMPLERATE; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
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
#endif

// Switch declarations

int onSwitch() {
  return !digitalRead(SWITCH);
}

#ifndef NOHR
BPMProcessor bpm(SAMPLERATE);
int beatCount = 0;
#endif

void errorHalt(const char * error) {
  Serial.println(error);
  while(1);
}

#ifndef NOSD
// SD Card
FATFS fs;
const char * path = "TEST.TXT";

void startSD() {
  // initialize file system
  if (pf_mount(&fs)) errorHalt("pf_mount");
  if (pf_open("TEST.TXT")) errorHalt("pf_open");
  pf_lseek(0);
  // if (sd.begin(chipSelect, SD_SCK_MHZ(50))) {
  //   #ifdef DEBUG
  //   Serial.println("# SD initialization successful.");
  //   #endif
  // } else {
  //   #ifdef DEBUG
  //   Serial.println("# SD initialization failed.");
  //   #endif
  //   while (1);
  // }
}

void openFile() {
  // open file
  // file = sd.open(path, FILE_WRITE);
  // if (file) {
  //   #ifdef DEBUG
  //   Serial.println("# File open successful.");
  //   #endif
  //   #ifdef NOTEMP
  //   file.println("Time (us),Index,HR Signal,Latest Beat,Heart Rate (bpm)");
  //   #else
  //   file.println("Time (us),Index,HR Signal,Temperature (C),Latest Beat,Heart Rate (bpm)");
  //   #endif
  // } else {
  //   #ifdef DEBUG
  //   Serial.println("# File open failed.");
  //   #endif
  //   file.close();
  //   while (1);
  // }
  pf_open("TEST.TXT");
}

void writeData() {
  // file.print(micros());
  // file.print(",");
  // #ifndef NOHR
  // file.print(test_sampleCount(bpm));
  // file.print(",");
  // file.print(bpm.y);
  // if (bpm.available()) {
  //   #ifndef NOTEMP
  //   file.print(",");
  //   file.print(tempsensor.readTempC());
  //   #endif
  //   file.print(",");
  //   file.print(test_lastBeat(bpm));
  //   file.print(",");
  //   file.print(bpm.getBPM());
  // }
  // #else
  // #ifndef NOTEMP
  // file.print(",");
  // file.print(tempsensor.readTempC());
  // #endif
  // #endif
  // file.println();
  UINT nr;
  char * buff[100];
sdfsldkjf
  pf_write(buf,10,&nr);
  pf_write("\n",1,&nr);
}

void closeFile() {
  // close file
  file.close();
  #ifdef DEBUG
  Serial.println("# File closed.");
  #endif
}

#endif

#ifdef DEBUG
void printOut() {
  #ifndef NOHR
  Serial.print(test_sampleCount(bpm));
  Serial.print(",");
  Serial.print(bpm.x);
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
    bpm.next();
  }
  #else
  Serial.print(tempsensor.readTempC());
  delay(2);
  #endif
  Serial.println();
}
#endif

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
  paint.SetWidth(16);
  paint.SetHeight(32);
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
      }
      if (bpm.available()) {
        bpm.next();
        state = beat;
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
      #ifndef NOHR
      if (++beatCount>40) {
        #ifdef DEBUG
        Serial.println("# 40 beats counted...");
        #endif
        beatCount = 0;
        state = display;
      }
      #endif
      break;
    }
    case display: {
      #ifndef NOEPD
      #ifdef DEBUG
      Serial.print("Displaying... ");
      #endif
      drawDisplay(bpm.getBPM());
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