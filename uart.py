import serial
import time
import random 
import json
import zlib

port     = "COM7"
baudrate = 9600

ser = serial.Serial(port, baudrate=baudrate)

print("Serial ok")

def testThermal(name, telemetryJson):
    if name in telemetryJson:
        print("---->",telemetryJson["t0"])
        if (telemetryJson[name] < 20) or (telemetryJson["t0"] > 30):
            print("ERRROR!!!!!")
            exit(-1)
        else:
            print("T0 - ",telemetryJson["t0"])
    else:
        print("Temperature ERROR")
        exit(-1)

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
    #time.sleep(1)
    
    ser.write('a'.encode('UTF-8'))
    
    reasonJson = json.loads(ser.readline())
    
    print(reasonJson)
    
    
    ser.write('g'.encode('UTF-8'))
    print("-------")
    
    telemetryData    = ser.readline()
    print(telemetryData)
    telemetryJson    = json.loads(telemetryData)
    telemetryJsonCrc = json.loads(ser.readline())
    
    
    crcFromTelemetry = telemetryJsonCrc["crc"]
    crcCalculated    = zlib.crc32(telemetryData[:-2])
    
    # print(telemetryJson)
    
    if crcFromTelemetry != crcCalculated:
        print("ERROR BAD CRC!!!!!")
        exit(-1)
    
    testThermal("t0", telemetryJson)
    testThermal("t1", telemetryJson)
    testThermal("t2", telemetryJson)
    testThermal("t3", telemetryJson)
    testThermal("t4", telemetryJson)
    
    #print("-------")

    
    if reasonJson["last_reset_reasons"] != 1:
        print("ERROR RESETTED!")
        #exit(-1)
    
    print("-------")
   
    testfunc('1','q')
    testfunc('2','w')
    testfunc('3','e')
    testfunc('4','r')
    testfunc('5','t')
    testfunc('6','y')
    counter += 1