import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto("1", ("192.168.70.177", 8080))
