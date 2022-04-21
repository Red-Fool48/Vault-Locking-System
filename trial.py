import mysql.connector
mydb = mysql.connector.connect(
    host="localhost", user="root", password="root")
mycursor = mydb.cursor()
try:
    dbname = 'iot'
    mycursor.execute('create database if not exists {}'.format(dbname))
    print('ok')
except mysql.connector.Error as err:
    print('error: ', err)
except:
    print("failed!")
try:
    mycursor.execute("use {}".format('iot'))
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
    mycursor.execute("use {}".format('iot'))
    usernames = ['lorem', 'ipsum']
    emails = ['lorem@mail.com', 'ipsum@mail.com']
    passwords = ['merol', 'muspi']
    for i in range(len(usernames)):
        try:
            mycursor.execute(
                "insert into vaultMapping (`username`, `name`, `password`, `email`, `pin`) values(%s, %s, %s, %s, %s)", (usernames[i], usernames[i], passwords[i], emails[i], i+2))
            mydb.commit()
        except Exception as e:
            print('failed! :()((', e)
except Exception as ex:
    print('failed::(', ex)
