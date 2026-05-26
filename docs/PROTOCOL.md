# LoRa Protocol

## Packet Format

```text
ID,STATE,LATx1e6,LONx1e6,SPEED,SAT,ALT,GSV,COUNTER,MODE
```

## Example

```text
T1,FIX,39963205,32791445,1,8,1024,10,12,NORMAL
```

## Why CSV?

CSV is shorter than JSON and easier to decode on small receivers.
