#include <Arduino.h>

class Potentiometer {
    int _pin;
    float _currentValue;
    float _alpha;
    int _bufferIndex = 0;
    float* _valueBuffer;
    int _windowSize = 5;

public:
    Potentiometer(int pin, float alpha);
    ~Potentiometer();
    
    float read() const;
    float raw() const;
    void configureADC();
    void resizeBuffer(int newSize);
    
    void updateExponential(int raw);
    void updateMovingAvg(int raw);
    void updateMedian(int raw);
    void updateKalman(int raw);
    void updateCombined(int raw);
};

namespace TouchControl {
    enum Algorithm {
        EXPONENTIAL,
        MOVING_AVG,
        MEDIAN_5,
        MEDIAN_9,
        MEDIAN_13,
        KALMAN_LITE,
        COMBINED,
        ALGO_COUNT
    };

    void begin(int touchPin);
    bool checkRelease();
    const char* getAlgorithmName();
    Algorithm currentAlgorithm();
}
