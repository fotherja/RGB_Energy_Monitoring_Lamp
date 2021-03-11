import serial
import datetime
import time

ser = serial.Serial(
                       port='/dev/serial0',					#port='/dev/ttyUSB0',        /dev/ttyAMA0
                       baudrate = 115200,
                       parity=serial.PARITY_NONE,
                       stopbits=serial.STOPBITS_ONE,
                       bytesize=serial.EIGHTBITS,
                       timeout=None
                   )				   
				   
Minute_Flashes = 0
High_Count = 0
				   
while(True):
	data = ser.readline()   								# read a '\n' terminated line		
	Minute_Flashes = int(data.decode())

	if(Minute_Flashes == 0):		 						# A new minute has been determined on the Arduino, log data in the current minute slot
		x = datetime.datetime.now()
		DateLogPath = "/home/pi/Energy_Lamp/Energy_Logs/"
		FileTodayStr = DateLogPath + "Energy-" + x.strftime("%d") + "-" + x.strftime("%b") + "-" + x.strftime("%y") + ".txt"
		CurrentTimeStr = x.strftime("%H") + ":" + x.strftime("%M") + " "

		f = open(FileTodayStr, "a")
		f.write(CurrentTimeStr)
		f.write(str(High_Count) + "\n")
		f.close()

	High_Count = Minute_Flashes
	print(Minute_Flashes)
	