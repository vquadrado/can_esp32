# Fontes — Yaris XS 2023 / CAN / ESP32

Pesquisa usada no [prd.md](../prd.md). Nenhuma destas fontes substitui medição no VIN do carro.

## Veículo

- [Toyota Yaris (XP150)](https://en.wikipedia.org/wiki/Toyota_Yaris_(XP150)) — plataforma BR, 2NR-FBE, Sorocaba, TSS no facelift XS/XLS.
- [Yaris 2023 BR — versões e motor](https://www.mobiauto.com.br/catalogo/carros/toyota/yaris/2023)
- [Yaris 1.5 XP150 Brazil specs](https://fastestlaps.com/models/toyota-yaris-xp150-brazil)

## DLC3 / pinout / testes Toyota

- [Toyota/Lexus OBD-II DLC pinout](https://pinoutguide.com/CarElectronics/Toyota_lexus_obd2_diagnostic_pinout.shtml)
- [2023 Toyota Yaris pinouts](https://pinoutguide.com/dev/Toyota/Yaris/2023/)
- [OBD2 pinout (Toyota)](https://www.flexihub.com/oobd2-pinout/)
- [Yaris — Check CAN Communication Connection](https://www.toyaris4.com/toyota_yaris_check_can_communication_connection-1445.html) — 6/14, ~60 Ω, CGW.
- [Yaris — System Description CAN](https://www.toyaris4.com/toyota_yaris_system_description-1434.html) — CGW, buses, terminadores 120 Ω.
- [Yaris — Terminals of ECU](https://www.toyaris4.com/toyota_yaris_terminals_of_ecu-1437.html)
- [Yaris — Diagnosis System CAN](https://www.toyaris4.com/toyota_yaris_diagnosis_system-1438.html)
- [Yaris — DLC ISO 15765-4, 54–69 Ω](https://www.toyaris4.com/toyota_yaris_general_information-390.html)
- [Camry XV70 — DLC só valida o diagnosis bus](https://www.tocamry.com/how_to_proceed_with_troubleshooting-3825.html)
- [C-HR — mesmo aviso de DLC vs CGW](https://www.tochr.net/904/how_to_proceed_with_troubleshooting.html)

## Yaris Brasil (local do plug, fusível, scanners)

- [Yaris Clube — conector OBD2, ELM 1.5 vs 2.1](https://yarisclube.directorioforuns.com/t96-conector-obd2)
- [Caixa de fusíveis Yaris 2023 BR — fusível OBD 7,5 A](https://www.opinautos.com.br/toyota/yaris/info/fusiveis/2023/br)
- [Onde fica o OBD2 (outros Yaris; local típico sob o volante)](https://www.scegliauto.com/pt/video/toyota/tutorial/111465/)

## Protocolos OBD / ISO-TP / UDS

- [OBD2 protocols (ISO 15765-4 variantes)](https://www.obdtester.com/obd2_protocols)
- [Reading OBD2 data without ELM327 (IDs 7DF / 7E8)](https://m0agx.eu/reading-obd2-data-without-elm327-part-1-can.html)
- [UDS intro — CSS Electronics](https://www.csselectronics.com/pages/uds-protocol-tutorial-unified-diagnostic-services)
- Resolução CONAMA nº 415/2009 e [IN IBAMA/CONAMA nº 5/2013 (OBDBr-D)](https://www.normasbrasil.com.br/norma/instrucao-normativa-5-2013_251166.html) — acesso ISO 15031 no DLC (texto diesel; Otto usa a mesma família ISO).
- [Yaris 2007 — ISO 15765-4 CAN 11-bit 500 kbps](http://www.obdkey.com/featuredvehicle.asp?vehicle=1382) — geração anterior; só analogia de protocolo.

## IDs Toyota (não validados neste VIN)

- [Notas listen-only Yaris — P1kachu/talking-with-cars](https://github.com/P1kachu/talking-with-cars/blob/master/notes/toyota-yaris.md)
- [commaai/opendbc](https://github.com/commaai/opendbc) — `TOYOTA_YARIS` é JDM/SecOC 2023, **não** XP150 BR.
- [OBDb/Toyota-Yaris](https://github.com/OBDb/toyota-Yaris)

## Gateway / o que o OBD não mostra

- [r/CarHacking — CAN Bus OBD Gateway](https://www.reddit.com/r/CarHacking/comments/1clwgl9/can_bus_obd_gateway/)

## Transceptor do projeto (SN65HVD230)

- [Datasheet TI SN65HVD23x](https://www.ti.com/lit/ds/symlink/sn65hvd230.pdf) — 3,3 V, pino RS (high-speed / slope / standby).
- [CJMCU-230 pinout (3V3, GND, CTX, CRX, CANH, CANL)](https://www.phippselectronics.com/support/cjmcu-230-sn65hvd230-can-bus-transceiver-communication-module-support-documentation/)

## ESP32-S3 TWAI e hardware OBD

- [ESP-IDF TWAI (ESP32-S3)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/twai.html)
- [ESP32 + SN65HVD230](https://zbotic.in/esp32-can-bus-industrial-communication-with-sn65hvd230/)
- [TWAI ISO-TP (Arduino)](https://github.com/Kostovite/TWAI_ISO-TP)
- [esp32-obd2](https://github.com/LaXiS96/esp32-obd2)
- [OBD2 CAN Bus Library](https://github.com/muki01/OBD2_CAN_Bus_Library)
- [esp32-autosport — TJA1051 silent, terminador off no DLC](https://github.com/MrBlahhhh/esp32-autosport)
- [Spec ESP32-S3 OBD2 logger (proteção 12 V, TVS CAN)](https://www.flux.ai/apx271/esp32-s3-obd2-data-logger~ri/files/project-specification-esp32-s3-obd2-data-logger~xv)
- [esp-can — buck no pino 16, sem 120 Ω extra](https://github.com/pasqualehun/esp-can)
