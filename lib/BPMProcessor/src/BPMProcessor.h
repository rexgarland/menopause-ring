#ifndef BPMPROCESSOR_H
#define BPMPROCESSOR_H

#include "HRFilter.h"

#define UPDATE_RATE 0.3

class BPMProcessor {
  private:
    int sampleRate, interval;
    long sampleCount, searchWindow, lastBeat, beat, nextBeat;
    HRFilter hrFilt;
    float prior, posterior, minPosterior;
    float quadSF;
    enum ProcessorState { bpm_first_beat, bpm_processing, bpm_update } state;
    void setPrior(void);
    void updateBPM(int);
    void updateSearchWindow(void);
  public:
    BPMProcessor(int);
    void start(void);
    void update(float);
    char available(void);
    float getBPM(void);
    void next(void);
    float x, y, minY;

    // test methods
    friend long test_sampleCount(BPMProcessor bpm);
    friend float test_prior(BPMProcessor bpm);
    friend float test_posterior(BPMProcessor bpm);
    friend int test_state(BPMProcessor bpm);
    friend long test_beat(BPMProcessor bpm);
    friend long test_lastBeat(BPMProcessor bpm);
    friend long test_nextBeat(BPMProcessor bpm);
};

#endif