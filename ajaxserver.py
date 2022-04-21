from flask import *
from flask_wtf import *
from wtforms import *
from wtforms.fields import *
from wtforms.validators import *
import json
from pyfirmata import Arduino, util, STRING_DATA, SERVO, INPUT
import time
import mysql.connector
arduino = Arduino('COM1')
for i in range(2, 7):
    # arduino.pinmode(i, OUTPUT)
    arduino.digital[i].write(0)
mydb = mysql.connector.connect(
    host="localhost", user="root", password="root")
mycursor = mydb.cursor(buffered=True)
try:
    dbname = 'iot'
    mycursor.execute('create database if not exists iot')
    print('ok')
except mysql.connector.Error as err:
    print('error: ', err)
except:
    print("failed!")
try:
    mycursor.execute("use iot")
    TABLES = {}
    TABLES['vaultMapping'] = (
        "CREATE TABLE if not exists `vaultMapping` ("
        " `id` int(11) NOT NULL AUTO_INCREMENT,"
        "`username` varchar(100) not null,"
        " `name` varchar(100) NOT NULL,"
        " `password` varchar(100) NOT NULL,"
        " `email` varchar(100) NOT NULL,"
        " `pin` varchar(5) not null,"
        " PRIMARY KEY (`id`)"
        ") ENGINE=InnoDB")
    for table in TABLES:
        mycursor.execute(TABLES[table])
except mysql.connector.Error as err:
    print('failed to create! :(')
    print(err)

try:
    mycursor.execute("use iot")
    mycursor.execute(
        'select name,username,password,pin from vaultMapping')

except Exception as e:
    print(e)

app = Flask(__name__)

app.config['SECRET_KEY'] = 'C2HWGVoMGfNTBsrYQg8EcMrdTimkZfAb'


@app.route('/', methods=['GET'])
def getData():
    return render_template('asyncLogin.html')


@app.route('/', methods=['POST'])
def postData():
    postedData = request.get_json()
    name = request.get_json()['name']
    username = request.get_json()['username']
    email = request.get_json()['email']
    password = request.get_json()['password']

    print(name, username, email, password)
    mycursor.execute(
        "select username, password, pin from iot.vaultMapping where username=%s and password=%s", (username, password))
    for (username, password, pin) in mycursor:
        print(username, pin, password)
        print(pin)
        pinNo = int(pin)
    if pinNo != -1:
        arduino.digital[pinNo].write(1)
        time.sleep(500)
        arduino.digital[pinNo].write(0)
    return render_template('asyncLogin.html')


app.run(port=5000)
