import tkinter as tk
import matplotlib.pyplot as plt
import tkinter.ttk as ttk
import os
import copy
import serialMCUComm
import treeviewmenuclass as tvmc
from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg, NavigationToolbar2Tk)
from matplotlib.figure import Figure
from collections import deque
import time


def append_list_to_file(the_filename, the_list):
    os.makedirs(os.path.dirname(the_filename), exist_ok=True)
    with open(the_filename, 'a+') as f:
        f.write("\t".join([str(i) for i in the_list])+'\n')


class ValidationWindow(object):
    def __init__(self, masterapp, microcontroleur: serialMCUComm.SerialMCUObject, treeviewmenu: tvmc.TreeviewMenu):
        self.subwindow = tk.Frame(masterapp)
        self.microcontroleur = microcontroleur
        self.menu = treeviewmenu
        self.button_connect = tk.Button(self.subwindow, text="Connect", command=self.command_connect)
        self.button_stop = tk.Button(self.subwindow, text="Stop", command=self.command_stop)
        self.button_single = tk.Button(self.subwindow, text="Single", command=self.command_single)
        self.button_run = tk.Button(self.subwindow, text="Run", command=self.command_run)
        self.button_exposure = tk.Button(self.subwindow, text="Set exposure", command=self.command_exposure)
        self.button_run.grid(row=0, column=0, columnspan=2, sticky='ew')
        self.button_single.grid(row=0, column=2, columnspan=2, sticky='ew')
        self.button_stop.grid(row=0, column=4, columnspan=2, sticky='ew')
        self.button_connect.grid(row=1, column=0, columnspan=3, sticky='ew')
        self.button_exposure.grid(row=1, column=3, columnspan=3, sticky='ew')
        self.plotwindow = None  # PlotWindow(masterapp, microcontroleur)  # self.subwindow, microcontroleur)
        self.averaged_data = copy.copy(self.microcontroleur.data_adc_y)
        self.data_saving = True
        self.data_file_name = 'data.txt'
        self.data_folder_path = 'data/'
        self.data_append_increment = True

    def command_stop(self):
        self.microcontroleur.send_cmd(b'STP')

    def command_connect(self):
        if self.microcontroleur.state == "DISCONNECTED":
            self.microcontroleur.connection()
            self.button_connect['text'] = 'Disconnect'
            if self.microcontroleur.serial.inWaiting() > 0:
                print("the buffer is not empty")
                self.microcontroleur.serial.reset_input_buffer()
                self.microcontroleur.serial.reset_output_buffer()
                if self.microcontroleur.serial.inWaiting() > 0:
                    print("the buffer is still not empty")
                else:
                    print("the buffer is now empty")
            print('connected')
        else:
            self.microcontroleur.close()
            self.button_connect['text'] = 'Connect'
            print('disconnected')

    def command_single(self):
        self.microcontroleur.send_cmd(b'SGL')
        self.microcontroleur.data_last_cmd = self.microcontroleur.read_cmd(self.microcontroleur.size_word, timeout=1)
        print(self.microcontroleur.data_last_cmd)
        self.microcontroleur.data_raw_uart = \
            self.microcontroleur.read_cmd_until(b'FINISHED', max_size=self.microcontroleur.size_adc + 320,
                                                timeout=(self.microcontroleur.exposure+1))
        print("read :", len(self.microcontroleur.data_raw_uart), "points")
        self.microcontroleur.data_raw_uart = self.microcontroleur.data_raw_uart[8:-8]
        self.microcontroleur.convert_raw_uart_adc_data(len(self.microcontroleur.data_raw_uart))
        self.plotwindow.update_data(self.microcontroleur)
        self.assess_data_file()
        self.save_data()

    def command_run(self):
        self.microcontroleur.state = 'RUNNING'
        self.microcontroleur.send_cmd(b'RUN')
        self.microcontroleur.data_last_cmd = self.microcontroleur.read_cmd(self.microcontroleur.size_word, timeout=1)
        print(self.microcontroleur.data_last_cmd)
        counter = 0
        while self.microcontroleur.state == 'RUNNING':

            thread_wait_for_data(self.microcontroleur, self.plotwindow, cnt=counter, keyword=b'SENDING:',
                                 timeout=(self.microcontroleur.exposure+1))
            self.assess_data_file()
            self.save_data()
            counter += 1

    def command_exposure(self):
        self.microcontroleur.send_cmd(b'PSC', self.microcontroleur.prescaler)
        print("sending PSC :", self.microcontroleur.read_cmd(self.microcontroleur.size_word, timeout=1))
        self.microcontroleur.send_cmd(b'ARR', self.microcontroleur.autoreload)
        print("sending ARR :", self.microcontroleur.read_cmd(self.microcontroleur.size_word, timeout=1))
        self.microcontroleur.send_cmd(b'BIN', self.microcontroleur.binning)
        print("sending BIN :", self.microcontroleur.read_cmd(self.microcontroleur.size_word, timeout=1))

    def save_data(self):
        if not self.data_append_increment:
            self.data_file_name = self.increment_file_name(self.data_file_name)
            self.menu.item('Next Data', values=[self.data_file_name])
        self.assess_data_file()
        if self.data_folder_path[-1] == '/':
            file_path = self.data_folder_path + self.data_file_name
        else:
            file_path = self.data_folder_path + '/' + self.data_file_name
        append_list_to_file(file_path,self.microcontroleur.data_adc_y)

    def assess_data_file(self):
        self.data_file_name = self.menu.item('Next Data')['values'][0]
        self.data_saving = self.menu.item('Save')['values'][0]
        self.data_append_increment = self.menu.item('Append(True)/Inc. File(False)')['values'][0]
        self.data_folder_path = self.menu.item('File path')['values'][0]


    def increment_file_name(self,filename: str):
        temporary = filename.split("_")
        number = temporary[-1].split(".")
        if number[0].isdigit():
            number[0] = str(int(number[0]) + 1)
        else:
            number[0] = number[0] + "_1"
        return "_".join(["_".join(temporary[:-1]), '.'.join(number)])


