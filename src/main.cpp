#include "potentiometer.h"

const int TOUCH_PIN = T0;
Potentiometer pot(15, 0.2);

constexpr int HISTORY_SIZE = 500;
float valueHistory[HISTORY_SIZE] = {0};
unsigned long timeHistory[HISTORY_SIZE] = {0};
int historyIndex = 0;

void setup() {
    Serial.begin(115200);
    pot.configureADC();
    TouchControl::begin(TOUCH_PIN);
}

void loop() {
    static unsigned long lastPrint = 0;
    constexpr unsigned long printInterval = 50;
    const int rawValue = pot.raw();
    const unsigned long currentMillis = millis();

    if(TouchControl::checkRelease()) {
        Serial.printf("\n[Mode] %s\n", TouchControl::getAlgorithmName());
    }

    switch(TouchControl::currentAlgorithm()) {
        case TouchControl::EXPONENTIAL:
            pot.resizeBuffer(1);
            pot.updateExponential(rawValue);
            break;
        case TouchControl::MOVING_AVG:
            pot.resizeBuffer(5);
            pot.updateMovingAvg(rawValue);
            break;
        case TouchControl::MEDIAN_5:
            pot.resizeBuffer(5);
            pot.updateMedian(rawValue);
            break;
        case TouchControl::MEDIAN_9:
            pot.resizeBuffer(9);
            pot.updateMedian(rawValue);
            break;
        case TouchControl::MEDIAN_13:
            pot.resizeBuffer(13);
            pot.updateMedian(rawValue);
            break;
        case TouchControl::KALMAN_LITE:
            pot.resizeBuffer(1);
            pot.updateKalman(rawValue);
            break;
        case TouchControl::COMBINED:
            pot.resizeBuffer(5);
            pot.updateCombined(rawValue);
            break;
    }

    valueHistory[historyIndex] = pot.read();
    timeHistory[historyIndex] = currentMillis;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;

    if(currentMillis - lastPrint >= printInterval) {
        float maxVal = -INFINITY, minVal = INFINITY;
        for(int i = 0; i < HISTORY_SIZE; i++) {
            if(currentMillis - timeHistory[i] <= 5000) {
                maxVal = fmax(maxVal, valueHistory[i]);
                minVal = fmin(minVal, valueHistory[i]);
            }
        }
        
        Serial.printf("%s: %.1f | Raw: %.1f | 5sΔ: %.1f | R-SΔ: %.1f\n", 
                     TouchControl::getAlgorithmName(),
                     pot.read(), 
                     rawValue,
                     maxVal - minVal,
                     rawValue - pot.read());
        lastPrint = currentMillis;
    }
    
    delay(10);
}
