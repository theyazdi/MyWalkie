# MyWaki

Half-duplex **walkie-talkie** firmware for **Arduino Nano** + **nRF24L01+ PA/LNA**, using [RF24Audio](https://github.com/nRF24/RF24Audio) for real-time voice over 2.4 GHz.

Build **two identical units**. Each board listens by default; hold **PTT** to transmit. Four RF channels let you switch groups without re-flashing.

```
  Mic (MAX9814) ──► Nano ──► nRF24 ──► ~~~ air ~~~ ──► nRF24 ──► Nano ──► Speaker (PAM8403)
```

## Features

- **Push-to-talk** on `A1` (active low, internal pull-up)
- **4 RF channels** (76–79) with LED indicators
- **Single firmware** for every unit — no separate TX/RX sketches
- **RF24Audio** at 16 kHz / 1 Mbps with oversampling (stable audio path)
- Optional **debug sketches**: speaker-only test and raw RF ping test

## Bill of materials

Build **one unit**; repeat for the second walkie.

| Qty | Part | Notes |
|-----|------|--------|
| 1 | Arduino Nano (ATmega328P) | CH340 USB |
| 1 | nRF24L01+ PA/LNA module | Externally regulated 3.3 V |
| 1 | AMS1117-3.3V regulator | Powers the radio from 5 V |
| 2 | 10 µF electrolytic capacitor | Regulator in / out |
| 2 | 100 nF ceramic capacitor | Regulator in / out |
| 1 | MAX9814 electret mic module | Auto-gain, analog out |
| 1 | PAM8403 amplifier module | Mono wiring (left channel) |
| 1 | Small speaker | 4–8 Ω, 0.5–3 W typical |
| 4 | 3 mm LED | Channel indicators |
| 4 | 220 Ω resistor | LED current limit |
| 2 | Tactile push button | PTT + channel |
| — | Jumper wire, breadboard or perfboard | Common GND everywhere |

### Optional

| Part | Purpose |
|------|---------|
| 4.7 µF capacitor in series with D9 → PAM8403 LIN | Reduces speaker hiss |
| Short wire antennas | Already on most nRF24 PA/LNA boards |

## Pin map (both units — identical wiring)

| Nano pin | Function | Connect to |
|----------|----------|------------|
| **D7** | nRF24 CE | CE |
| **D8** | nRF24 CSN | CSN |
| **D9** | Audio PWM out | PAM8403 **LIN** |
| **D11** | SPI MOSI | nRF24 MOSI |
| **D12** | SPI MISO | nRF24 MISO |
| **D13** | SPI SCK + onboard LED | nRF24 SCK; LED = TX indicator |
| **D2** | CH1 LED | LED → 220 Ω → GND |
| **D3** | CH2 LED | LED → 220 Ω → GND |
| **D6** | CH3 LED | LED → 220 Ω → GND |
| **D5** | CH4 LED | LED → 220 Ω → GND |
| **A0** | Microphone | MAX9814 **OUT** |
| **A1** | PTT button | Button → **GND** |
| **A2** | Channel button | Button → **GND** |
| **5 V** | Power | MAX9814, PAM8403, AMS1117 IN |
| **GND** | Ground | All modules |

### Unused pins (do not wire)

| Pin | Reason |
|-----|--------|
| D0, D1 | USB serial |
| D4 | Reserved / free |
| D10 | Used internally by RF24Audio (PWM) — no LED here |
| A3–A5 | Volume / remote pins not used in this build |
| nRF24 IRQ | Leave disconnected |

## Wiring details

### nRF24L01+ ↔ Arduino Nano

| Radio | Nano |
|-------|------|
| GND | GND |
| VCC | **3.3 V** (AMS1117 OUT — never 5 V) |
| CE | D7 |
| CSN | D8 |
| SCK | D13 |
| MOSI | D11 |
| MISO | D12 |

### AMS1117-3.3 V

| Regulator | Connection |
|-----------|------------|
| IN | 5 V |
| GND | GND |
| OUT | nRF24 VCC |
| 10 µF + 100 nF | IN ↔ GND |
| 10 µF + 100 nF | OUT ↔ GND |

### MAX9814 microphone

| MAX9814 | Nano |
|---------|------|
| GND | GND |
| VDD | 5 V |
| OUT | A0 |
| GAIN | **5 V** (recommended — highest sensitivity) |
| AR | Not connected |

### PAM8403 amplifier (mono)

| PAM8403 | Nano |
|---------|------|
| VCC | 5 V |
| GND | GND |
| LIN | D9 |
| RIN | **GND** (mono mix) |
| L+ / L− | Speaker |

> Do **not** connect the speaker to Arduino GND. Only across L+ and L−.

### Buttons (internal pull-ups enabled in firmware)

```
A1 ─── [ PTT ] ─── GND     hold = transmit
A2 ─── [ CH  ] ─── GND     release = next channel
```

### Power tree

```text
USB
 └── Arduino Nano 5 V
      ├── MAX9814 VDD
      ├── PAM8403 VCC
      └── AMS1117 ──► 3.3 V ──► nRF24 VCC
```

## RF channels

| Channel | LED on | RF channel |
|---------|--------|------------|
| 1 | D2 | 76 |
| 2 | D3 | 77 |
| 3 | D6 | 78 |
| 4 | D5 | 79 |

- Boot always starts on **CH1** (D2 on).
- **Release** the channel button to advance 1 → 2 → 3 → 4 → 1.
- **Both walkies must use the same channel** to hear each other.

## Software setup

### 1. Install Arduino IDE or arduino-cli

Board: **Arduino Nano** — processor **ATmega328P (Old Bootloader)** if upload fails on clones.

### 2. Install libraries

| Library | Source |
|---------|--------|
| [RF24](https://github.com/nRF24/RF24) | Library Manager or GitHub |
| [RF24Audio](https://github.com/nRF24/RF24Audio) | Library Manager or GitHub |

### 3. Sync RF24Audio configuration

RF24Audio reads `userConfig.h` from **inside its library folder**. After any edit to `firmware/walkie/userConfig.h`:

```powershell
cd tools
.\sync_rf24_config.ps1
```

Or copy `firmware/walkie/userConfig.h` manually to:

`Documents/Arduino/libraries/RF24Audio/userConfig.h`

### 4. Upload walkie firmware

Close the **Serial Monitor** before uploading.

**Arduino IDE:** open `firmware/walkie/walkie.ino` and upload to each Nano.

**arduino-cli:**

```bash
arduino-cli lib install "RF24" "RF24Audio"
arduino-cli compile -b arduino:avr:nano firmware/walkie
arduino-cli upload -b arduino:avr:nano -p COM7 firmware/walkie
arduino-cli upload -b arduino:avr:nano -p COM8 firmware/walkie
```

Replace `COM7` / `COM8` with your ports.

## Usage

1. Power both units (USB).
2. Confirm both show **the same channel LED** (start with D2 = CH1).
3. On unit A: **hold PTT** and speak into the mic.
4. On unit B: listen on the speaker (release PTT on B).
5. Swap roles to talk back.

| LED | Meaning |
|-----|---------|
| D2–D5 (one on) | Selected RF channel |
| D13 solid | Local unit is transmitting (PTT held) |
| D13 faint flicker | Normal — SPI activity on SCK (shared with D13) |

## Project layout

```text
github/
├── README.md                 ← you are here
├── LICENSE
├── firmware/
│   └── walkie/
│       ├── walkie.ino        ← main firmware (flash to every unit)
│       └── userConfig.h      ← RF24Audio tuning (sync to library)
├── examples/
│   ├── rf_debug/             ← raw RF ping test (no audio)
│   └── rx_hw_test/           ← speaker beep test (no RF)
└── tools/
    └── sync_rf24_config.ps1  ← copy userConfig.h into RF24Audio lib
```

## Troubleshooting

| Symptom | Check |
|---------|--------|
| No audio either way | Same channel on both? nRF24 on **3.3 V**? Run `examples/rf_debug` on both |
| RF debug OK, no voice | MAX9814 **GAIN → 5 V**; re-sync `userConfig.h`; re-upload **walkie** |
| No speaker at all | Upload `examples/rx_hw_test`; wiring D9 → LIN, speaker on L+/L− |
| Upload fails `not in sync` | Close Serial Monitor; try Old Bootloader |
| Hiss / crackle | Tie MAX9814 GAIN to VDD; optional 4.7 µF in series on D9; use stock RF24Audio settings (no sidetone hacks) |
| Weak volume | MAX9814 GAIN → VDD; adjust `PLAY_VOLUME` in `walkie.ino` (default 6) |

### Suggested debug order

1. `examples/rx_hw_test` — speaker works?
2. `examples/rf_debug` on both boards, same channel — `[RX]` lines when peer holds PTT?
3. `firmware/walkie` — voice path

## How it works

Audio uses **RF24Audio** with PTT handled by `rfAudio.handleButtons()` (reads `A1` / `TX_PIN`). The library streams 8-bit PCM at 16 kHz over nRF24 multicast pipes. Channel changes only call `radio.setChannel()` — TX/RX state stays under library control.

## Credits

- [RF24](https://github.com/nRF24/RF24) — nRF24L01 driver
- [RF24Audio](https://github.com/nRF24/RF24Audio) — real-time audio streaming (TMRh20)

## License

MIT — see [LICENSE](LICENSE).
