# UDP广播案例-发送端
from socket import *
import time
 
# 设定目标地址
dest = ('192.168.33.255', 9999)  # 192.168.31是我的网段,255代表任意IP
s = socket(AF_INET, SOCK_DGRAM)
s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
data = '我是客户端小白,我的时间是{}'.format(time.time())
str = s.sendto(data.encode('utf-8'), dest)  # 发送广播
s.settimeout(30)  # 设置等待超时时间为30s
msg, addr = s.recvfrom(1024)  # recvfrom为阻塞方法
print('接收回复==服务端地址:{},响应内容:{}'.format(addr, msg.decode('utf-8')))
s.close()
