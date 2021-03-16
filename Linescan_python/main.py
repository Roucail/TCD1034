import tkinter as tk
import treeviewmenuclass as tvmc
import serial.tools.list_ports
import serialMCUComm
import applicationspeciticwindows as asw

my_file = 'last_configuration.txt'


def quit_button():
    print(tv_menu.lines_list)
    tv_menu.write_to_place(my_file)
    main_window.destroy()


def transpose_list_menu_to_microcotroleur(treeviewmenu: tvmc.TreeviewMenu,
                                          mcu_conf: serialMCUComm.SerialMCUObject):
    for __, parameter, component, *__ in treeviewmenu.lines_list:
        if isinstance(component, type(list())):
            component = component[0]
        if parameter == 'Com. port':
            mcu_conf.serial_port = component
        elif parameter == 'Baud rate':
            mcu_conf.baud_rate = int(component)
        elif parameter == 'Exposure time':
            mcu_conf.exposure = float(component)
        elif parameter == 'Prescaler register':
            mcu_conf.prescaler = int(component)
        elif parameter == 'Autoreload register':
            mcu_conf.autoreload = int(component)
        elif parameter == 'Size word':
            mcu_conf.size_word = int(component)
        elif parameter == 'Size ADC':
            mcu_conf.size_adc = int(component)
        elif parameter == 'Binning':
            mcu_conf.binning = int(component)


def transpose_list_menu_from_microcotroleur(treeviewmenu: tvmc.TreeviewMenu,
                                            mcu_conf: serialMCUComm.SerialMCUObject):
    treeviewmenu.item('Com. port', values=[mcu_conf.serial_port])
    treeviewmenu.update_lines_list('Com. port', str(mcu_conf.serial_port))
    treeviewmenu.item('Baud rate', values=[mcu_conf.baud_rate])
    treeviewmenu.update_lines_list('Baud rate', str(mcu_conf.baud_rate))
    treeviewmenu.item('Exposure time', values=[mcu_conf.exposure])
    treeviewmenu.update_lines_list('Exposure time', str(mcu_conf.exposure))
    treeviewmenu.item('Prescaler register', values=[mcu_conf.prescaler])
    treeviewmenu.update_lines_list('Prescaler register', str(mcu_conf.prescaler))
    treeviewmenu.item('Autoreload register', values=[mcu_conf.autoreload])
    treeviewmenu.update_lines_list('Autoreload register', str(mcu_conf.autoreload))
    treeviewmenu.item('Size word', values=[mcu_conf.size_word])
    treeviewmenu.update_lines_list('Size word', str(mcu_conf.size_word))
    treeviewmenu.item('Size ADC', values=[mcu_conf.size_adc])
    treeviewmenu.update_lines_list('Size ADC', str(mcu_conf.size_adc))
    treeviewmenu.item('Binning', values=[mcu_conf.binning])
    treeviewmenu.update_lines_list('Binning', str(mcu_conf.binning))


def get_comport():
    return tuple([comport.device for comport in serial.tools.list_ports.comports()])


def exposure_time_update(mcu_conf: serialMCUComm.SerialMCUObject, exposure_time):
    mcu_conf.exposure = exposure_time
    mcu_conf.exposure_time_to_prescaler_autoreload()


def prescaler_update(mcu_conf: serialMCUComm.SerialMCUObject, prescaler):
    mcu_conf.prescaler = prescaler
    mcu_conf.exposure_time_from_prescaler_autoreload()


def callbackfunction_autoreload_prescaler_from_exposure(treeviewmenu_in, mcu_conf_in):

    def functiontocallback_autoreload_prescaler_from_exposure(treeviewmenu=treeviewmenu_in, mcu_conf=mcu_conf_in):
        transpose_list_menu_to_microcotroleur(treeviewmenu, mcu_conf)
        mcu_conf.exposure_time_to_prescaler_autoreload()
        transpose_list_menu_from_microcotroleur(treeviewmenu, mcu_conf)
        treeviewmenu.callback_at_validation = lambda: None
    return functiontocallback_autoreload_prescaler_from_exposure


