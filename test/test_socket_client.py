import socket,threading,logging, signal
from time import sleep

DATEFMT="%H:%M:%S"
FORMAT = "[%(asctime)s]\t [%(threadName)s,%(thread)d] %(message)s"
logging.basicConfig(level=logging.INFO, format=FORMAT, datefmt=DATEFMT)
# https://www.cnblogs.com/i-honey/p/8098677.html

class ChatClient:
    def __init__(self,ip='127.0.0.1',port=9999):
        self.sock = socket.socket()
        self.addr = (ip,port)

        self.event = threading.Event()
        self.start()

    def start(self):
        self.sock.connect(self.addr)
        threading.Thread(target=self._recv,name='recv').start()

    def _recv(self):
        while not self.event.is_set():
            data = self.sock.recv(1024) #阻塞
            logging.info("{}".format(data.decode()))

    def stop(self):
        logging.info("{} broken".format(self.addr))
        self.sock.close()

        self.event.wait(3)
        self.event.set()
        logging.info("byebye")

def main():
    run_flag = True
    e = threading.Event()
    cc = ChatClient()
    cc.start()

    def handler(signum, frame):
        nonlocal run_flag
        run_flag = False

    signal.signal(signal.SIGINT, handler)
    signal.signal(signal.SIGTERM, handler)

    while run_flag:
        sleep(1.0)

    cc.stop()

if __name__ == '__main__':
    main()