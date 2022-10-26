import socket
import random
import time
import string
from urllib import request
import argparse

HOST = "127.0.0.1"
PORT = 27015


def generateRandomString(N : int) -> str :

    return ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(N))


if __name__ == '__main__':


    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--seed', type=int, help="sets random seed")

    parser.add_argument('-l', '--len', type=int, help="sets string length")

    args = parser.parse_args()

    if args.seed:
        random.seed(args.seed)
    else:
        random.seed(time.time_ns())

    N = 100

    if args.len:
        N = args.len

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:

        request_str = generateRandomString(N)
        
        s.connect((HOST, PORT))
        s.sendall(request_str.encode())

        print(f"Send string {request_str}")

        s.shutdown(socket.SHUT_WR)

        response_str = str()

        while True:

            data = s.recv(1024)

            if len(data) == 0:
                #print("0 bytes recieved")
                break

            #print(f"Received {data!r}")

            part_str = data.decode()
            
            response_str += part_str

            #print(type(data))

        print(f"Recieved string {response_str}")

        assert(request_str == response_str)
