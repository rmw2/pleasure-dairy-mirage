import board
import busio
import socketpool
import time
import wifi

from adafruit_mpr121 import MPR121
from secrets import secrets

REMOTE_HOST = '192.168.1.76'
REMOTE_PORT = 42069

# Seconds before giving up on TCP data transfer.
SOCKET_TIMEOUT = 2 
# Seconds between registering touches.
TOUCH_TIMEOUT = 0.5

def main():
    i2c = busio.I2C(board.IO6, board.IO5)
    mpr121 = MPR121(i2c)
    print("Initialized MPR121 touch controller.")

    print("Available wifi networks:")
    for net in wifi.radio.start_scanning_networks():
        print(f"\t{net.ssid} ({net.authmode})")
    wifi.radio.stop_scanning_networks()
    
    while True:
        try:
            pool = connect()
            with pool.socket(pool.AF_INET, pool.SOCK_STREAM) as sock:
                sock.connect((REMOTE_HOST, REMOTE_PORT))
                sock.settimeout(SOCKET_TIMEOUT)

                print(f"Connected to {REMOTE_HOST}:{REMOTE_PORT}")
                waitfortouch(mpr121, sock)

        except Exception as err:
            print("Damn homie: ", err)

def connect():
    print(f"Tryna connect to {secrets['SSID']}...")
    wifi.radio.connect(ssid=secrets['SSID'], password=secrets['PASSWORD'])

    print(f"Connected to WIFI gateway at {wifi.radio.ipv4_gateway}")
    print(f"IP: {wifi.radio.ipv4_address}")
    return socketpool.SocketPool(wifi.radio)


def waitfortouch(mpr121, sock):
    print("Waiting for touches...")
    while True:
        if any(mpr121.touched_pins):
            print("PIN TOUCHED BABY:", mpr121.touched_pins)
            packet = bytes("HELLO\n", "utf-8") #bytes(mpr121.touched_pins)
            sock.sendall(packet)
            time.sleep(TOUCH_TIMEOUT)

main()