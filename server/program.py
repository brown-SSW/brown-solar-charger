#!/usr/bin/python

import sqlite3
import paho.mqtt.client as mqtt
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
import json

jsonData=['hhh']

def dataCallback(client,userdata,message):
	global jsonData
	topic=str(message.topic)
	message=str(message.payload.decode("utf-8"))
	print(topic+': '+message)
	splitMessage=message.split(",")
	if len(splitMessage)==2:
		connection=sqlite3.connect("testDB.db")
		cursor=connection.cursor()
		cursor.execute("""insert into data values(datetime('now'),?,?)""",(splitMessage[0],splitMessage[1]))
		jsonData=cursor.execute("select * from data").fetchall()
		print(jsonData)
		connection.commit()

class MyServer(BaseHTTPRequestHandler):
	def do_GET(self):
		self.send_response(200)
		self.send_header("Content-type", "application/json")
		self.end_headers()
		print(jsonData)
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
