#ifndef HRFILTER_H
#define HRFILTER_H

// // floating point
// class BiQuad {
//     enum FilterState { initialized, started } fstate = initialized;
//     double b0, b1, b2, a1, a2;
//     double w[3];
//     void start(double);
//     public:
//         BiQuad(double, double, double, double, double);
//         double iter(double);
//         void restart(void);
// };

// fixed point
class BiQuad {
    enum FilterState { initialized, started } fstate = initialized;
    double b0, b1, b2, a1, a2;
    double w[3];
    void start(double);
    public:
        BiQuad(double, double, double, double, double);
        double iter(double);
        void restart(void);
};

class HRFilter {
    enum HRFilterState { initialized, started } hrstate;
    BiQuad f1 = BiQuad(0.00208057, 0.00416113, 0.00208057, -1.86689228,  0.87521455);
    BiQuad f2 = BiQuad(0.92704034, -1.09317117,  0.92704034, -1.09317117,  0.85408069);
    BiQuad f3 = BiQuad(0.99375596, -0.99375596, 0, -0.98751193, 0);
    BiQuad f4 = BiQuad(0.03634612, 0.03634612, 0, -0.92730777, 0);
    BiQuad f5 = BiQuad(0.97697628, -0.97697628, 0, -0.9539525, 0);
    void start(void);
    public:
        HRFilter(void);
        double iter(double);
        void restart(void);
};

#endif