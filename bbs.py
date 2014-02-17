# -*- coding: utf-8 -*-

import telnetlib
import time

# CONFIG
WAIT 		= 0.2

# USER INFORMATION
HOST 		= "cd.twbbs.org"
USER_ID 	= ""
PASSWORD 	= ""

# DECISION
R_USER 		= "帳號".decode("utf-8").encode("big5")
R_PASS 		= "密碼".decode("utf-8").encode("big5")
MENU_CHECK 	= "主功能表".decode("utf-8").encode("big5")
KICK 		= "重複".decode("utf-8").encode("big5")
ANY_KEY 	= "任意鍵".decode("utf-8").encode("big5")

def readResponse():
	time.sleep(WAIT)              		# wait for greeter
	content = tn.read_very_eager()		# optional step
	CD = content.decode('big5', 'ignore').encode('utf-8')
	print CD

tn = telnetlib.Telnet( HOST )
tn.set_debuglevel(2)

# Login
q = tn.expect([R_USER] , 10)
tn.write( USER_ID + '\r' )
tn.expect([R_PASS] , 10 )
tn.write( PASSWORD + '\r' )

time.sleep(WAIT)              		# wait for greeter
content = tn.read_very_eager()		# optional step
CD = content.decode('big5', 'ignore').encode('utf-8')
print CD

# Another device is using
if KICK in content:
	print "重複登入"
	tn.write( "Y\r".encode("big5") )
	time.sleep(WAIT)              		# wait for greeter
	content = tn.read_very_eager()		# optional step
	CD = content.decode('big5', 'ignore').encode('utf-8')
	print CD

# Bullentin
while ANY_KEY in content:
	tn.write( "\r" )
	time.sleep(WAIT)
	content = tn.read_very_eager()
	CD = content.decode('big5', 'ignore').encode('utf-8')
	print CD

# Main menu, switch to CCU_market
tn.write("s")
time.sleep(WAIT)
tn.write("CCU_market\r".encode("big5"))
time.sleep(WAIT)
content = tn.read_very_eager()
CD = content.decode('big5', 'ignore').encode('utf-8')
print CD

# Board Bullentin
if ANY_KEY in content:
	tn.write( "\r" )
	time.sleep(WAIT)
	content = tn.read_very_eager()
	CD = content.decode('big5', 'ignore').encode('utf-8')
	print CD

print "DONE"





