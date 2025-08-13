#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 22 // neopixelの数
#define DATA_PIN 6 // arduinoのd6ピン
#define BRIGHTNESS 100 // LEDの最大輝度(0-255)

CRGB leds[NUM_LEDS];

enum LedState {
    COMM_LOST,      // 1.通信遮断 : 赤点滅
    NORMAL,         // 2.通常(通信OK) : 白グラデーション
    AUTO,           // 3.自動 : 黄色
    SEMI_AUTO,      // 4.半自動(センサ・システム動作中) : 黄色点滅
    HIGH_SPEED,     // 5.高速モード : 赤色
    LOW_SPEED,      // 6.低速モード : 青色
};

LedState currentState = NORMAL;

unsigned long previousMillis = 0;
bool blinkState = false;

/**
 * @brief 赤点滅(500ms間隔)
 */
void handleCommLost() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500) {
        previousMillis = currentMillis;
        blinkState = !blinkState;
    }
    
    CRGB color = blinkState ? CRGB::Red : CRGB::Black;
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
}

/**
 * @brief 白グラデーション
 */
void handleNormal() {
    // 白のウェーブ
    const float WAVE_SPEED = 240.0;     // 波の移動速度（数値を大きくすると遅くなる）
    const float WAVE_LENGTH = 5.0;    // 波長（数値を大きくすると山の間隔が広がる）
    const float WAVE_SHARPNESS = 6.0;  // 波の鋭さ（数値を大きくすると山が鋭くなり、谷が広がる）

    for (int i = 0; i < NUM_LEDS; i++) {
        // 1. 時間とLEDの位置から、sin関数の角度を計算
        float angle = (millis() / WAVE_SPEED) + (i / WAVE_LENGTH);

        // 2. sin関数の結果 (-1.0 ～ 1.0) を、0.0 ～ 1.0 の範囲に変換
        float sin_0_to_1 = (sin(angle) + 1.0) / 2.0;

        // 3.【重要】値を累乗して、波の山の部分を鋭く、谷の部分を広くする
        float wave_factor = pow(sin_0_to_1, WAVE_SHARPNESS);

        // 4. 計算した係数を、実際の明るさ (0 ～ 255) に変換
        uint8_t brightness = wave_factor * 255;
        
        // 5. i番目のLEDに色を設定
        leds[i] = CRGB(brightness, brightness, brightness);
    }

    // 全てのLEDの色情報を一度に更新
    FastLED.show();

    // 以下は、ただsin関数を使った白のグラデーション
    // uint8_t breath = (sin(millis() / 2000.0 * PI) + 1.0) / 2.0 * 255;
    // fill_solid(leds, NUM_LEDS, CRGB(breath, breath, breath));
    // FastLED.show();
}

/**
 * @brief 黄色点灯
 */
void handleAuto() {
    fill_solid(leds, NUM_LEDS, CRGB::Yellow);
    FastLED.show();
}

/**
 * @brief 黄色点滅(500ms間隔)
 */
void handleSemiAuto() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500) {
        previousMillis = currentMillis;
        blinkState = !blinkState;
    }

    CRGB color = blinkState ? CRGB::Yellow : CRGB::Black;
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
}

/**
 * @brief 赤色点灯
 */
void handleHighSpeed() {
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
}

/**
 * @brief 青色点灯
 */
void handleLowSpeed() {
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();
}

void checkSerialInput() {
    if (Serial.available() > 0) {
        char command = Serial.read();
        LedState previousState = currentState;

        switch (command) {
            case '1': currentState = COMM_LOST;   break;
            case '2': currentState = NORMAL;      break;
            case '3': currentState = AUTO;        break;
            case '4': currentState = SEMI_AUTO;   break;
            case '5': currentState = HIGH_SPEED;  break;
            case '6': currentState = LOW_SPEED;   break;
            default: return;
        }

        // 状態が変わったらメッセージを表示
        if (previousState != currentState) {
            Serial.print("State changed to: ");
            Serial.println(command);
            previousMillis = millis();
            blinkState = true;
        }
    }
}

void setup() {
    Serial.begin(9600);
    delay(2000); // 起動待機

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    Serial.println("Hello, world");
}

void loop() {
    checkSerialInput();

    switch (currentState) {
        case COMM_LOST:     handleCommLost();     break;
        case NORMAL:        handleNormal();       break;
        case AUTO:          handleAuto();         break;
        case SEMI_AUTO:     handleSemiAuto();     break;
        case HIGH_SPEED:    handleHighSpeed();    break;
        case LOW_SPEED:     handleLowSpeed();     break;
    }
}