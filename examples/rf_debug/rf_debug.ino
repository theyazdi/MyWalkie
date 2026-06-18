/*
 * MyWaki — RF link test (no audio)
 * Same PTT / channel pins and RF map as walkie firmware.
 *
 * Serial 115200 — close monitor before upload.
 * Hold PTT (A1) to send pings; the other board should print [RX] lines.
 */

#include <SPI.h>
#include <RF24.h>

#define CE_PIN 7
#define CSN_PIN 8
#define PTT_PIN A1
#define CH_PIN A2

#define CH_COUNT 4
#define CH_RELEASE_GAP_MS 25
#define DIAG_MS 500

const uint8_t CH_LED_PINS[CH_COUNT] = {2, 3, 6, 5};
const uint8_t RF_CHANNELS[CH_COUNT] = {76, 77, 78, 79};
const uint64_t RF_PIPE = 0x544d52687CLL;

RF24 radio(CE_PIN, CSN_PIN);

uint8_t activeCh = 0;
uint32_t txOk = 0;
uint32_t txFail = 0;
uint32_t rxPkts = 0;
unsigned long lastChReleaseMs = 0;
unsigned long lastDiagMs = 0;
bool chWasPressed = false;
bool txActive = false;

void setupRadio() {
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(RF_CHANNELS[activeCh]);
  radio.setPayloadSize(32);
  radio.setAddressWidth(5);
  radio.setAutoAck(true);
  radio.setRetries(5, 15);
  radio.setCRCLength(RF24_CRC_16);
  radio.openReadingPipe(1, RF_PIPE);
  radio.startListening();
}

void updateLeds() {
  for (uint8_t i = 0; i < CH_COUNT; i++) {
    digitalWrite(CH_LED_PINS[i], i == activeCh ? HIGH : LOW);
  }
}

bool channelReleased() {
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

void pollRx() {
  while (radio.available()) {
    char msg[32];
    radio.read(msg, sizeof(msg) - 1);
    msg[31] = '\0';
    rxPkts++;
    Serial.print(F("[RX] "));
    Serial.println(msg);
  }
}

void sendPing() {
  char pkt[32];
  snprintf(pkt, sizeof(pkt), "PING ch%u #%lu", activeCh + 1, txOk);
  radio.stopListening();
  radio.openWritingPipe(RF_PIPE);
  bool ok = radio.write(pkt, strlen(pkt) + 1);
  if (ok) {
    txOk++;
  } else {
    txFail++;
  }
  radio.openReadingPipe(1, RF_PIPE);
  radio.startListening();
}

void setup() {
  pinMode(PTT_PIN, INPUT_PULLUP);
  pinMode(CH_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  for (uint8_t i = 0; i < CH_COUNT; i++) {
    pinMode(CH_LED_PINS[i], OUTPUT);
  }

  Serial.begin(115200);
  delay(300);
  Serial.println(F("=== MyWaki RF debug ==="));
  Serial.println(F("PTT = send ping | peer should show [RX]"));

  if (!radio.begin()) {
    Serial.println(F("ERR radio.begin() failed"));
    while (true) {
      delay(500);
    }
  }
  setupRadio();
  updateLeds();
  Serial.println(F("ready"));
}

void loop() {
  bool ptt = digitalRead(PTT_PIN) == LOW;
  digitalWrite(LED_BUILTIN, ptt ? HIGH : LOW);

  if (channelReleased()) {
    activeCh = (activeCh + 1) % CH_COUNT;
    setupRadio();
    updateLeds();
    Serial.print(F("[EV] channel -> CH"));
    Serial.println(activeCh + 1);
  }

  pollRx();

  if (ptt) {
    if (!txActive) {
      txActive = true;
      Serial.println(F("[EV] TX ping start"));
    }
    static unsigned long lastTx = 0;
    if (millis() - lastTx >= 80) {
      lastTx = millis();
      sendPing();
    }
  } else if (txActive) {
    txActive = false;
    Serial.println(F("[EV] TX ping stop"));
  }

  if (millis() - lastDiagMs >= DIAG_MS) {
    lastDiagMs = millis();
    Serial.print(F("[STAT] CH"));
    Serial.print(activeCh + 1);
    Serial.print(F(" RF"));
    Serial.print(RF_CHANNELS[activeCh]);
    Serial.print(F(" tx_ok="));
    Serial.print(txOk);
    Serial.print(F(" tx_fail="));
    Serial.print(txFail);
    Serial.print(F(" rx_total="));
    Serial.print(rxPkts);
    Serial.print(F(" rpd="));
    Serial.println(radio.testRPD() ? 1 : 0);
  }
}
