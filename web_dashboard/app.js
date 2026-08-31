// CAN Sentinel CSOC Web Visualizer & Hardware Telemetry Simulator

// Test buffers matching STM32 firmware datasets
const DATASETS = {
  normal: {
    name: "Normal Traffic",
    values: [12, 12, 12, 14, 12, 12, 13, 12, 12, 12, 11, 12, 12, 13, 12, 12],
    anomalies: [0.02, 0.02, 0.02, 0.025, 0.02, 0.02, 0.018, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.019, 0.02, 0.02]
  },
  dos: {
    name: "DoS Attack",
    values: [255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255],
    anomalies: [0.79, 0.85, 0.94, 0.99, 0.98, 0.95, 0.91, 0.88, 0.92, 0.96, 0.99, 0.97, 0.94, 0.91, 0.89, 0.95]
  },
  fuzzy: {
    name: "Fuzzy Attack",
    values: [142, 89, 210, 15, 244, 99, 178, 43, 201, 111, 19, 254, 88, 165, 32, 220],
    anomalies: [0.65, 0.82, 0.91, 0.88, 0.93, 0.85, 0.79, 0.95, 0.90, 0.87, 0.82, 0.96, 0.89, 0.91, 0.84, 0.88]
  },
  impersonation: {
    name: "Impersonation",
    values: [80, 80, 80, 240, 240, 80, 80, 245, 245, 80, 80, 250, 250, 80, 80, 240],
    anomalies: [0.35, 0.42, 0.78, 0.94, 0.96, 0.55, 0.48, 0.91, 0.95, 0.52, 0.44, 0.98, 0.97, 0.49, 0.41, 0.93]
  }
};

let currentStreamKey = 'normal';
let offset = 0;
let soundEnabled = false;
let audioCtx = null;
let activeOscillator = null;

// Serial Connection state
let serialPort = null;
let serialReader = null;
let serialWriter = null;
let isSerialConnected = false;

// History vectors for chart
const MAX_CHART_POINTS = 30;
const historyScores = new Array(MAX_CHART_POINTS).fill(0.02);
const historyValues = new Array(MAX_CHART_POINTS).fill(12);

// DOM Elements
const systemStatusIndicator = document.getElementById('system-status-indicator');
const statusText = document.getElementById('status-text');
const btnConnectSerial = document.getElementById('btn-connect-serial');

const servoGaugeFill = document.getElementById('servo-gauge-fill');
const servoNeedle = document.getElementById('servo-needle');
const servoAngleText = document.getElementById('servo-angle-text');
const servoModeText = document.getElementById('servo-mode-text');
const servoStateLabel = document.getElementById('servo-state-label');

const relaySwitch = document.getElementById('relay-switch');
const relayStatusText = document.getElementById('relay-status-text');
const relayStateLabel = document.getElementById('relay-state-label');
const nodeGateway = document.getElementById('node-gateway');

const speakerIcon = document.getElementById('speaker-icon');
const btnToggleAudio = document.getElementById('btn-toggle-audio');
const audioStatusText = document.getElementById('audio-status-text');

const oledStream = document.getElementById('oled-stream');
const oledVal = document.getElementById('oled-val');
const oledAnom = document.getElementById('oled-anom');
const oledBar = document.getElementById('oled-bar');
const oledAct = document.getElementById('oled-act');
const oledBus = document.getElementById('oled-bus');
const oledStatus = document.getElementById('oled-status');

const logTableBody = document.getElementById('log-table-body');
const btnExportCsv = document.getElementById('btn-export-csv');

