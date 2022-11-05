# udp广播案例-接收端
from socket import *
import time
import traceback

s = socket(AF_INET,SOCK_DGRAM)
# 设置套接字
s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
# 选择一个接收地址
s.bind(('0.0.0.0', 9999))
while True:
    try:
        msg, addr=s.recvfrom(1024)
        print('接受客户端:{}, 消息内容:{}'.format(addr, msg.decode('utf-8')))
        s.sendto("我是服务端, 我的时间是{}".format(time.time()).encode('utf-8'), addr)
    except:
        print("接收消息异常:{}".format(traceback.format_exc()))
s.close()
