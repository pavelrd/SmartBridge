Программа для контроллера atmega16 предоставляет uart мост для возможности управления
оптронами и/или реле и получения данных температуры. Также не дает возможности слишком часто 
включать оптроны и реле, чтобы от слишком частого включения не сломалось оборудование. Это сделано
путем настраиваемой для каждого канала в отдельности задержки перед включением. Выключение реализовано немедленно.
В процессе разработки. 

Плата AtmegaDebugBoard. Схема подключения реле, оптронов и датчиков:
![image](https://github.com/user-attachments/assets/4167a024-93a4-4a80-b9a8-e8c8163609b1)

