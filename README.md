# RGB_Energy_Monitoring_Lamp
An RGB Lamp that changes it's colour based on the current electricity consumption


There are a few different all these files are placed an run on:


 Arduino Pro Mini at the electricity meter acting as the transmitter unit: 
  - Electricity_Monitoring_Lamp_Tx.ino
 
 Arduino Pro Mini in the lamp working as the receiver and LED PWM Controller: 
  - Electricity_Monitoring_Lamp_rx.ino
  - Average.h
 
 Raspberry Pi3 python3 daemon:
  - LoggerDaemon.py
 
 Rasbperry Pi3 /var/www/html/ web server directoy:
  - index.html
  - style.css
  - Z1.js
  - data_parser.php

