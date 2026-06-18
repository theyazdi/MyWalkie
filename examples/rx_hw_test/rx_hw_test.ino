/*
 * MyWaki — local speaker test (no RF)
 * D9 -> PAM8403 LIN, RIN -> GND (mono left channel)
 * Plays 440 / 880 / 1200 Hz tones in a loop.
 */

#define PIN_AUDIO 9
#define SILENCE 128

void audioOnSetup() {
  pinMode(PIN_AUDIO, OUTPUT);
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
  ICR1 = 255;
}

void playPwm(uint8_t v) {
  OCR1A = v;
}

const uint8_t PROGMEM sine64[64] = {
  128, 140, 152, 165, 176, 188, 199, 209,
  218, 226, 234, 240, 245, 250, 252, 254,
  255, 254, 252, 250, 245, 240, 234, 226,
  218, 209, 199, 188, 176, 165, 152, 140,
  128, 116, 104,  91,  80,  68,  57,  47,
   38,  30,  22,  16,  11,   6,   4,   2,
    1,   2,   4,   6,  11,  16,  22,  30,
   38,  47,  57,  68,  80,  91, 104, 116
};

void playSine(uint16_t freqHz, uint16_t durationMs) {
  audioOnSetup();
  uint16_t phase = 0;
  unsigned long end = millis() + durationMs;

  while (millis() < end) {
    phase += (uint16_t)((256UL * freqHz) / 8000UL);
    uint8_t v = pgm_read_byte(&sine64[(phase >> 2) & 0x3F]);
    playPwm(v);
    delayMicroseconds(125);
  }

  TCCR1A = 0;
  TCCR1B = 0;
  digitalWrite(PIN_AUDIO, LOW);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  playSine(440, 600);
  delay(250);
  playSine(880, 600);
  delay(250);
  playSine(1200, 180);
  delay(120);
  playSine(1200, 180);
  delay(120);
  playSine(1200, 180);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1800);
}
