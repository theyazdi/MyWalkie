/**
 * RF24Audio userConfig for MyWaki walkie firmware.
 * Copy into the RF24Audio library folder before compiling (see tools/sync_rf24_config).
 */

#define SAMPLE_RATE 16000
#define RF_SPEED RF24_1MBPS

#define ANALOG_PIN A0

#define speakerPin 9
#define speakerPin2 10

#define oversampling

#define TX_PIN A1
#define VOL_UP_PIN A4
#define VOL_DN_PIN A5
#define REMOTE_TX_PIN A3
#define REMOTE_RX_PIN 4

#define buffSize 32

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
 #define rampMega
#endif

const uint64_t pipes[14] = {
  0xABCDABCD71LL, 0x544d52687CLL, 0x544d526832LL, 0x544d52683CLL,
  0x544d526846LL, 0x544d526850LL, 0x544d52685ALL, 0x544d526820LL,
  0x544d52686ELL, 0x544d52684BLL, 0x544d526841LL, 0x544d526855LL,
  0x544d52685FLL, 0x544d526869LL
};
