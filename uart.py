import serial
import time
import random 
import json
import zlib

port     = "COM7"
baudrate = 9600

ser = serial.Serial(port, baudrate=baudrate)

print("Serial ok")

def testfunc(one, two):
    if random.randint(0, 1): 
        ser.write(one.encode('UTF-8'))
    else:
        ser.write(two.encode('UTF-8'))
    print(ser.readline())
    print("-------")

counter = 0

while True:
    print("Test num: ", counter)
    time.sleep(1)
    ser.write('g'.encode('UTF-8'))
    print("-------")
    
    telemetryData    = ser.readline()
    
    telemetryJson    = json.loads(telemetryData)
    telemetryJsonCrc = json.loads(ser.readline())
    
    
    crcFromTelemetry = telemetryJsonCrc["crc"]
    crcCalculated    = zlib.crc32(telemetryData[:-2])
    
    print(telemetryJson)
    
    if crcFromTelemetry != crcCalculated:
        print("ERROR BAD CRC!!!!!")
        exit(-1)
    
    print("-------")
    ser.write('a'.encode('UTF-8'))
    print(ser.readline())
    print("-------")
   
    testfunc('1','q')
    testfunc('2','w')
    testfunc('3','e')
    testfunc('4','r')
    testfunc('5','t')
    testfunc('6','y')
    counter += 1