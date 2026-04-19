import socket
import struct
import time
import random

SERVER_IP = "127.0.0.1"
SERVER_PORT = 8080

CMD_HELLO = 0
CMD_CONFIG = 1
CMD_DATA = 2
CMD_START = 3
CMD_STATUS = 4
CMD_RESULT = 5

def recv_full(sock, n):
    data = bytearray()
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            return None
        data.extend(packet)
    return data

def main():
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client.connect((SERVER_IP, SERVER_PORT))
        print(f"[Python Client] Connected to {SERVER_IP}:{SERVER_PORT}")

        # 2. Handshake(CMD_HELLO)
        header = struct.pack('!II', 0, CMD_HELLO)
        client.sendall(header)
        response = client.recv(9).decode()
        print(f"[Server Response] {response}")

        # 3. Config(CMD_CONFIG)
        N = 500
        threads = 8
        header = struct.pack('!II', 8, CMD_CONFIG)
        payload = struct.pack('!II', N, threads)
        client.sendall(header + payload)
        print(f"[Config] Sent: N={N}, Threads={threads}")
        print(f"[Server Response] {client.recv(6).decode()}")

        # 4. Data(CMD_DATA)
        print("[Data] Generating and sending matrix...")
        total_data_size = N * N * 4
        header = struct.pack('!II', total_data_size, CMD_DATA)
        client.sendall(header)

        for i in range(N):
            row = [random.randint(1, 10) for _ in range(N)]
            row_packed = struct.pack(f'!{N}i', *row)
            client.sendall(row_packed)

        print(f"[Server Response] {client.recv(7).decode()}")

        # 5. Start(CMD_START)
        client.sendall(struct.pack('!II', 0, CMD_START))
        print(f"[Server Response] {client.recv(7).decode()}")

        # 6. Status Loop(CMD_STATUS)
        while True:
            client.sendall(struct.pack('!II', 0, CMD_STATUS))
            status = client.recv(64).decode()
            print(f"[Status] {status}")
            if status == "READY":
                break
            time.sleep(0.5)

        # 7. Result(CMD_RESULT)
        print("[Result] Requesting data...")
        client.sendall(struct.pack('!II', 0, CMD_RESULT))

        for i in range(N):
            row_data = recv_full(client, N * 4)
            if row_data is None:
                print("[Error] Incomplete data received")
                break
            row = struct.unpack(f'!{N}i', row_data)
            if i == 0:
                print(f"[Check] Diagonal element [0][0]: {row[0]}")

        print("[Python Client] Work complete. Success!")

    except Exception as e:
        print(f"[Error] {e}")
    finally:
        client.close()

if __name__ == "__main__":
    main()