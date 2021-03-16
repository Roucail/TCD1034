import tkinter as tk
import treeviewmenuclass as tvmc
import load_dump_file_treeview as ldftvm
import tkinter.ttk as ttk
import copy
import json
import os

my_file = 'test.txt'
def quit_button():
    ldftvm.write_to_place(my_file, tv_menu)
    main_window.destroy()


main_window = tk.Tk()
tv_menu = tvmc.TreeviewMenu(main_window, columns=("values 0", "values 1"), width_value=100)
lines_parameters1 = [("", "param0", ["1", "5"], ["a b", "c", "g", "d"], True),
                     ("param0", "param1", "2f", bool(), True),
                     ("param1", "param2", "hello, world!", "Text", True),
                     ("", "param3", "4", bool(), True),
                     ("param1", "param5", "3", "Text", True), ]
tv_menu.lines_list = lines_parameters1
tv_menu.create_parameter_list()

ldftvm.update_from_file(my_file, tv_menu)
tv_menu.clear_parameter_list()
tv_menu.create_parameter_list()


bt_quit = tk.Button(main_window, text="quit", command=quit_button )
tv_menu.pack()
bt_quit.pack()

main_window.update()
main_window.focus_set()
tv_menu.focus_set()
main_window.mainloop()