// Web Serial API Handler
if (btnConnectSerial) {
  btnConnectSerial.addEventListener('click', async () => {
    if (!('serial' in navigator)) {
      alert('Web Serial API is not supported in this browser. Please use Chrome, Edge, or Opera to connect directly to Wokwi/STM32 serial port.');
      return;
    }
    try {
      if (isSerialConnected && serialPort) {
        if (serialReader) await serialReader.cancel();
        await serialPort.close();
        isSerialConnected = false;
        btnConnectSerial.textContent = '🔌 Connect Wokwi Serial';
        btnConnectSerial.style.background = 'rgba(255, 255, 255, 0.05)';
        return;
      }

      serialPort = await navigator.serial.requestPort();
      await serialPort.open({ baudRate: 115200 });
      isSerialConnected = true;
      btnConnectSerial.textContent = '🟢 Connected to Wokwi MCU';
      btnConnectSerial.style.background = 'rgba(48, 209, 88, 0.2)';

      const textDecoder = new TextDecoderStream();
      const readableStreamClosed = serialPort.readable.pipeTo(textDecoder.writable);
      const reader = textDecoder.readable.getReader();
      serialReader = reader;

      const encoder = new TextEncoderStream();
      encoder.readable.pipeTo(serialPort.writable);
      serialWriter = encoder.writable.getWriter();

      let buffer = '';
      while (true) {
        const { value, done } = await reader.read();
        if (done) break;
        if (value) {
          buffer += value;
          const lines = buffer.split('\n');
          buffer = lines.pop(); // Keep incomplete line
          for (const line of lines) {
            handleSerialLine(line.trim());
          }
        }
      }
    } catch (err) {
      console.error('Serial Error:', err);
      btnConnectSerial.textContent = '🔌 Connect Wokwi Serial';
      btnConnectSerial.style.background = 'rgba(255, 255, 255, 0.05)';
      isSerialConnected = false;
    }
  });
}

async function sendSerialCmd(cmdChar) {
  if (isSerialConnected && serialWriter) {
    try {
      await serialWriter.write(cmdChar);
    } catch (e) {
      console.error('Failed to write command to serial:', e);
    }
  }
}

function handleSerialLine(line) {
  if (line.startsWith('TELEMETRY_JSON:')) {
    try {
      const jsonStr = line.substring(15);
      const data = JSON.parse(jsonStr);
      applyTelemetryData(data);
    } catch (e) {
      console.error('Failed to parse telemetry JSON:', e);
    }
  }
}

function applyTelemetryData(data) {
  // Live update from real hardware/Wokwi JSON telemetry
  const val = data.val;
  const anomaly = data.anomaly;
  const isIntrusion = data.anomaly > 0.30;
  const streamName = data.stream;

  historyScores.push(anomaly);
  historyScores.shift();
  historyValues.push(val);
  historyValues.shift();

  if (isIntrusion) {
    systemStatusIndicator.classList.add('alert');
    statusText.textContent = `ALERT: ${streamName.toUpperCase()} DETECTED (WOKWI HARDWARE)`;
    spawnPacket(true);
  } else {
    systemStatusIndicator.classList.remove('alert');
    statusText.textContent = 'SYSTEM SECURE (WOKWI MCU LIVE)';
    spawnPacket(false);
  }

  const targetAngle = data.actuator;
  const rotationDeg = (targetAngle / 180) * 180 - 90;
  servoNeedle.style.transform = `rotate(${rotationDeg}deg)`;
  servoAngleText.textContent = `${targetAngle}°`;
  
  if (isIntrusion) {
    servoModeText.textContent = 'SAFE-STOP ENGAGED (0°)';
    servoModeText.classList.add('alert');
    servoStateLabel.textContent = 'SERVO: 0° [BRAKE]';
    servoStateLabel.style.color = '#ff3b30';
  } else {
    servoModeText.textContent = 'THROTTLE OPEN (90°)';
    servoModeText.classList.remove('alert');
    servoStateLabel.textContent = 'SERVO: 90° [DRIVE]';
    servoStateLabel.style.color = '#30d158';
  }

  if (data.relay === 0) {
    relaySwitch.classList.add('tripped');
    relayStatusText.textContent = 'BUS ISOLATED [OPEN]';
    relayStatusText.classList.add('alert');
    relayStateLabel.textContent = 'RELAY: OPEN (CUT)';
    nodeGateway.classList.add('tripped');
  } else {
    relaySwitch.classList.remove('tripped');
    relayStatusText.textContent = 'CIRCUIT CLOSED [OK]';
    relayStatusText.classList.remove('alert');
    relayStateLabel.textContent = 'RELAY: CLOSED (PASS)';
    nodeGateway.classList.remove('tripped');
  }

  if (isIntrusion) {
    playTone(1500);
  } else {
    stopTone();
  }

  oledStream.textContent = streamName;
  oledVal.textContent = val.toFixed(1);
  oledAnom.textContent = anomaly.toFixed(3);
  let barLength = Math.min(16, Math.max(0, Math.floor(anomaly * 16)));
  oledBar.textContent = '='.repeat(barLength) + ' '.repeat(16 - barLength);

  if (isIntrusion) {
    oledAct.textContent = 'SAFE-STOP (0deg)';
    oledBus.textContent = 'ISOLATED [CUT]';
    oledStatus.textContent = '! INTRUSION DETECTED !';
    oledStatus.className = 'oled-line status-alert';
  } else {
    oledAct.textContent = 'NORMAL (90deg)';
    oledBus.textContent = 'CONNECTED [OK]';
    oledStatus.textContent = 'STATUS: SECURE [OK]';
    oledStatus.className = 'oled-line status-ok';
  }
}

