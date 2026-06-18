/*
 * MyWaki — half-duplex walkie-talkie firmware
 *
 * Hardware: Arduino Nano, nRF24L01+ PA/LNA, MAX9814 mic, PAM8403 amplifier
 * Audio stack: RF24Audio (16 kHz, 1 Mbps, oversampling)
 *
 * Controls
 *   A1  PTT      — hold to GND to transmit
 *   A2  CHANNEL  — release to cycle CH1..CH4
 *   D2,D3,D6,D5 — channel indicator LEDs
 *   D13         — onboard LED (solid while PTT pressed)
 *
 * Both units run the same sketch. Match RF channels before talking.
 */

#include <SPI.h>
#include <RF24.h>
#include <RF24Audio.h>

#define CE_PIN 7
#define CSN_PIN 8
#define RADIO_ID 1
#define PTT_PIN A1
#define CH_PIN A2
#define PLAY_VOLUME 6

#define CH_COUNT 4
#define CH_RELEASE_GAP_MS 25

const uint8_t CH_LED_PINS[CH_COUNT] = {2, 3, 6, 5};
const uint8_t RF_CHANNELS[CH_COUNT] = {76, 77, 78, 79};

RF24 radio(CE_PIN, CSN_PIN);
RF24Audio rfAudio(radio, RADIO_ID);

uint8_t activeCh = 0;
unsigned long lastChReleaseMs = 0;
bool chWasPressed = false;

void updateChannelLeds() {
  for (uint8_t i = 0; i < CH_COUNT; i++) {
    digitalWrite(CH_LED_PINS[i], i == activeCh ? HIGH : LOW);
  }
}

bool channelButtonReleased() {
  bool high = digitalRead(CH_PIN) == HIGH;
  unsigned long now = millis();
  bool released = false;

  if (!chWasPressed && !high) {
    chWasPressed = true;
  }
  if (chWasPressed && high && (now - lastChReleaseMs) >= CH_RELEASE_GAP_MS) {
    released = true;
    lastChReleaseMs = now;
    chWasPressed = false;
  }
  return released;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(CH_PIN, INPUT_PULLUP);

  for (uint8_t i = 0; i < CH_COUNT; i++) {
    pinMode(CH_LED_PINS[i], OUTPUT);
  }

  rfAudio.begin();
  rfAudio.setVolume(PLAY_VOLUME);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(RF_CHANNELS[0]);
  rfAudio.receive();
  updateChannelLeds();
}

void loop() {
  rfAudio.handleButtons();

  if (channelButtonReleased()) {
    activeCh = (activeCh + 1) % CH_COUNT;
    radio.setChannel(RF_CHANNELS[activeCh]);
    updateChannelLeds();
  }

  digitalWrite(LED_BUILTIN, digitalRead(PTT_PIN) == LOW ? HIGH : LOW);
}
