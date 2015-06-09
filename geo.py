#!/usr/bin/env python 
import urllib
import urllib2
from urllib2 import urlopen
from contextlib import closing
import json

stadt = ''
longitude = ''
latitude = ''

# Automatically geolocate the connecting IP
# url = 'http://freegeoip.net/json/'
url ='http://ip-api.com/json'
try:
	with closing(urlopen(url)) as response:
		location = json.load(response)
		print location
		stadt = location['city']
		#longitude = location['longitude']
		#latitude = location['latitude']
		longitude = location['lon']
		latitude = location['lat']		
		print stadt
		print longitude
		print latitude
except:
	print("Location could not be determined automatically")


