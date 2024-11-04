In the process of development...

Program for arduino nano provides a uart bridge to control relay and get voltages, digital states, and temperature data from several DS18B20 

Program also does not allow relays(control[0-4] pins) to be turned on too often(to prevent equipment from breaking down).
This is done by setting a delay before switching on separately for each channel. Switching off is implemented immediately.


For control[0-2] minimum interval between on is 20 second

For control[3-4] minimum interval between on is 180 second

For adc[0-7] multiplier for output is 1, show voltage as is

DS18B20 temperature sensors number is 1, address not used. Get temperature without use address.

Control names in json output is in c[0-4]


This paratemets can be changed in file AVR_ATMEL/pins.h


Connection circuit:

![image](https://github.com/user-attachments/assets/155fd41b-09dc-4202-8deb-c40bc7e3bdc5)

<hr>
Python telegram bot:

<hr>
<b>Programming with arduino</b>:

[Coming soon]

<b>Programming without arduino</b>:

  Programming fuses:
  
    avrdude.exe -c usbasp -p m328p -U lfuse:w:0xff:m -U hfuse:w:0xd9:m -U efuse:w:0xfc:m 
  
  Main program:
  
    avrdude.exe -c usbasp -p m328p -B 125kHz -U flash:w:"SmartBridgeWithoutArduinoBootloader.hex":i

Programmer: https://aliexpress.ru/wholesale?SearchText=usbasp --- need 10 pin to 6 pin adapter
              
Avrdude: https://github.com/avrdudes/avrdude

<hr>

<b>Protocol</b>: UART, baudrate - 9600

<hr>

<b>Command</b>: 'g' - get all pin and sensors state

<b>Response example(without ds18b20 connected)</b>: 

               {"vent":0,"heat":0,"light":1,"res0":1,"res1":0,"res2":1,"d0":0,"d1":0,"d2":0,"adc0": 0.00,"adc1": 0.00,"adc2": 0.16,"adc3": 0.00,"adc4": 0.00,"adc5": 0.00,"adc6": 0.00,"adc7": 0.00}\r\n{'crc': 4236205978}\r\n

<b>Response example(with 5 DS18B20 connected)</b>:

               {"vent":0,"heat":0,"light":0,"res0":1,"res1":0,"res2":1,"t0":26.75,"t1":26.75,"t2":26.94,"t3":26.88,"t4":58.19,"d0":0,"d1":1,"d2":1,"adc0": 0.00,"adc1": 0.00,"adc2": 0.16,"adc3": 0.00,"adc4": 0.00,"adc5": 0.00,"adc6": 0.00,"adc7": 0.00}\r\n{'crc': 288843757}\r\n

<hr>

<b>Command</b> : '1', '2', '3', '4', '5' - connect certain pin to VCC(set 1 state)

<b>Success response</b>: 

               {"state":"ok","message":"on pin ok"}\r\n'

<b>Bad responses</b>:

               {"state":"error","message":"already on"}\r\n'     --- pin is already in on state
               
               {"state":"error","message":"too often"}\r\n'      --- can't be on, too often request, need wait safe time 
               
               {"state":"error","message":"pin corrupted"}\r\n'  --- pin corrupted, on or off command has no effect and pin was disabled
               
               {"state":"error","message":"shorted to GND"}\r\n' --- command has no effect, pin leave GND state, pin was disabled for safety and mark as corrupted
               
<hr>
<b>Command</b> : 'q', 'w', 'e', 'r', 't' - connect certain pin to GND(set 0 state)

<b>Success response</b>: 

               {"state":"ok","message":"off pin ok"}\r\n'

<b>Bad responses</b>:     


               {"state":"error","message":"already off"}\r\n'    --- pin is already in off state
               
               {"state":"error","message":"pin corrupted"}\r\n'  --- pin corrupted, on or off command has no effect and pin was disabled
               
               {"state":"error","message":"shorted to VCC"}\r\n' --- command has no effect, pin leave GND state, pin was disabled for safety and mark as corrupted

<hr>
<b>Command</b> : 'a' - get error state, number of reboots

<b>Success response</b>: 

               {'last_reset_reasons': 1, 'reset_counter': 0, 'last_error': 1}\r\n

<b>Bad responses(example)</b>:     

               {'last_reset_reasons': 8, 'reset_counter': 5, 'last_error': 4}\r\n
