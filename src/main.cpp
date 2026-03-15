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
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
    :root {
      --bg: #0d0f14;
      --surface: #161a24;
      --surface2: #1e2334;
      --border: rgba(255,255,255,0.07);
      --accent: #e94560;
      --accent-glow: rgba(233,69,96,0.25);
      --green: #34d399;
      --green-glow: rgba(52,211,153,0.25);
      --text: #f0f4ff;
      --muted: #6b7a99;
      --radius: 12px;
    }
    body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif; background: var(--bg); color: var(--text); min-height: 100vh; padding: 1.5rem 1rem 2rem; }
    .page { max-width: 520px; margin: 0 auto; display: flex; flex-direction: column; gap: 1rem; }

    /* Header */
    .header { display: flex; align-items: center; gap: 0.75rem; padding: 1rem 1.25rem; background: var(--surface); border: 1px solid var(--border); border-radius: var(--radius); }
    .header-icon { width: 38px; height: 38px; flex-shrink: 0; }
    .header-title { font-size: 1.05rem; font-weight: 700; letter-spacing: 0.01em; color: var(--text); }
    .header-sub { font-size: 0.72rem; color: var(--muted); margin-top: 1px; }
    .dot { width: 8px; height: 8px; border-radius: 50%; background: var(--green); margin-left: auto; flex-shrink: 0; box-shadow: 0 0 8px var(--green); animation: pulse 2s infinite; }
    @keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.4} }

    /* Latest measurement card */
    .latest-card { padding: 1.5rem 1.5rem 1.25rem; background: var(--surface); border: 1px solid var(--border); border-radius: var(--radius); position: relative; overflow: hidden; transition: border-color 0.4s; }
    .latest-card::before { content:''; position:absolute; inset:0; background: radial-gradient(ellipse at top left, var(--accent-glow), transparent 65%); pointer-events:none; transition: background 0.4s; }
    body.target-reached .latest-card::before { background: radial-gradient(ellipse at top left, var(--green-glow), transparent 65%); }
    .latest-chip { font-size: 0.62rem; font-weight: 700; text-transform: uppercase; letter-spacing: 0.12em; color: var(--accent); background: rgba(233,69,96,0.12); padding: 0.22em 0.65em; border-radius: 20px; display: inline-block; margin-bottom: 0.6rem; transition: color 0.3s, background 0.3s; }
    body.target-reached .latest-chip { color: var(--green); background: rgba(52,211,153,0.12); }
    .latest-value { font-size: 3rem; font-weight: 800; line-height: 1; letter-spacing: -0.03em; color: var(--text); }
    .latest-sub { font-size: 0.78rem; color: var(--muted); margin-top: 0.4rem; }

    /* Stats row */
    .stats-row { display: grid; grid-template-columns: 1fr 1fr; gap: 0.75rem; }
    .stat-card { padding: 1rem 1.25rem; background: var(--surface); border: 1px solid var(--border); border-radius: var(--radius); }
    .stat-label { font-size: 0.62rem; font-weight: 700; text-transform: uppercase; letter-spacing: 0.1em; color: var(--muted); margin-bottom: 0.4rem; }
    .stat-value { font-size: 1.65rem; font-weight: 800; letter-spacing: -0.02em; color: var(--accent); transition: color 0.3s; }
    body.target-reached .stat-card.total-card .stat-value { color: var(--green); }
    .stat-count { font-size: 1.65rem; font-weight: 800; letter-spacing: -0.02em; color: var(--text); }

    /* Progress bar */
    .progress-wrap { background: var(--surface); border: 1px solid var(--border); border-radius: var(--radius); padding: 1rem 1.25rem; display: none; }
    .progress-wrap.visible { display: block; }
    .progress-header { display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 0.65rem; }
    .progress-label { font-size: 0.62rem; font-weight: 700; text-transform: uppercase; letter-spacing: 0.1em; color: var(--muted); }
    .progress-pct { font-size: 0.85rem; font-weight: 700; color: var(--accent); transition: color 0.3s; }
    body.target-reached .progress-pct { color: var(--green); }
    .progress-track { height: 6px; background: var(--surface2); border-radius: 99px; overflow: hidden; }
    .progress-bar { height: 100%; border-radius: 99px; background: linear-gradient(90deg, #e94560, #ff6b8a); transition: width 0.4s ease, background 0.3s; }
    body.target-reached .progress-bar { background: linear-gradient(90deg, #34d399, #6ee7b7); }

    /* Goal input */
    .goal-card { padding: 1rem 1.25rem; background: var(--surface); border: 1px solid var(--border); border-radius: var(--radius); }
    .goal-label { font-size: 0.62rem; font-weight: 700; text-transform: uppercase; letter-spacing: 0.1em; color: var(--muted); margin-bottom: 0.4rem; }
    input[type="number"] { width: 100%; padding: 0.55em 0.8em; font-size: 1rem; font-weight: 600; background: var(--surface2); border: 1px solid var(--border); border-radius: 8px; color: var(--text); outline: none; -moz-appearance: textfield; transition: border-color 0.2s; }
    input[type="number"]::-webkit-inner-spin-button, input[type="number"]::-webkit-outer-spin-button { -webkit-appearance: none; display: none; }
    input[type="number"]:focus { border-color: rgba(233,69,96,0.5); }
    input[type="number"]::placeholder { color: var(--muted); font-weight: 400; }

    /* Boards list */
    .boards-card { background: var(--surface); border: 1px solid var(--border); border-radius: var(--radius); overflow: hidden; }
    .boards-header { padding: 0.8rem 1.25rem; border-bottom: 1px solid var(--border); }
    .boards-title { font-size: 0.62rem; font-weight: 700; text-transform: uppercase; letter-spacing: 0.1em; color: var(--muted); }
    .boards-list { max-height: 200px; overflow-y: auto; scrollbar-width: thin; scrollbar-color: rgba(233,69,96,0.4) transparent; }
    .boards-list::-webkit-scrollbar { width: 3px; }
    .boards-list::-webkit-scrollbar-track { background: transparent; }
    .boards-list::-webkit-scrollbar-thumb { background: rgba(233,69,96,0.4); border-radius: 99px; }
    .board-item { display: flex; align-items: center; gap: 0.75rem; padding: 0.6rem 1.25rem; border-bottom: 1px solid var(--border); }
    .board-item:last-child { border-bottom: none; }
    .board-idx { font-size: 0.68rem; font-weight: 700; color: var(--muted); width: 1.8rem; flex-shrink: 0; text-align: right; }
    .board-len { font-size: 0.9rem; font-weight: 600; color: var(--text); min-width: 3.5rem; }
    .board-bar-wrap { flex: 1; height: 3px; background: var(--surface2); border-radius: 99px; overflow: hidden; }
    .board-bar { height: 100%; border-radius: 99px; background: var(--accent); opacity: 0.35; }
    .board-item:first-child .board-bar { opacity: 0.85; }
    .boards-empty { padding: 1.75rem 1.25rem; text-align: center; font-size: 0.85rem; color: var(--muted); }

    /* Buttons */
    .btn-row { display: flex; gap: 0.5rem; }
    .btn { flex: 1; padding: 0.7em 1em; font-size: 0.875rem; font-weight: 700; cursor: pointer; border: none; border-radius: 8px; transition: filter 0.15s, transform 0.1s; letter-spacing: 0.01em; font-family: inherit; }
    .btn:active { transform: scale(0.97); }
    .btn-danger { background: var(--accent); color: #fff; }
    .btn-danger:hover { filter: brightness(1.15); }
    body.target-reached .btn-danger { background: var(--green); color: #0d1f14; }
    .btn-ghost { background: var(--surface2); color: var(--muted); border: 1px solid var(--border); }
    .btn-ghost:hover { color: var(--text); border-color: rgba(255,255,255,0.14); }
    .btn-outline { background: transparent; color: var(--text); border: 1px solid var(--border); flex: 0 0 auto; padding: 0.7em 1.25em; font-family: inherit; }
    .btn-outline:hover { border-color: rgba(255,255,255,0.18); background: var(--surface2); }

    /* Flash animations */
    @keyframes flashRed {
      0%   { box-shadow: none; background: var(--surface); transform: scale(1); }
      12%  { box-shadow: 0 0 0 5px rgba(233,69,96,0.9), 0 0 60px 20px rgba(233,69,96,0.5); background: rgba(233,69,96,0.22); transform: scale(1.03); }
      35%  { box-shadow: 0 0 0 3px rgba(233,69,96,0.5), 0 0 40px 10px rgba(233,69,96,0.25); background: rgba(233,69,96,0.08); transform: scale(1.01); }
      100% { box-shadow: none; background: var(--surface); transform: scale(1); }
    }
    @keyframes flashGreen {
      0%   { box-shadow: none; background: var(--surface); transform: scale(1); }
      12%  { box-shadow: 0 0 0 5px rgba(52,211,153,0.9), 0 0 60px 20px rgba(52,211,153,0.5); background: rgba(52,211,153,0.22); transform: scale(1.03); }
      35%  { box-shadow: 0 0 0 3px rgba(52,211,153,0.5), 0 0 40px 10px rgba(52,211,153,0.25); background: rgba(52,211,153,0.08); transform: scale(1.01); }
      100% { box-shadow: none; background: var(--surface); transform: scale(1); }
    }
    .flash-red   { animation: flashRed   1.5s cubic-bezier(0.22, 1, 0.36, 1); }
    .flash-green { animation: flashGreen 1.5s cubic-bezier(0.22, 1, 0.36, 1); }
  </style>
</head>
<body>
  <div class="page">

    <header class="header">
      <svg class="header-icon" viewBox="0 0 38 38" fill="none" xmlns="http://www.w3.org/2000/svg">
        <rect width="38" height="38" rx="9" fill="url(#hg)"/>
        <path d="M7 19h24" stroke="#fff" stroke-width="2.2" stroke-linecap="round"/>
        <path d="M7 13h18M7 25h14" stroke="rgba(255,255,255,0.45)" stroke-width="1.6" stroke-linecap="round"/>
        <circle cx="31" cy="19" r="3" fill="#e94560"/>
        <defs><linearGradient id="hg" x1="0" y1="0" x2="38" y2="38" gradientUnits="userSpaceOnUse"><stop stop-color="#1e2d5a"/><stop offset="1" stop-color="#0d1120"/></linearGradient></defs>
      </svg>
      <div>
        <div class="header-title">Stripe Measurement</div>
        <div class="header-sub">Live board tracker</div>
      </div>
      <div class="dot"></div>
    </header>

    <div id="latestCard" class="latest-card">
      <div class="latest-chip">Latest board</div>
      <div id="latestValue" class="latest-value">--</div>
      <div id="latestSub" class="latest-sub">&nbsp;</div>
    </div>

    <div class="stats-row">
      <div class="stat-card total-card">
        <div class="stat-label">Total length</div>
        <div id="totalValue" class="stat-value">--</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Boards</div>
        <div id="countValue" class="stat-count">0</div>
      </div>
    </div>

    <div class="goal-card">
      <div class="goal-label">Target total (ft)</div>
      <input type="number" id="desiredTotalFt" min="0" step="0.1" placeholder="e.g. 20">
    </div>

    <div id="progressWrap" class="progress-wrap">
      <div class="progress-header">
        <div class="progress-label">Progress to target</div>
        <div id="progressPct" class="progress-pct">0%</div>
      </div>
      <div class="progress-track">
        <div id="progressBar" class="progress-bar" style="width:0%"></div>
      </div>
    </div>

    <div class="boards-card">
      <div class="boards-header">
        <div class="boards-title">Board log</div>
      </div>
      <div id="boardsList" class="boards-list">
        <div class="boards-empty">No boards measured yet</div>
      </div>
    </div>

    <div class="btn-row">
      <button class="btn btn-danger" onclick="resetTotal()">Reset</button>
      <button class="btn btn-ghost" onclick="removeLastBoard()">Remove last</button>
    </div>
    <div class="btn-row">
      <button class="btn btn-outline" onclick="addManual(1.0)">+ 1.0&quot;</button>
      <button class="btn btn-outline" onclick="addManual(3.0)">+ 3.0&quot;</button>
    </div>

  </div>
  <script>
    var prevCount = 0;
    var maxLen = 0;
    function toIn(v) { return v.toFixed(1) + '"'; }
    function toFt(v) { return (v / 12).toFixed(2) + ' ft'; }
    function fetchData() {
      fetch('/api/data').then(function(r){ return r.json(); }).then(function(d) {
        var boards = d.boards || [];
        var total  = d.total || 0;
        var n      = boards.length;
        var latestVal = document.getElementById('latestValue');
        var latestSub = document.getElementById('latestSub');
        if (n === 0) {
          latestVal.textContent = '--';
          latestSub.innerHTML = '&nbsp;';
        } else {
          var last = boards[n - 1];
          latestVal.textContent = toIn(last);
          latestSub.textContent = 'Board #' + n + '  \u2022  ' + (last / 12).toFixed(3) + ' ft';
        }
        document.getElementById('totalValue').textContent = n === 0 ? '--' : toFt(total);
        document.getElementById('countValue').textContent = n;
        var desiredFt = parseFloat(document.getElementById('desiredTotalFt').value);
        var desiredIn = (desiredFt > 0) ? desiredFt * 12 : 0;
        var reached   = desiredIn > 0 && total >= desiredIn;
        var pw = document.getElementById('progressWrap');
        if (desiredIn > 0) {
          var pct = Math.min(100, (total / desiredIn) * 100);
          pw.classList.add('visible');
          document.getElementById('progressBar').style.width = pct.toFixed(1) + '%';
          document.getElementById('progressPct').textContent  = pct.toFixed(0) + '%';
        } else {
          pw.classList.remove('visible');
        }
        if (reached) { document.body.classList.add('target-reached'); }
        else          { document.body.classList.remove('target-reached'); }
        var list = document.getElementById('boardsList');
        if (n === 0) {
          list.innerHTML = '<div class="boards-empty">No boards measured yet</div>';
          maxLen = 0;
        } else {
          maxLen = 0;
          for (var i = 0; i < n; i++) { if (boards[i] > maxLen) maxLen = boards[i]; }
          list.innerHTML = '';
          for (var i = n - 1; i >= 0; i--) {
            var el = document.createElement('div');
            el.className = 'board-item';
            var barPct = maxLen > 0 ? (boards[i] / maxLen * 100).toFixed(1) : 0;
            el.innerHTML =
              '<span class="board-idx">#' + (i + 1) + '</span>' +
              '<span class="board-len">' + toIn(boards[i]) + '</span>' +
              '<div class="board-bar-wrap"><div class="board-bar" style="width:' + barPct + '%"></div></div>';
            list.appendChild(el);
          }
        }
        if (n > prevCount) {
          var card = document.getElementById('latestCard');
          card.classList.remove('flash-red', 'flash-green');
          card.offsetHeight;
          card.classList.add(reached ? 'flash-green' : 'flash-red');
          setTimeout(function(){ card.classList.remove('flash-red','flash-green'); }, 1500);
        }
        prevCount = n;
      });
    }
    function removeLastBoard() { fetch('/api/remove-last', { method: 'POST' }).then(fetchData); }
    function addManual(v)      { fetch('/api/add-manual?val=' + v, { method: 'POST' }).then(fetchData); }
    function resetTotal()      { fetch('/api/reset', { method: 'POST' }).then(fetchData); }
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
