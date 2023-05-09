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
