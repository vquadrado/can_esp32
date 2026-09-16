# Arquitetura RTOS — telemetria CAN

O que vale do esboço de bancada + FreeRTOS. Veículo, DLC, OBD e segurança: [prd.md](../prd.md). Fontes: [sources.md](sources.md).

C++ no **ESP-IDF + FreeRTOS**. Sem Arduino. Dois apps: `firmware/simulator` e `firmware/reader`, código comum em `components/`. Spec do leitor: [firmware/reader/SPEC.md](../firmware/reader/SPEC.md).

---

## 1. Objetivo

ESP32-S3 + FreeRTOS para:

1. receber frames CAN (TWAI + SN65HVD230);
2. decodificar;
3. logar;
4. telemetria (serial; Wi-Fi depois; BLE opcional);
5. desenvolver na bancada com um simulador;
6. ligar o **mesmo leitor** no Yaris XS 2023 via OBD-II.

O pipeline do leitor não muda. O simulador fala a **mesma forma de fio** que o carro (500 kbit/s, 11-bit, broadcasts capturados e/ou OBD `7DF`/`7E8`). IDs inventados (`0x100`…) não entram.

O DLC do Yaris é diagnosis bus até o CGW, não o CAN do motor. Detalhe no PRD.

---

## 2. Duas fases, mesmo leitor

```text
Fase 1                          Fase 2
ESP32 simulador                 Yaris XS 2023
        │                              │
        └──────── CAN-H / CAN-L ───────┤
                                       ▼
                              ESP32 leitor
                              FreeRTOS
                              ├─ CAN RX
                              ├─ Parser
                              ├─ Logger
                              ├─ Telemetry
                              └─ System
                                       │
                              Serial / Wi-Fi / BLE
```

---

## 3. Hardware

Já tem: 2× ESP32, 2× SN65HVD230, protoboard, USB.

**Bancada:** 120 Ω **do próprio módulo**, um em cada ponta. Não soldar resistor extra em paralelo. GND comum. CAN do módulo em **3V3**, nunca 5 V. Sem MCP2515.

**Carro:** o 120 Ω do módulo do **leitor sai**. Breakout OBD, buck 12 V, fusível 1–2 A, TVS. Primeiro boot: TWAI listen-only.

```text
ESP32 TWAI TX/RX → SN65HVD230 CTX/CRX → CAN-H / CAN-L
```

```text
[ESP #1]----CANH----[ESP #2]
   120 Ω              120 Ω
[ESP #1]----CANL----[ESP #2]
         GND comum
```

---

## 4. Tasks

`app_main` só cria filas, mutexes, event groups e tasks.

**Simulador** — modelo (engine/vehicle) separado do TX:

| Task | Prio (relativa) |
| --- | ---: |
| CAN TX | 5 |
| OBD responder | 4 |
| Engine / vehicle | 3 |
| Fault injection | 2 |
| CLI | 1 |

**Leitor** — RX nunca espera Wi-Fi nem SD:

| Task | Prio (relativa) |
| --- | ---: |
| CAN RX | 5 |
| Parser / ISO-TP | 4 |
| Monitor | 3 |
| Logger / telemetry | 2 |
| Display (opcional) | 1 |
| OBD client | 3 | só depois do sniff; TX explícito |

ISR: só driver TWAI → fila. Watchdog nas tasks longas. Métricas: RX, erros, high-water da fila, drops, heap, stack watermark, uptime.

---

## 5. Contrato do parser

```cpp
struct CanFrame {
    uint32_t timestamp_us;
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
};

struct DecodedSignal {
    SignalType type;   // RPM, SPEED, TEMPERATURE, …
    uint32_t timestamp_us;
    float value;
};
```

Parser não sabe se o frame veio do simulador ou do DLC. Tabela de sinais = DBC/Yaris (preenchida após o sniff).

```text
TWAI → can_rx → Queue<CanFrame> → parser → DecodedSignal
                                      ├→ logger
                                      └→ telemetry
```

Log inicial (candump/CSV):

```text
timestamp_us,can_id,dlc,data
```

---

## 6. Simulador

Gera estado (OFF → IDLE → ACCEL → CRUISE → DECEL…) e **emite o que o leitor vai ver no carro**:

- respostas OBD (PIDs de RPM, velocidade, ECT, TPS, …);
- depois do sniff: replay dos IDs reais.

Fault injection na bancada (`fault can`, fila cheia, parser, watchdog). Nunca no DLC.

---

## 7. Repo

```text
firmware/reader/main/
firmware/simulator/main/
components/can parser isotp obd telemetry diagnostics
test/
```

---

## 8. Milestones

0. Um frame físico #1 → #2.
1. Tasks + queue + prioridades.
2. Modelo de veículo → frames no formato alvo (OBD e/ou DBC).
3. Parser → sinais.
4. Logger + métricas + overflow.
5. Wi-Fi / dashboard (HTTP+WS).
6. Fault injection.
7. Yaris listen-only; TX OBD só com bitrate e bus estáveis.

Critério da bancada: dois nós terminados, leitor não cai com frame inválido, telemetria acessível, watchdog ok.

Critério do carro: PRD §8–§9. Sem TX arbitrário, sem atuadores, sem simulador no mesmo cabo.