class PlotWindow(object):
    def __init__(self, masterapp, microcontroleur: serialMCUComm.SerialMCUObject):
        self.window = tk.Toplevel(masterapp)
        self.window.title("Plot window")
        self.frame = tk.Frame(self.window)
        self.figureplot = Figure(figsize=(5, 4), dpi=100)
        self.ax = self.figureplot.add_subplot(111)
        self.line1, = self.ax.plot(microcontroleur.data_adc_y)
        self.canvas = FigureCanvasTkAgg(self.figureplot, master=self.window)
        self.canvas.draw()
        self.frame.pack()
        self.toolbar = NavigationToolbar2Tk(self.canvas, self.window, pack_toolbar=False)
        self.toolbar.update()

        self.toolbar.pack(side=tk.BOTTOM, fill=tk.X)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

    def update_data(self, microcontroleur):
        self.ax.clear()
        self.ax.plot(microcontroleur.data_adc_y)
        self.canvas.draw()
        self.canvas.flush_events()


class CircularBuffer(deque):
    def __init__(self, size=0):
        super(CircularBuffer, self).__init__(maxlen=size)

    def to_bytes(self):
        return bytes([int.from_bytes(c, 'big') for c in self])


def thread_wait_for_data(microcontroler: serialMCUComm.SerialMCUObject, plotwindow: PlotWindow,
                         cnt=0, keyword=b'SENDING:', timeout=2):
    buffer = CircularBuffer(8)  # Create a circular buffer to assess command
    time_start = time.monotonic()
    while buffer.to_bytes() != keyword and time.monotonic()-time_start < timeout:
        while not microcontroler.serial.inWaiting() and time.monotonic()-time_start < timeout:
            pass
        buffer.append(microcontroler.serial.read(1))
        if buffer.to_bytes() == keyword:
            microcontroler.state = 'RUNNING'
        if buffer.to_bytes() == b'STOPING!':
            microcontroler.state = 'CONNECTED'
    if microcontroler.state == 'RUNNING':
        microcontroler.data_raw_uart = \
            microcontroler.read_cmd_until(b'FINISHED', max_size=microcontroler.size_adc + 320,
                                          timeout=(microcontroler.exposure+1), optional_stop_condition=b'STOPING!')
        print("read :", len(microcontroler.data_raw_uart), "points")
        if not len(microcontroler.data_raw_uart) or (microcontroler.data_raw_uart[-8:] == b'STOPING!'):
            microcontroler.state = 'CONNECTED'
    if microcontroler.state == 'RUNNING':
        microcontroler.data_raw_uart = microcontroler.data_raw_uart[0:-8]
        microcontroler.convert_raw_uart_adc_data(len(microcontroler.data_raw_uart))
        plotwindow.update_data(microcontroler)
    # microcontroler.serial.flushInput()
