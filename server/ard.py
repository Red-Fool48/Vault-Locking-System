from pyfirmata import Arduino, util, STRING_DATA, Pin, SERVO, INPUT
import time

arduino = Arduino('COM1')
arduino.digital[7].mode = SERVO
ninePin = arduino.get_pin('d:9:i')
ninePin.mode = INPUT
# arduino.digital[7].write(True)
# arduino.digital[8].write(True)
# pin = arduino.get_pin('d:8:o')
while 1:
    data = input('enter id\t')
    if data == 'hello':
        # arduino.digital[8].write(1)
        # time.sleep(1.5)
        # arduino.digital[8].write(0)
        # time.sleep(1.5)
        arduino.send_sysex(STRING_DATA, util.str_to_two_byte_iter('SUCCESS!'))
        arduino.digital[7].write(180)
        arduino.digital[8].write(1)
        # print(ninePin)
        # print(arduino.digital[9].read())
        time.sleep(20)
        arduino.digital[7].write(0)
        # while 1:
        #     if arduino.digital[9].read() == 1:
        #         print("ok!@")
        #         break
        # arduino.digital[8].write(0)
        # time.sleep(2)
        # if arduino.digital[9].read()==1:

        # time.sleep(2)
        arduino.digital[8].write(0)
    else:
        arduino.send_sysex(STRING_DATA, util.str_to_two_byte_iter('WRONG!'))
        print("wrong!")
    # data = input('Enter data\t')
    # if data == '':
    #    arduino.digital[8].write(False)
    #    print(pin.read())
        # arduino.send_sysex(STRING_DATA, util.str_to_two_byte_iter('EXIT!!'))
        # arduino.digital[8].write(1)
        # print("..\n")
        # time.sleep(0.2)
        # arduino.digital[8].write(0)
        # print(arduino.get_pin('d:8:o'))
        # break
    # elif data == 'hello':
    #     pin.write(True)
    #     print('here!')
    #    arduino.send_sysex(STRING_DATA, util.str_to_two_byte_iter('hola!'))
    # else:
    #    arduino.digital[8].write(False)
        # print(pin.read())
    #    arduino.send_sysex(STRING_DATA, util.str_to_two_byte_iter(data))
        # arduino.digital[8].write(1)
        # arduino.digital[8].write(1)
        # time.sleep(0.2)
        # print("here!")
        # arduino.digital[8].write(0)
        # print(arduino.get_pin('d:8:o'))
