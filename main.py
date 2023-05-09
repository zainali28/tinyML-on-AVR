# import serial
# import matplotlib.pyplot as plt
# from sklearn.linear_model import LinearRegression
# from sklearn.model_selection import train_test_split
# import pandas as pd
# import numpy as np
# from random import randint
# import threading
# import keyboard

# df = pd.read_csv('')
# X = df.drop('', axis=1)
# y = df['']
#
# X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0)
#
# reg = LinearRegression()
#
# reg.fit(X_train, y_train)
#
# M = reg.coef_
# C = reg.intercept_


# ser = serial.Serial('COM3', 4800)
# lock = threading.Lock()
#
# fig, ax = plt.subplots(figsize=(10, 10))
# ax.set(title='Serial Plotter', xlabel='time', ylabel='Temperature')
# x, y = [], []


# for i in range(100):
#     a = randint(0, 10)
#     x.append(i)
#     y.append(a)
#
#     print(a)
#     ax.plot(x, y)
#     plt.pause(0.5)
#
# plt.show()

# while True:
#     for i in range(10):
#         a = randint(0, 10)
#         x.append(i)
#         y.append(a)
#
#         print(a)
#         ax.plot(x, y)
#         plt.pause(0.5)
#
#     # x.clear()
#     # y.clear()
#     plt.cla()
#
# plt.show()

# initial = 0
# user_input = None

# def fun():
#     global initial
#
#     while ser.is_open:
#         with lock:
#             if ser.in_waiting > 0 and ser.out_waiting == 0:
#                 a = ser.read().decode().rstrip()
#                 y.append(a)
#                 x.append(initial)
#                 initial += 1
#
#                 print(a)
#
#                 ax.plot(x, y)
#                 plt.pause(0.5)
#
#
#         plt.show()
#
#
# def fun2():
#     while ser.is_open:
#         user_input = input('INPUT: ')
#         with lock:
#             if ser.in_waiting == 0:
#                 ser.write(user_input)
#                 print('DONE!')
#
#
# thread1 = threading.Thread(target=fun)
# thread2 = threading.Thread(target=fun2)
#
# thread1.start()
# thread2.start()
#
# thread1.join()
# thread2.join()

# while ser.is_open:
#
#     if ser.in_waiting > 0:
#         if len(y) < 10:
#             a = ser.read().decode()
#
#             y.append(a)
#             x.append(initial + 1)
#             x.pop()
#
#             print(a)
#
#             ax.plot(x, y)
#             plt.pause(0.5)
#
#         else:
#             x.clear()
#             y.clear()
#             plt.cla()
#
#     ser.write(str(input('Send command: ')).encode())
#
# plt.show()

import serial
import threading
import time
import matplotlib.pyplot as plt
import keyboard

ser = serial.Serial('COM5', 4800) # open serial port
lock = threading.Lock() # create a lock for accessing the serial port

x = []
y = []
initial = 0
user_input = ""

def read_serial():
    global initial

    while ser.is_open:
        with lock:
            if ser.in_waiting > 0 and ser.out_waiting == 0:
                a = ser.read().decode().rstrip()
                y.append(a)
                x.append(initial)
                initial += 1

                print(a)

        time.sleep(0.1)

def plot_data():
    fig, ax = plt.subplots()

    while True:
        with lock:
            ax.plot(x, y)
            plt.draw()
        plt.pause(0.1)

def write_serial():
    global user_input
    last_input = ""
    while ser.is_open:
        with lock:
            if user_input != last_input:
                ser.write(user_input.encode())
                print(f'{user_input}')
                last_input = user_input

        time.sleep(0.1)

def on_key_press(event):
    global user_input

    if event.event_type == "down":
            user_input = event.name

keyboard.on_press(on_key_press)

# create three threads, one for reading, one for plotting, and one for writing
read_thread = threading.Thread(target=read_serial)
plot_thread = threading.Thread(target=plot_data)
write_thread = threading.Thread(target=write_serial)

# start the threads
read_thread.start()
plot_thread.start()
write_thread.start()

# wait for the threads to finish (this will never happen in this example)
read_thread.join()
plot_thread.join()
write_thread.join()

# close the serial port
ser.close()