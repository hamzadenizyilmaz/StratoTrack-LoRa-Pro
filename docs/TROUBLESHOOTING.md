# Troubleshooting

## GPS DATA but no FIX

GPS UART works, but the module has no satellite lock. Move outdoors.

## LoRa P2P OLD

The RAK module kept a previous working P2P config. If TX works, this is acceptable.

## Web panel not opening

Disable mobile data and open `http://192.168.4.1`.

## Serial monitor disconnects

Add these to `platformio.ini`:

```ini
monitor_rts = 0
monitor_dtr = 0
```
