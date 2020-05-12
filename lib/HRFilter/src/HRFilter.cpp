#include "HRFilter.h"

BiQuad::BiQuad(double c0, double c1, double c2, double d1, double d2) {
   b0 = c0;
   b1 = c1;
   b2 = c2;
   a1 = d1;
   a2 = d2;
}

void BiQuad::start(double x0) {
   w[0] = x0/(1+a1+a2);
   w[1] = w[0];
   w[2] = w[1];
   fstate = started;
}

double BiQuad::iter(double x) {
   if (!(fstate==started)) {
      start(x);
   }
   w[0] = x - a1*w[1] - a2*w[2];
   double y = b0*w[0] + b1*w[1] + b2*w[2];
   w[2] = w[1];
   w[1] = w[0];
   return y;
}

void BiQuad::restart(void) {
   fstate = initialized;
}

// ----------- HRFilter -------------

HRFilter::HRFilter(void) {
   restart();
}

void HRFilter::start(void) {
   f1.restart();
   f2.restart();
   f3.restart();
   f4.restart();
   f5.restart();
   hrstate = started;
}

void HRFilter::restart(void) {
   hrstate = initialized;
}

double HRFilter::iter(double val) {
   if (!(hrstate==started)) {
      start();
   }
   return f5.iter(f4.iter(f3.iter(f2.iter(f1.iter(val)))));
}