// Initialize Web Audio API
function initAudio() {
  if (!audioCtx) {
    audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  }
}

btnToggleAudio.addEventListener('click', () => {
  initAudio();
  soundEnabled = !soundEnabled;
  if (soundEnabled) {
    btnToggleAudio.textContent = 'Mute Audio';
    audioStatusText.textContent = 'SIREN READY';
  } else {
    btnToggleAudio.textContent = 'Enable Sound';
    audioStatusText.textContent = 'MUTED';
    stopTone();
  }
});

function playTone(freq) {
  if (!soundEnabled || !audioCtx) return;
  if (audioCtx.state === 'suspended') {
    audioCtx.resume();
  }
  if (!activeOscillator) {
    activeOscillator = audioCtx.createOscillator();
    const gainNode = audioCtx.createGain();
    gainNode.gain.value = 0.1;
    activeOscillator.type = 'sawtooth';
    activeOscillator.connect(gainNode);
    gainNode.connect(audioCtx.destination);
    activeOscillator.start();
  }
  activeOscillator.frequency.setValueAtTime(freq, audioCtx.currentTime);
}

function stopTone() {
  if (activeOscillator) {
    activeOscillator.stop();
    activeOscillator.disconnect();
    activeOscillator = null;
  }
}

// Scenario Selectors
document.querySelectorAll('.btn-scenario').forEach(btn => {
  btn.addEventListener('click', (e) => {
    document.querySelectorAll('.btn-scenario').forEach(b => b.classList.remove('active'));
    const target = e.currentTarget;
    target.classList.add('active');
    currentStreamKey = target.getAttribute('data-stream');
    offset = 0;

    // Send serial command to Wokwi microcontroller if connected
    if (currentStreamKey === 'normal') sendSerialCmd('1');
    else if (currentStreamKey === 'dos') sendSerialCmd('2');
    else if (currentStreamKey === 'fuzzy') sendSerialCmd('3');
    else if (currentStreamKey === 'impersonation') sendSerialCmd('4');
  });
});

// Canvas Setup for Chart
const chartCanvas = document.getElementById('waveform-chart');
const ctx = chartCanvas.getContext('2d');

function resizeChart() {
  chartCanvas.width = chartCanvas.parentElement.clientWidth;
  chartCanvas.height = chartCanvas.parentElement.clientHeight;
}
window.addEventListener('resize', resizeChart);
resizeChart();

// Canvas Setup for Packet Animation
const packetCanvas = document.getElementById('packet-canvas');
const pCtx = packetCanvas.getContext('2d');
let packets = [];

function resizePackets() {
  packetCanvas.width = packetCanvas.parentElement.clientWidth;
  packetCanvas.height = packetCanvas.parentElement.clientHeight;
}
window.addEventListener('resize', resizePackets);
resizePackets();

