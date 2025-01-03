import serial
import time
import random 
import json
import zlib

port     = "COM7"
baudrate = 9600

ser = serial.Serial(port, baudrate=baudrate)

print("Serial ok")

numberOfTemperatureError = 0

def testThermal(name, telemetryJson):
    if name in telemetryJson:
        #if (telemetryJson[name] < 20) or (telemetryJson[name] > 30):
        #    print("ERRROR!!!!!")
        #    exit(-1)
        #else:
        print(name," - ",telemetryJson[name])
    else:
        #print("Temperature ERROR")
        return False
        #exit(-1)
    return True
    
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
    time.sleep( random.randint(0,100) / 100.0 )
    
    ser.write('a'.encode('UTF-8'))
    
    reasonJson = json.loads(ser.readline())
    
    print(reasonJson)
    
    
    ser.write('g'.encode('UTF-8'))
    print("-------")
    
    telemetryData    = ser.readline()
    print(telemetryData)
    telemetryJson    = json.loads(telemetryData)
    telemetryJsonCrc = json.loads(ser.readline())
    
    print(telemetryJsonCrc)
    crcFromTelemetry = telemetryJsonCrc["crc"]
    crcCalculated    = zlib.crc32(telemetryData[:-2])
    
    # print(telemetryJson)
    
    if crcFromTelemetry != crcCalculated:
        print("ERROR BAD CRC!!!!!")
        exit(-1)
    
    for i in range(2,3):
        if False == testThermal("t"+str(i), telemetryJson):
            numberOfTemperatureError += 1
            time.sleep(2)
            break
    
    #print("-------")

    
    if reasonJson["last_reset_reasons"] != 1:
        print("ERROR RESETTED!")
        #exit(-1)
        
    print("-------")

    print("Number of terr: ", numberOfTemperatureError)
    
    print("-------")
   
    testfunc('1','q')
    testfunc('2','w')
    testfunc('3','e')
    testfunc('4','r')
    testfunc('5','t')
    testfunc('6','y')
    counter += 1