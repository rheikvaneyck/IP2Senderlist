#!/bin/env python
# -*- codepage: utf-8 -*-
import urllib
import urllib2
from urllib2 import urlopen
from contextlib import closing
import json
import dryscrape
import re
from decimal import Decimal
from operator import itemgetter

#
# Geolokation per IP-Adresse
#
# stadt = 'Berlin'
# longitude = '13.4584'
# latitude = '52.4975'
stadt = ''
longitude = ''
latitude = ''

url ='http://ip-api.com/json'
try:
	with closing(urlopen(url)) as response:
		location = json.load(response)
		print location
		stadt = location['city']
		longitude = location['lon']
		latitude = location['lat']		
		print stadt
		print longitude
		print latitude
except:
	print("Location could not be determined automatically")

#
# Suche lokaler Radiosender mit fmscan.org
#
url = 'http://www.fmscan.org/'
sess = dryscrape.Session(base_url = url)

# we don't need images
sess.set_attribute('auto_load_images', False)

# visit homepage and search for a term
sess.visit('/')
freq = sess.at_xpath('//a[@href="form.php?m=s&ex=0"]')
freq.click()

q = sess.at_xpath('//*[@name="search"]')
search_term = stadt
q.set(search_term)
q.form().submit()


# sess.render('geo.png')
# print "Screenshot written to 'geo.png'"

locations = []
# extract all links
for link in sess.xpath('//a[@href]'):
  url = link['href']
  m = re.search('qthset=([\w\s\./,]+)&lset=([-]*\d+\.\d+)&bset=([-]*\d+\.\d+)', url)
  if m is not None:
  	# print 'Stadt:', m.group(1),'longitude:', m.group(2), 'latitude:', m.group(3)
  	diff = abs(Decimal(longitude) - Decimal(m.group(2))) + abs(Decimal(latitude) - Decimal(m.group(3)))
  	locations.append((url, m.group(1), m.group(2), m.group(3), diff))

s = sorted(locations, key=itemgetter(4))
print 'In der Datenbank gefunden:'
print 'url:', s[0][0]
print 'Stadt:', s[0][1]
print 'longitude:', s[0][2]
print 'latitude:', s[0][3]
print 'Entfernung:', "{:5.5f}".format(s[0][4]), "°"

url = s[0][0]
sess.visit(url)

g = sess.at_xpath('//input[@value="Generate"]')
g.form().submit()

stations = [('Freq','RDS','Programm','Distanz','dBuV')]
rows = sess.xpath('//div[2]/table[1]/tbody/tr')
for row in rows:
	if (len(row.xpath('./td/a/u'))>0):
		freq = row.xpath('./td/a/u')[0].text()
	else:
		freq = ''
	if (len(row.xpath('./td[5]'))>0):
		rds = row.xpath('./td[5]')[0].text()
	else:
		rds = ''
	if (len(row.xpath('./td[6]'))>0):
		program = row.xpath('./td[6]')[0].text()
	else:
		program = ''
	if (len(row.xpath('./td[10]'))>0):
		dist = row.xpath('./td[10]')[0].text()
	else:
		dist = ''
	if (len(row.xpath('./td[12]'))>0):
		dBuV = Decimal(re.search('(\d+)',row.xpath('./td[12]')[0].text()).group(1))
		# dBuV = row.xpath('./td[12]')[0].text()
	else:
		dBuV = 0.0
	stations.append((freq, rds, program, dist, dBuV))

s = sorted(stations, key=itemgetter(4), reverse=True)
station_file = 'stations.txt'
open(station_file,'w').write('\n'.join('%s;%s;%s;%s;%s' % x for x in s))
print 'Lokale Radiosender in die Datei %s geschrieben' % station_file
