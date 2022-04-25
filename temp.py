from flask import *
from flask_wtf import *
from wtforms import *
from wtforms.fields import *
from wtforms.validators import *
from pyfirmata import Arduino, util, STRING_DATA, SERVO, INPUT
import time
from flask_session import Session
import mysql.connector
import mysql.connector
try:
    mydb = mysql.connector.connect(
        host="127.0.0.1", user='root', password='root', port='3306')
    print('ok!')
except:
    print('Failed!')
mycursor = mydb.cursor()

try:
    dbname = 'iot'
    mycursor.execute('create database if not exists iot')
    print('ok')
except mysql.connector.Error as err:
    print('error: ', err)
except:
    print("failed!")
try:
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
    TABLES['admin'] = (
        "CREATE TABLE if not exists `admin`(",
        "`username` varchar(100) not null",
        "`password` varchar(100) not null",
        ") ENGINE=InnoDB")
    # for table in TABLES:
    #     mycursor.execute(TABLES[table])
except mysql.connector.Error as err:
    print('failed to create! :(')


class adminForm(FlaskForm):
    username = StringField('Username', validators=[
                           DataRequired(), Length(min=2, max=20)])
    password = PasswordField('Password', validators=[
                             DataRequired(), Length(max=1000)])
    submit = SubmitField('Submit')
