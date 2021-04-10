import serial
import datetime
import time

ser = serial.Serial(
                       port='/dev/serial0',					
                       baudrate = 115200,
                       parity=serial.PARITY_NONE,
                       stopbits=serial.STOPBITS_ONE,
                       bytesize=serial.EIGHTBITS,
                       timeout=1
                   )				   
				   
TimeStamp = time.time()
Previous_Flash_Count = 0
Latest_Flash_Count = 0
				   
while(True):
	data = ser.readline().decode().strip()   						# read a '\n' terminated line with a 1 second time-out	
	
	if data:
		if(Previous_Flash_Count == 0):								# The pi must be starting up so just start with the current flash count
			Previous_Flash_Count = int(data)		
		
		Latest_Flash_Count = int(data)
	
	if(time.time() - TimeStamp > 60):								# a minute has elapsed since our last entry into the log
		TimeStamp = time.time()
		
		Flashes_Last_Minute = Latest_Flash_Count - Previous_Flash_Count
		Previous_Flash_Count = Latest_Flash_Count	
	
		x = datetime.datetime.now()
		DateLogPath = "/var/www/html/Energy_Logs/"
		FileTodayStr = DateLogPath + "Energy-" + x.strftime("%y") + "-" + x.strftime("%m") + "-" + x.strftime("%d") + ".txt"
		
		# Check how many minutes into the day we are atm. And check where we are up to in the file. If we're missing data in the file, pad with zeros...
		x = datetime.datetime.now()		
		daystart = datetime.datetime(x.year, x.month, x.day)			
		time_today = x - daystart		
		minutes_today = int(time_today.total_seconds() / 60)		# Minutes into the day so far		
		
		f = open(FileTodayStr, "a")									# ensures the file exists if it doesn't already
		f.close()
		
		line_count = len(open(FileTodayStr).readlines())			# reads the number of lines (i.e minutes into the file)
		
		f = open(FileTodayStr, "a")									# re-opens the file for appending to
		for x in range(max(minutes_today - line_count, 0)):			# pad any missing values			
			f.write("0\n")	
			
		f.write(str(Flashes_Last_Minute) + "\n")					# write our current minutes energy use.
		f.close()

		
		
		

	