def callbackfunction_autoreload_prescaler_to_exposure(treeviewmenu_in, mcu_conf_in):

    def functiontocallback_autoreload_prescaler_to_exposure(treeviewmenu=treeviewmenu_in, mcu_conf=mcu_conf_in):
        transpose_list_menu_to_microcotroleur(treeviewmenu, mcu_conf)
        mcu_conf.exposure_time_from_prescaler_autoreload()
        transpose_list_menu_from_microcotroleur(treeviewmenu, mcu_conf)
        treeviewmenu.callback_at_validation = lambda: None
    return functiontocallback_autoreload_prescaler_to_exposure


def callbackfunction_binning(treeviewmenu_in, mcu_conf_in):

    def functiontocallback_binning(treeviewmenu=treeviewmenu_in, mcu_conf=mcu_conf_in):
        print("set binning")
        transpose_list_menu_to_microcotroleur(treeviewmenu, mcu_conf)
        print(treeviewmenu.lines_list)
        transpose_list_menu_from_microcotroleur(treeviewmenu, mcu_conf)
        treeviewmenu.callback_at_validation = lambda: None
    return functiontocallback_binning


main_window = tk.Tk()
main_window.title("Menu")
bt_quit = tk.Button(main_window, text="Quitter", command=quit_button)
bt_quit.bind("<KeyPress-Return>", lambda arg: quit_button())
tv_menu = tvmc.TreeviewMenu(main_window, height=12, width_parameter=230, width_value=100)
microcontroleur = serialMCUComm.SerialMCUObject()


def auto_adjust_Autoreload_prescaler_from_exposure(arg1=tv_menu, arg2=microcontroleur):
    return callbackfunction_autoreload_prescaler_from_exposure(arg1, arg2)


def auto_adjust_binning(arg1=tv_menu, arg2=microcontroleur):
    return callbackfunction_binning(arg1, arg2)


def auto_adjust_Autoreload_prescaler_to_exposure(arg1=tv_menu, arg2=microcontroleur):
    return callbackfunction_autoreload_prescaler_to_exposure(arg1, arg2)


list_baudrate = tuple([110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200,
                       38400, 57600, 115200, 230400, 460800, 921600])
list_menu = [('', 'Serial Configuration', [''], None, False),
             ('Serial Configuration', 'Com. port', ['COM5'], get_comport, True),
             ('Serial Configuration', 'Baud rate', ['115200'], list_baudrate, True),
             ('Serial Configuration', 'Microcontroler configuration', [''], None, True),
             ('Microcontroler configuration', 'Size word', ['8'], 'Text', True),
             ('Microcontroler configuration', 'Size ADC', ['7388'], 'Text', True),
             ('', 'Sensor configuration', [''], None, True),
             ('Sensor configuration', 'Exposure time', ['22e-5'], auto_adjust_Autoreload_prescaler_from_exposure, True),
             ('Sensor configuration', 'Prescaler register', ['0'], auto_adjust_Autoreload_prescaler_to_exposure, True),
             ('Sensor configuration', 'Autoreload register', ['10'], auto_adjust_Autoreload_prescaler_to_exposure, True),
             ('Sensor configuration', 'Binning', ['1'], auto_adjust_binning, True),
             ('', 'Data file', [''], None, True),
             ('Data file', 'File path', ['data'], 'Text', True),
             ('Data file', 'Save', ['False'], bool(), True),
             ('Data file', 'Append(True)/Inc. File(False)', ['False'], bool(), True),
             ('Data file', 'Next Data', ['Data_0000'], 'Text', True)]


tv_menu.lines_list = list_menu

tv_menu.create_parameter_list()
tv_menu.update_from_file(my_file)
tv_menu.clear_parameter_list()
tv_menu.create_parameter_list()


subwindow_button = asw.ValidationWindow(main_window, microcontroleur, tv_menu)

tv_menu.pack(fill='x')
subwindow_button.subwindow.pack(expand='yes')
bt_quit.pack()
subwindow_button.plotwindow = asw.PlotWindow(subwindow_button.subwindow, microcontroleur)

main_window.geometry("333x355+200+200")
main_window.update()

pos_main = main_window.geometry()
pos_main = pos_main.split('+')
pos_main = pos_main[0].split('x') + pos_main[1:]

subwindow_button.plotwindow.window.geometry("+"+str(int(pos_main[0])+int(pos_main[2]))+"+"+pos_main[3])
# print(main_window.geometry())
# main_window.focus_set()
tv_menu.focus_set()
main_window.mainloop()
