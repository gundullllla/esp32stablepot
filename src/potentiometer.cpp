#include "potentiometer.h"
#include <algorithm>
#include <numeric> // ADD THIS LINE FOR STD::ACCUMULATE

// Potentiometer Class Implementation
Potentiometer::Potentiometer(int pin, float alpha) 
    : _pin(pin), _alpha(alpha) {
    _valueBuffer = new float[_windowSize]();
    pinMode(_pin, INPUT);
}

Potentiometer::~Potentiometer() {
    delete[] _valueBuffer;
}

void Potentiometer::configureADC() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_6db);
}

float Potentiometer::read() const { return _currentValue; }
float Potentiometer::raw() const { return analogRead(_pin); }

void Potentiometer::resizeBuffer(int newSize) {
    delete[] _valueBuffer;
    _windowSize = newSize;
    _valueBuffer = new float[_windowSize]();
    _bufferIndex = 0;
}

// Filter Implementations
void Potentiometer::updateExponential(int raw) {
    _currentValue = _alpha * raw + (1 - _alpha) * _currentValue;
}

void Potentiometer::updateMovingAvg(int raw) {
    _valueBuffer[_bufferIndex] = raw;
    _bufferIndex = (_bufferIndex + 1) % _windowSize;
    _currentValue = std::accumulate(_valueBuffer, _valueBuffer + _windowSize, 0.0f) / _windowSize;
}

void Potentiometer::updateMedian(int raw) {
    _valueBuffer[_bufferIndex] = raw;
    _bufferIndex = (_bufferIndex + 1) % _windowSize;
    float temp[_windowSize];
    std::copy(_valueBuffer, _valueBuffer + _windowSize, temp);
    std::nth_element(temp, temp + _windowSize/2, temp + _windowSize);
    _currentValue = temp[_windowSize/2];
}

void Potentiometer::updateKalman(int raw) {
    static float estimate = 0.0f;
    static float error = 1.0f;
    const float Q = 0.1f;
    const float R = 0.1f;
    
    error += Q;
    const float gain = error / (error + R);
    estimate += gain * (raw - estimate);
    error *= (1 - gain);
    
    _currentValue = estimate;
}

void Potentiometer::updateCombined(int raw) {
    updateMedian(raw);
    updateExponential(_currentValue);
    updateMovingAvg(_currentValue);
}

// TouchControl Implementation
namespace TouchControl {
    int _touchPin;
    bool _lastTouchState = false;
    Algorithm _currentAlgo = EXPONENTIAL;

    void begin(int touchPin) {
        _touchPin = touchPin;
        touchAttachInterrupt(_touchPin, nullptr, 20);
    }

    bool checkRelease() {
        const bool currentState = touchRead(_touchPin) < 20;
        if(_lastTouchState && !currentState) {
            _currentAlgo = static_cast<Algorithm>((static_cast<int>(_currentAlgo) + 1) % ALGO_COUNT);
            _lastTouchState = currentState;
            return true;
        }
        _lastTouchState = currentState;
        return false;
    }

    const char* getAlgorithmName() {
        const char* names[] = {
            "Exponential", "Moving Avg", "Median 5", 
            "Median 9", "Median 13", "Kalman", "Combined"
        };
        return names[_currentAlgo];
    }

    Algorithm currentAlgorithm() { return _currentAlgo; }
}
