// web_dashboard.h
// Single-file production dashboard, served directly from flash (PROGMEM)
// so V1 needs no filesystem upload step. Served at GET / on port 80.
// Connects to the WebSocket server on port 81 for <10ms telemetry.

#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"HTMLDOC(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>Reaction Timer</title>
<style>
  :root {
    --bg: #0b0d10;
    --panel: #14181d;
    --line: #262c33;
    --text: #e8edf2;
    --muted: #8b96a3;
    --red: #e0302f;
    --red-dim: #3a1414;
    --green: #33c26a;
    --amber: #f0a623;
    --blue: #3a8dff;
  }
  * { box-sizing: border-box; }
  body {
    margin: 0;
    background: var(--bg);
    color: var(--text);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Arial, sans-serif;
    padding: 16px;
    padding-bottom: 48px;
  }
  h1 {
    font-size: 1.1rem;
    text-align: center;
    letter-spacing: 0.08em;
    text-transform: uppercase;
    color: var(--muted);
    margin: 4px 0 20px;
  }
  .status-bar {
    display: flex;
    justify-content: center;
    gap: 10px;
    align-items: center;
    margin-bottom: 20px;
    font-size: 0.8rem;
    color: var(--muted);
  }
  .dot { width: 9px; height: 9px; border-radius: 50%; background: var(--red); }
  .dot.on { background: var(--green); }

  .tree {
    display: flex;
    justify-content: center;
    gap: 14px;
    background: var(--panel);
    border: 1px solid var(--line);
    border-radius: 16px;
    padding: 22px 18px;
    max-width: 520px;
    margin: 0 auto 22px;
  }
  .bulb {
    width: 15vw;
    height: 15vw;
    max-width: 64px;
    max-height: 64px;
    border-radius: 50%;
    background: var(--red-dim);
    border: 3px solid #241010;
    transition: background 60ms linear, box-shadow 60ms linear;
  }
  .bulb.lit {
    background: var(--red);
    box-shadow: 0 0 24px 6px rgba(224,48,47,0.75);
    border-color: #5a1010;
  }

  .lanes {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
    max-width: 640px;
    margin: 0 auto 18px;
  }
  .lane {
    background: var(--panel);
    border: 2px solid var(--line);
    border-radius: 14px;
    padding: 18px 12px;
    text-align: center;
    position: relative;
    overflow: hidden;
  }
  .lane.winner { border-color: var(--green); box-shadow: 0 0 18px rgba(51,194,106,0.35); }
  .lane.falsestart { border-color: var(--red); }
  .lane .name { color: var(--muted); font-size: 0.75rem; text-transform: uppercase; letter-spacing: 0.08em; }
  .lane .time {
    font-variant-numeric: tabular-nums;
    font-size: clamp(1.6rem, 7vw, 2.6rem);
    font-weight: 700;
    margin: 8px 0 4px;
  }
  .lane .tag {
    font-size: 0.75rem;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    min-height: 1em;
  }
  .lane .tag.win { color: var(--green); }
  .lane .tag.fs { color: var(--red); }
  .lane .tag.dnf { color: var(--amber); }
  .badge {
    position: absolute;
    top: 8px;
    right: 10px;
    font-size: 1.3rem;
  }

  .controls {
    display: flex;
    justify-content: center;
    gap: 12px;
    margin: 22px auto;
    max-width: 640px;
    flex-wrap: wrap;
  }
  button {
    font: inherit;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    border: none;
    border-radius: 10px;
    padding: 14px 26px;
    cursor: pointer;
    color: #fff;
  }
  #startBtn { background: var(--green); }
  #startBtn:disabled { background: #2a3630; color: var(--muted); cursor: not-allowed; }
  #resetBtn { background: var(--blue); }

  .alert {
    max-width: 640px;
    margin: 0 auto 18px;
    text-align: center;
    background: var(--red-dim);
    border: 1px solid var(--red);
    color: #ff9d9c;
    border-radius: 10px;
    padding: 10px;
    font-weight: 600;
    display: none;
  }
  .alert.show { display: block; }

  table {
    width: 100%;
    max-width: 640px;
    margin: 0 auto;
    border-collapse: collapse;
    font-size: 0.85rem;
  }
  caption { text-align: left; color: var(--muted); text-transform: uppercase; font-size: 0.75rem; letter-spacing: 0.08em; margin-bottom: 8px; }
  th, td { padding: 6px 8px; border-bottom: 1px solid var(--line); text-align: center; }
  th { color: var(--muted); font-weight: 600; }

  .testmode {
    max-width: 640px;
    margin: 26px auto 0;
    border-top: 1px dashed var(--line);
    padding-top: 12px;
    display: flex;
    justify-content: center;
    align-items: center;
    gap: 10px;
    font-size: 0.8rem;
    color: var(--muted);
  }
