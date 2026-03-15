#include <Arduino.h>
#include "OTAUpdater/ota_updater.h"
#include "StateMachine/Idle.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ MAIN - STRIPE MEASUREMENT DEVICE ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

AsyncWebServer server(80);

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Stripe Measurement Device</title>
  <link rel="icon" type="image/svg+xml" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'%3E%3Crect width='32' height='32' rx='6' fill='%230f3460'/%3E%3Cpath d='M6 16h16' stroke='%23fff' stroke-width='2.5' stroke-linecap='round'/%3E%3Ccircle cx='26' cy='16' r='3' fill='%23e94560'/%3E%3C/svg%3E">
  <style>
    body { font-family: system-ui, sans-serif; max-width: 560px; margin: 2em auto; padding: 1em; background: #1a1a2e; color: #eee; }
    .main-wrap { display: flex; gap: 1em; align-items: flex-start; }
    .main-content { flex: 1; min-width: 0; }
    .total-box-wrap { flex-shrink: 0; width: 140px; }
    .total-box { background: #0f3460; border: 2px solid #e94560; border-radius: 8px; padding: 1em; text-align: center; min-height: 100px; display: flex; flex-direction: column; justify-content: center; }
    .total-box .total-label { font-size: 0.75rem; color: #a0aec0; text-transform: uppercase; letter-spacing: 0.05em; margin-bottom: 0.25em; }
    .total-box .total-value { font-size: 2rem; font-weight: 700; color: #e94560; line-height: 1.2; }
    .header { text-align: center; margin-bottom: 1.5em; }
    .logo { width: 72px; height: 72px; margin: 0 auto 0.5em; display: block; }
    h1 { font-size: 1.25rem; font-weight: 600; margin: 0; letter-spacing: 0.02em; color: #fff; }
    .row { display: flex; justify-content: space-between; margin: 0.5em 0; padding: 0.5em; background: #16213e; border-radius: 6px; }
    .label { font-weight: 600; color: #a0aec0; }
    .total { font-size: 1.5rem; background: #0f3460; }
    .boards-section { margin-top: 1em; padding: 0.75em; background: #16213e; border-radius: 6px; }
    .boards-section .label { font-weight: 600; color: #a0aec0; margin-bottom: 0.5em; display: block; }
    .boards-list { max-height: 12em; overflow-y: auto; scrollbar-width: thin; scrollbar-color: #e94560 #16213e; }
    .boards-list::-webkit-scrollbar { width: 10px; }
    .boards-list::-webkit-scrollbar-track { background: #16213e; border-radius: 5px; }
    .boards-list::-webkit-scrollbar-thumb { background: linear-gradient(180deg, #e94560 0%, #c73e54 100%); border-radius: 5px; }
    .boards-list::-webkit-scrollbar-thumb:hover { background: linear-gradient(180deg, #ff6b6b 0%, #e94560 100%); }
    .board-item { padding: 0.25em 0; font-size: 0.9rem; color: #a0aec0; }
    .latest-row { padding: 0.5em; background: #0f3460; border-radius: 6px; border: 2px solid transparent; font-size: 1.35rem; font-weight: 600; margin-bottom: 0.5em; }
    button { margin-top: 1em; padding: 0.5em 1em; font-size: 1rem; cursor: pointer; background: #e94560; color: #fff; border: none; border-radius: 6px; font-weight: 600; }
    button:hover { background: #ff6b6b; }
    button.remove-btn { background: #4a5568; margin-left: 0.5em; }
    button.remove-btn:hover { background: #5a6578; }
    @keyframes latestRowFlash {
      0% { box-shadow: 0 0 0 0 rgba(233, 69, 96, 0); border: 2px solid transparent; background: #0f3460; transform: scale(1); }
      15% { box-shadow: 0 0 30px 10px rgba(233, 69, 96, 0.8), inset 0 0 25px rgba(233, 69, 96, 0.25); border-color: #e94560; background: rgba(233, 69, 96, 0.31); transform: scale(1.025); }
      40% { box-shadow: 0 0 40px 15px rgba(233, 69, 96, 0.55); border-color: #e94560; background: rgba(233, 69, 96, 0.15); transform: scale(1.012); }
      100% { box-shadow: 0 0 0 0 rgba(233, 69, 96, 0); border: 2px solid transparent; background: #0f3460; transform: scale(1); }
    }
    .latest-row-flash { animation: latestRowFlash 1.4s cubic-bezier(0.34, 1.56, 0.64, 1); }
    .desired-row { margin-bottom: 0.5em; }
    .desired-row label { font-size: 0.7rem; color: #a0aec0; text-transform: uppercase; letter-spacing: 0.05em; display: block; margin-bottom: 0.2em; }
    .desired-row input { width: 100%; box-sizing: border-box; padding: 0.4em 0 0.4em 0.4em; font-size: 1rem; background: #16213e; border: 1px solid #2a3a5e; border-radius: 4px; color: #eee; }
    .desired-row input[type="number"]::-webkit-inner-spin-button,
    .desired-row input[type="number"]::-webkit-outer-spin-button { -webkit-appearance: none; appearance: none; margin: 0; opacity: 1; cursor: pointer; background-color: #2a3a5e; background-repeat: no-repeat; background-position: center; border-left: 1px solid #3a4a6e; }
    .desired-row input[type="number"]::-webkit-outer-spin-button { display: none; }
    .desired-row input[type="number"]::-webkit-inner-spin-button { height: 1.8em; width: 1.4em; background-size: 0.55em 1em; background-image: url('data:image/svg+xml,<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 12 20\"><path fill=\"%23a0aec0\" d=\"M6 2L2 8h8z\"/><path fill=\"%23a0aec0\" d=\"M6 18L2 12h8z\"/></svg>'); border-radius: 0 4px 4px 0; }
    .desired-row input[type="number"]::-webkit-inner-spin-button:hover { background-color: #3a4a6e; background-image: url('data:image/svg+xml,<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 12 20\"><path fill=\"%23e94560\" d=\"M6 2L2 8h8z\"/><path fill=\"%23e94560\" d=\"M6 18L2 12h8z\"/></svg>'); border-left-color: #e94560; }
    .total-box.reached { border-color: #48bb78; background: rgba(72, 187, 120, 0.2); }
    .total-box.reached .total-value { color: #48bb78; }
    body.target-reached .reset-btn { background: #48bb78; }
    body.target-reached .reset-btn:hover { background: #5fd08b; }
    body.target-reached .boards-list { scrollbar-color: #48bb78 #16213e; }
    body.target-reached .boards-list::-webkit-scrollbar-thumb { background: linear-gradient(180deg, #48bb78 0%, #38a169 100%); }
    body.target-reached .boards-list::-webkit-scrollbar-thumb:hover { background: linear-gradient(180deg, #5fd08b 0%, #48bb78 100%); }
    body.target-reached .latest-row { border-color: #48bb78; background: rgba(72, 187, 120, 0.2); color: #48bb78; }
  </style>
</head>
<body>
  <header class="header">
    <svg class="logo" viewBox="0 0 64 64" fill="none" xmlns="http://www.w3.org/2000/svg" aria-hidden="true">
      <rect width="64" height="64" rx="12" fill="url(#lg)"/>
      <path d="M12 32h40" stroke="#fff" stroke-width="3" stroke-linecap="round"/>
      <path d="M12 24h32M12 40h28" stroke="rgba(255,255,255,0.7)" stroke-width="2" stroke-linecap="round"/>
      <circle cx="52" cy="32" r="4" fill="#e94560"/>
      <defs><linearGradient id="lg" x1="0" y1="0" x2="64" y2="64" gradientUnits="userSpaceOnUse"><stop stop-color="#0f3460"/><stop offset="1" stop-color="#16213e"/></linearGradient></defs>
    </svg>
    <h1>Stripe Measurement Device</h1>
  </header>
  <div class="main-wrap">
  <div class="main-content">
  <div id="latestRow" class="latest-row"><span id="latestLabel">--</span></div>
  <div class="boards-section">
    <div class="label">Boards measured</div>
    <div id="boardsList" class="boards-list"></div>
  </div>
  <div style="margin-top: 1em;">
    <button id="resetTotalBtn" class="reset-btn" onclick="resetTotal()">Reset Total</button>
    <button class="remove-btn" onclick="removeLastBoard()">Remove last board</button>
  </div>
  <div style="margin-top: 1em;">
    <button onclick="addManual(1.0)">+1.0"</button>
    <button onclick="addManual(3.0)">+3.0"</button>
  </div>
  </div>
  <div class="total-box-wrap">
    <div class="desired-row">
      <label for="desiredTotalFt">Desired total (ft)</label>
      <input type="number" id="desiredTotalFt" min="0" step="0.1">
    </div>
    <div id="totalBox" class="total-box">
      <div class="total-label">Total</div>
      <div id="totalValue" class="total-value">--</div>
    </div>
  </div>
  </div>
  <script>
    var previousBoardCount = 0;
    function fetchData() {
      fetch('/api/data').then(r => r.json()).then(d => {
        var list = document.getElementById('boardsList');
        var latestLabel = document.getElementById('latestLabel');
        var totalValue = document.getElementById('totalValue');
        var boards = d.boards || [];
        var total = d.total !== undefined ? d.total : 0;
        function toIn(inches) { return inches.toFixed(1) + ' in'; }
        function toFt(inches) { return (inches / 12).toFixed(1) + ' ft'; }
        list.innerHTML = '';
        totalValue.textContent = boards.length === 0 ? '--' : toFt(total);
        var desiredFt = parseFloat(document.getElementById('desiredTotalFt').value);
        var desiredIn = (desiredFt > 0) ? desiredFt * 12 : 0;
        var totalBox = document.getElementById('totalBox');
        if (desiredIn > 0 && total >= desiredIn) {
          totalBox.classList.add('reached');
          document.body.classList.add('target-reached');
        } else {
          totalBox.classList.remove('reached');
          document.body.classList.remove('target-reached');
        }
        if (boards.length === 0) {
          latestLabel.textContent = '--';
        } else {
          for (var i = boards.length - 1; i >= 0; i--) {
            var el = document.createElement('div');
            el.className = 'board-item';
            el.textContent = '#' + (i + 1) + '  ' + toIn(boards[i]);
            list.appendChild(el);
          }
          var last = boards[boards.length - 1];
          latestLabel.textContent = '#' + boards.length + '  ' + toIn(last);
        }
        if (boards.length > previousBoardCount) {
          previousBoardCount = boards.length;
          var row = document.getElementById('latestRow');
          row.classList.remove('latest-row-flash');
          row.offsetHeight;
          row.classList.add('latest-row-flash');
          setTimeout(function() { row.classList.remove('latest-row-flash'); }, 500);
        } else {
          previousBoardCount = boards.length;
        }
      });
    }
    function removeLastBoard() {
      fetch('/api/remove-last', { method: 'POST' }).then(function() { fetchData(); });
    }
    function addManual(inches) {
      fetch('/api/add-manual?val=' + inches, { method: 'POST' }).then(function() { fetchData(); });
    }
    function resetTotal() {
      fetch('/api/reset', { method: 'POST' }).then(function() { fetchData(); });
    }
    setInterval(fetchData, 250);
    fetchData();
  </script>
</body>
</html>
)rawliteral";

void handleRoot(AsyncWebServerRequest* request) {
  request->send(200, "text/html", HTML_PAGE);
}

void handleApiData(AsyncWebServerRequest* request) {
  JsonDocument doc;
  doc["sensorValid"] = HasValidSensorReading();
  doc["boardLength"] = GetBoardLengthInches();
  doc["total"] = GetTotalInches();
  JsonArray arr = doc["boards"].to<JsonArray>();
  int n = GetBoardCount();
  for (int i = 0; i < n; i++) {
    arr.add(GetBoardLengthAtIndex(i));
  }

  String buf;
  serializeJson(doc, buf);
  request->send(200, "application/json", buf);
}

void handleApiReset(AsyncWebServerRequest* request) {
  if (request->method() == HTTP_POST) {
    ResetTotal();
  }
  request->send(200);
}

void handleApiRemoveLast(AsyncWebServerRequest* request) {
  if (request->method() == HTTP_POST) {
    RemoveLastBoard();
  }
  request->send(200);
}

void handleApiAddManual(AsyncWebServerRequest* request) {
  if (request->method() == HTTP_POST) {
    if (request->hasParam("val")) {
      float val = request->getParam("val")->value().toFloat();
      AddBoardToList(val);
    }
  }
  request->send(200);
}

void setup() {
  Serial.begin(115200);

  setupOTA();
  SetupIdle();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/data", HTTP_GET, handleApiData);
  server.on("/api/reset", HTTP_POST, handleApiReset);
  server.on("/api/remove-last", HTTP_POST, handleApiRemoveLast);
  server.on("/api/add-manual", HTTP_POST, handleApiAddManual);

  server.begin();
}

void loop() {
  handleOTA();
  RunIdle();
}
