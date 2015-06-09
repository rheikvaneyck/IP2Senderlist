#!/usr/bin/env python 
import serial
import re
import serial.tools.list_ports
import time

station_file = 'stations.txt'
counter = 0
top15_sender = []

for line in open(station_file):

  if re.match(r'^\d+',line):
    counter += 1
    top15_sender.append(line.rstrip().split(';'))
  if counter > 15:
    break

for sender in top15_sender:
  print ';'.join(sender[0:2])

ser_port = ''
ports = list(serial.tools.list_ports.comports())
for p in ports:
  if re.match(r'.*2341:8036',p[2]):
    print "Arduino Lenonardo gefunden"
    ser_port = p[0]
  elif re.match(r'.*2341:0043',p[2]):
    print "Arduino Uno gefunden"
    ser_port = p[0]
  elif re.match(r'.*0403:6001',p[2]):
    print "Arduino Nano gefunden"
    ser_port = p[0]
  elif re.match(r'.*2341:003d',p[2]):
    print "Arduino Due gefunden"
    ser_port = p[0]

if ser_port == '':
  print "Keinen passenden Arduino gefunden"
  print p
  exit()

ser = serial.Serial(ser_port, 38400, timeout=None)
ser.write('knock')
line = ser.readline().rstrip()
if line == 'OK':
  print line
  for sender in top15_sender:
    to_send = ';'.join(sender[0:2])
    ser.write(to_send)
    print to_send
    line = ser.readline().rstrip()
    if line != 'OK':
      print "Da ging was schief"
      exit()
    time.sleep(3)

# while (1):
#   line = ser.readline()
#   if (len(line) > 0):
#     print line,
# ser.close()