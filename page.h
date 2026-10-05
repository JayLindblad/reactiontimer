/*
  page.h — web page for the ESP8266 Reaction Trainer (1–4 lanes + button test)
  Replace the entire contents of your page.h tab with this file.
*/
#pragma once
#include <Arduino.h>
const char PAGE[] PROGMEM = R"PAGE(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover">
<meta name="theme-color" content="#07080c">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="apple-mobile-web-app-title" content="Reaction">
<title>Reaction Trainer</title>
<style>
:root{--bg:#07080c;--panel:#0f1118;--line:rgba(255,255,255,.08);--txt:#eef1f7;--dim:#858b9d;--red:#ff2231;--green:#1fe06a;--amber:#ffb020;--accent:#e6007e;--lane0:#e6007e;--lane1:#27d3ff;--lane2:#ff6a1a;--lane3:#8b5cf6}
*{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent}
html{-webkit-text-size-adjust:100%}
html,body{height:100%}
body{background:radial-gradient(ellipse at 40% -10%,#1a1030 0%,var(--bg) 55%) fixed,var(--bg);color:var(--txt);font-family:system-ui,-apple-system,"Segoe UI",sans-serif;overflow:hidden}
.app{display:grid;grid-template-columns:1fr 330px;height:100%}
main{display:flex;flex-direction:column;padding:24px 32px 18px;gap:22px;min-width:0}
body.many main{gap:16px;padding-top:18px}
header{display:flex;align-items:center;justify-content:space-between;gap:16px}
.brand{font-weight:900;font-style:italic;letter-spacing:.05em;font-size:22px;white-space:nowrap}
.brand span{color:var(--accent)}
.hdr-r{display:flex;align-items:center;gap:16px}
.conn{display:flex;align-items:center;gap:8px;font-size:12px;letter-spacing:.14em;color:var(--dim);font-weight:700}
.conn i{width:9px;height:9px;border-radius:50%;background:var(--amber);flex:none}
.conn.ok i{background:var(--green);box-shadow:0 0 10px var(--green)}
.conn.bad i{background:var(--red)}
.status{font-weight:800;letter-spacing:.22em;font-size:13px;padding:8px 18px;border-radius:99px;border:1px solid var(--line);color:var(--dim);min-width:120px;text-align:center;white-space:nowrap}
.status.live{color:var(--amber);border-color:var(--amber)}
.status.go{color:var(--green);border-color:var(--green);box-shadow:0 0 20px rgba(31,224,106,.35)}

.gantry-wrap{display:flex;flex-direction:column;align-items:center}
.gantry{display:flex;gap:clamp(12px,2.2vw,34px);padding:clamp(16px,2.2vw,32px) clamp(20px,3vw,44px);background:linear-gradient(#171922,#0a0b10);border:1px solid var(--line);border-radius:22px 22px 0 0;box-shadow:0 30px 80px rgba(0,0,0,.6),inset 0 1px 0 rgba(255,255,255,.06)}
body.many .gantry{padding:clamp(12px,1.6vw,24px) clamp(16px,2.4vw,36px)}
.checker{height:14px;width:100%;background:repeating-conic-gradient(#e9ecf3 0 25%,#111 0 50%) 0 0/14px 14px;border-radius:0 0 8px 8px;opacity:.85}
.gnote{font-size:11px;letter-spacing:.18em;color:var(--dim);margin-top:10px;text-align:center}
.lamp{width:clamp(56px,8.5vw,124px);aspect-ratio:1;border-radius:50%;background:radial-gradient(circle at 35% 30%,#3a0e13,#160406 70%);box-shadow:inset 0 -6px 14px rgba(0,0,0,.7),0 0 0 6px #040406,0 0 0 7px rgba(255,255,255,.05)}
body.many .lamp{width:clamp(44px,6vw,96px)}
.lamp.on{background:radial-gradient(circle at 35% 30%,#ffc2c6,#ff2231 45%,#9a000b 100%);box-shadow:inset 0 -6px 14px rgba(0,0,0,.3),0 0 0 6px #040406,0 0 40px var(--red),0 0 100px rgba(255,34,49,.55)}
.lamp.amber{background:radial-gradient(circle at 35% 30%,#ffe7b0,#ffb020 45%,#9a5a00 100%);box-shadow:inset 0 -6px 14px rgba(0,0,0,.3),0 0 0 6px #040406,0 0 40px var(--amber),0 0 90px rgba(255,176,32,.45)}

.lanes{display:grid;grid-template-columns:1fr 1fr;gap:22px;flex:1;min-height:0}
.lanes.n1{grid-template-columns:1fr}
.lanes.n3{grid-template-columns:repeat(3,1fr);gap:16px}
.lanes.n4{grid-template-rows:1fr 1fr;gap:16px}
#lane0{--lc:var(--lane0)} #lane1{--lc:var(--lane1)} #lane2{--lc:var(--lane2)} #lane3{--lc:var(--lane3)}
.lane{background:linear-gradient(180deg,rgba(255,255,255,.035),rgba(255,255,255,.01));border:1px solid var(--line);border-radius:18px;padding:20px 24px;display:flex;flex-direction:column;gap:8px;position:relative;overflow:hidden;min-height:0}
.lanes.n3 .lane,.lanes.n4 .lane{padding:14px 20px;gap:6px}
.lane::before{content:"";position:absolute;left:0;top:0;bottom:0;width:5px;background:var(--lc);box-shadow:0 0 18px var(--lc)}
.lh{display:flex;justify-content:space-between;align-items:baseline;gap:12px}
.tag{font-size:11px;letter-spacing:.2em;color:var(--lc);font-weight:800;white-space:nowrap}
.nm{font-size:clamp(18px,2vw,28px);font-weight:800;outline:none;padding:2px 8px;margin-left:-8px;border-radius:8px;min-width:60px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.lanes.n3 .nm,.lanes.n4 .nm{font-size:clamp(16px,1.6vw,22px)}
.nm:hover{background:rgba(255,255,255,.05)}
.nm:focus{background:rgba(255,255,255,.1)}
.mid{flex:1;display:flex;flex-direction:column;justify-content:center;min-height:0}
.chart{width:100%;height:clamp(90px,19vh,210px);display:block;overflow:visible}
.lanes.n3 .chart{height:clamp(80px,16vh,180px)}
.lanes.n4 .chart{height:clamp(64px,10.5vh,150px)}
.chart text{font-family:system-ui,sans-serif;font-size:11px;fill:var(--dim);font-variant-numeric:tabular-nums}
.chart .ce{font-size:13px}
.chart .grid{stroke:rgba(255,255,255,.07);stroke-width:1}
.chart .ln{fill:none;stroke:var(--lc);stroke-width:2.5;stroke-linejoin:round;stroke-linecap:round;filter:drop-shadow(0 0 6px var(--lc))}
.chart .pt{fill:var(--bg);stroke:var(--lc);stroke-width:2.5}
.chart .pt.last{fill:var(--lc)}
.chart .pt.anti{stroke:var(--amber)}
.chart .jx{stroke:var(--red);stroke-width:2.5;stroke-linecap:round}
.chart .best{stroke:var(--green);stroke-width:1.2;stroke-dasharray:4 5;opacity:.8}
.chart .avg{stroke:rgba(255,255,255,.45);stroke-width:1.2;stroke-dasharray:2 5}
.chart .tb{fill:var(--green)} .chart .ta{fill:rgba(255,255,255,.6)} .chart .tv{fill:var(--txt);font-weight:800;font-size:12px} .chart .tj{fill:var(--red);font-weight:800;font-size:9px;letter-spacing:.1em}
.hidden{display:none!important}
.rt{font-size:clamp(46px,7.2vw,118px);font-weight:900;font-variant-numeric:tabular-nums;letter-spacing:-.02em;line-height:1}
.lanes.n3 .rt,.lanes.n4 .rt{font-size:clamp(38px,5vw,88px)}
.rt small{font-size:.35em;color:var(--dim);font-weight:700;margin-left:.15em}
.rt.jump{color:var(--red);font-size:clamp(34px,4.8vw,76px)}
.lanes.n3 .rt.jump,.lanes.n4 .rt.jump{font-size:clamp(26px,3.2vw,54px)}
.rt.anti{color:var(--amber)}
.rt.none{color:var(--dim);font-size:clamp(28px,3.6vw,56px)}
.lanes.n3 .rt.none,.lanes.n4 .rt.none{font-size:clamp(22px,2.6vw,42px)}
.rt.pb{color:var(--green);text-shadow:0 0 30px rgba(31,224,106,.45)}
.badge{display:none;font-size:10px;font-weight:900;letter-spacing:.18em;color:#04140a;background:var(--green);padding:4px 10px;border-radius:99px;white-space:nowrap;box-shadow:0 0 16px rgba(31,224,106,.6)}
.lane.win{border-color:var(--green);box-shadow:0 0 0 1px var(--green),0 0 44px rgba(31,224,106,.28),inset 0 0 70px rgba(31,224,106,.07);animation:winpop .5s cubic-bezier(.2,1.6,.4,1)}
.lane.win .badge{display:inline-block}
.lane.win .tag{display:none}
.lane.win .rt:not(.jump):not(.none){color:var(--green);text-shadow:0 0 30px rgba(31,224,106,.45)}
@keyframes winpop{0%{transform:scale(.97)}60%{transform:scale(1.015)}100%{transform:scale(1)}}
.sub{font-size:14px;color:var(--dim);min-height:20px;margin-top:6px;letter-spacing:.04em}
.lanes.n3 .sub,.lanes.n4 .sub{font-size:12px;margin-top:4px}
.sub.pb{color:var(--green);font-weight:800;letter-spacing:.18em}
.sub.jump{color:var(--red);font-weight:700}
.sub.anti{color:var(--amber)}
.stats{display:grid;grid-template-columns:repeat(4,1fr);gap:10px;padding-top:14px;border-top:1px solid var(--line)}
.lanes.n3 .stats,.lanes.n4 .stats{padding-top:10px;gap:6px}
.stats b{display:block;font-size:clamp(16px,1.6vw,22px);font-variant-numeric:tabular-nums}
.lanes.n3 .stats b,.lanes.n4 .stats b{font-size:clamp(14px,1.3vw,18px)}
.stats small{font-size:10px;letter-spacing:.16em;color:var(--dim)}
.lanes.n3 .stats small,.lanes.n4 .stats small{font-size:9px;letter-spacing:.1em}

.controls{display:flex;gap:10px;align-items:center;flex-wrap:wrap}
button{font:inherit;font-size:13px;font-weight:700;letter-spacing:.06em;color:var(--txt);background:var(--panel);border:1px solid var(--line);padding:10px 16px;border-radius:10px;cursor:pointer;transition:border-color .15s,background .15s,transform .08s;touch-action:manipulation;display:inline-flex;align-items:center;justify-content:center;gap:6px}
button:hover{border-color:rgba(255,255,255,.25)}
button:active{transform:scale(.97)}
button:disabled{opacity:.35;cursor:not-allowed}
button.primary{background:var(--accent);border-color:var(--accent)}
button.on{border-color:var(--green);color:var(--green)}
.ic{width:20px;height:20px;display:none;fill:none;stroke:currentColor;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.ic.f{fill:currentColor;stroke:none}
kbd{font-family:inherit;font-size:11px;opacity:.6;border:1px solid currentColor;border-radius:4px;padding:0 5px}
.hint{margin-left:auto;font-size:12px;color:var(--dim)}

aside{border-left:1px solid var(--line);background:rgba(0,0,0,.25);padding:24px 22px;display:flex;flex-direction:column;gap:26px;overflow-y:auto}
aside h3{font-size:11px;letter-spacing:.24em;color:var(--dim);margin-bottom:10px}
ol,ul{list-style:none}
.lb li{display:grid;grid-template-columns:26px 1fr auto;gap:8px;align-items:center;padding:8px 10px;border-radius:8px;font-size:14px}
.lb li:nth-child(odd){background:rgba(255,255,255,.03)}
.lb .rk{color:var(--dim);font-weight:800}
.lb li:first-child .rk{color:var(--amber)}
.lb b{font-variant-numeric:tabular-nums}
.lb .av{grid-column:2/4;font-size:11px;color:var(--dim);margin-top:-4px}
.recent li{display:flex;align-items:center;gap:8px;padding:6px 2px;font-size:13px;border-bottom:1px solid var(--line)}
.recent li span{flex:1;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.dot{width:8px;height:8px;border-radius:50%;flex:none}
.recent b{font-variant-numeric:tabular-nums}
.recent .jump{color:var(--red)}.recent .anti{color:var(--amber)}
.lb li.empty,.recent li.empty{display:block;background:none;border:1px dashed rgba(255,255,255,.12);border-radius:10px;padding:14px 12px;text-align:center;font-size:12px;letter-spacing:.08em;color:var(--dim)}

/* ---------- Sheets (settings + button test) ---------- */
.sheet{position:fixed;inset:0;background:rgba(0,0,0,.6);display:none;align-items:center;justify-content:center;z-index:30}
.sheet.on{display:flex}
.card{background:#12141d;border:1px solid rgba(255,255,255,.12);border-radius:16px;padding:24px 26px;width:min(480px,92vw);max-height:92vh;overflow-y:auto;-webkit-overflow-scrolling:touch;display:flex;flex-direction:column;gap:14px;box-shadow:0 30px 80px rgba(0,0,0,.7)}
.card-h{display:flex;justify-content:space-between;align-items:center}
.card h2{font-size:16px;letter-spacing:.12em}
.card .x{padding:6px 10px;font-size:16px;line-height:1}
.card h4{font-size:11px;letter-spacing:.2em;color:var(--dim);margin-top:6px}
#colorRows{display:flex;flex-direction:column;gap:14px}
.row{display:flex;justify-content:space-between;align-items:center;gap:14px;font-size:14px}
.row span small{display:block;color:var(--dim);font-size:11px}
.row input[type=number]{width:80px}
input,select{font:inherit;font-size:14px;background:#0a0b10;color:var(--txt);border:1px solid rgba(255,255,255,.15);border-radius:8px;padding:6px 8px}
input[type=checkbox]{width:20px;height:20px;accent-color:var(--accent)}
.colors{display:flex;align-items:center;gap:8px;flex-wrap:wrap;justify-content:flex-end}
.sw{width:24px;height:24px;border-radius:50%;border:2px solid transparent;cursor:pointer;padding:0}
.sw:hover{border-color:#fff}
input[type=color]{width:36px;height:30px;padding:2px;cursor:pointer}
.card .btns{display:flex;justify-content:space-between;gap:10px;margin-top:6px}
#clear.arm{border-color:var(--red);color:var(--red)}

/* ---------- Button test ---------- */
.dcard{width:min(680px,94vw)}
.dnote{font-size:13px;color:var(--dim);line-height:1.45}
.dgrid{display:grid;grid-template-columns:repeat(5,1fr);gap:10px}
.tile{--tc:var(--green);border:1px solid var(--line);border-radius:14px;padding:14px 8px 12px;text-align:center;background:rgba(255,255,255,.03);transition:background .06s,border-color .06s,box-shadow .06s}
.tile .tn{font-size:11px;font-weight:800;letter-spacing:.14em;color:var(--tc)}
.tile .tg{font-size:10px;color:var(--dim);letter-spacing:.1em;margin-top:2px}
.tile .ts{font-size:13px;font-weight:900;letter-spacing:.1em;margin:14px 0 10px;color:var(--dim)}
.tile .tcnt{font-size:30px;font-weight:900;font-variant-numeric:tabular-nums;line-height:1}
.tile .tcl{font-size:9px;color:var(--dim);letter-spacing:.16em;margin-top:4px}
.tile .te{font-size:10px;color:var(--dim);margin-top:10px;min-height:13px;font-variant-numeric:tabular-nums}
.tile.down{background:color-mix(in srgb,var(--tc) 22%,transparent);border-color:var(--tc);box-shadow:0 0 26px color-mix(in srgb,var(--tc) 45%,transparent)}
.tile.down .ts{color:var(--tc)}
.dstat{display:flex;flex-wrap:wrap;gap:8px 18px;font-size:12px;color:var(--dim);padding:10px 12px;border:1px solid var(--line);border-radius:10px}
.dstat i{display:inline-block;width:8px;height:8px;border-radius:50%;background:var(--green);margin-right:6px;box-shadow:0 0 8px var(--green)}
.dhelp{font-size:12px;color:var(--dim);display:flex;flex-direction:column;gap:6px;line-height:1.45}
.dhelp b{color:var(--txt)}
#stOut{flex-direction:column;gap:4px}
#stOut b.ok{color:var(--green)} #stOut b.bad{color:var(--red)} #stOut b.warn{color:var(--amber)}

/* ---------- Tablet & small laptop: page scrolls, lanes 2-up ---------- */
@media (max-width:900px){
  html,body{height:auto}
  body{overflow-x:hidden;overflow-y:auto}
  .app{display:block;height:auto}
  main,body.many main{padding:16px 16px 0;gap:16px}
  .lanes,.lanes.n3,.lanes.n4{grid-template-columns:1fr 1fr;grid-template-rows:none;flex:none;gap:14px}
  .lanes.n1{grid-template-columns:1fr}
  #lanes .lane{padding:14px 16px 12px 18px}
  #lanes .chart{height:clamp(110px,16vh,160px)}
  #lanes .rt{font-size:clamp(40px,7vw,72px)}
  #lanes .rt.jump{font-size:clamp(28px,4.4vw,44px)}
  #lanes .rt.none{font-size:clamp(24px,3.6vw,34px)}
  aside{display:grid;grid-template-columns:1fr 1fr;gap:14px;border-left:0;background:none;overflow:visible;padding:14px 16px calc(96px + env(safe-area-inset-bottom))}
  aside>div{background:rgba(255,255,255,.025);border:1px solid var(--line);border-radius:16px;padding:16px}
  .controls{position:fixed;left:0;right:0;bottom:0;z-index:20;flex-wrap:nowrap;gap:8px;padding:10px 12px calc(10px + env(safe-area-inset-bottom));background:rgba(7,8,12,.86);backdrop-filter:blur(16px);-webkit-backdrop-filter:blur(16px);border-top:1px solid var(--line)}
  .controls button{padding:14px 14px;border-radius:12px}
  #bStart{flex:1;font-size:16px;letter-spacing:.14em;padding:16px}
  kbd,.hint{display:none}
}

/* ---------- Phone: everything stacks ---------- */
@media (max-width:600px){
  main,body.many main{padding:calc(12px + env(safe-area-inset-top)) 12px 0;gap:14px}
  header{flex-wrap:wrap;gap:10px}
  .brand{font-size:18px}
  .hdr-r{width:100%;justify-content:space-between;gap:10px}
  .conn{font-size:10px;letter-spacing:.1em}
  .status{min-width:0;padding:6px 14px;font-size:11px;letter-spacing:.18em}
  .gantry,body.many .gantry{gap:9px;padding:14px 16px;border-radius:16px 16px 0 0}
  .lamp,body.many .lamp{width:clamp(36px,12.5vw,62px);box-shadow:inset 0 -4px 10px rgba(0,0,0,.7),0 0 0 4px #040406}
  .lamp.on{box-shadow:inset 0 -4px 10px rgba(0,0,0,.3),0 0 0 4px #040406,0 0 24px var(--red),0 0 60px rgba(255,34,49,.55)}
  .lamp.amber{box-shadow:inset 0 -4px 10px rgba(0,0,0,.3),0 0 0 4px #040406,0 0 24px var(--amber),0 0 50px rgba(255,176,32,.45)}
  .checker{height:10px;background-size:10px 10px}
  .gnote{font-size:9px;letter-spacing:.12em}
  .lanes,.lanes.n1,.lanes.n3,.lanes.n4{grid-template-columns:1fr;gap:12px}
  #lanes .lane{border-radius:16px}
  #lanes .nm{font-size:20px}
  .tag{font-size:10px}
  #lanes .chart{height:120px}
  #lanes .rt{font-size:clamp(48px,17vw,76px)}
  #lanes .rt.jump{font-size:clamp(30px,10vw,46px)}
  #lanes .rt.none{font-size:clamp(26px,8vw,36px)}
  #lanes .sub{font-size:12px}
  #lanes .stats{gap:6px;padding-top:10px}
  #lanes .stats b{font-size:16px}
  #lanes .stats small{font-size:8.5px;letter-spacing:.08em}
  aside{grid-template-columns:1fr;padding-left:12px;padding-right:12px}
  .controls .tx{display:none}
  .controls #bStart .tx{display:inline}
  .controls .ic{display:block}
  .controls button:not(#bStart){width:48px;padding:14px 0}

  /* sheets become bottom sheets */
  .sheet{align-items:flex-end}
  .card,.dcard{width:100%;max-height:90vh;border-radius:22px 22px 0 0;border-bottom:0;padding:10px 18px calc(18px + env(safe-area-inset-bottom))}
  .card::before{content:"";width:40px;height:5px;border-radius:3px;background:rgba(255,255,255,.2);align-self:center;margin-bottom:4px}
  .row{flex-wrap:wrap;gap:8px 14px;padding:4px 0}
  .row>span{flex:1 1 55%}
  input,select{font-size:16px;padding:10px 10px}
  .row input[type=number]{width:96px}
  select{max-width:100%}
  .colors{width:100%;justify-content:flex-start;gap:10px}
  .sw{width:32px;height:32px}
  input[type=color]{width:44px;height:36px}
  .card .btns{position:sticky;bottom:calc(-18px - env(safe-area-inset-bottom));background:#12141d;padding:12px 0 calc(6px + env(safe-area-inset-bottom));margin-top:4px}
  .card .btns button{flex:1;padding:14px 8px;font-size:12px}
  .dgrid{grid-template-columns:repeat(2,1fr)}
  .tile:last-child{grid-column:span 2}
  .tile .ts{margin:10px 0 8px}
}
</style>
</head>
<body>
<div class="app">
<main>
  <header>
    <div class="brand">REACTION<span>/</span>TRAINER</div>
    <div class="hdr-r"><div class="conn" id="conn"><i></i><span>CONNECTING…</span></div><div class="status" id="status">READY</div></div>
  </header>

  <div class="gantry-wrap">
    <div class="gantry">
      <div class="lamp"></div><div class="lamp"></div><div class="lamp"></div><div class="lamp"></div><div class="lamp"></div>
    </div>
    <div class="checker"></div>
    <div class="gnote">REACT TO THESE LIGHTS · DRAWN ON THE ESP'S CLOCK</div>
  </div>

  <section class="lanes n2" id="lanes"></section>

  <div class="controls">
    <button class="primary" id="bStart" disabled><span class="tx">START</span><kbd>SPACE</kbd></button>
    <button id="bAbort" title="Abort" aria-label="Abort"><svg class="ic f" viewBox="0 0 24 24"><rect x="6" y="6" width="12" height="12" rx="2"/></svg><span class="tx">ABORT</span><kbd>ESC</kbd></button>
    <button id="bAuto" title="Auto mode" aria-label="Auto mode"><svg class="ic" viewBox="0 0 24 24"><path d="M4 12a8 8 0 0 1 14-5.3M20 12a8 8 0 0 1-14 5.3"/><path d="M18 3v4h-4M6 21v-4h4"/></svg><span class="tx">AUTO</span><kbd>A</kbd></button>
    <button id="bTest" title="Button test" aria-label="Button test"><svg class="ic" viewBox="0 0 24 24"><path d="M3 12h4l3-7 4 14 3-7h4"/></svg><span class="tx">TEST</span><kbd>T</kbd></button>
    <button id="bSet" title="Settings" aria-label="Settings"><svg class="ic" viewBox="0 0 24 24"><path d="M4 6h10M18 6h2M4 12h4M12 12h8M4 18h12"/><circle cx="16" cy="6" r="2"/><circle cx="10" cy="12" r="2"/><circle cx="18" cy="18" r="2"/></svg><span class="tx">SETTINGS</span><kbd>S</kbd></button>
    <button id="bFs" title="Fullscreen" aria-label="Fullscreen"><svg class="ic" viewBox="0 0 24 24"><path d="M4 9V4h5M20 9V4h-5M4 15v5h5M20 15v5h-5"/></svg><span class="tx">FULLSCREEN</span><kbd>F</kbd></button>
    <span class="hint">Times are measured on the ESP8266 · Click a name to change driver</span>
  </div>
</main>

<aside>
  <div><h3>LEADERBOARD · BEST</h3><ol class="lb" id="lb"></ol></div>
  <div><h3>RECENT</h3><ul class="recent" id="recent"></ul></div>
</aside>
</div>

<!-- Settings -->
<div id="settings" class="sheet">
  <div class="card">
    <div class="card-h"><h2>SETTINGS</h2><button class="x" id="sX" aria-label="Close">✕</button></div>
    <h4>RACE</h4>
    <div class="row"><span>Lanes</span><select id="sLanes"><option value="1">1 lane</option><option value="2">2 lanes</option><option value="3">3 lanes</option><option value="4">4 lanes</option></select></div>
    <div class="row"><span>Solid LED hold — min (s)<small>Random time the LED stays on before going out</small></span><input type="number" inputmode="decimal" id="sMin" step="0.1" min="0.3" max="10"></div>
    <div class="row"><span>Solid LED hold — max (s)</span><input type="number" inputmode="decimal" id="sMax" step="0.1" min="0.3" max="10"></div>
    <div class="row"><span>Anticipation threshold (ms)<small>Faster than this is flagged as a guess. 0 = off</small></span><input type="number" inputmode="numeric" id="sAnti" step="10" min="0"></div>
    <div class="row"><span>Auto mode restart (s)</span><input type="number" inputmode="numeric" id="sAuto" step="1" min="1"></div>
    <div class="row"><span>Screen lag (ms)<small>Your display's own delay, subtracted from every time. 0 if unknown</small></span><input type="number" inputmode="numeric" id="sLag" step="1" min="0" max="200"></div>
    <div class="row"><span>Beep on each flash</span><input type="checkbox" id="sSound"></div>
    <div class="row"><span>Runs shown on graph</span><select id="sRuns"><option value="5">Last 5</option><option value="10">Last 10</option></select></div>
    <h4>LANE COLORS</h4>
    <div id="colorRows"></div>
    <div class="btns"><button id="clear">CLEAR ALL RESULTS</button><button class="primary" id="sClose">DONE</button></div>
  </div>
</div>

<!-- Button test -->
<div id="debug" class="sheet">
  <div class="card dcard">
    <div class="card-h"><h2>BUTTON TEST</h2><button class="x" id="dX" aria-label="Close">✕</button></div>
    <p class="dnote">Press each button — its tile lights up the moment the ESP sees it. Rounds can't be started while this panel is open (including the physical START button).</p>
    <div class="dgrid" id="dGrid"></div>
    <div class="dstat" id="dStat">Waiting for the ESP…</div>
    <div class="dhelp">
      <p><b>Lit up without touching it?</b> The signal is shorted to ground — check the keystone punch-down and the switch wiring.</p>
      <p><b>Never lights up?</b> Open circuit: broken wire, loose punch-down, wrong GPIO, or the switch is wired to a spare pair.</p>
      <p><b>More than 1 edge per press</b> is normal switch bounce and harmless — only the first edge is ever timed.</p>
    </div>
    <div class="btns"><button id="dLed">TEST LED: OFF</button><button id="dBeep" class="on">BEEP: ON</button><button id="dReset">RESET COUNTS</button></div>
    <h4>TIMING SELF-TEST</h4>
    <p class="dnote">10 quick rounds. In each, the ESP briefly pulls lane 1's input low itself at a moment it records, which checks press timing end to end. It also checks how close this screen's lights-out lands to the ESP's schedule. Keep this tab in front; nothing is saved. Don't press lane 1 while it runs.</p>
    <div class="dstat" id="stOut">Not run yet.</div>
    <div class="btns"><button class="primary" id="stRun">RUN TIMING SELF-TEST</button></div>
  </div>
</div>

<script>
const $=s=>document.querySelector(s);
const MAX_LANES=4;
const PIN_LABELS=['GPIO 5','GPIO 4','GPIO 12','GPIO 13'];
const DEFAULT_COLORS=['#e6007e','#27d3ff','#ff6a1a','#8b5cf6'];
const PRESETS=['#e6007e','#27d3ff','#ff6a1a','#ffd21f','#8b5cf6','#1fe06a','#ff3b5c','#ffffff'];
const LANE_IDX=[...Array(MAX_LANES).keys()];

// ---------- build the lane cards ----------
const lanesEl=$('#lanes');
lanesEl.innerHTML=LANE_IDX.map(i=>`
  <div class="lane" id="lane${i}">
    <div class="lh"><div class="nm" contenteditable spellcheck="false">Driver ${i+1}</div><span class="badge">WINNER</span><span class="tag">LANE ${i+1} · ${PIN_LABELS[i]}</span></div>
    <div class="mid"><svg class="chart"></svg><div class="rt hidden"></div><div class="sub hidden">&nbsp;</div></div>
    <div class="stats"><div><b class="best">—</b><small>BEST</small></div><div><b class="avg">—</b><small>AVG · LAST 10</small></div><div><b class="cnt">0</b><small>STARTS</small></div><div><b class="jmp">0</b><small>JUMPS</small></div></div>
  </div>`).join('');
const laneEls=[...lanesEl.querySelectorAll('.lane')];
$('#colorRows').innerHTML=LANE_IDX.map(i=>`<div class="row crow"><span>Lane ${i+1}</span><div class="colors" data-lane="${i}"></div></div>`).join('');
const colorRows=[...document.querySelectorAll('.crow')];

const lamps=[...document.querySelectorAll('.lamp')];
const S={lanes:2,minHold:1.0,maxHold:3.0,antiMs:100,screenLag:0,autoDelay:4,sound:true,auto:false,graphRuns:10,colors:[...DEFAULT_COLORS]};
let data={history:[],names:LANE_IDX.map(i=>`Driver ${i+1}`)};
let state='idle', laneRes=LANE_IDX.map(()=>null), autoT=null, ws=null, connected=false;
let dbgOpen=false;
const view=LANE_IDX.map(()=>'chart');

// ---------- helpers ----------
const fmt=ms=>(Math.max(ms,0)/1000).toFixed(3);
const esc=s=>String(s).replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
function setStatus(cls,txt){const el=$('#status');el.className='status '+cls;el.textContent=txt}
function setConn(cls,txt){const el=$('#conn');el.className='conn '+cls;el.querySelector('span').textContent=txt;$('#bStart').disabled=cls!=='ok'}
function lampsOff(){lamps.forEach(l=>l.className='lamp')}
function allLamps(c){lamps.forEach(l=>l.className='lamp '+c)}
function showAllCharts(){laneEls.forEach(el=>el.classList.remove('win'));LANE_IDX.forEach(showChart)}

// ---------- audio ----------
let ctx;
function ac(){if(!ctx)ctx=new (window.AudioContext||window.webkitAudioContext)();if(ctx.state==='suspended')ctx.resume();return ctx}
function beep(f,d,force){if(!S.sound&&!force)return;try{const c=ac(),o=c.createOscillator(),g=c.createGain(),t=c.currentTime;o.type='square';o.frequency.value=f;g.gain.setValueAtTime(.08,t);g.gain.exponentialRampToValueAtTime(.001,t+d);o.connect(g).connect(c.destination);o.start(t);o.stop(t+d+.02)}catch(e){}}

// ---------- ESP8266 link ----------
function connect(){
  if(location.protocol==='https:'){setConn('bad','OPEN THIS PAGE FROM THE ESP8266');return}
  const host=location.hostname||'reaction.local';
  setConn('wait','CONNECTING…');
  try{ws=new WebSocket(`ws://${host}:81/`)}catch(e){setTimeout(connect,2000);return}
  ws.onopen=()=>{connected=true;setConn('ok','ESP CONNECTED');sendCfg();if(dbgOpen)send('debug 1');syncStart()};
  ws.onclose=()=>{
    connected=false;setConn('bad','RECONNECTING…');syncStop();stopPlan();
    if(ST.on)stEnd('Lost connection to the ESP during the test.');
    if(state==='sequence'||state==='go'){state='idle';lampsOff();setStatus('','READY');showAllCharts()}
    if(dbgOpen)$('#dStat').textContent='Lost connection to the ESP — reconnecting…';
    setTimeout(connect,2000);
  };
  ws.onerror=()=>{try{ws.close()}catch(e){}};
  ws.onmessage=e=>{let m;try{m=JSON.parse(e.data)}catch(_){return}handle(m)};
}
document.addEventListener('visibilitychange',()=>{if(!document.hidden&&(!ws||ws.readyState>1))connect()});
const send=t=>{if(ws&&ws.readyState===1)ws.send(t)};
function sendCfg(){send(`cfg ${S.lanes} ${Math.round(S.minHold*1000)} ${Math.round(S.maxHold*1000)}`)}
function requestStart(){if(!connected||dbgOpen||state==='sequence'||state==='go')return;clearTimeout(autoT);try{ac()}catch(e){}send('start')}
function requestAbort(){clearTimeout(autoT);send('abort');stopPlan();if(S.auto)toggleAuto();if(state!=='sequence'&&state!=='go'){lampsOff();setStatus('','READY')}}

// ---------- clock sync ----------
// The ESP owns the clock. We ping it a few times a second; each reply gives
// offset = espMs - (sent+received)/2. The reply with the shortest round trip
// has the least room for error (at most half its round trip), so we use the
// best one from the last 10 s — recent enough that crystal drift stays < 1 ms.
const SYNC={samples:[],pend:{},seq:0,timer:null};
function syncPing(){const n=++SYNC.seq;SYNC.pend[n]=performance.now();send('sync '+n)}
function syncStart(){syncStop();for(let k=0;k<8;k++)setTimeout(syncPing,k*40);SYNC.timer=setInterval(syncPing,250)}
function syncStop(){clearInterval(SYNC.timer);SYNC.samples=[];SYNC.pend={}}   // a reconnect may mean the ESP rebooted
function onSync(m){
  const t0=SYNC.pend[m.n],t1=performance.now(); if(t0==null)return; delete SYNC.pend[m.n];
  SYNC.samples.push({at:t1,rtt:t1-t0,off:m.t-(t0+t1)/2});
  if(SYNC.samples.length>80)SYNC.samples.shift();
  const c=clockBest();
  if(connected&&c)$('#conn span').textContent=`ESP CONNECTED · SYNC ±${(c.rtt/2).toFixed(1)} ms`;
}
function clockBest(){
  const now=performance.now();let b=null;
  for(const s of SYNC.samples)if(now-s.at<10000&&(!b||s.rtt<b.rtt))b=s;
  return b;
}

// ---------- light schedule ----------
// Each round the ESP sends its plan in ESP time. We convert it with the sync
// offset (frozen for the round) and make each lamp change in the display frame
// that reaches the screen closest to its scheduled time. The frame where GO was
// drawn is noted, and each time is corrected by how far that landed from the
// ESP's lights-out (always under half a frame unless the tab was busy/hidden).
let plan=null,rafId=0,frameMs=1000/60,lastTs=0;
function startPlan(m){
  const c=clockBest(), off=c?c.off:m.now-performance.now();   // unsynced fallback: assume zero latency
  const L=t=>t-off, cyc=m.on+m.off, ev=[];
  for(let k=0;k<m.n;k++)ev.push({at:L(m.t+k*cyc),f:()=>{if(lamps[k])lamps[k].className='lamp amber';beep(880,.06)}});
  ev.push({at:L(m.t+m.n*cyc),f:()=>{allLamps('on');setStatus('live','HOLD')}});
  const go=L(m.t+m.n*cyc+m.hold);
  ev.push({at:go,go:1,f:()=>{lampsOff();state='go';setStatus('go','GO')}});
  plan={ev,go,shown:null,synced:!!c,rtt:c?c.rtt:null,test:!!m.test};
  cancelAnimationFrame(rafId);rafId=requestAnimationFrame(tick);
}
function tick(ts){
  rafId=0; if(!plan)return;
  const d=ts-lastTs; lastTs=ts; if(d>4&&d<50)frameMs+=(d-frameMs)*.1;   // learns 60/120/144 Hz
  const shows=ts+frameMs;            // changes made now appear at the next refresh
  while(plan.ev.length&&plan.ev[0].at<=shows+frameMs/2){const e=plan.ev.shift();e.f();if(e.go)plan.shown=shows}
  if(plan.ev.length)rafId=requestAnimationFrame(tick);
}
function flushPlan(){   // tab hidden or frames stalled: catch up now
  if(!plan)return;
  while(plan.ev.length){const e=plan.ev.shift();e.f();if(e.go)plan.shown=performance.now()}
}
function stopPlan(){cancelAnimationFrame(rafId);rafId=0;plan=null}
// ESP time from its scheduled lights-out → time from when this screen went dark
function screenRt(us){return us/1000-(plan&&plan.shown!=null?plan.shown-plan.go:0)-(S.screenLag||0)}

function handle(m){
  if(m.ev==='sync'){onSync(m);return}
  if(plan&&plan.test&&(m.ev==='result'||m.ev==='done')){   // self-test rounds never touch the history
    if(m.ev==='done')stDone(); else if(ST.on)stResult(m); return}
  switch(m.ev){
    case 'start':
      clearTimeout(autoT); state='sequence'; lampsOff(); startPlan(m);
      if(m.test)break;
      laneRes=LANE_IDX.map(()=>null);
      showAllCharts(); setStatus('live','GET READY'); break;
    case 'result':
      onResult(m); break;
    case 'done':
      finish(); break;
    case 'aborted':
      stopPlan(); if(ST.on)stEnd('Test aborted.');
      state='idle'; lampsOff(); setStatus('','READY'); showAllCharts(); break;
    // button test
    case 'pins':
      if(Array.isArray(m.v)) m.v.forEach((v,i)=>{const d=dbg[i];if(!d)return;const down=v===0;if(down&&!d.down)d.since=performance.now();d.down=down;renderTile(i)});
      break;
    case 'pin':
      dbgPin(m.i,m.v,m.e); break;
    case 'stat':
      if(dbgOpen) dbgStat(m); break;
  }
}
function onResult(m){
  const i=m.lane; if(!(i>=0&&i<MAX_LANES))return;
  let res;
  if(m.status==='jump') res={status:'jump'};
  else if(m.status==='none') res={status:'none'};
  else{
    flushPlan();
    const rt=screenRt(m.us), status=(S.antiMs>0&&rt<S.antiMs)?'anti':'ok';
    const prevBest=stats(data.names[i]).best;
    res={rt,status,pb:status==='ok'&&(prevBest==null||rt<prevBest)};
  }
  laneRes[i]=res;
  if(res.status!=='none') record(i,res.rt==null?null:res.rt,res.status);
  showResult(i);
}
function markWinner(){
  if(S.lanes<2)return null;
  const ok=LANE_IDX.filter(i=>i<S.lanes&&laneRes[i]&&laneRes[i].status==='ok');
  if(!ok.length)return null;
  const best=Math.min(...ok.map(i=>laneRes[i].rt));
  const winners=ok.filter(i=>laneRes[i].rt===best);
  winners.forEach(i=>laneEls[i].classList.add('win'));
  // everyone else with a valid time sees their gap to the winner
  ok.filter(i=>!winners.includes(i)&&!laneRes[i].pb).forEach(i=>{
    const s=laneEls[i].querySelector('.sub');
    s.className='sub';s.textContent=`+${fmt(laneRes[i].rt-best)} behind ${data.names[winners[0]]}`;
  });
  return winners;
}
function finish(){
  state='done';
  const w=markWinner();
  setStatus('',w?(w.length>1?'DEAD HEAT':`WINNER · LANE ${w[0]+1}`):'DONE');
  setTimeout(()=>{if(state==='done')lampsOff()},1500);
  renderAll(); save();
  if(S.auto){setStatus('',`NEXT IN ${S.autoDelay}s`);autoT=setTimeout(requestStart,S.autoDelay*1000)}
}

// ---------- results ----------
function record(i,rt,status){
  data.history.push({name:data.names[i],lane:i+1,rt,status,t:Date.now()});
  if(data.history.length>3000) data.history.splice(0,data.history.length-3000);
  renderAll(); save();
}
function stats(name){
  const mine=data.history.filter(h=>h.name===name);
  const ok=mine.filter(h=>h.status==='ok').map(h=>h.rt);
  const last=ok.slice(-10);
  return {best:ok.length?Math.min(...ok):null,avg:last.length?last.reduce((a,b)=>a+b,0)/last.length:null,cnt:mine.length,jumps:mine.filter(h=>h.status==='jump').length};
}
function showChart(i){
  view[i]='chart';
  const el=laneEls[i];
  el.querySelector('.chart').classList.remove('hidden');
  el.querySelector('.rt').classList.add('hidden');
  el.querySelector('.sub').classList.add('hidden');
  drawChart(i);
}
function showResult(i){
  const res=laneRes[i]; if(!res)return;
  view[i]='result';
  const el=laneEls[i], r=el.querySelector('.rt'), s=el.querySelector('.sub');
  el.querySelector('.chart').classList.add('hidden'); r.classList.remove('hidden'); s.classList.remove('hidden');
  if(res.status==='jump'){r.className='rt jump';r.textContent='JUMP START';s.className='sub jump';s.textContent='Pressed before the light went out'}
  else if(res.status==='none'){r.className='rt none';r.textContent='NO PRESS';s.className='sub';s.innerHTML='&nbsp;'}
  else if(res.status==='anti'){r.className='rt anti';r.innerHTML=fmt(res.rt)+'<small>s</small>';s.className='sub anti';s.textContent=`Under ${S.antiMs} ms — likely anticipated`}
  else{
    r.className='rt'+(res.pb?' pb':'');r.innerHTML=fmt(res.rt)+'<small>s</small>';
    if(res.pb){s.className='sub pb';s.textContent='PERSONAL BEST'}
    else{const a=stats(data.names[i]).avg;s.className='sub';s.textContent=a?`${res.rt<=a?'−':'+'}${fmt(Math.abs(res.rt-a))} vs average`:''}
  }
}

// ---------- chart (lower = faster) ----------
function drawChart(i){
  if(i>=S.lanes)return;
  const svg=laneEls[i].querySelector('.chart');
  const W=svg.clientWidth||320, H=svg.clientHeight||120;
  const small=W<420;
  svg.setAttribute('viewBox',`0 0 ${W} ${H}`);
  const runs=data.history.filter(h=>h.name===data.names[i]&&(h.status==='ok'||h.status==='anti'||h.status==='jump')).slice(-S.graphRuns);
  if(!runs.length){svg.innerHTML=`<text class="ce" x="${W/2}" y="${H/2}" text-anchor="middle" dominant-baseline="middle">No runs yet for ${esc(data.names[i])}${small?'':' — press START'}</text>`;return}
  const pl=small?40:48, pr=small?40:56, pt=18, pb=22;
  const timed=runs.filter(r=>r.status!=='jump').map(r=>r.rt);
  let lo=timed.length?Math.min(...timed):150, hi=timed.length?Math.max(...timed):350;
  const pad=Math.max(25,(hi-lo)*.25); lo=Math.max(0,lo-pad); hi=hi+pad;
  const n=runs.length, iw=W-pl-pr, ih=H-pt-pb;
  const x=k=>n===1?pl+iw/2:pl+k*iw/(n-1);
  const y=v=>pt+(hi-v)/(hi-lo)*ih;
  let s=`<defs><linearGradient id="fill${i}" x1="0" y1="0" x2="0" y2="1"><stop offset="0" style="stop-color:var(--lc);stop-opacity:.28"/><stop offset="1" style="stop-color:var(--lc);stop-opacity:0"/></linearGradient></defs>`;
  for(let g=0;g<3;g++){const v=lo+(hi-lo)*g/2, yy=y(v);s+=`<line class="grid" x1="${pl}" x2="${W-pr}" y1="${yy}" y2="${yy}"/><text x="${pl-6}" y="${yy}" text-anchor="end" dominant-baseline="middle">${fmt(v)}</text>`}
  const ok=runs.filter(r=>r.status==='ok').map(r=>r.rt);
  if(ok.length){
    const b=Math.min(...ok), a=ok.reduce((p,c)=>p+c,0)/ok.length;
    s+=`<line class="avg" x1="${pl}" x2="${W-pr}" y1="${y(a)}" y2="${y(a)}"/><text class="ta" x="${W-pr+6}" y="${y(a)}" dominant-baseline="middle">AVG</text>`;
    s+=`<line class="best" x1="${pl}" x2="${W-pr}" y1="${y(b)}" y2="${y(b)}"/><text class="tb" x="${W-pr+6}" y="${y(b)}" dominant-baseline="middle">BEST</text>`;
  }
  const pts=runs.map((r,k)=>({r,k})).filter(p=>p.r.status!=='jump');
  if(pts.length>1){
    const d=pts.map((p,j)=>`${j?'L':'M'}${x(p.k)},${y(p.r.rt)}`).join(' ');
    s+=`<path d="${d} L${x(pts[pts.length-1].k)},${H-pb} L${x(pts[0].k)},${H-pb} Z" fill="url(#fill${i})"/>`;
    s+=`<path class="ln" d="${d}"/>`;
  }
  runs.forEach((r,k)=>{
    const xx=x(k), last=k===n-1;
    if(r.status==='jump'){const yy=pt+6;s+=`<g><title>Jump start</title><line class="jx" x1="${xx-5}" y1="${yy-5}" x2="${xx+5}" y2="${yy+5}"/><line class="jx" x1="${xx+5}" y1="${yy-5}" x2="${xx-5}" y2="${yy+5}"/></g><text class="tj" x="${xx}" y="${pt-8}" text-anchor="middle">JUMP</text>`}
    else{
      const yy=y(r.rt);
      s+=`<circle class="pt${last?' last':''}${r.status==='anti'?' anti':''}" cx="${xx}" cy="${yy}" r="${last?6:4.5}"><title>${fmt(r.rt)} s${r.status==='anti'?' (anticipated)':''}</title></circle>`;
      if(last) s+=`<text class="tv" x="${xx}" y="${yy-12}" text-anchor="${n===1?'middle':'end'}">${fmt(r.rt)}</text>`;
    }
  });
  s+=`<text x="${pl}" y="${H-4}">LAST ${n} · LOWER = FASTER</text>`;
  svg.innerHTML=s;
}

function renderLane(i){
  const st=stats(data.names[i]), el=laneEls[i];
  el.querySelector('.best').textContent=st.best!=null?fmt(st.best):'—';
  el.querySelector('.avg').textContent=st.avg!=null?fmt(st.avg):'—';
  el.querySelector('.cnt').textContent=st.cnt;
  el.querySelector('.jmp').textContent=st.jumps;
  if(view[i]==='chart') drawChart(i);
}
function renderAll(){
  LANE_IDX.forEach(renderLane);
  const names=[...new Set(data.history.filter(h=>h.status==='ok').map(h=>h.name))];
  const rows=names.map(n=>({n,...stats(n)})).sort((a,b)=>a.best-b.best).slice(0,10);
  $('#lb').innerHTML=rows.length?rows.map((r,k)=>`<li><span class="rk">${k+1}</span><span>${esc(r.n)}</span><b>${fmt(r.best)}</b><span class="av">avg ${fmt(r.avg)} · ${r.cnt} starts · ${r.jumps} jumps</span></li>`).join(''):'<li class="empty">No valid times yet</li>';
  const rec=data.history.slice(-14).reverse();
  $('#recent').innerHTML=rec.length?rec.map(h=>`<li><i class="dot" style="background:var(--lane${Math.min(h.lane,MAX_LANES)-1})"></i><span>${esc(h.name)}</span><b class="${h.status}">${h.status==='jump'?'JUMP':fmt(h.rt)+(h.status==='anti'?' *':'')}</b></li>`).join(''):'<li class="empty">No starts yet</li>';
}
new ResizeObserver(()=>{LANE_IDX.forEach(i=>{if(view[i]==='chart')drawChart(i)})}).observe(lanesEl);

// ---------- lane count ----------
function applyLaneCount(){
  lanesEl.className='lanes n'+S.lanes;
  document.body.classList.toggle('many',S.lanes>=3);
  laneEls.forEach((el,i)=>el.classList.toggle('hidden',i>=S.lanes));
  colorRows.forEach((el,i)=>el.classList.toggle('hidden',i>=S.lanes));
  renderAll();
}

// ---------- names ----------
laneEls.forEach((lane,i)=>{
  const el=lane.querySelector('.nm');
  el.addEventListener('blur',()=>{const v=el.textContent.trim().slice(0,30)||`Driver ${i+1}`;el.textContent=v;data.names[i]=v;renderAll();save()});
});

// ---------- lane colors ----------
function applyColors(){
  S.colors.forEach((c,i)=>document.documentElement.style.setProperty(`--lane${i}`,c));
  document.querySelectorAll('.colors').forEach(box=>{const inp=box.querySelector('input');if(inp)inp.value=S.colors[+box.dataset.lane]});
}
document.querySelectorAll('.colors').forEach(box=>{
  const i=+box.dataset.lane;
  box.innerHTML=PRESETS.map(c=>`<button class="sw" data-c="${c}" style="background:${c}" title="${c}" aria-label="${c}"></button>`).join('')+`<input type="color" title="Custom color">`;
  box.querySelectorAll('.sw').forEach(b=>b.onclick=()=>{S.colors[i]=b.dataset.c;applyColors();save()});
  box.querySelector('input').addEventListener('input',e=>{S.colors[i]=e.target.value;applyColors();save()});
});

// ---------- settings ----------
function applyUI(){
  $('#sLanes').value=String(S.lanes);$('#sMin').value=S.minHold;$('#sMax').value=S.maxHold;
  $('#sAnti').value=S.antiMs;$('#sLag').value=S.screenLag;$('#sAuto').value=S.autoDelay;$('#sSound').checked=S.sound;$('#sRuns').value=String(S.graphRuns);
  laneEls.forEach((el,i)=>el.querySelector('.nm').textContent=data.names[i]);
  $('#bAuto').classList.toggle('on',S.auto);
  applyColors(); applyLaneCount();
}
function readUI(){
  S.lanes=Math.min(MAX_LANES,Math.max(1,+$('#sLanes').value||2));
  S.minHold=Math.min(10,Math.max(0.3,+$('#sMin').value||1));
  S.maxHold=Math.min(10,Math.max(0.3,+$('#sMax').value||3));
  S.antiMs=Math.max(0,+$('#sAnti').value||0);S.screenLag=Math.min(200,Math.max(0,+$('#sLag').value||0));S.autoDelay=Math.max(1,+$('#sAuto').value||4);S.sound=$('#sSound').checked;S.graphRuns=+$('#sRuns').value;
  applyLaneCount();save();sendCfg();
}
document.querySelectorAll('#settings select,#settings input[type=number],#settings input[type=checkbox]').forEach(el=>el.addEventListener('change',readUI));
function toggleSettings(){
  const on=$('#settings').classList.toggle('on');
  if(!on&&document.activeElement&&document.activeElement.blur)document.activeElement.blur();
}
function toggleAuto(){S.auto=!S.auto;$('#bAuto').classList.toggle('on',S.auto);save();if(S.auto&&(state==='idle'||state==='done'))requestStart();if(!S.auto)clearTimeout(autoT)}
let clearArmed=false;
$('#clear').onclick=()=>{
  if(!clearArmed){clearArmed=true;$('#clear').classList.add('arm');$('#clear').textContent='TAP AGAIN TO CONFIRM';setTimeout(()=>{clearArmed=false;$('#clear').classList.remove('arm');$('#clear').textContent='CLEAR ALL RESULTS'},3000);return}
  data.history=[];clearArmed=false;$('#clear').classList.remove('arm');$('#clear').textContent='CLEAR ALL RESULTS';renderAll();save();
};
$('#sClose').onclick=toggleSettings;
$('#sX').onclick=toggleSettings;
$('#settings').addEventListener('click',e=>{if(e.target.id==='settings')toggleSettings()});

// ---------- button test ----------
const DBG=[
  {n:'LANE 1',g:5,c:'var(--lane0)'},{n:'LANE 2',g:4,c:'var(--lane1)'},
  {n:'LANE 3',g:12,c:'var(--lane2)'},{n:'LANE 4',g:13,c:'var(--lane3)'},
  {n:'START',g:14,c:'var(--green)'}
];
const dbg=DBG.map(()=>({down:false,count:0,edges:0,since:0}));
let dbgLed=false, dbgBeep=true, dbgHB=null, dbgTick=null;
$('#dGrid').innerHTML=DBG.map((p,i)=>`<div class="tile" id="t${i}" style="--tc:${p.c}"><div class="tn">${p.n}</div><div class="tg">GPIO ${p.g}</div><div class="ts">RELEASED</div><div class="tcnt">0</div><div class="tcl">PRESSES</div><div class="te">&nbsp;</div></div>`).join('');
function renderTile(i){
  const d=dbg[i], el=$('#t'+i); if(!el)return;
  el.classList.toggle('down',d.down);
  el.querySelector('.ts').textContent=d.down?'PRESSED':'RELEASED';
  el.querySelector('.tcnt').textContent=d.count;
  el.querySelector('.te').textContent=d.down
    ? `held ${((performance.now()-d.since)/1000).toFixed(1)} s`
    : (d.count ? (d.edges>1 ? `${d.edges} edges · bounce OK` : 'clean press') : '\u00a0');
}
function dbgPin(i,v,e){
  const d=dbg[i]; if(!d)return;
  const down=v===0;
  if(down&&!d.down){d.count++;d.edges=Math.max(1,e||1);d.since=performance.now();if(dbgBeep)beep(i===4?520:700+i*120,.05,true)}
  d.down=down; renderTile(i);
}
function dbgStat(m){
  const up=m.up|0, h=Math.floor(up/3600), mi=Math.floor(up/60)%60, s=up%60;
  const upt=`${h}:${String(mi).padStart(2,'0')}:${String(s).padStart(2,'0')}`;
  let wifi;
  if(m.ap) wifi='hotspot mode';
  else{const r=m.rssi;wifi=`${r} dBm · ${r>=-60?'excellent':r>=-70?'good':r>=-80?'fair':'weak'}`}
  $('#dStat').innerHTML=`<span><i></i>ESP online</span><span>WiFi: ${wifi}</span><span>Uptime ${upt}</span><span>Free memory ${Math.round(m.heap/1024)} KB</span>`;
}
function openDebug(){
  if(dbgOpen)return;
  if($('#settings').classList.contains('on'))toggleSettings();
  dbgOpen=true; $('#debug').classList.add('on'); try{ac()}catch(e){}
  $('#dStat').textContent=connected?'Waiting for the ESP…':'Not connected to the ESP — tiles will update once it reconnects.';
  send('debug 1');
  dbgHB=setInterval(()=>send('debug 1'),4000);   // heartbeat — the ESP leaves test mode if this stops
  dbgTick=setInterval(()=>dbg.forEach((d,i)=>{if(d.down)renderTile(i)}),100);
}
function closeDebug(){
  if(!dbgOpen)return;
  if(ST.on){send('abort');stEnd('Stopped — the panel was closed.')}
  dbgOpen=false; $('#debug').classList.remove('on');
  clearInterval(dbgHB); clearInterval(dbgTick);
  if(dbgLed){dbgLed=false;send('led 0');$('#dLed').textContent='TEST LED: OFF';$('#dLed').classList.remove('on')}
  send('debug 0');
}
$('#dLed').onclick=()=>{dbgLed=!dbgLed;send('led '+(dbgLed?1:0));$('#dLed').textContent='TEST LED: '+(dbgLed?'ON':'OFF');$('#dLed').classList.toggle('on',dbgLed)};
$('#dBeep').onclick=()=>{dbgBeep=!dbgBeep;$('#dBeep').textContent='BEEP: '+(dbgBeep?'ON':'OFF');$('#dBeep').classList.toggle('on',dbgBeep)};
$('#dReset').onclick=()=>{dbg.forEach((d,i)=>{d.count=0;d.edges=0;renderTile(i)})};
$('#dX').onclick=closeDebug;
$('#debug').addEventListener('click',e=>{if(e.target.id==='debug')closeDebug()});
dbg.forEach((_,i)=>renderTile(i));

// ---------- timing self-test ----------
const ST={on:false,left:0,rows:[]}, ST_ROUNDS=10;
function stRun(){
  if(ST.on||!connected||state==='sequence'||state==='go')return;
  Object.assign(ST,{on:true,left:ST_ROUNDS,rows:[]});
  $('#stRun').disabled=true; $('#stOut').textContent=`Running… 0/${ST_ROUNDS}`; send('selftest');
}
function stResult(m){
  flushPlan();
  ST.rows.push({err:m.status==='ok'&&m.inj!=null?m.us-m.inj:null,scr:plan.shown!=null?plan.shown-plan.go:null,synced:plan.synced,status:m.status});
}
function stDone(){
  state='idle'; setStatus('','READY'); setTimeout(()=>{if(state==='idle')lampsOff()},300);
  if(!ST.on)return;
  ST.left--; $('#stOut').textContent=`Running… ${ST_ROUNDS-ST.left}/${ST_ROUNDS}`;
  if(ST.left>0)setTimeout(()=>{if(ST.on)send('selftest')},400); else stReport();
}
function stEnd(msg){ST.on=false;$('#stRun').disabled=false;if(msg)$('#stOut').textContent=msg}
function stReport(){
  stEnd();
  const r=ST.rows, hit=r.filter(x=>x.err!=null), scr=r.filter(x=>x.scr!=null).map(x=>x.scr), c=clockBest();
  const tag=(ok,warn)=>ok?'<b class="ok">PASS</b>':warn?'<b class="warn">CHECK</b>':'<b class="bad">FAIL</b>';
  const L=[];
  if(!hit.length) L.push(`${tag(false)} ESP press timing — no self-press registered. Lane 1 (GPIO 5) may be held low or shorted; see the tile above.`);
  else{
    const e=Math.max(...hit.map(x=>Math.abs(x.err)));
    L.push(`${tag(e<=100&&hit.length===r.length,e<=500)} ESP press timing — worst error ${e} µs over ${hit.length}/${r.length} presses (interrupt → result)`);
  }
  if(scr.length){
    const w=Math.max(...scr.map(Math.abs)), mean=scr.reduce((a,b)=>a+b,0)/scr.length;
    L.push(`${tag(w<=frameMs*.6,w<=frameMs*2)} Screen lights-out vs ESP schedule — average ${mean>=0?'+':''}${mean.toFixed(1)} ms, worst ${w.toFixed(1)} ms (one frame = ${frameMs.toFixed(1)} ms). Corrected in every result.`);
  }
  const unsynced=r.some(x=>!x.synced);
  if(!c||unsynced) L.push(`${tag(false)} Clock sync — ${unsynced?'some rounds ran before the clock was synced':'no sync replies'}.`);
  else{const u=c.rtt/2;L.push(`${tag(u<=5,u<=15)} Clock sync — worst case ±${u.toFixed(1)} ms (half the best round trip, ${c.rtt.toFixed(1)} ms); usually far less. This bounds what's left of the error after correction.${u>5?' A faster round trip means a tighter bound: stronger WiFi signal or the ESP hotspot.':''}`)}
  L.push(`<span>Not covered: your display's own lag after the frame is sent. That needs a light sensor; enter it under Settings → Screen lag.</span>`);
  $('#stOut').innerHTML=L.map(x=>`<div>${x}</div>`).join('');
}
$('#stRun').onclick=stRun;

// ---------- persistence (saved in this browser) ----------
const KEY='reaction-trainer-esp:data';
async function storeGet(){try{if(window.storage){const r=await window.storage.get(KEY,false);if(r&&r.value)return r.value}}catch(e){}try{return window['local'+'Storage'].getItem(KEY)}catch(e){return null}}
async function storeSet(v){try{if(window.storage){await window.storage.set(KEY,v,false);return}}catch(e){}try{window['local'+'Storage'].setItem(KEY,v)}catch(e){}}
async function load(){
  try{
    const v=await storeGet();
    if(v){
      const d=JSON.parse(v);
      if(d.history)data.history=d.history;
      if(Array.isArray(d.names))d.names.forEach((n,i)=>{if(i<MAX_LANES&&n)data.names[i]=n});
      if(d.settings)Object.assign(S,d.settings,{auto:false});
    }
  }catch(e){}
  if(!Array.isArray(S.colors))S.colors=[];
  for(let i=0;i<MAX_LANES;i++) if(!S.colors[i]) S.colors[i]=DEFAULT_COLORS[i];
  S.lanes=Math.min(MAX_LANES,Math.max(1,+S.lanes||2));
  applyUI(); connect();
}
let saveT;
function save(){clearTimeout(saveT);saveT=setTimeout(()=>storeSet(JSON.stringify({history:data.history,names:data.names,settings:S})),400)}

// ---------- input ----------
const canFs=!!(document.fullscreenEnabled&&document.documentElement.requestFullscreen);
if(!canFs) $('#bFs').style.display='none';
const fs=()=>{if(!canFs)return;document.fullscreenElement?document.exitFullscreen():document.documentElement.requestFullscreen()};
$('#bStart').onclick=e=>{e.currentTarget.blur();requestStart()};
$('#bAbort').onclick=e=>{e.currentTarget.blur();requestAbort()};
$('#bAuto').onclick=e=>{e.currentTarget.blur();toggleAuto()};
$('#bTest').onclick=e=>{e.currentTarget.blur();openDebug()};
$('#bSet').onclick=e=>{e.currentTarget.blur();toggleSettings()};
$('#bFs').onclick=e=>{e.currentTarget.blur();fs()};
addEventListener('keydown',e=>{
  const t=e.target;
  if(t.isContentEditable){if(e.key==='Enter'){e.preventDefault();t.blur()}return}
  if(['INPUT','SELECT'].includes(t.tagName))return;
  if(e.repeat)return;
  const k=e.key.toLowerCase();
  if(e.code==='Space'){e.preventDefault();requestStart()}
  else if(k==='escape'){if(dbgOpen)closeDebug();else if($('#settings').classList.contains('on'))toggleSettings();else requestAbort()}
  else if(k==='t'){dbgOpen?closeDebug():openDebug()}
  else if(k==='a')toggleAuto();
  else if(k==='s')toggleSettings();
  else if(k==='f')fs();
});
load();
</script>
</body>
</html>
)PAGE";