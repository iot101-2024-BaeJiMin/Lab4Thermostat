#include <Arduino.h>
#include <TFT_eSPI.h>  // TFT 디스플레이 라이브러리
#include <DHTesp.h>    // DHT 센서 라이브러리

// 핀 설정
const int DHT_PIN = 15;   // DHT 센서 핀
const int RELAY_PIN = 27; // 릴레이 핀
const int pulseA = 12;    // Rotary Encoder A핀
const int pulseB = 13;    // Rotary Encoder B핀
const int pushSW = 2;     // Rotary Encoder 스위치 핀

// TFT 디스플레이 및 DHT 객체 생성
TFT_eSPI tft = TFT_eSPI();
DHTesp dht;

// Rotary Encoder 값 관련 변수
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
int setPoint = 0;  // Rotary Encoder로 설정한 값 (0 ~ 60)

// 인터럽트 함수 - Rotary Encoder 값 읽기
IRAM_ATTR void handleRotary() {
  int MSB = digitalRead(pulseA); // MSB
  int LSB = digitalRead(pulseB); // LSB
  int encoded = (MSB << 1) | LSB; // 2비트 값을 단일 숫자로 변환
  int sum = (lastEncoded << 2) | encoded; // 이전 값을 더해줌

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;

  if (encoderValue > 60) encoderValue = 60; // 최대 값 60
  if (encoderValue < 0) encoderValue = 0;   // 최소 값 0
  lastEncoded = encoded;  // 현재 인코딩 값을 저장
}

void setup() {
  Serial.begin(115200);

  // TFT 초기화
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);

  // DHT 센서 설정
  dht.setup(DHT_PIN, DHTesp::DHT22);

  // 릴레이 핀 설정
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // 릴레이 OFF

  // Rotary Encoder 핀 설정 및 인터럽트 연결
  pinMode(pulseA, INPUT_PULLUP);
  pinMode(pulseB, INPUT_PULLUP);
  attachInterrupt(pulseA, handleRotary, CHANGE);
  attachInterrupt(pulseB, handleRotary, CHANGE);
}

void loop() {
  // DHT22 온도 읽기
  float temperature = dht.getTemperature();

  // TFT에 온도 및 설정 값 표시
  tft.fillScreen(TFT_BLACK); // 화면 지우기
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE);
  tft.printf("Temperature: %.1fC\n", temperature);
  tft.printf("Set Point: %ldC\n", encoderValue);

  // 온도가 설정 값보다 낮으면 릴레이 ON
  if (temperature < encoderValue) {
    digitalWrite(RELAY_PIN, HIGH); // 릴레이 ON
    tft.setTextColor(TFT_RED);
    tft.println("Relay: ON");
  } else {
    digitalWrite(RELAY_PIN, LOW);  // 릴레이 OFF
    tft.setTextColor(TFT_GREEN);
    tft.println("Relay: OFF");
  }

  delay(1000); // 1초마다 업데이트
}
