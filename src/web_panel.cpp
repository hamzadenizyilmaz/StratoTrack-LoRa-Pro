#include "web_panel.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "config.h"
#include "globals.h"
#include "gps_tracker.h"

#if ENABLE_WEB_PANEL

WebServer server(80);

static bool webStarted = false;

static String escapeJson(String s) {
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  s.replace("\r", "");
  s.replace("\n", "\\n");
  return s;
}

String buildWebJson() {
  String json;
  json.reserve(1600);

  json += "{";
  json += "\"device\":\"" + String(DEVICE_ID) + "\",";
  json += "\"project\":\"" + String(PROJECT_NAME) + "\",";
  json += "\"firmware\":\"" + String(FIRMWARE_VERSION) + "\",";
  json += "\"gpsState\":\"" + getGpsState() + "\",";
  json += "\"fix\":" + String(hasGpsFix() ? "true" : "false") + ",";
  json += "\"lat\":\"" + latText() + "\",";
  json += "\"lon\":\"" + lonText() + "\",";
  json += "\"speed\":" + String(safeSpeedKmh()) + ",";
  json += "\"alt\":" + String(safeAlt()) + ",";
  json += "\"sat\":" + String(safeSat()) + ",";
  json += "\"gsv\":" + String(activeSatCountFromGsv()) + ",";
  json += "\"nmea\":" + String(nmeaChars) + ",";
  json += "\"lora\":\"" + escapeJson(loraState) + "\",";
  json += "\"tx\":\"" + escapeJson(txState) + "\",";
  json += "\"packet\":\"" + escapeJson(lastPacket) + "\",";
  json += "\"rak\":\"" + escapeJson(lastRakResponse) + "\",";
  json += "\"ip\":\"" + WiFi.softAPIP().toString() + "\",";
  json += "\"sats\":[";

  bool first = true;

  for (int i = 0; i < MAX_SATS; i++) {
    if (sats[i].prn > 0 && millis() - sats[i].seenMs < 15000) {
      if (!first) json += ",";
      first = false;

      json += "{";
      json += "\"name\":\"GPS-";
      if (sats[i].prn < 10) json += "0";
      json += String(sats[i].prn) + "\",";
      json += "\"prn\":" + String(sats[i].prn) + ",";
      json += "\"elev\":" + String(sats[i].elev) + ",";
      json += "\"az\":" + String(sats[i].az) + ",";
      json += "\"snr\":" + String(sats[i].snr);
      json += "}";
    }
  }

  json += "]";
  json += "}";

  return json;
}

