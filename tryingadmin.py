
from logging import exception
from flask import *
from flask_wtf import *
from sqlalchemy import true
from wtforms import *
from wtforms.fields import *
from wtforms.validators import *
from pyfirmata import Arduino, util, STRING_DATA, SERVO, INPUT
import time
from flask_session import Session
import mysql.connector
arduino = Arduino('COM1')
for i in range(2, 7):
    # arduino.pinmode(i, OUTPUT)
    arduino.digital[i].write(0)
mydb = mysql.connector.connect(
    host="127.0.0.1", user='root', password='root', port='3306')
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
        " `pinStatus` varchar(10) not null default 0',"
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
    print(err)


class adminForm(FlaskForm):
    username = StringField('Username', validators=[
                           DataRequired(), Length(min=2, max=20)])
    password = PasswordField('Password', validators=[
                             DataRequired(), Length(max=1000)])
    submit = SubmitField('Submit')


class addUserForm(FlaskForm):
    name = StringField('Name', validators=[DataRequired()])
    username = StringField('Username', validators=[
                           DataRequired(), Length(min=2, max=20)])
    password = PasswordField('Password', validators=[
                             DataRequired(), Length(max=1000)])
    email = StringField('Email', validators=[DataRequired()])
    # birth_date = DateField('DOB', format='%Y-%m-%d',
    #                        validators=[DataRequired()])
    pin = IntegerField('Pin number', validators=[
        DataRequired()])
    submit = SubmitField('Submit')


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


class modifyForm(FlaskForm):
    name = StringField('Name', validators=[DataRequired()])
    username = StringField('Username', validators=[
        DataRequired(), Length(min=2, max=20)])
    email = StringField('Email', validators=[DataRequired()])
    pin = IntegerField('Pin', validators=[DataRequired()])
    submit = SubmitField('Submit')


app = Flask(__name__)

app.config['SECRET_KEY'] = 'C2HWGVoMGfNTBsrYQg8EcMrdTimkZfAb'
app.config['SESSION_TYPE'] = 'filesystem'
app.config['SESSION_PERMANENT'] = False
Session(app)

# user login:
# @app.route('/', methods=['GET'])
# def getData():
#     form = inpForm()
#     return render_template('home.html', form=form)

# user login processing:
# @app.route('/', methods=['POST'])
# def postData():
#     try:
#         pinNo = -1
#         form = inpForm()
#         id = -1
#         username, password = form.username.data, form.password.data
#         mycursor.execute(
#             "select id, username, password, pin from iot.vaultMapping where username=%s and password=%s", (username, password))
#         l = []
#         for (id, username, password, pin) in mycursor:
#             print(username, pin, password)
#             print(pin)
#             temp = {}
#             temp['pin'] = int(pin)
#             pinNo = int(pin)
#             id = id
#             l.append(temp)
#         if pinNo != -1:
#             arduino.digital[pinNo].write(1)
#             time.sleep(500)
#             arduino.digital[pinNo].write(0)
#             # arduino.send_sysex(
#             #     STRING_DATA, util.str_to_two_byte_iter('SUCCESS!\n'))
#             return render_template('home.html', form=form, flash='LoggedIn!')
#         else:
#             return render_template('home.html', form=form)
#     except Exception as e:
#         print('err!', e)

# USER LOGIN:


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
    id = -1
    pinNo = -1
    mycursor.execute(
        "select id, pin from iot.vaultMapping where username=%s and password=%s", (username, password))
    for (id, pin) in mycursor:
        id = id
        pinNo = int(pin)
        print(id, pinNo)
    if pinNo != -1:
        arduino.digital[pinNo].write(1)
        time.sleep(500)
        arduino.digital[pinNo].write(0)
    return render_template('asyncLogin.html')


# admin Login


@app.route('/admin', methods=['GET', 'POST'])
def login():
    if not session.get('adminLoggedIn'):
        if request.method == 'GET':
            form = adminForm()
            return render_template('adminLogin.html', form=form)
        elif request.method == 'POST':
            form = adminForm()
            username = form.username.data
            password = form.password.data
            try:
                mycursor.execute('use iot')
                mycursor.execute(
                    "select * from iot.admin where username=%s and password=%s", (username, password))
                session['adminLoggedIn'] = True
                session['username'] = username
                return render_template('admin.html', name=username)
            except:
                return render_template('adminLogin.html', form=form)
    else:
        return render_template('admin.html', name=session['username'])


