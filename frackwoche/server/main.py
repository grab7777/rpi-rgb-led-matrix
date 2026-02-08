# a server that executes a command on the command line
# use a popular python library to create a server that listens on a port and executes a command

import socket
import subprocess

HOST = "localhost"
PORT = 9990
LED_ROWS = 64
LED_COLS = 64
FONT = "../../fonts/5x8.bdf"
command = f"sudo ../textdisplay --led-rows={LED_ROWS} --led-cols={LED_COLS} -f {FONT} -t"
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Server listening on port {PORT}...")
    # if program is killed, close socket!
    while True:
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            data = conn.recv(1024)
            if not data:
                break
            message = data.decode("utf-8")
            message = message.split("\r\n\r\n")[-1]  # get the body of the request
            print(f"Sending message: {message}")
            print(f"Executing command: {command} '{message}'")
            try:
                result = subprocess.check_output(
                    f"{command} '{message}'", shell=True, stderr=subprocess.STDOUT
                )
                conn.sendall(result)
            except subprocess.CalledProcessError as e:
                conn.sendall(e.output)
            except Exception as e:
                conn.sendall(str(e).encode("utf-8"))
            except KeyboardInterrupt:
                print("Shutting down server...")
                break
