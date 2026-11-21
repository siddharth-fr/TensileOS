#ifndef TENSILE_NET_WEBUI_H
#define TENSILE_NET_WEBUI_H

#pragma once
// TensileOS Virtual Monitor Client
// Ultra-fast local cursor rendering (144Hz/0ms latency), event-driven WebSocket transport.
static const char WEB_UI_HTML[] = R"HTML(<!doctype html>
<html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>TensileOS Workstation</title>
<style>
html,body{margin:0;height:100%;background:#0a0a0f;color:#8a93a0;font:13px system-ui,-apple-system,sans-serif;overflow:hidden}
#wrap{display:flex;flex-direction:column;align-items:center;justify-content:center;height:100%}
canvas{outline:none;box-shadow:0 0 50px #000c;background:#008080;image-rendering:pixelated;image-rendering:crisp-edges;border:2px solid #222;
cursor:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='19' viewBox='0 0 12 19'%3E%3Cpath d='M0,0 L0,16 L3.5,12.5 L6,18 L8,17 L5.5,11.5 L9.5,11.5 Z' fill='%23ffffff' stroke='%23000000' stroke-width='1' stroke-linejoin='miter'/%3E%3C/svg%3E"),default}
#st{height:28px;line-height:28px;white-space:nowrap}
.ok{color:#6c6}.bad{color:#c66}
</style></head><body>
<div id="wrap"><canvas id="c" tabindex="0" width="320" height="240"></canvas><div id="st">connecting...</div></div>
<script>
(()=>{
const cv = document.getElementById('c'), g = cv.getContext('2d'), st = document.getElementById('st');
let ws = null, retry = 500, bytes = 0, ip = location.host, connected = false;

// Classic 16-Color Palette (RGB)
const PAL16 = [
  [0x00,0x00,0x00], // 0 Black
  [0x00,0x00,0x80], // 1 Navy
  [0x00,0x80,0x00], // 2 Green
  [0x00,0x80,0x80], // 3 Teal
  [0x80,0x00,0x00], // 4 Maroon
  [0x80,0x00,0x80], // 5 Purple
  [0x80,0x80,0x00], // 6 Olive
  [0xC0,0xC0,0xC0], // 7 Silver
  [0x80,0x80,0x80], // 8 Grey
  [0x00,0x00,0xFF], // 9 Blue
  [0x00,0xFF,0x00], // 10 Lime
  [0x00,0xFF,0xFF], // 11 Cyan
  [0xFF,0x00,0x00], // 12 Red
  [0xFF,0x00,0xFF], // 13 Magenta
  [0xFF,0xFF,0x00], // 14 Yellow
  [0xFF,0xFF,0xFF]  // 15 White
];

function fit(){
  const s = Math.min(innerWidth / 320, (innerHeight - 36) / 240);
  cv.style.width = Math.floor(320 * s) + 'px';
  cv.style.height = Math.floor(240 * s) + 'px';
}
fit();
addEventListener('resize', fit);

// WebAudio Sound Synthesizer (Opcode 0x03)
let actx = null;
function initAudio(){
  if (!actx) {
    const AC = window.AudioContext || window.webkitAudioContext;
    if (AC) actx = new AC();
  }
  if (actx && actx.state === 'suspended') actx.resume();
}
function playTone(freq, dur){
  initAudio();
  if (!actx || freq <= 0 || dur <= 0) return;
  try {
    const t0 = actx.currentTime;
    const osc = actx.createOscillator();
    const gain = actx.createGain();
    osc.type = 'square';
    osc.frequency.setValueAtTime(freq, t0);
    gain.gain.setValueAtTime(0.12, t0);
    gain.gain.exponentialRampToValueAtTime(0.001, t0 + dur / 1000);
    osc.connect(gain);
    gain.connect(actx.destination);
    osc.start(t0);
    osc.stop(t0 + dur / 1000);
  } catch(e){}
}

function onMsg(buf){
  const d = new Uint8Array(buf);
  bytes += d.length;
  if (d[0] === 4) { // Opcode 0x04: Pixel Framebuffer Slice (100% ESP32 Rasterized)
    const y0 = (d[1] << 8) | d[2];
    const numLines = d[3];
    const img = g.createImageData(320, numLines);
    const data = img.data;
    let p = 4;
    let dp = 0;
    for (let j = 0; j < numLines; j++) {
      for (let i = 0; i < 320; i += 2) {
        const b = d[p++];
        const c1 = b >> 4;
        const c2 = b & 0x0F;
        const rgb1 = PAL16[c1];
        data[dp++] = rgb1[0];
        data[dp++] = rgb1[1];
        data[dp++] = rgb1[2];
        data[dp++] = 255;
        const rgb2 = PAL16[c2];
        data[dp++] = rgb2[0];
        data[dp++] = rgb2[1];
        data[dp++] = rgb2[2];
        data[dp++] = 255;
      }
    }
    g.putImageData(img, 0, y0);
  } else if (d[0] === 3) {
    const freq = (d[1] << 8) | d[2], dur = (d[3] << 8) | d[4];
    playTone(freq, dur);
  }
}

// Mouse coordinate translation (maps client clicks to 320x240 ESP32 coordinate space)
function getPixelCoords(e){
  const r = cv.getBoundingClientRect();
  const scaleX = 320 / r.width;
  const scaleY = 240 / r.height;
  const x = Math.max(0, Math.min(319, Math.floor((e.clientX - r.left) * scaleX)));
  const y = Math.max(0, Math.min(239, Math.floor((e.clientY - r.top) * scaleY)));
  return [x, y];
}

let lastX = -1, lastY = -1, lastB = 0;
let mouseRaf = false, pendX = -1, pendY = -1, pendB = 0;

function sendMousePkt(x, y, b){
  if (ws && ws.readyState === 1) {
    ws.send(new Uint8Array([0xF1, (x >> 8) & 0xFF, x & 0xFF, y, b]));
  }
}

function handleMouse(e, force){
  const [x, y] = getPixelCoords(e);
  const b = (e.buttons & 1 ? 1 : 0) | (e.buttons & 2 ? 2 : 0);
  if (force) {
    lastX = x; lastY = y; lastB = b;
    sendMousePkt(x, y, b);
  } else {
    pendX = x; pendY = y; pendB = b;
    if (!mouseRaf) {
      mouseRaf = true;
      requestAnimationFrame(() => {
        mouseRaf = false;
        if (pendX !== lastX || pendY !== lastY || pendB !== lastB) {
          lastX = pendX; lastY = pendY; lastB = pendB;
          sendMousePkt(lastX, lastY, lastB);
        }
      });
    }
  }
}

cv.addEventListener('mousedown', e => { initAudio(); cv.focus(); handleMouse(e, true); e.preventDefault(); });
addEventListener('mouseup', e => handleMouse(e, true));
cv.addEventListener('mousemove', e => handleMouse(e, false));
cv.addEventListener('contextmenu', e => e.preventDefault());

// Keyboard
function send(b){ if (ws && ws.readyState === 1) ws.send(Uint8Array.of(b)); }
const SPECIAL = {ArrowUp:0x80,ArrowDown:0x81,ArrowLeft:0x82,ArrowRight:0x83,Home:0x84,End:0x85,
  Delete:0x86,PageUp:0x87,PageDown:0x88,Insert:0x89,Enter:13,Backspace:8,Tab:9,Escape:27,
  F1:0x90,F2:0x91,F3:0x92,F4:0x93,F6:0x95,F7:0x96,F8:0x97,F9:0x98,F10:0x99};
const CTRL_PASS = 'vrtwnq'.replace('q','');
addEventListener('keydown', e => {
  initAudio();
  if (e.metaKey) return;
  let k = null;
  if (e.ctrlKey && !e.altKey && e.key.length === 1) {
    const l = e.key.toLowerCase();
    if (l >= 'a' && l <= 'z' && CTRL_PASS.indexOf(l) < 0) k = l.charCodeAt(0) - 96;
  } else if (SPECIAL[e.key] !== undefined) {
    k = SPECIAL[e.key];
  } else if (e.key.length === 1 && (!e.ctrlKey || e.altKey)) {
    const c = e.key.charCodeAt(0); if (c >= 32 && c < 127) k = c;
  }
  if (k !== null) { send(k); e.preventDefault(); }
});

// WebSocket Connection
function status(t, cls){ st.textContent = t; st.className = cls || ''; }
function connect(){
  status('connecting...','');
  ws = new WebSocket((location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/ws');
  ws.binaryType = 'arraybuffer';
  ws.onopen = () => { retry = 500; connected = true; };
  ws.onmessage = e => onMsg(e.data);
  ws.onclose = () => { connected = false; status('disconnected - retrying', 'bad'); setTimeout(connect, retry); retry = Math.min(retry * 1.5, 2000); };
  ws.onerror = () => { try { ws.close(); } catch(e){} };
}
setInterval(() => {
  if (connected) { status('TensileOS \\u2022 320x240 Pixel Framebuffer \\u2022 ' + bytes + ' B/s', 'ok'); }
  bytes = 0;
}, 1000);
setInterval(() => {
  if (ws && ws.readyState === 1) ws.send(new Uint8Array([0xFE])); // Keep-alive ping
}, 3000);

connect();
})();
</script></body></html>
)HTML";

#endif // TENSILE_NET_WEBUI_H
