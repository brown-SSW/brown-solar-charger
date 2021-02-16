#!/usr/bin/python

import sqlite3
import paho.mqtt.client as mqtt
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import struct

jsonData=[]

def dataCallback(client,userdata,message):
	global jsonData
	topic=str(message.topic)
	counter=0
	msgArr=message.payload


	if(len(msgArr)==8):

		vala=struct.unpack('i',msgArr[counter:counter+4])[0]
		counter+=4

		valb=struct.unpack('f',msgArr[counter:counter+4])[0]
		counter+=4

		connection=sqlite3.connect("testDB.db")
		cursor=connection.cursor()
		cursor.execute("""insert into data values(datetime('now'),?,?)""",(vala,valb))
		jsonData=cursor.execute("select * from data").fetchall()
		connection.commit()


class MyServer(BaseHTTPRequestHandler):
	def do_GET(self):
		self.send_response(200)
		self.send_header("Content-type", "application/json")
		self.end_headers()
		self.wfile.write(json.dumps(jsonData).encode('utf-8'))



if __name__ == "__main__":        
	webServer = HTTPServer(('', 8080), MyServer)

	myClient=mqtt.Client("data_processor")
	myClient.connect("localhost",1883)
	myClient.subscribe("dataTopic")
	myClient.on_message=dataCallback
	myClient.loop_start()

	try:
		webServer.serve_forever()
	except KeyboardInterrupt:
		pass

	webServer.server_close()
	print("Server stopped.")