function spawnPacket(isAttack) {
  packets.push({
    x: 20,
    y: isAttack ? 95 : 135,
    speed: isAttack ? 6 + Math.random() * 4 : 3 + Math.random() * 2,
    color: isAttack ? '#ff3b30' : '#00f2fe',
    size: isAttack ? 6 : 4
  });
}

function updatePackets() {
  pCtx.clearRect(0, 0, packetCanvas.width, packetCanvas.height);
  for (let i = packets.length - 1; i >= 0; i--) {
    const p = packets[i];
    p.x += p.speed;
    pCtx.beginPath();
    pCtx.arc(p.x, p.y, p.size, 0, Math.PI * 2);
    pCtx.fillStyle = p.color;
    pCtx.shadowBlur = 8;
    pCtx.shadowColor = p.color;
    pCtx.fill();
    
    if (p.x > packetCanvas.width) {
      packets.splice(i, 1);
    }
  }
}

// Simulation Main Loop (Every 500 ms)
function tick() {
  const stream = DATASETS[currentStreamKey];
  const val = stream.values[offset % stream.values.length];
  const anomaly = stream.anomalies[offset % stream.anomalies.length];
  const isIntrusion = anomaly > 0.30;

  // Push to history
  historyScores.push(anomaly);
  historyScores.shift();
  historyValues.push(val);
  historyValues.shift();

  // 1. System Status
  if (isIntrusion) {
    systemStatusIndicator.classList.add('alert');
    statusText.textContent = `ALERT: ${stream.name.toUpperCase()} THREAT DETECTED`;
    spawnPacket(true);
  } else {
    systemStatusIndicator.classList.remove('alert');
    statusText.textContent = 'SYSTEM SECURE (TINYML WATCHDOG OK)';
    spawnPacket(false);
  }

  // 2. Servo Actuator (90 deg normal -> 0 deg emergency lockout)
  const targetAngle = isIntrusion ? 0 : 90;
  // Needle rotation calculation: 90 deg = -90deg rotation in SVG needle transform, 0 deg = -180deg
  const rotationDeg = (targetAngle / 180) * 180 - 90;
  servoNeedle.style.transform = `rotate(${rotationDeg}deg)`;
  servoAngleText.textContent = `${targetAngle}°`;
  
  if (isIntrusion) {
    servoModeText.textContent = 'SAFE-STOP ENGAGED (0°)';
    servoModeText.classList.add('alert');
    servoStateLabel.textContent = 'SERVO: 0° [BRAKE]';
    servoStateLabel.style.color = '#ff3b30';
  } else {
    servoModeText.textContent = 'THROTTLE OPEN (90°)';
    servoModeText.classList.remove('alert');
    servoStateLabel.textContent = 'SERVO: 90° [DRIVE]';
    servoStateLabel.style.color = '#30d158';
  }

  // 3. Relay Firewall
  if (isIntrusion) {
    relaySwitch.classList.add('tripped');
    relayStatusText.textContent = 'BUS ISOLATED [OPEN]';
    relayStatusText.classList.add('alert');
    relayStateLabel.textContent = 'RELAY: OPEN (CUT)';
    nodeGateway.classList.add('tripped');
  } else {
    relaySwitch.classList.remove('tripped');
    relayStatusText.textContent = 'CIRCUIT CLOSED [OK]';
    relayStatusText.classList.remove('alert');
    relayStateLabel.textContent = 'RELAY: CLOSED (PASS)';
    nodeGateway.classList.remove('tripped');
  }

  // 4. Acoustic Siren
  if (isIntrusion) {
    const toneFreq = (currentStreamKey === 'dos') ? 1200 : (currentStreamKey === 'fuzzy' ? 2000 : 1600);
    playTone(toneFreq);
  } else {
    stopTone();
  }

  // 5. OLED Mirror Update
  oledStream.textContent = stream.name;
  oledVal.textContent = val.toFixed(1);
  oledAnom.textContent = anomaly.toFixed(3);
  
  let barLength = Math.min(16, Math.max(0, Math.floor(anomaly * 16)));
  let barStr = '='.repeat(barLength) + ' '.repeat(16 - barLength);
  oledBar.textContent = barStr;

  if (isIntrusion) {
    oledAct.textContent = 'SAFE-STOP (0deg)';
    oledBus.textContent = 'ISOLATED [CUT]';
    oledStatus.textContent = '! INTRUSION DETECTED !';
    oledStatus.className = 'oled-line status-alert';
  } else {
    oledAct.textContent = 'NORMAL (90deg)';
    oledBus.textContent = 'CONNECTED [OK]';
    oledStatus.textContent = 'STATUS: SECURE [OK]';
    oledStatus.className = 'oled-line status-ok';
  }

  // 6. Append Log Table Entry
  const now = new Date().toLocaleTimeString();
  const tr = document.createElement('tr');
  tr.innerHTML = `
    <td>${now}</td>
    <td>${stream.name}</td>
    <td>0x${val.toString(16).toUpperCase().padStart(2, '0')} (${val})</td>
    <td>${anomaly.toFixed(3)}</td>
    <td><span class="badge-log ${isIntrusion ? 'alert' : 'ok'}">${isIntrusion ? 'MALICIOUS' : 'BENIGN'}</span></td>
    <td>${isIntrusion ? 'Servo Safe-Stop (0°) & Relay Trip' : 'Pass-through'}</td>
  `;
  logTableBody.insertBefore(tr, logTableBody.firstChild);
  if (logTableBody.children.length > 25) {
    logTableBody.removeChild(logTableBody.lastChild);
  }

  offset++;
}

