Program for atmega16(arduino nano coming soon) provides a uart bridge to control relay and get voltages, digital states and temperature data from several DS18B20 

Program also does not allow relays to be turned on too often(to prevent equipment from breaking down).
This is done by setting a delay before switching on separately for each channel. Switching off is implemented immediately.

In the process of development...

Protocol(UART, baudrate - 9600):

Command  - 'g' - get all pin and sensors state

Response example(without ds18b20 connected): 

{'vent': 0, 'heat': 0, 'light': 0, 'res0': 0, 'res1': 0, 'res2': 0, 'd0': 1, 'd1': 0, 'd2': 1, 'adc0': 0.31, 'adc1': 0.0, 'adc2': 0.08, 'adc3': 0.0, 'adc4': 0.0, 'adc5': 0.0, 'adc6': 0.0, 'adc7': 0.0}\r\n
{'crc': 12345 }\r\n

Command - '1', '2', '3', '4', '5', '6' - connect certain pin to VCC(set 1 state)

Success response: {"state":"ok","message":"on pin ok"}\r\n'

Bad responses:

               {"state":"error","message":"already on"}\r\n'     --- pin is already in on state
               
               {"state":"error","message":"too often"}\r\n'      --- can't be on, too often request, need wait safe time 
               
               {"state":"error","message":"pin corrupted"}\r\n'  --- pin corrupted, on or off command has no effect and pin was disabled
               
               {"state":"error","message":"shorted to GND"}\r\n' --- command has no effect, pin leave GND state, pin was disabled for safety and mark as corrupted
               

Command - 'q', 'w', 'e', 'r', 't', 'y' - connect certain pin to VCC(set 0 state)

Success response: {"state":"ok","message":"off pin ok"}\r\n'

Bad responses:     


               {"state":"error","message":"already off"}\r\n'    --- pin is already in off state
               
               {"state":"error","message":"pin corrupted"}\r\n'  --- pin corrupted, on or off command has no effect and pin was disabled
               
               {"state":"error","message":"shorted to VCC"}\r\n' --- command has no effect, pin leave GND state, pin was disabled for safety and mark as corrupted
               
Command for load fuses for AtmegaDebugBoard: 

avrdude.exe -c usbasp -p m16 -U lfuse:w:0x3f:m -U hfuse:w:0xd9:m

Command for load fuses for ArduinoNano:

[in process]
![image](https://github.com/user-attachments/assets/d4554ee4-bd03-4e85-9d48-b60d2b09ace3)