# admin AllData
@app.route('/admin/allData', methods=['GET'])
def getAllData():
    if not session.get('adminLoggedIn'):
        return redirect(url_for('getData'))
    elif session.get('adminLoggedIn') == True:
        get_users = ("SELECT id,name,username,email,pin FROM iot.vaultMapping")
        mycursor.execute(get_users)
        users = []
        for (id, name, username, email, pin) in mycursor:
            temp = {}
            temp['id'] = id
            #print("ID: ", id)
            temp['name'] = name
            #print("Name: ", name)
            temp['username'] = username
            temp['email'] = email
            temp['pin'] = pin
            users.append(temp)
        return render_template('data.html', title='All users', len=len(users), data=users)
    else:
        return redirect(url_for('getData'))
# Modify data:


@app.route('/admin/modify/<id>', methods=['GET', 'POST'])
def handleRequest(id):
    if not session.get('adminLoggedIn') and request.method == 'GET':
        return redirect(url_for('getData'))
    if request.method == 'POST' and session.get('adminLoggedIn'):
        form = modifyForm()
        if int(form.pin.data) < 2 or int(form.pin.data) > 13:
            return redirect(url_for('handleRequest'), id=id)
        else:
            # if data modified successfully:
            query = 'update iot.vaultMapping set username=%s, email=%s, pin=%s where id=%s'
            mycursor.execute(query, (form.username.data,
                             form.email.data, form.pin.data, id))
            mydb.commit()
            return redirect(url_for('getAllData'))
    elif request.method == 'GET' and session.get('adminLoggedIn'):
        query = 'select username, email, pin from iot.vaultMapping where id={}'.format(
            id)
        data = mycursor.execute(query)
        res = []
        for (username, email, pin) in mycursor:
            temp = {}
            temp['username'] = username
            temp['email'] = email
            temp['pin'] = pin
            temp['id'] = id
            res.append(temp)
            break
        print(res)
        form = modifyForm()
        return render_template('modify.html', form=form, data=res)
    else:
        return redirect(url_for('getData'))

# log out


@app.route('/logout', methods=['GET', 'POST'])
def logout():
    session.pop('userLoggedIn', None)
    return redirect(url_for('getData'))

# admin log out


@app.route('/adminLogout', methods=['GET', 'POST'])
def adminLogout():
    if session.get('adminLoggedIn'):
        session.pop('adminLoggedIn')
        return redirect(url_for('getData'))
    else:
        return redirect(url_for('getData'))


@app.route('/admin/ViewAllPins', methods=['GET'])
def showAllPins():
    if session.get('adminLoggedIn'):
        data = []
        for i in range(2, 13):
            temp = {}
            temp['id'] = i
            data.append(temp)
        return render_template('viewAllPinStatus.html', data=data, len=len(data))
    return redirect('getData')


@app.route('/admin/switchOn/<id>', methods=['GET', 'POST'])
def switchOn(id):
    if session.get('adminLoggedIn'):
        print(id)
        arduino.digital[int(id)].write(1)
        return redirect(url_for('showAllPins'))
    return redirect('getData')


@app.route('/admin/switchOff/<id>', methods=['GET', 'POST'])
def switchOff(id):
    if session.get('adminLoggedIn'):
        print(id)
        arduino.digital[int(id)].write(0)
        return redirect(url_for('showAllPins'))
    return redirect('getData')


@app.route('/admin/addUser', methods=['POST', 'GET'])
def handleAddUser():
    if session.get('adminLoggedIn'):
        if request.method == 'POST':
            try:
                form = addUserForm()
                username, password, name, pin, email = form.username.data, form.password.data, form.name.data, form.pin.data, form.email.data
                if pin > 13 or pin < 2:
                    raise Exception('Bad pin!')
                query = 'insert into iot.vaultMapping(username,password,name,pin,email) values(%s,%s,%s,%s,%s)'
                mycursor.execute(query, (username, password, name, pin, email))
                mydb.commit()
                return redirect(url_for('login'))
            except Exception as err:
                print(err)
        elif request.method == 'GET':
            form = addUserForm()
            return render_template('addNewUser.html', form=form)
    return redirect('getData')


if __name__ == "__main__":
    app.run(port=5000)
