#!/usr/bin/python3
import cgi

# Création de l'objet CGI
form = cgi.FieldStorage()

# Récupération des paramètres fname et lname
fname = form.getvalue('fname', '(undefined)')
lname = form.getvalue('lname', '(undefined)')

# Contenu de la page
content = [
    "<html>",
    "<head>",
    "<title>Hello - Second CGI Program</title>",
    "</head>",
    "<body>",
    "<h2>Hello {} {}</h2>".format(fname, lname),
    "</body>",
    "</html>"
]

# Calcul de la longueur du contenu
contentLength = sum(len(line) for line in content)

# Impression des en-têtes HTTP
print("HTTP/1.1 200 OK")
print("Content-Length: {}".format(contentLength))
print("Content-Type: text/html\r\n")
# Impression du contenu de la page
for line in content:
    print(line)
