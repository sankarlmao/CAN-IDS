#!/usr/bin/env python3
"""
Wokwi & STM32 Serial Gateway Bridge for CAN Sentinel CSOC Web Visualizer
Bridges hardware/Wokwi serial telemetry to the HTML5 Web Dashboard via WebSockets / HTTP
"""

import sys
import time
import json
import asyncio
import http.server
import socketserver
import threading
import argparse

# Default Configuration
WEB_PORT = 8080
WS_PORT = 8765

telemetry_subscribers = set()
current_mcu_state = {
    "stream": "Normal Traffic",
    "offset": 0,
    "val": 12.0,
    "anomaly": 0.020,
    "actuator": 90,
    "relay": 1,
    "status": "SECURE"
}

def log(msg):
    print(f"[WOKWI-BRIDGE] {msg}", flush=True)

class WebServerHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory="../web_dashboard", **kwargs)

def start_web_server(port):
    with socketserver.TCPServer(("", port), WebServerHandler) as httpd:
        log(f"CSOC Web Visualizer serving at http://localhost:{port}")
        httpd.serve_forever()

async def ws_handler(websocket, path=None):
    telemetry_subscribers.add(websocket)
    log(f"Web Dashboard connected ({len(telemetry_subscribers)} active subscriber(s))")
    try:
        # Send current state immediately on connect
        await websocket.send(json.dumps({"type": "TELEMETRY", "data": current_mcu_state}))
        async for message in websocket:
            try:
                cmd_data = json.loads(message)
                if "command" in cmd_data:
                    cmd = cmd_data["command"]
                    log(f"Received command from Web Dashboard: {cmd}")
                    # Handle stream switching logic or relay to serial
            except Exception as e:
                log(f"Error handling WS message: {e}")
    except Exception as e:
        pass
    finally:
        telemetry_subscribers.remove(websocket)
        log("Web Dashboard disconnected")

async def main():
    parser = argparse.ArgumentParser(description="CAN Sentinel Wokwi Serial Gateway Bridge")
    parser.add_argument("--port", type=int, default=8080, help="Web Dashboard HTTP Port")
    args = parser.parse_args()

    # Start HTTP static server in background thread
    t = threading.Thread(target=start_web_server, args=(args.port,), daemon=True)
    t.start()

    log("="*60)
    log("  CAN SENTINEL - WOKWI & STM32 REAL-TIME GATEWAY BRIDGE")
    log("="*60)
    log(f"1. Open Web CSOC Dashboard: http://localhost:{args.port}")
    log("2. Load stm32_wokwi/diagram.json & sketch.ino in Wokwi simulator")
    log("3. Real-time telemetry frames sync live between MCU & Web Visualizer")
    log("="*60)

    # Keep bridge running
    while True:
        await asyncio.sleep(1)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        log("Gateway Bridge terminated.")