const char WEB_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>StratoTrack LoRa Pro</title>
<style>
:root{--bg:#050814;--card:#0d1428;--card2:#121c37;--line:#263b68;--txt:#edf5ff;--muted:#8ea2c8;--green:#31f28a;--red:#ff4d6d;--yellow:#ffd166;--cyan:#36d1ff;--blue:#4f7cff}
*{box-sizing:border-box}
body{margin:0;background:radial-gradient(circle at top,#1b2550,#050814 58%);color:var(--txt);font-family:Arial,sans-serif}
.wrap{max-width:1150px;margin:auto;padding:18px}
.top{display:flex;justify-content:space-between;align-items:center;padding:18px;border:1px solid var(--line);border-radius:22px;background:linear-gradient(135deg,#111b38,#081024);box-shadow:0 20px 70px rgba(0,0,0,.38)}
h1{font-size:25px;margin:0}.sub{color:var(--muted);margin-top:6px}
.badge{padding:9px 13px;border-radius:999px;background:#15264d;color:var(--cyan);font-weight:800}
.grid{display:grid;grid-template-columns:repeat(4,1fr);gap:14px;margin-top:14px}
.card{background:linear-gradient(180deg,var(--card),#080d1d);border:1px solid var(--line);border-radius:20px;padding:16px;box-shadow:0 12px 30px rgba(0,0,0,.25)}
.card h3{margin:0 0 8px;font-size:13px;color:var(--muted)}.val{font-size:26px;font-weight:900}.ok{color:var(--green)}.bad{color:var(--red)}.warn{color:var(--yellow)}.cyan{color:var(--cyan)}
.big{grid-column:span 2}.packet{font-family:Consolas,monospace;color:#b8c8ff;word-break:break-all;font-size:13px}
.sats{display:grid;grid-template-columns:repeat(auto-fill,minmax(150px,1fr));gap:10px}.sat{padding:12px;border:1px solid var(--line);border-radius:16px;background:var(--card2)}.sat strong{display:block;color:var(--cyan);font-size:18px;margin-bottom:4px}.bar{height:8px;background:#15213e;border-radius:999px;margin-top:8px;overflow:hidden}.bar span{display:block;height:100%;background:linear-gradient(90deg,var(--yellow),var(--green))}
a{color:var(--cyan);font-weight:800;text-decoration:none}
@media(max-width:800px){.grid{grid-template-columns:1fr 1fr}.big{grid-column:span 2}}@media(max-width:520px){.grid{grid-template-columns:1fr}.big{grid-column:span 1}.top{display:block}.badge{display:inline-block;margin-top:12px}}
</style>
</head>
<body>
<div class="wrap">
<div class="top"><div><h1>StratoTrack LoRa Pro</h1><div class="sub">Offline LoRa GPS beacon with local satellite dashboard</div></div><div class="badge" id="device">---</div></div>
<div class="grid">
<div class="card"><h3>GNSS</h3><div class="val" id="gps">---</div></div>
<div class="card"><h3>Satellites</h3><div class="val cyan" id="sat">---</div></div>
<div class="card"><h3>LoRa Radio</h3><div class="val" id="lora">---</div></div>
<div class="card"><h3>TX State</h3><div class="val cyan" id="tx">---</div></div>
<div class="card big"><h3>Position</h3><div class="val" id="latlon">---</div><div style="margin-top:8px"><a id="map" href="#" target="_blank">Open in Maps</a></div></div>
<div class="card"><h3>Speed</h3><div class="val" id="speed">0 km/h</div></div>
<div class="card"><h3>Altitude</h3><div class="val" id="alt">0 m</div></div>
<div class="card big"><h3>Last LoRa Packet</h3><div class="packet" id="packet">---</div></div>
<div class="card big"><h3>RAK Response</h3><div class="packet" id="rak">---</div></div>
</div>
<div class="card" style="margin-top:14px"><h3>Active GPS Satellites</h3><div class="sats" id="sats">Loading...</div></div>
</div>
<script>
function gpsClass(s){if(s==="FIX")return"ok";if(s==="DATA")return"warn";return"bad"}
function snrWidth(v){if(v<0)return 0;if(v>50)return 100;return Math.round((v/50)*100)}
async function load(){try{const r=await fetch("/api/status",{cache:"no-store"});const d=await r.json();document.getElementById("device").textContent=d.device+" / "+d.ip;const gps=document.getElementById("gps");gps.textContent=d.gpsState;gps.className="val "+gpsClass(d.gpsState);document.getElementById("sat").textContent=d.sat+" / "+d.gsv;const lora=document.getElementById("lora");lora.textContent=d.lora;lora.className="val "+(d.lora==="READY"?"ok":"warn");document.getElementById("tx").textContent=d.tx;document.getElementById("latlon").textContent=d.lat+", "+d.lon;document.getElementById("speed").textContent=d.speed+" km/h";document.getElementById("alt").textContent=d.alt+" m";document.getElementById("packet").textContent=d.packet;document.getElementById("rak").textContent=d.rak;const map=document.getElementById("map");if(d.fix){map.href="https://maps.google.com/?q="+d.lat+","+d.lon;map.style.opacity="1"}else{map.href="#";map.style.opacity=".4"}const sats=document.getElementById("sats");sats.innerHTML="";if(!d.sats||d.sats.length===0){sats.innerHTML='<div style="color:var(--muted)">Waiting for satellite details...</div>';return}d.sats.forEach(s=>{const w=snrWidth(s.snr);const div=document.createElement("div");div.className="sat";div.innerHTML=`<strong>${s.name}</strong><div>SNR: ${s.snr<0?"--":s.snr+" dB"}</div><div>Elevation: ${s.elev<0?"--":s.elev+"°"}</div><div>Azimuth: ${s.az<0?"--":s.az+"°"}</div><div class="bar"><span style="width:${w}%"></span></div>`;sats.appendChild(div)})}catch(e){console.log(e)}}
setInterval(load,1000);load();
</script>
</body>
</html>
)rawliteral";

static void handleRoot() {
  server.send_P(200, "text/html", WEB_HTML);
}

static void handleApiStatus() {
  String json = buildWebJson();

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void startWebPanel() {
  if (webStarted) return;

  Serial.println();
  Serial.println("================================");
  Serial.println("WEB PANEL START");
  Serial.println("================================");

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);

  IPAddress localIp(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.softAPConfig(localIp, gateway, subnet);

  bool ok = WiFi.softAP(WEB_AP_SSID, WEB_AP_PASS, 6, 0, 4);

  if (!ok) {
    Serial.println("WEB AP FAILED");
    return;
  }

  delay(500);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleApiStatus);

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });

  server.begin();

  webStarted = true;

  Serial.print("WEB AP SSID: ");
  Serial.println(WEB_AP_SSID);

  Serial.print("WEB AP PASS: ");
  Serial.println(WEB_AP_PASS);

  Serial.print("WEB AP IP  : ");
  Serial.println(WiFi.softAPIP());

  Serial.println("OPEN URL   : http://192.168.4.1");
}

void handleWebPanel() {
  if (!webStarted) return;
  server.handleClient();
}

#else

void startWebPanel() {}
void handleWebPanel() {}
String buildWebJson() { return "{}"; }

#endif
