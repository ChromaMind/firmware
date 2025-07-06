import websocket
import random
import time

ESP32_WS_URL = "ws://10.151.240.37:81"


def send_random_modes(count=100):
    try:
        print(f"Connecting to {ESP32_WS_URL}...")
        ws = websocket.create_connection(ESP32_WS_URL)
        print("Connected!")

        for _ in range(count):
            if _ == 0:
                mode = 1
            else:
                mode = random.randint(1, 8)  # Supported modes: 1 or 2

            blink_rate = random.choice([50, 50])
            message = f"{mode};{blink_rate};5"
            print("Sending:", message)
            ws.send(message)
            if _ == 0:
                time.sleep(20)
            else:
                time.sleep(random.random() * 5)  # wait before sending next command

        ws.close()
        print("Closed connection.")

    except Exception as e:
        print("❌ Failed:", e)


if __name__ == "__main__":
    send_random_modes()