</style>
</head>
<body>
  <h1>Head-to-Head Reaction Timer</h1>
  <div class="status-bar">
    <span class="dot" id="connDot"></span>
    <span id="connText">connecting...</span>
    <span>&bull;</span>
    <span id="stateText">IDLE</span>
  </div>

  <div class="tree" id="tree">
    <div class="bulb" data-i="0"></div>
    <div class="bulb" data-i="1"></div>
    <div class="bulb" data-i="2"></div>
    <div class="bulb" data-i="3"></div>
    <div class="bulb" data-i="4"></div>
  </div>

  <div class="alert" id="alertBox"></div>

  <div class="lanes">
    <div class="lane" id="lane1">
      <div class="name">Lane 1</div>
      <div class="time" id="lane1Time">--.--</div>
      <div class="tag" id="lane1Tag"></div>
      <div class="badge" id="lane1Badge"></div>
    </div>
    <div class="lane" id="lane2">
      <div class="name">Lane 2</div>
      <div class="time" id="lane2Time">--.--</div>
      <div class="tag" id="lane2Tag"></div>
      <div class="badge" id="lane2Badge"></div>
    </div>
  </div>

  <div class="controls">
    <button id="startBtn">Start Race</button>
    <button id="resetBtn">Reset</button>
  </div>

  <table>
    <caption>Race History</caption>
    <thead><tr><th>#</th><th>Lane 1</th><th>Lane 2</th><th>Winner</th></tr></thead>
    <tbody id="historyBody"></tbody>
  </table>

  <div class="testmode">
    <label><input type="checkbox" id="testModeToggle"> Test mode (F = Lane 1, J = Lane 2, no hardware needed)</label>
  </div>

