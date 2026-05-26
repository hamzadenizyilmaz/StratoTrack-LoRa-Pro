# Second LoRa Receiver

This document explains how to add a second RAK3172 / RAK3272S LoRa receiver to StratoTrack LoRa Pro.

The receiver is an Arduino/ESP32 device connected to another RAK LoRa module. It receives the tracker packet, decodes the HEX payload, parses the CSV protocol, and prints readable tracking information to Serial Monitor.

## Hardware

Required:

- ESP32 or Arduino-compatible board with a spare UART
- RAK3172 or RAK3272S module
- LoRa antenna
- USB cable

## Wiring

```text
RAK UART2_TX -> ESP32 GPIO4
RAK UART2_RX -> ESP32 GPIO5
RAK 3V3      -> ESP32 3V3
RAK GND      -> ESP32 GND
```

## LoRa Configuration

The receiver must use the same P2P configuration as the tracker:

```text
868100000:12:125:0:8:14
```

## Receiver Sketch

The Arduino sketch is located at:

```text
examples/RAK3172_LoRa_Receiver/RAK3172_LoRa_Receiver.ino
```

## Expected Output

```text
LORA PACKET RECEIVED
RAW     : T1,FIX,39963205,32791445,1,8,1024,10,12,NORMAL
DEVICE  : T1
STATE   : FIX
LAT     : 39.963205
LON     : 32.791445
SPEED   : 1 km/h
SAT     : 8
ALT     : 1024 m
GSV     : 10
COUNT   : 12
MODE    : NORMAL
RSSI    : -84
SNR     : 10
```

## Troubleshooting

If no packet arrives:

- Make sure both antennas are connected.
- Make sure the receiver is in continuous receive mode: `AT+PRECV=65534`.
- Make sure the tracker shows `+EVT:TXP2P DONE`.
- Make sure both devices use the same P2P configuration.
- Try a short distance test first.
