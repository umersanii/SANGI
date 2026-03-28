#ifndef WEB_UI_H
#define WEB_UI_H

// Full SANGI control page stored in ESP32 flash (PROGMEM).
// Served via send_P() — zero RAM copy. Target size: <8KB.

#include <pgmspace.h>

const char WEB_UI_HTML[] PROGMEM = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SANGI</title>
<style>
:root{--p:#00D4E8;--op:#00272E;--pc:#00414F;--opc:#97F0FF;--bg:#0A0C0F;--sf:#131618;--sc:#1C2024;--sch:#282B2E;--os:#E2E2E6;--osv:#8E9099;--ov:#434549}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--os);font-family:system-ui,sans-serif;font-size:14px;padding:16px;max-width:480px;margin:auto}
.hdr{display:flex;justify-content:space-between;align-items:center;margin-bottom:20px}
.ht{font-size:30px;font-weight:800;color:var(--p)}
.hi{font-size:11px;color:var(--osv);background:var(--sc);padding:4px 12px;border-radius:20px}
.card,details{background:var(--sf);border-radius:20px;margin-bottom:14px;box-shadow:0 2px 16px rgba(0,212,232,.08)}
.card{padding:16px}
.sl{font-size:10px;font-weight:700;letter-spacing:.8px;color:var(--osv);text-transform:uppercase;margin-bottom:12px}
.sr{display:flex;justify-content:space-between;align-items:center;padding:7px 0;border-bottom:1px solid var(--sch)}
.sr:last-child{border-bottom:none}
.sk,.sv{font-size:13px}
.sk{color:var(--osv)}
.sv{font-weight:600;color:var(--os)}
.arc{display:flex;gap:5px}
.ac{flex:1;padding:9px 3px;text-align:center;font-size:9px;font-weight:700;border-radius:50px;background:var(--sc);color:var(--osv);border:2px solid transparent;transition:.25s}
.ac.on{background:var(--pc);color:var(--opc);border-color:var(--p)}
.eg{display:grid;grid-template-columns:repeat(4,1fr);gap:6px}
.eb{padding:10px 2px;border-radius:50px;border:1.5px solid var(--ov);background:var(--sc);color:var(--os);font-family:inherit;font-size:11px;font-weight:500;cursor:pointer;transition:.2s;width:100%}
.eb:hover,.eb.on{background:var(--pc);border-color:var(--p);color:var(--opc)}
.eb.on{background:var(--p);color:var(--op)}
.gr{display:flex;gap:8px}
.gb{flex:1;padding:14px 6px;border-radius:50px;border:none;background:var(--pc);color:var(--opc);font-family:inherit;font-size:13px;font-weight:700;cursor:pointer;transition:.2s}
.gb:hover{background:var(--p);color:var(--op)}
summary{padding:16px;cursor:pointer;font-size:13px;font-weight:600;list-style:none;display:flex;align-items:center;gap:8px;color:var(--os)}
summary::before{content:'›';font-size:20px;color:var(--p)}
summary::-webkit-details-marker{display:none}
.cb{padding:0 16px 16px}
.row{display:flex;align-items:center;gap:8px;padding:5px 0}
.row label{width:136px;font-size:12px;color:var(--osv);flex-shrink:0}
.row input[type=range]{flex:1;accent-color:var(--p)}
.row span{width:64px;font-size:12px;font-weight:600;text-align:right;color:var(--os)}
.cbb{display:flex;gap:8px;margin-top:14px}
.cbb button{flex:1;padding:13px;border-radius:50px;font-family:inherit;font-size:13px;font-weight:700;cursor:pointer;transition:.2s}
.bs{background:var(--p);color:var(--op);border:none}
.br{background:transparent;color:var(--p);border:2px solid var(--p)}
.cbb button:hover{opacity:.85}
.sm{font-size:12px;color:var(--p);height:18px;margin-top:6px;font-weight:600}
</style>
</head>
<body>
<div class="hdr">
  <span class="ht">SANGI</span>
  <span class="hi">192.168.4.1</span>
</div>
<div class="card">
  <div class="sl">Status</div>
  <div class="sr"><span class="sk">Emotion</span><span class="sv" id="se">—</span></div>
  <div class="sr"><span class="sk">Battery</span><span class="sv" id="sb">—</span></div>
  <div class="sr"><span class="sk">Uptime</span><span class="sv" id="su">—</span></div>
  <div class="sr"><span class="sk">Heap</span><span class="sv" id="sh">—</span></div>
</div>
<div class="card">
  <div class="sl">Attention Arc</div>
  <div class="arc">
    <div class="ac" id="a0">NORMAL</div>
    <div class="ac" id="a1">BORED</div>
    <div class="ac" id="a2">SAD</div>
    <div class="ac" id="a3">CONFUSED</div>
    <div class="ac" id="a4">ANGRY</div>
  </div>
</div>
<div class="card">
  <div class="sl">Emotions</div>
  <div class="eg" id="emotions"></div>
</div>
<div class="card">
  <div class="sl">Gestures</div>
  <div class="gr">
    <button class="gb" onclick="sg('tap')">Tap</button>
    <button class="gb" onclick="sg('long')">Long Press</button>
    <button class="gb" onclick="sg('double')">Double Tap</button>
  </div>
</div>
<details>
<summary>Personality Config</summary>
<div class="cb">
  <div id="cfgsliders"></div>
  <div class="cbb">
    <button class="bs" onclick="saveConfig()">Save</button>
    <button class="br" onclick="resetConfig()">Reset</button>
  </div>
  <div class="sm" id="savemsg"></div>
</div>
</details>
<script>
var EMOS=['IDLE','HAPPY','SLEEPY','EXCITED','SAD','ANGRY','CONFUSED','THINKING','LOVE','SURPRISED','DEAD','BORED','BLINK'];
var FIELDS=[
  ['attentionStage1Ms','Stage 1 (BORED)',60000,1800000,10000,'ms'],
  ['attentionStage2Ms','Stage 2 (SAD)',60000,1800000,10000,'ms'],
  ['attentionStage3Ms','Stage 3 (CONFUSED)',60000,1800000,10000,'ms'],
  ['attentionStage4Ms','Stage 4 (ANGRY)',60000,1800000,10000,'ms'],
  ['moodDriftIntervalMs','Mood Drift',30000,600000,5000,'ms'],
  ['microExpressionChance','Micro-expr %',0,100,1,'%'],
  ['jitterPercent','Jitter %',0,50,1,'%']
];
var cur=0,cfg={};

function fmt(ms){var m=Math.floor(ms/60000),s=Math.floor((ms%60000)/1000);return m?m+'m '+s+'s':s+'s';}
function fmtUp(ms){var h=Math.floor(ms/3600000),m=Math.floor((ms%3600000)/60000),s=Math.floor((ms%60000)/1000);return h?h+'h '+m+'m':m?m+'m '+s+'s':s+'s';}

var eg=document.getElementById('emotions');
EMOS.forEach(function(n,i){
  var b=document.createElement('button');
  b.className='eb';b.textContent=n;b.onclick=function(){se(i);};
  eg.appendChild(b);
});

var cs=document.getElementById('cfgsliders');
FIELDS.forEach(function(f){
  var k=f[0],lbl=f[1],mn=f[2],mx=f[3],step=f[4],unit=f[5];
  var d=document.createElement('div');d.className='row';
  d.innerHTML='<label>'+lbl+'</label><input type="range" id="r-'+k+'" min="'+mn+'" max="'+mx+'" step="'+step+'"><span id="l-'+k+'">&#8212;</span>';
  cs.appendChild(d);
  document.getElementById('r-'+k).addEventListener('input',function(){ul(k,unit);});
});

function pollStatus(){
  fetch('/api/status').then(function(r){return r.json();}).then(function(d){
    cur=d.emotion;
    var pct=Math.max(0,Math.min(100,Math.round((d.batteryVoltage-3.0)/1.2*100)));
    document.getElementById('se').textContent=d.emotionName;
    document.getElementById('sb').textContent=d.batteryVoltage.toFixed(2)+'V ('+pct+'%)';
    document.getElementById('su').textContent=fmtUp(d.uptimeMs);
    document.getElementById('sh').textContent=d.freeHeap+'B free';
    document.querySelectorAll('.eb').forEach(function(b,i){b.classList.toggle('on',i===cur);});
    for(var i=0;i<5;i++){document.getElementById('a'+i).classList.toggle('on',i===d.attentionStage);}
  }).catch(function(){});
}

function se(id){
  fetch('/api/emotion',{method:'POST',body:new URLSearchParams({emotion:id})});
  cur=id;
  document.querySelectorAll('.eb').forEach(function(b,i){b.classList.toggle('on',i===id);});
}

function sg(g){fetch('/api/gesture',{method:'POST',body:new URLSearchParams({gesture:g})});}

function ul(k,unit){
  var v=parseInt(document.getElementById('r-'+k).value);
  cfg[k]=v;
  document.getElementById('l-'+k).textContent=unit==='ms'?fmt(v):v+'%';
}

function loadConfig(){
  fetch('/api/config').then(function(r){return r.json();}).then(function(d){
    cfg=d;
    FIELDS.forEach(function(f){
      var k=f[0],unit=f[5],el=document.getElementById('r-'+k);
      if(el){el.value=cfg[k];document.getElementById('l-'+k).textContent=unit==='ms'?fmt(cfg[k]):cfg[k]+'%';}
    });
  }).catch(function(){});
}

function saveConfig(){
  var params=new URLSearchParams();
  Object.keys(cfg).forEach(function(k){params.append(k,String(cfg[k]));});
  fetch('/api/config',{method:'POST',body:params}).then(function(){
    var m=document.getElementById('savemsg');
    m.textContent='Saved!';
    setTimeout(function(){m.textContent='';},2000);
  });
}

function resetConfig(){
  fetch('/api/config/reset',{method:'POST'}).then(function(){loadConfig();});
}

pollStatus();
setInterval(pollStatus,3000);
loadConfig();
</script>
</body>
</html>)rawhtml";

#endif // WEB_UI_H
