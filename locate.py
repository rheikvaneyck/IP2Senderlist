#!/bin/env python
import urllib

ip_addr = ''

response = urllib.urlopen('http://api.hostip.info/get_html.php?ip=173.245.51.105&position=true').read()

print(response)