// Chart Render Loop
function drawChart() {
  const w = chartCanvas.width;
  const h = chartCanvas.height;
  ctx.clearRect(0, 0, w, h);

  // Draw Grid Lines
  ctx.strokeStyle = 'rgba(255, 255, 255, 0.05)';
  ctx.lineWidth = 1;
  for (let y = 0; y < h; y += 40) {
    ctx.beginPath();
    ctx.moveTo(0, y);
    ctx.lineTo(w, y);
    ctx.stroke();
  }

  // Draw Threshold Line (0.30)
  const threshY = h - (0.30 * h);
  ctx.strokeStyle = 'rgba(255, 59, 48, 0.8)';
  ctx.setLineDash([6, 4]);
  ctx.beginPath();
  ctx.moveTo(0, threshY);
  ctx.lineTo(w, threshY);
  ctx.stroke();
  ctx.setLineDash([]);

  // Draw Anomaly Score Line
  ctx.strokeStyle = '#00f2fe';
  ctx.lineWidth = 2.5;
  ctx.beginPath();
  const stepX = w / (MAX_CHART_POINTS - 1);
  for (let i = 0; i < MAX_CHART_POINTS; i++) {
    const x = i * stepX;
    const y = h - (historyScores[i] * h);
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }
  ctx.stroke();

  // Draw Payload Value (Normalized 0-255 to canvas height)
  ctx.strokeStyle = '#ff007f';
  ctx.lineWidth = 1.5;
  ctx.beginPath();
  for (let i = 0; i < MAX_CHART_POINTS; i++) {
    const x = i * stepX;
    const y = h - ((historyValues[i] / 255.0) * h);
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }
  ctx.stroke();
}

// Render Animation Frame
function animationLoop() {
  updatePackets();
  drawChart();
  requestAnimationFrame(animationLoop);
}

// CSV Export Feature
btnExportCsv.addEventListener('click', () => {
  let csv = 'Timestamp,Traffic Stream,Payload Byte,Anomaly Score,Threat Class,Action\n';
  const rows = logTableBody.querySelectorAll('tr');
  rows.forEach(r => {
    const cols = r.querySelectorAll('td');
    if (cols.length) {
      const rowData = Array.from(cols).map(c => `"${c.textContent.trim()}"`).join(',');
      csv += rowData + '\n';
    }
  });
  
  const blob = new Blob([csv], { type: 'text/csv' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `can_ids_forensic_log_${Date.now()}.csv`;
  a.click();
  URL.revokeObjectURL(url);
});

// Start Intervals
setInterval(tick, 500);
animationLoop();
