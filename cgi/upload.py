#!/usr/bin/python3
import cgi
import os

form = cgi.FieldStorage()

fileitem = form['filename']

if fileitem.filename:
   open(os.getcwd() + '/cgi/' + os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
   message = 'The file "' + os.path.basename(fileitem.filename) + '" was uploaded to ' + os.getcwd() + '/cgi/'
else:
   message = 'Uploading Failed'

print("HTTP/1.1 200 OK")
print("Content-Type: text/html\r\n")
print("<H1> " + message + " </H1>")