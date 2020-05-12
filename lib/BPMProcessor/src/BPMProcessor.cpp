#include "BPMProcessor.h"
#include "Arduino.h"

#define DELTA_HIGH  700
#define DELTA_LOW   180

BPMProcessor::BPMProcessor(int rate) {
  sampleRate = rate;
  quadSF = ((float)(sampleRate*sampleRate))/16.0f;
  start();
}

void BPMProcessor::start(void) {
  sampleCount = 0UL;
  state = bpm_first_beat;
  beat = 0UL;
  lastBeat = 0UL;
  prior = 1;
  posterior = 0;
  minPosterior = 0; // something large
  minY = 0;
  searchWindow = 200UL;
  interval = 307;
  nextBeat = lastBeat + ((unsigned long) 24000.0/getBPM());
}

void BPMProcessor::setPrior(void) {
  if (state==bpm_first_beat) {
    prior = 1.0f;
  } else {
    // this function produces a hump with width 0.5s around the optimal point
    float toNext = sampleCount-nextBeat;
    float halfNextBeat = lastBeat + (12000.0/getBPM());
    float toHalfNext = sampleCount-halfNextBeat;
    // float quad = 2.0-(toNext*toNext)/quadSF;
    // prior = (quad>1.0) ? quad : 1.0f;
    prior = 0.4*exp(-toNext*toNext/(2000))+0.6*exp(-toHalfNext*toHalfNext/(1000))+0.5;
  }
}

void BPMProcessor::updateBPM(int intervalNew) {
  interval = UPDATE_RATE*intervalNew + (1-UPDATE_RATE)*interval;
}

void BPMProcessor::updateSearchWindow(void) {
  int delta = (int) (38000.0/getBPM()); // add buffer for variation
  if (delta<DELTA_LOW) {
    delta = DELTA_LOW;
  } else if (delta>DELTA_HIGH) {
    delta = DELTA_HIGH;
  }
  searchWindow = lastBeat + delta;
}

void BPMProcessor::update(float val) {
  x = val;
  y = hrFilt.iter(val);
  sampleCount++;
  // update beat-related fields, setup for next beat
  if (sampleCount>searchWindow) {
    if (!(state==bpm_first_beat)) {
      // update BPM with the new delta
      int intervalNew = beat-lastBeat;
      updateBPM(intervalNew);
    }
    state = bpm_update;
    lastBeat = beat;
    beat = lastBeat + DELTA_LOW;
    nextBeat = lastBeat + ((unsigned long) 24000.0/getBPM());
    updateSearchWindow();
    minPosterior = 0;
  } else { // search for beat
    setPrior();
    posterior = prior*y;
    if ((posterior<minPosterior) & (sampleCount>beat)) {
      minPosterior = posterior;
      minY = y;
      beat = sampleCount;
    }
  }
}

float BPMProcessor::getBPM() {
  return 24000.0/interval;
}

char BPMProcessor::available(void) {
  return (state==bpm_update);
}

void BPMProcessor::next(void) {
  // move to the next beat after reading BPM
  state = bpm_processing;
}


// -------------- test functions ---------------
long test_sampleCount(BPMProcessor bpm) {
    return bpm.sampleCount;
}

float test_prior(BPMProcessor bpm) {
    return bpm.prior;
}

float test_posterior(BPMProcessor bpm) {
    return bpm.posterior;
}

int test_state(BPMProcessor bpm) {
    return bpm.state;
}

long test_beat(BPMProcessor bpm) {
    return bpm.beat;
}

long test_lastBeat(BPMProcessor bpm) {
    return bpm.lastBeat;
}

long test_nextBeat(BPMProcessor bpm) {
    return bpm.nextBeat;
}