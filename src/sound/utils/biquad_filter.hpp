#pragma once
#include <cmath>

class BiquadFilter {
public:
    BiquadFilter();
    void setup_bandpass(float f1, float f2, float fs);
    float process(float x0);

private:
    // Attributes
    float a0, a1, a2, b1, b2;
    float x1, x2;
    float y1, y2;
};
