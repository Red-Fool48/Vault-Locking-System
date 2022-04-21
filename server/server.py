from flask import *
from flask_wtf import *
from wtforms import *
from wtforms.fields import *
from wtforms.validators import *
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


class inpForm(FlaskForm):
    name = StringField('Name', validators=[DataRequired()])
    username = StringField('Username', validators=[
                           DataRequired(), Length(min=2, max=20)])
    password = PasswordField('Password', validators=[
                             DataRequired(), Length(max=1000)])
    email = StringField('Email', validators=[DataRequired()])
    # birth_date = DateField('DOB', format='%Y-%m-%d',
    #                        validators=[DataRequired()])
    submit = SubmitField('Submit')


@app.route('/', methods=['GET'])
def getData():
    form = inpForm()
    return render_template('home.html', form=form)


@app.route('/', methods=['POST'])
def postData():
    pinNo = -1
    form = inpForm()
    username, password = form.username.data, form.password.data
    mycursor.execute(
        "select username, password, pin from iot.vaultMapping where username=%s and password=%s", (username, password))
    for (username, password, pin) in mycursor:
        print(username, pin, password)
        print(pin)
        pinNo = int(pin)
    if pinNo != -1:
        arduino.digital[pinNo].write(1)
        time.sleep(5)
        arduino.digital[pinNo].write(0)
        # arduino.send_sysex(
        #     STRING_DATA, util.str_to_two_byte_iter('SUCCESS!\n'))
        return render_template('home.html', form=form, flash='LoggedIn!')
    else:

        return render_template('home.html', form=form)


app.run(port=5000)
