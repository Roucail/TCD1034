from tkinter import Tk     # from tkinter import Tk for Python 3.x
from tkinter.filedialog import askopenfilename


def pad_line(input_list,target_length):
    return input_list+list(['0'])*(target_length-len(input_list))


Tk().withdraw()  # we don't want a full GUI, so keep the root window from appearing
filename = askopenfilename()  # show an "Open" dialog box and return the path to the selected file
if filename:
    with open(filename) as f:
        result = f.read()
    print(filename)
    result = [[j for j in i.split('\t')] for i in result.split('\n')]
    max_length = max([len(i) for i in result])
    result = [pad_line(i, max_length) for i in result[:-1]]
    result = ['\t'.join(i) for i in result]
    result = '\n'.join(result)
    filename = filename[:-4]+'1.txt'
    print(filename)
    with open(filename,'w') as f:
        f.write(result)
