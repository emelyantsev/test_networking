import socket

HOST = "127.0.0.1"
PORT = 27015

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    
    s.connect((HOST, PORT))
    s.sendall(b"Hi, Lorence")

    s.shutdown(socket.SHUT_WR)

    response_str = str()

    while True:

        data = s.recv(1024)

        if len(data) == 0:
            print("0 bytes recieved")
            break

        print(f"Received {data!r}")

        received_str = data.decode()
        response_str = response_str + received_str

        print(type(data))

    print(response_str)
