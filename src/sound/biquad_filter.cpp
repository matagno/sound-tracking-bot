#include "biquad_filter.hpp"

BiquadFilter::BiquadFilter()
    : a0(0), a1(0), a2(0), b1(0), b2(0), x1(0), x2(0), y1(0), y2(0) {}

// Config bandpass filter
void BiquadFilter::setup_bandpass(float f1, float f2, float fs) {
    float f0 = sqrt(f1 * f2);       // Centre frequency
    float Q  = sqrt(f2 / f1);       // Quality factor
    float w0 = 2.0f * M_PI * f0 / fs;
    float alpha = sin(w0) / (2.0f * Q);
    float cosw0 = cos(w0);

    float b0 =  alpha;
    float b1 =  0.0f;
    float b2 = -alpha;
    float a0 =  1.0f + alpha;
    float a1 = -2.0f * cosw0;
    float a2 =  1.0f - alpha;

    // Normalisation
    this->a0 = b0 / a0;
    this->a1 = b1 / a0;
    this->a2 = b2 / a0;
    this->b1 = a1 / a0;
    this->b2 = a2 / a0;

    x1 = x2 = y1 = y2 = 0.0f;
}

// Process sample
float BiquadFilter::process(float x0) {
    float y0 = a0*x0 + a1*x1 + a2*x2 - b1*y1 - b2*y2;
    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;
    return y0;
}