<script>
(function () {
  const wsUrl = "ws://" + location.hostname + ":81/";
  let ws;
  let testMode = false;

  const connDot = document.getElementById("connDot");
  const connText = document.getElementById("connText");
  const stateText = document.getElementById("stateText");
  const bulbs = Array.from(document.querySelectorAll(".bulb"));
  const alertBox = document.getElementById("alertBox");
  const startBtn = document.getElementById("startBtn");
  const resetBtn = document.getElementById("resetBtn");
  const historyBody = document.getElementById("historyBody");
  const testModeToggle = document.getElementById("testModeToggle");

  const laneEls = {
    1: { root: document.getElementById("lane1"), time: document.getElementById("lane1Time"), tag: document.getElementById("lane1Tag"), badge: document.getElementById("lane1Badge") },
    2: { root: document.getElementById("lane2"), time: document.getElementById("lane2Time"), tag: document.getElementById("lane2Tag"), badge: document.getElementById("lane2Badge") }
  };

  function connect() {
    ws = new WebSocket(wsUrl);
    ws.onopen = () => { connDot.classList.add("on"); connText.textContent = "connected"; startBtn.disabled = false; };
    ws.onclose = () => { connDot.classList.remove("on"); connText.textContent = "disconnected - retrying..."; startBtn.disabled = true; setTimeout(connect, 1000); };
    ws.onerror = () => ws.close();
    ws.onmessage = (evt) => {
      let msg;
      try { msg = JSON.parse(evt.data); } catch (e) { return; }
      handleMessage(msg);
    };
  }

  function send(obj) {
    if (ws && ws.readyState === WebSocket.OPEN) ws.send(JSON.stringify(obj));
  }

  function resetLaneUI() {
    for (const l of [1, 2]) {
      laneEls[l].root.classList.remove("winner", "falsestart");
      laneEls[l].time.textContent = "--.--";
      laneEls[l].tag.textContent = "";
      laneEls[l].tag.className = "tag";
      laneEls[l].badge.textContent = "";
    }
    alertBox.classList.remove("show");
    alertBox.textContent = "";
  }

  function handleMessage(msg) {
    if (msg.type === "state") {
      stateText.textContent = msg.state;
      bulbs.forEach((b, i) => b.classList.toggle("lit", !!(msg.lights && msg.lights[i])));
      if (msg.state === "SEQUENCE" || msg.state === "HOLD") {
        resetLaneUI();
        startBtn.disabled = true;
      }
      if (msg.state === "IDLE") {
        startBtn.disabled = false;
      }
    } else if (msg.type === "false_start") {
      stateText.textContent = "FALSE START";
      alertBox.textContent = "FALSE START - Lane " + msg.lane;
      alertBox.classList.add("show");
      laneEls[msg.lane].root.classList.add("falsestart");
      laneEls[msg.lane].tag.textContent = "Jumped";
      laneEls[msg.lane].tag.className = "tag fs";
      startBtn.disabled = false;
    } else if (msg.type === "result") {
      stateText.textContent = "FINISHED";
      for (const l of [1, 2]) {
        const r = msg["lane" + l];
        if (!r) continue;
        if (r.dnf) {
          laneEls[l].time.textContent = "--.--";
          laneEls[l].tag.textContent = "DNF";
          laneEls[l].tag.className = "tag dnf";
        } else {
          laneEls[l].time.textContent = r.reactionMs.toFixed(2);
          laneEls[l].tag.textContent = "ms";
          laneEls[l].tag.className = "tag";
        }
      }
      if (msg.winner === 1 || msg.winner === 2) {
        laneEls[msg.winner].root.classList.add("winner");
        laneEls[msg.winner].badge.textContent = "🏆";
        laneEls[msg.winner].tag.textContent = "WINNER";
        laneEls[msg.winner].tag.className = "tag win";
      }
      startBtn.disabled = false;
    } else if (msg.type === "history") {
      historyBody.innerHTML = "";
      msg.races.forEach((r, idx) => {
        const tr = document.createElement("tr");
        const l1 = r.lane1.falseStart ? "FS" : (r.lane1.dnf ? "DNF" : r.lane1.reactionMs.toFixed(2));
        const l2 = r.lane2.falseStart ? "FS" : (r.lane2.dnf ? "DNF" : r.lane2.reactionMs.toFixed(2));
        const win = r.winner ? ("Lane " + r.winner) : "-";
        tr.innerHTML = "<td>" + (idx + 1) + "</td><td>" + l1 + "</td><td>" + l2 + "</td><td>" + win + "</td>";
        historyBody.appendChild(tr);
      });
    }
  }

  startBtn.addEventListener("click", () => { send({ cmd: "start" }); resetLaneUI(); });
  resetBtn.addEventListener("click", () => { send({ cmd: "reset" }); resetLaneUI(); });

  testModeToggle.addEventListener("change", (e) => { testMode = e.target.checked; });
  document.addEventListener("keydown", (e) => {
    if (!testMode) return;
    if (e.repeat) return;
    if (e.key === "f" || e.key === "F") send({ cmd: "simulate", lane: 1 });
    if (e.key === "j" || e.key === "J") send({ cmd: "simulate", lane: 2 });
  });

  connect();
})();
</script>
</body>
</html>
)HTMLDOC";
