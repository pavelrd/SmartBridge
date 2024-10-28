Программа для контроллера atmega16 предоставляет uart мост для возможности управления
оптронами и/или реле и получения данных температуры. Также не дает возможности слишком часто 
включать оптроны и реле, чтобы от слишком частого включения не сломалось оборудование. Это сделано
путем настраиваемой для каждого канала в отдельности задержки перед включением. Выключение реализовано немедленно.
В процессе разработки. 

Плата AtmegaDebugBoard. Схема подключения реле, оптронов и датчиков:
![image](https://github.com/user-attachments/assets/4167a024-93a4-4a80-b9a8-e8c8163609b1)

Command for load fuses for AtmegaDebugBoard: 

avrdude.exe -c usbasp -p m16 -U lfuse:w:0xff:m -U hfuse:w:0xd9:m

Command for load fuses for ArduinoNano:

[in process]

Protocol:

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
