import serial
import math
import time


class SerialMCUObject(object):

    master_clock_period = 1/500e3  # second
    command_list = [b'PSC', b'PSF', b'ARR', b'ARF', b'SGL', b'RUN', b'UPE', b'STP', b'BIN']

    def __init__(self, exposure_time=20e-6, serial_port='COM5', baud_rate=115200, size_word=8, size_adc=2*3694):
        self.state = "DISCONNECTED"
        self.data_last_cmd = b''
        self.data_raw_uart = bytes(size_adc)
        self.data_adc_y = 0 * list()
        self.data_adc_x = list()
        self.serial = serial.Serial()
        self.serial_port = serial_port
        self.baud_rate = baud_rate
        self.size_word = size_word
        self.size_adc = size_adc
        self.exposure = exposure_time
        self.binning = 1
        self.prescaler = 0
        self.autoreload = 0
        self.exposure_time_to_prescaler_autoreload()

    def exposure_time_from_prescaler_autoreload(self, prescaler=-1, autoreload=-1):
        if prescaler < 0:
            prescaler = self.prescaler
        else:
            self.prescaler = prescaler
        if autoreload < 0:
            autoreload = self.autoreload
        else:
            self.autoreload = autoreload
        self.exposure = self.master_clock_period*(prescaler+1)*(autoreload+1)

    def exposure_time_to_prescaler_autoreload(self, exposuretime=-1):
        if exposuretime < 0:
            exposuretime = self.exposure
        number_of_step = math.ceil(exposuretime/self.master_clock_period)
        if number_of_step > 2**32:
            print("Too long exposure, it should not be larger than 8589 second, exposure set to 8589s ")
            self.prescaler = 2**16-1
            self.autoreload = 2**16-1
            self.exposure = ((2**16)**2)/500e3
        else:
            self.prescaler = math.floor(number_of_step/(2**16))
            self.autoreload = math.ceil(number_of_step/(self.prescaler+1))-1
            self.exposure = (self.prescaler+1)*(self.autoreload+1) *\
                self.master_clock_period

    def convert_raw_uart_adc_data(self, size_adc=2*3694):
        self.data_adc_y = [self.data_raw_uart[i-1] * 2 ** 8 + self.data_raw_uart[i] for i in range(1, size_adc, 2)]
        return self.data_adc_y

    def connection(self):
        try:
            self.serial = serial.Serial(self.serial_port, self.baud_rate, parity=serial.PARITY_NONE,
                                        stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS, timeout=1)
            self.state = "CONNECTED"
        except ValueError:
            print("cannot establish connection")

    def close(self):
        if self.state == "DISCONNECTED":
            print("Not connected")
        else:
            self.serial.close()
            self.state = "DISCONNECTED"

    def read_cmd(self, size_word=-1, timeout=0):
        if self.state == "DISCONNECTED":
            print("No connection available")
            return None
        else:
            temp = b''
            start_time = time.time()
            if size_word < 0:
                size_word = self.size_word
            while (self.serial.inWaiting() > 0 or timeout > (time.time() - start_time)) and len(temp) < size_word:
                temp += self.serial.read(1)
            return temp

    def read_cmd_until(self, stop_condition, max_size=math.inf, timeout=1, optional_stop_condition=None):
        if self.state == "DISCONNECTED":
            print("No connection available")
            return None
        else:
            temp = b''
            start_time = time.time()
            while (self.serial.inWaiting() > 0 or timeout > (time.time() - start_time))\
                    and len(temp) < max_size and not\
                    ((temp[-len(stop_condition):] == stop_condition) or
                     (temp[-len(stop_condition):] == optional_stop_condition)):
                temp += self.serial.read(1)
            if timeout < (time.time() - start_time) or not\
                    ((temp[-len(stop_condition):] == stop_condition) or
                     (temp[-len(stop_condition):] == optional_stop_condition)):
                print("stop condition was not reached")
            return temp

    def send_cmd(self, command, parameter=b'0000'):
        parameter = check_parameter_uint16(parameter)
        if self.state == "DISCONNECTED":
            print("No connection available")
        elif parameter is None:
            print('Invalid parameter')
        elif command not in self.command_list:
            print(command, ' is not a valid command')
        else:
            command += b' '+parameter
            self.serial.write(command)


def check_parameter_uint16(parameter):
    if type(parameter) == bytes:
        parameter = parameter.upper()
        if len(parameter) != 4:
            print("Parameter is not the right length")
            return None
        elif [True for i in parameter if i not in b'0123456789ABCDEF']:
            print("Invalid parameter value, bytes must be in 0-9A-Za-z")
            return None
        else:
            return parameter
    if type(parameter) == int:
        temp = parameter
        parameter = b''
        if temp < 0 or temp > 2**16-1:
            print("Parameter must be in [0 : 65536[")
            return None
        for i in range(3, -1, -1):
            tempchar = math.floor(temp/(16**i))
            if tempchar < 10:
                tempchar = (ord('0')+tempchar).to_bytes(1, 'big')
            else:
                tempchar = (ord('A')+tempchar-10).to_bytes(1, 'big')
            parameter += tempchar
            temp -= math.floor(temp/(16**i))*16**i
        return parameter


if __name__ == "__main__":

    import matplotlib.pyplot as plt

    def main():
            ii = 1
            a = SerialMCUObject(exposure_time=20e-6, serial_port="COM5", baud_rate=115200, size_word=8, size_adc=2*3694)
            a.connection()
            if a.serial.inWaiting() > 0:
                print("the buffer is not empty")
                a.serial.reset_input_buffer()
                a.serial.reset_output_buffer()
                if a.serial.inWaiting() > 0:
                    print("the buffer is still not empty")
                else:
                    print("the buffer is now empty")
            a.exposure_time_to_prescaler_autoreload()

            print("sending single pulse command")
            a.send_cmd(b'SGL')
            a.data_last_cmd = a.read_cmd(a.size_word, timeout=1)
            print(a.data_last_cmd)
            a.data_raw_uart = a.read_cmd_until(b'FINISHED', max_size= a.size_adc+320, timeout=2)
            print(a.data_raw_uart[0:16])
            print(a.data_raw_uart[-16:])
            print(len(a.data_raw_uart))
            a.data_raw_uart = a.data_raw_uart[8:-8]
            a.convert_raw_uart_adc_data(len(a.data_raw_uart))
            print(a.serial.inWaiting())
            plt.plot(a.data_adc_y)
            plt.show()


    # execute only if run as a script
    main()
