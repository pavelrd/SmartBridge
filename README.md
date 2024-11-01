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

![image](https://github.com/user-attachments/assets/4f47a603-d2b1-42dd-b7e9-972971a49ad5)

Python telegram bot:

Programming:

avrdude.exe -c usbasp -p m328 -U lfuse:w:0xff:m -U hfuse:w:0xd9:m -U efuse:w:0xfd:m 

Protocol:

UART, baudrate - 9600

Command  - 'g' - get all pin and sensors state

Response example(without ds18b20 connected): 


{'c0': 0, 'c1': 0, 'c2': 0, 'c3': 0, 'c4': 0, 'd0': 1, 'd1': 0, 'd2': 1, 'd3' : 1, 'd4': 0, adc0': 0.31, 'adc1': 0.0, 'adc2': 0.08, 'adc3': 0.0, 'adc4': 0.0, 'adc5': 0.0, 'adc6': 0.0, 'adc7': 0.0}\r\n
{'crc': 12345 }\r\n


Response example(with 5 DS18B20 connected):


{'c0': 0, 'c1': 0, 'c2': 0, 'c3': 0, 'c4': 0, 't0' : 22.1, 't1' : 22.2, 't2' : 22.3, 't3' : 22.7, 't4' : 22.1, 'd0': 1, 'd1': 0, 'd2': 1, 'd3' : 1, adc0': 0.31, 'adc1': 0.0, 'adc2': 0.08, 'adc3': 0.0, 'adc4': 0.0, 'adc5': 0.0, 'adc6': 0.0, 'adc7': 0.0}\r\n
{'crc': 12345 }\r\n


Command - '1', '2', '3', '4', '5' - connect certain pin to VCC(set 1 state)

Success response: 

{"state":"ok","message":"on pin ok"}\r\n'

Bad responses:

               {"state":"error","message":"already on"}\r\n'     --- pin is already in on state
               
               {"state":"error","message":"too often"}\r\n'      --- can't be on, too often request, need wait safe time 
               
               {"state":"error","message":"pin corrupted"}\r\n'  --- pin corrupted, on or off command has no effect and pin was disabled
               
               {"state":"error","message":"shorted to GND"}\r\n' --- command has no effect, pin leave GND state, pin was disabled for safety and mark as corrupted
               

Command - 'q', 'w', 'e', 'r', 't' - connect certain pin to VCC(set 0 state)

Success response: 

{"state":"ok","message":"off pin ok"}\r\n'

Bad responses:     


               {"state":"error","message":"already off"}\r\n'    --- pin is already in off state
               
               {"state":"error","message":"pin corrupted"}\r\n'  --- pin corrupted, on or off command has no effect and pin was disabled
               
               {"state":"error","message":"shorted to VCC"}\r\n' --- command has no effect, pin leave GND state, pin was disabled for safety and mark as corrupted
               
