#!/usr/bin/python3

import datetime
import cgi

print("HTTP/1.1 200 OK")
print("Content-Type: text/html\r\n")
print("\r\n\r\n")
print("<html>")
print("<head>")
print(datetime.datetime.strftime(datetime.datetime.now(), "<h1>  %H:%M:%S </h1>"))
