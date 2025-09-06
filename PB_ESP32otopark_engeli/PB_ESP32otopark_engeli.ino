

#include <picobricks.h>
#include <Wire.h>


#define RGB_PIN 32
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define SCREEN_ADDRESS 0x3C


// Pin tanımları
#define TRIG_PIN 26     // PicoBricks G4 → sensör Trig
#define ECHO_PIN 27     // PicoBricks G5 → sensör Echo
#define RGB_COUNT 3
motorDriver motor;
long duration;
float distance;
char str[10];
NeoPixel strip(RGB_COUNT, RGB_PIN);

// Bariyer kontrol için ek değişkenler
bool barrierOpen = false;
unsigned long openTimestamp = 0;
// Bariyerin açık kalacağı süre (ms cinsinden)
const unsigned long PASS_DELAY = 5000;  // 5 saniye

// Mesafe eşiği (cm)
const float THRESHOLD = 10.0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  pinMode(RGB_PIN, OUTPUT);
  strip.begin(); 
  strip.show();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Bariyer başlangıçta kapalı
  motor.servo(1, 90);
  
  strip.setPixelColor(0, 255, 0, 0); // Set LED 1 to white
  strip.setPixelColor(1, 255, 0, 0); // Set LED 2 to white
  strip.setPixelColor(2, 255, 0, 0); // Set LED 3 to white
  strip.show();
}

void loop() {
  // Mesafeyi ölç
  distance = measureDistance();
  unsigned long now = millis();
  
  // 1) Bariyer kapalıysa ve araç eşiği geçerse → aç
  if (!barrierOpen && distance > 0 && distance <= THRESHOLD) {
    strip.setPixelColor(0, 0, 255, 0); // Set LED 1 to white
    strip.setPixelColor(1, 0, 255, 0); // Set LED 2 to white
    strip.setPixelColor(2, 0, 255, 0); // Set LED 3 to white
    strip.show();
    motor.servo(1, 20);       // Bariyeri kaldır
    barrierOpen   = true;     // Durumu güncelle
    openTimestamp = now;      // Açılış zamanını kaydet
    
  }

  // 2) Bariyer açıksa…
  if (barrierOpen) {
    
    if (distance > THRESHOLD) {
      // Araç sensörden uzaklaştıysa, süreyi kontrol edip kapat
      if (now - openTimestamp >= PASS_DELAY) {
        motor.servo(1, 90);    // Bariyeri indir
        barrierOpen = false;  // Durumu güncelle
        strip.setPixelColor(0, 255, 0, 0); // Set LED 1 to white
        strip.setPixelColor(1, 255, 0, 0); // Set LED 2 to white
        strip.setPixelColor(2, 255, 0, 0); // Set LED 3 to white
        strip.show();
      }
      // süre dolmadıysa bekle
    } else {
      // Araç hâlâ önündeyse, kapatma sayacını sıfırla
      openTimestamp = now;
    }
  }

  delay(100);  // Döngüyü biraz yavaşlat (opsiyonel)
}

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30 ms timeout
  if (duration == 0) return -1;              // timeout → hata

  // Ses hızı: 0.034 cm/µs; mesafe = (süre × hız) / 2
  return (duration * 0.034 / 2.0);
}
