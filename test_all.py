import websocket
import random
import time

ips = [
    "172.18.177.18",
    "172.18.177.73",
    "172.18.177.55",
    "172.18.177.92",
]

connections = {}

def open_all_connections():
    for ip in ips:
        try:
            url = f"ws://{ip}:81"
            print(f"[{ip}] Connecting...")
            ws = websocket.create_connection(url)
            connections[ip] = ws
            print(f"[{ip}] Connected!")
        except Exception as e:
            print(f"[{ip}] ❌ Failed to connect:", e)

def send_to_all(message):
    for ip, ws in connections.items():
        try:
            print(f"[{ip}] Sending: {message}")
            ws.send(message)
        except Exception as e:
            print(f"[{ip}] ❌ Failed to send:", e)

def close_all_connections():
    for ip, ws in connections.items():
        try:
            ws.close()
            print(f"[{ip}] Closed connection.")
        except Exception as e:
            print(f"[{ip}] ❌ Failed to close:", e)

def run_sequence(count=100):
    open_all_connections()

    for i in range(count):
        if i == 0:
            mode = 1
        else:
            mode = random.randint(1, 6)

        blink_rate = 50
        power = 5
        message = f"{mode};{blink_rate};{power}"
        send_to_all(message)

        if i == 0:
            time.sleep(20)
        else:
            time.sleep(random.uniform(0.5, 5))

    close_all_connections()

if __name__ == "__main__":
    run_sequence()

