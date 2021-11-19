# https://wiki.python.org/moin/TkInter
# https://towardsdatascience.com/top-10-python-gui-frameworks-for-developers-adca32fbe6fc
# https://github.com/rdbende/Sun-Valley-ttk-theme/blob/master/Dark%20screenshot.png


import tkinter as tk
from tkinter import Button, Label, ttk as ttk
from tkinter import scrolledtext as tkscroll
from tkinter import messagebox as msgbox
from tkinter import *
from tkinter import ttk
from ttkwidgets import *
import serial
import serial.tools.list_ports as list_ports
import time
import json
#import pandas as pd
import csv
import os

def get_str_of_chr(chr_in_byte):  # converts revived data into the approprate charcter
    cd = ord(chr_in_byte)  # ord converts a chacter into its hex value
    # if the chacters is less than or equal to 0x20 raw encoding (space) and (less than  = the "~" 0x7e OR less than inverted bang 0xA1)
    if 0x20 <= cd and cd <= 0x7e or 0xa1 <= cd:
        if cd == 92:  # if the character is a \
            return '\\\\'
        return chr(cd)  # Else retun the chacter of the ord number
    else:
        if cd == 9:     # if its a Tab
            return '\t'
        elif cd == 10:  # if its a new line
            return '\n'
        elif cd == 13:  # if its a Carriage return
            return "\\r"
    # \\ prints a backslash, x prints an x and {:02x prints the hex of the ASCII 2 charcter  }
    return '\\x{:02x}'.format(cd)


def get_hexstr_of_chr(chr_in_byte):
    cd = ord(chr_in_byte)      # find the hex of the chacter passed in
    st = '{:02X}'.format(cd)   # Format the chacter passed in see above
    if cd == 10:             # if we have a new line charcter
        st += '\n'          # add the new line character ot the end of the string
    else:
        st += ' '  # otherwise put a space
    return st


def is_hex_digit(a_byte):
    return b'0' <= a_byte and a_byte <= b'9' or b'A' <= a_byte and a_byte <= b'F' or b'a' <= a_byte and a_byte <= b'f'


def is_oct_digit(a_byte):
    return b'0' <= a_byte and a_byte <= b'7'


def decode_esc(str_of_chr):
    # for the number of characters in the string loop throught and convert them to a byte
    sbs = bytes([ord(c) for c in str_of_chr])
    dbs = b''  # create and enpty byte
    err = None  # error state
    idx = 0  # index
    while idx < len(sbs):  # while less than the length of the byte array
        # here we make an array of the current byte and the next one
        by = sbs[idx:idx+1]
        if by == b'\\':     # if the byte is the \\ meaning the ending bytes has been reached "\\r\n" or similar
            idx += 1        # We increment forward by one byte to look at the ending bytes
            by = sbs[idx:idx+1]  # take in the new ending bytes
            if by == b'\\' or by == b"'" or by == b'"':  # if the bytes in the array are " or ' or \\
                dbs += by   # add them to the byte string
            elif by == b'0' and not is_oct_digit(sbs[idx+1:idx+2]):
                dbs += b'\0'
            elif by == b'a':
                dbs += b'\a'
            elif by == b'b':
                dbs += b'\b'
            elif by == b't':
                dbs += b'\t'
            elif by == b'n':
                dbs += b'\n'
            elif by == b'v':
                dbs += b'\v'
            elif by == b'f':
                dbs += b'\f'
            elif by == b'r':
                dbs += b'\r'
            elif by == b'x':
                # if its not one of special charcters above
                if is_hex_digit(sbs[idx+1:idx+2]):
                    if is_hex_digit(sbs[idx+2:idx+3]):
                        dbs += bytes([int(sbs[idx+1:idx+3], 16)])
                        idx += 3
                        continue
                err = {'from': idx-1, 'to': idx+3,
                       'msg': f'Value Error: invalid {str_of_chr[idx-1:idx+3]} escape at position {idx-1}'}
                break
            elif is_oct_digit(by):
                od = 1
                if is_oct_digit(sbs[idx+1:idx+2]):
                    od += 1
                    if is_oct_digit(sbs[idx+2:idx+3]):
                        od += 1
                ov = int(sbs[idx:idx+od], 8)
                if ov > 255:
                    od -= 1
                    ov >>= 3
                dbs += bytes([ov])
                idx += od
                continue
            else:
                if by:
                    ch = chr(ord(by))
                    to = idx + 1
                else:
                    ch = ''
                    to = idx
                err = {'from': idx-1, 'to': to,
                       'msg': f"Syntax Error: invalid escape sequence '\\{ch}' at position {idx-1}"}
                break
        else:
            dbs += by
        idx += 1
    return dbs, err


def sendCmd(event):
    global sentTexts, sentTextsPtr
    txt = str(txText.get())
    lst = len(sentTexts)
    if txt == '{about}':
        showAbout()
    if txt != '':
        bs, err = decode_esc(txt)
        if err:
            writeConsole(err['msg'] + '\n', 2)
            txText.xview(err['from'])
            txText.selection_range(err['from'], err['to'])
            txText.icursor(err['to'])
            return
        if lst > 0 and sentTexts[lst-1] != txt or lst == 0:
            sentTexts.append(txt)
        sentTextsPtr = len(sentTexts)
        if lineEndingCbo.current() == 1:  # line ending data
            bs += b'\n'
        elif lineEndingCbo.current() == 2:
            bs += b'\r'
        elif lineEndingCbo.current() == 3:
            bs += b'\r\n'
        currentPort.write(bs)
        if showSentTextVar.get():
            if dispHexVar.get():
                txt = ''.join([get_hexstr_of_chr(bytes([i])) for i in bs])
            else:
                txt = ''.join([get_str_of_chr(bytes([i])) for i in bs])
            writeConsole(txt, 1)
        txText.delete(0, tk.END)


def upKeyCmd(event):
    global sentTextsPtr, lastTxText
    if sentTextsPtr == len(sentTexts):
        lastTxText = str(txText.get())
    if sentTextsPtr > 0:
        sentTextsPtr -= 1
        txText.delete(0, tk.END)
        txText.insert(tk.END, sentTexts[sentTextsPtr])


def downKeyCmd(event):
    global sentTextsPtr
    if sentTextsPtr < len(sentTexts):
        sentTextsPtr += 1
        txText.delete(0, tk.END)
        if sentTextsPtr == len(sentTexts):
            txText.insert(tk.END, lastTxText)
        else:
            txText.insert(tk.END, sentTexts[sentTextsPtr])


def changePort(event):
    global portDesc
    if portCbo.get() == currentPort.port:
        return
    disableSending()
    if currentPort.is_open:
        currentPort.close()
        writeConsole(portDesc + ' closed.\n', 2)
    currentPort.port = portCbo.get()
    portDesc = ports[currentPort.port]
    writeConsole('Opening ' + portDesc + '...', 2)
    root.update()
    try:
        currentPort.open()
    except:
        root.title(APP_TITLE)
        portCbo.set('Select port')
        #msgbox.showerror(APP_TITLE, "Couldn't open the {} port.".format(portDesc))
        writeConsole('failed!!!\n', 2)
        currentPort.port = None
    if currentPort.is_open:
        root.title(APP_TITLE + ': ' + ports[currentPort.port])
        enableSending()
        rxPolling()
        writeConsole('done.\n', 2)
        #msgbox.showinfo(APP_TITLE, '{} opened.'.format(portDesc))


def changeBaudrate(event):
    currentPort.baudrate = BAUD_RATES[baudrateCbo.current()]


def clearOutputCmd():
    global isEndByNL, lastUpdatedBy
    rxText.configure(state=tk.NORMAL)
    rxText.delete('1.0', tk.END)
    rxText.configure(state=tk.DISABLED)
    isEndByNL = True
    lastUpdatedBy = 2


def showTxTextMenu(event):
    if txText.selection_present():
        sta = tk.NORMAL
    else:
        sta = tk.DISABLED
    for i in range(2):
        txTextMenu.entryconfigure(i, state=sta)
    try:
        root.clipboard_get()
        txTextMenu.menu.entryconfigure(2, state=tk.NORMAL)
    except:
        txTextMenu.menu.entryconfigure(2, state=tk.DISABLED)
    try:
        txTextMenu.menu.tk_popup(event.x_root, event.y_root)
    finally:
        txTextMenu.grab_release()


def showRxTextMenu(event):
    if len(rxText.tag_ranges(tk.SEL)):
        rxTextMenu.menu.entryconfigure(0, state=tk.NORMAL)
    else:
        rxTextMenu.menu.entryconfigure(0, state=tk.DISABLED)
    if currentPort.isOpen():
        rxTextMenu.menu.entryconfigure(2, state=tk.NORMAL)
    else:
        rxTextMenu.menu.entryconfigure(2, state=tk.DISABLED)
    try:
        rxTextMenu.menu.tk_popup(event.x_root, event.y_root)
    finally:
        rxTextMenu.grab_release()

def SetData():
    global Xpos,Ypos,AoAT,AoAB,Ystr,Xstr,Tstr,Bstr,AXstr,AYstr,ATstr,ABstr,SXstr,SYstr,STstr,SBstr,AY,AT,AB,SX,SY,ST,SB 
    #print("In Set Pos")
    #print(Xstr)
    #print(Ystr)
    #print(Tstr)
    #print(Bstr)
    try:
        Xpos=float(Xstr)
        #print(Xpos)
        Ypos=float(Ystr)
        #print(Ypos)
        AoAT=float(Tstr)
        #print(AoAT)
        AoAB=float(Bstr)
        #print(AoAB)
        AX=int(AXstr) 
        AY=int(AYstr)
        AT=int(ATstr)
        AB=int(ABstr)
        SX=int(SXstr)
        SY=int(SYstr)
        ST=int(STstr)
        SB=int(SBstr)
        Xstr=''
        Ystr=''
        Tstr=''
        Bstr=''
        AXstr=''
        AYstr=''
        ATstr=''
        ABstr=''
        SXstr=''
        SYstr=''
        STstr=''
        SBstr=''

    except:
        msgbox.showwarning("Error 1", "Warning:\n An Internal Value exception occored Borad and Computer are out of sync!\n Please Home All!")
    
    
def writeConsole(txt, upd=0):
    global isEndByNL, lastUpdatedBy,Ystr,Xstr,Tstr,Bstr,internalData,recv,newtxt,oneshot
    newtxt=''
    for x in txt:
        if x == '%':
            #print("Data")
            if internalData.get() == True:
                internalData.set(False)
            else:
                internalData.set(True)
                oneshot.set(True)# set to global
            #print(internalData.get())
            #print(oneshot.get())
        elif internalData.get() == True:
            #print(x)
            #print("else")
            if x == 'X':
                #print("set X")
                recv.set(1)
            elif x == 'Y':
                recv.set(2)
            elif x =='T':
                recv.set(3)
            elif x =='B':
                recv.set(4)
            elif x =='Q':
                recv.set(5)
            elif x =='W':
                recv.set(6)
            elif x =='E':
                recv.set(7)
            elif x =='R':
                recv.set(8)
            elif x =='A':
                recv.set(9)
            elif x =='S':
                recv.set(10)
            elif x =='D':
                recv.set(11)
            elif x =='F':
                recv.set(12)
            else: # it has to be one of the numbers follwing it 
                #print(recv.get())
                if recv.get()==1:
                    Xstr=Xstr+x
                    #print(Xstr)
                elif recv.get()==2:
                    Ystr=Ystr+x
                    #print(Ystr)
                elif recv.get()==3:
                    Tstr=Tstr+x
                elif recv.get()==4:
                    Bstr=Bstr+x
                elif recv.get()==5:
                    AXstr=AXstr+x
                elif recv.get()==6:
                    AYstr=AYstr+x
                elif recv.get()==7:
                    ATstr=ATstr+x
                elif recv.get()==8:
                    ABstr=ABstr+x
                elif recv.get()==9:
                    SXstr=SXstr+x
                elif recv.get()==10:
                    SYstr=SYstr+x
                elif recv.get()==11:
                    STstr=STstr+x
                elif recv.get()==12:
                    SBstr=SBstr+x
        else:
            #print(internalData.get())
            newtxt=newtxt+x
    if internalData.get() == False and oneshot.get() == True: # if we are out of the range of the % data and we have came accrosss a % previously update internal varible
        #print("Headed to Set Pos")
        SetData()
        oneshot.set(False) # dont fall into this crap unless you have to --> once per function call
    tm = ''
    ad = ''
    if upd != 2 and showTimestampVar.get():
        tm = time.strftime('%H:%M:%S.{}'.format(
            repr(time.time()).split('.')[1][:3]))
    if not upd:
        if not lastUpdatedBy and isEndByNL or lastUpdatedBy:
            if showSentTextVar.get() and showTimestampVar.get():
                ad += 'RX_' + tm
            elif showSentTextVar.get():
                ad += 'RX'
            elif showTimestampVar.get():
                ad += tm
            if ad:
                ad += ' >> '
            if not isEndByNL:
                ad = '\n' + ad
    elif upd == 1:
        if lastUpdatedBy == 1 and isEndByNL or lastUpdatedBy != 1:
            if showTimestampVar.get():
                ad = 'TX_' + tm
            else:
                ad = 'TX'
            ad += ' >> '
            if not isEndByNL:
                ad = '\n' + ad
    elif upd == 2:
        if lastUpdatedBy != 2:
            ad = '\n'
            if not isEndByNL:
                ad += '\n'
    else:
        return
    if upd != 2 and lastUpdatedBy == 2:
        ad = '\n' + ad
    ad += newtxt
    rxText.configure(state=tk.NORMAL)
    rxText.insert(tk.END, ad)
    if autoscrollVar.get() or upd == 2:
        rxText.see(tk.END)
    rxText.configure(state=tk.DISABLED)
    if txt[-1] == '\n':
        isEndByNL = True
    else:
        isEndByNL = False
    lastUpdatedBy = upd


def rxPolling():  # Pull Data in
    preset = time.perf_counter_ns()
    try:
        while currentPort.in_waiting > 0 and time.perf_counter_ns()-preset < 2000000:  # loop duration about 2ms
            #print(recv.get())
            ch = currentPort.read()  # read the current data on the port
            tm = time.strftime('%H:%M:%S.{}'.format(
                repr(time.time()).split('.')[1][:3]))
            txt = ''
            if dispHexVar.get():  # If display in Hex is true turn the chacter to it hex value
                txt += get_hexstr_of_chr(ch)
            else:  # else of the chacter is desired in a string convert the
                txt += get_str_of_chr(ch)
            #print(txt)
            #print(" word lenght" + str(len(txt)))
            writeConsole(txt)
    except serial.SerialException as se:
        closePort()
        msgbox.showerror(
            APP_TITLE, "Couldn't access the {} port".format(portDesc))
    root.after(10, rxPolling)  # polling in 10ms interval


def listPortsPolling():  # list the current Com Ports avalible
    global ports
    ps = {p.name: p.description for p in list_ports.comports()}
    pn = sorted(ps)
    if pn != portCbo['values']:
        portCbo['values'] = pn
        if len(ports) == 0:  # if no port before
            portCbo['state'] = 'readonly'
            portCbo.set('Select port')
            enableSending()
        elif len(pn) == 0:  # now if no port
            portCbo['state'] = tk.DISABLED
            portCbo.set('No port')
            disableSending()
        ports = ps
    root.after(1000, listPortsPolling)  # polling every 1 second


def disableSending():
    sendBtn['state'] = tk.DISABLED
    txText.unbind('<Return>')


def enableSending():
    sendBtn['state'] = tk.NORMAL
    txText.bind('<Return>', sendCmd)


def closePort():
    if currentPort.is_open:
        currentPort.close()
        writeConsole(portDesc + ' closed.\n', 2)
        currentPort.port = None
        disableSending()
        portCbo.set('Select port')
        root.title(APP_TITLE)


def showAbout():
    msgbox.showinfo(APP_TITLE, 'Designed by Collin Charvat\n Monitor credit: ZulNs @Gorontalo,\n Fall 21`~Spring 22`')


def exitRoot():
    data = {}
    data['autoscroll'] = autoscrollVar.get()
    data['showtimestamp'] = showTimestampVar.get()
    data['showsenttext'] = showSentTextVar.get()
    data['displayhex'] = dispHexVar.get()
    data['lineending'] = lineEndingCbo.current()
    data['baudrateindex'] = baudrateCbo.current()
    data['databits'] = currentPort.bytesize
    data['parity'] = currentPort.parity
    data['stopbits'] = currentPort.stopbits
    data['portindex'] = portCbo.current()
    data['portlist'] = ports
    with open(fname+'.json', 'w') as jfile:
        json.dump(data, jfile, indent=4)
        jfile.close()
    root.destroy()


def setting():  # settings Menu
    global settingDlg, dataBitsCbo, parityCbo, stopBitsCbo
    settingDlg = tk.Toplevel()
    settingDlg.title('Port Setting')
    if ico:
        settingDlg.iconphoto(False, ico)
    tk.Grid.rowconfigure(settingDlg, 0, weight=1)
    tk.Grid.rowconfigure(settingDlg, 1, weight=1)
    tk.Grid.rowconfigure(settingDlg, 2, weight=1)
    tk.Grid.rowconfigure(settingDlg, 3, weight=1)
    tk.Grid.columnconfigure(settingDlg, 0, weight=1)
    tk.Grid.columnconfigure(settingDlg, 1, weight=1)
    tk.Grid.columnconfigure(settingDlg, 2, weight=1)
    tk.Label(settingDlg, text='Data bits:').grid(
        row=0, column=1, padx=0, pady=12, sticky=tk.NE)
    ttk.Label(settingDlg, text='Parity:').grid(
        row=1, column=1, padx=0, pady=0, sticky=tk.NS+tk.E)
    ttk.Label(settingDlg, text='Stop bits:').grid(
        row=2, column=1, padx=0, pady=12, sticky=tk.NE)
    dataBitsCbo = ttk.Combobox(settingDlg, width=10, state='readonly')
    dataBitsCbo.grid(row=0, column=2, padx=12, pady=12, sticky=tk.NE)
    dataBitsCbo['values'] = DATABITS
    dataBitsCbo.set(currentPort.bytesize)
    parityCbo = ttk.Combobox(settingDlg, width=10, state='readonly')
    parityCbo.grid(row=1, column=2, padx=12, pady=0, sticky=tk.NS+tk.E)
    parityCbo['values'] = PARITY_VAL
    parityCbo.current(PARITY.index(currentPort.parity))
    stopBitsCbo = ttk.Combobox(settingDlg, width=10, state='readonly')
    stopBitsCbo.grid(row=2, column=2, padx=12, pady=12, sticky=tk.NE)
    stopBitsCbo['values'] = STOPBITS
    stopBitsCbo.set(currentPort.stopbits)
    ttk.Button(settingDlg, text='Default', width=10, command=defaultSetting).\
        grid(row=1, column=0, padx=12, pady=0, sticky=tk.NS+tk.W)
    ttk.Button(settingDlg, text='OK', width=10, command=lambda: setPort(None)).\
        grid(row=3, column=1, padx=0, pady=12, sticky=tk.S)
    cancelBtn = ttk.Button(settingDlg, text='Cancel',
                          width=10, command=lambda: hideSetting(None))
    cancelBtn.grid(row=3, column=2, padx=12, pady=12, sticky=tk.S)
    settingDlg.bind('<Return>', setPort)
    settingDlg.bind('<Escape>', hideSetting)
    settingDlg.update()
    rw = root.winfo_width()
    rh = root.winfo_height()
    rx = root.winfo_rootx()
    ry = root.winfo_rooty()
    dw = settingDlg.winfo_width()
    dh = settingDlg.winfo_height()
    settingDlg.geometry(f'{dw}x{dh}+{rx+int((rw-dw)/2)}+{ry+int((rh-dh)/2)}')
    settingDlg.minsize(dw, dh)
    settingDlg.maxsize(dw, dh)
    settingDlg.resizable(0, 0)
    settingDlg.grab_set()
    cancelBtn.focus_set()
 
def filePathSettings():  # settings Menu
    global filePathDlg, filePath, pText
    #filePath = tk.StringVar()
    #filePath="C:\\Users\\Collin\\Documents\\WSU_AIRFOIL_GITHUB\\WSU_AIRFOIL_PROJECT"
    filePathDlg = tk.Toplevel()
    filePathDlg.title('File Path Menu')
    if ico:
        filePathDlg.iconphoto(False, ico)
    tk.Grid.rowconfigure(filePathDlg, 0, weight=1)
    tk.Grid.rowconfigure(filePathDlg, 1, weight=1)
    tk.Grid.rowconfigure(filePathDlg, 2, weight=1)
    tk.Grid.rowconfigure(filePathDlg, 3, weight=1)
    tk.Grid.columnconfigure(filePathDlg, 0, weight=1)
    tk.Grid.columnconfigure(filePathDlg, 1, weight=1)
    tk.Grid.columnconfigure(filePathDlg, 2, weight=1)
    tk.Label(filePathDlg, text='File Path: Ex:C:\\Users\\Collin\\Documents\\WSU_AIRFOIL_GITHUB\\WSU_AIRFOIL_PROJECT').grid(row=0, column=1, padx=0, pady=12, sticky=tk.NE)
    # Insert the Faile path menu here
    pText = ttk.Entry(filePathDlg)
    pText.grid(row=2, column=1, columnspan=3,
                padx=4, pady=8, sticky=tk.N+tk.EW)
    pText.bind('<Up>', upKeyCmd)
    pText.bind('<Down>', downKeyCmd)
    ttk.Button(filePathDlg, text='Default', width=10, command=defaultFilePath).grid(row=1, column=0, padx=12, pady=0, sticky=tk.NS+tk.W)
    ttk.Button(filePathDlg, text='OK', width=10, command=lambda:setFilePath(None)).grid(row=3, column=1, padx=0, pady=12, sticky=tk.S)
    cancelBtn = ttk.Button(filePathDlg, text='Cancel',width=10, command=lambda:hideFilePath(None))
    cancelBtn.grid(row=3, column=2, padx=12, pady=12, sticky=tk.S)
    filePathDlg.bind('<Return>',setFilePath )
    filePathDlg.bind('<Escape>',hideFilePath)
    filePathDlg.update()
    rw = root.winfo_width()
    rh = root.winfo_height()
    rx = root.winfo_rootx()
    ry = root.winfo_rooty()
    dw = filePathDlg.winfo_width()
    dh = filePathDlg.winfo_height()
    filePathDlg.geometry(f'{dw}x{dh}+{rx+int((rw-dw)/2)}+{ry+int((rh-dh)/2)}')
    filePathDlg.minsize(dw, dh)
    filePathDlg.maxsize(dw, dh)
    filePathDlg.resizable(0, 0)
    filePathDlg.grab_set()
    cancelBtn.focus_set()

def change_theme():
    # NOTE: The theme's real name is sun-valley-<mode>
    if root.tk.call("ttk::style", "theme", "use") == "sun-valley-dark":
        # Set light theme
        root.tk.call("set_theme", "light")
        root.configure(bg='white')
    else:
        # Set dark theme
        root.tk.call("set_theme", "dark")
        root.configure(bg='#222222')

def defaultSetting():
    dataBitsCbo.set(serial.EIGHTBITS)
    parityCbo.current(PARITY.index(serial.PARITY_NONE))
    stopBitsCbo.set(serial.STOPBITS_ONE)
 
def defaultFilePath():
    filePath.set(r'C:\\Users\\Collin\\Documents\\WSU_AIRFOIL_GITHUB\\WSU_AIRFOIL_PROJECT')


def setPort(event):
    currentPort.bytesize = DATABITS[dataBitsCbo.current()]
    currentPort.parity = PARITY[parityCbo.current()]
    currentPort.stopbits = STOPBITS[stopBitsCbo.current()]
    settingDlg.destroy()

def setFilePath(event):
    txt = str(pText.get())
    filePath.set(txt)
    filePathDlg.destroy()

def hideSetting(event):  # hide the settings menu
    settingDlg.destroy()

def hideFilePath(event):  # hide the settings menu
    filePathDlg.destroy()

def GcodeSend(event):
    global tempString
    txt = str(tempString)
    lst = len(sentTexts)
    if txt == '{about}':
        showAbout()
    if txt != '':
        bs, err = decode_esc(txt)
        if err:
            writeConsole(err['msg'] + '\n', 2)
            txText.xview(err['from'])
            txText.selection_range(err['from'], err['to'])
            txText.icursor(err['to'])
            return
        if lst > 0 and sentTexts[lst-1] != txt or lst == 0:
            sentTexts.append(txt)
        sentTextsPtr = len(sentTexts)
        if lineEndingCbo.current() == 1:  # line ending data
            bs += b'\n'
        elif lineEndingCbo.current() == 2:
            bs += b'\r'
        elif lineEndingCbo.current() == 3:
            bs += b'\r\n'
        currentPort.write(bs)
        if showSentTextVar.get():
            if dispHexVar.get():
                txt = ''.join([get_hexstr_of_chr(bytes([i])) for i in bs])
            else:
                txt = ''.join([get_str_of_chr(bytes([i])) for i in bs])
            writeConsole(txt, 1)
        txText.delete(0, tk.END)
    #del tempString[:] # cleat Temp String Back out
    #print(tempString)

def scenarioMove(event):
    # get the value of the Senario Radio button
    global tempString
    index=scenariobBtnSelect.get()
    tempString=scenarioCommands[index]
    GcodeSend(None)
 
def Stop(event):
    global tempString
    tempString='<R>' # pass in the Reset String
    GcodeSend(None)
    
def homeAll(event):
    global tempString
    tempString='<G H>' # pass in the Home String
    GcodeSend(None)
    
def homeAxis(event):
    global tempString
    axisCommands=['<G H X>','<G H Y>','<G H AoAT>','<G H AoAB>','<G H X Y>','<G H AoAT AoAB>']
    index=axisBtnSelect.get()
    tempString=axisCommands[index]
    GcodeSend(None)
 
def moveButton(event):
    global tempString, Xpos, Ypos, AoAT,AoAB
    selectedAmmount=None
    selectedAmmountLast=None
    selectedAmmountmiddle=None
    index=directionBtnSelect.get()
    dir=''
    tempString=''
    if index == 0:
        dir=1
    else:
        dir=-1
    POS=[Xpos,Ypos,AoAT,AoAB]
    increment=[.05,.1,1,10,100]
    Xinc=[]
    Yinc=[]
    Tinc=[]
    Binc=[]
    print(Xpos)
    print(Ypos)
    if Xpos != 0 or Ypos !=0 or AoAT !=0 or AoAB !=0:
        for x in range(5):
            #print("case 1")
            Xinc.append(str(Xpos+dir*increment[x]))
            Yinc.append(str(Ypos+dir*increment[x]))
            Tinc.append(str(AoAT+dir*increment[x]))
            Binc.append(str(AoAB+dir*increment[x]))
    else:
        #print("case 2")
        for x in range(5):
            Xinc.append(str(dir*increment[x]))
            #print(Xinc[x])
            Yinc.append(str(dir*increment[x]))
            Tinc.append(str(dir*increment[x]))
            Binc.append(str(dir*increment[x]))
    # Take in the foat we are currently at for each axis
    # add the incremnts to the axis 
    # conver it to a string with the coreect endings 
    # pass it to the G code send
    #increment=['.05>','.1>','1>','10>','100>']
    #middleIncrement=['.05','.1','1','10','100']
    Axis=['<G X','<G Y','<G AoAT','<G AoAB','<G X',' Y','<G AoAT',' AoAB']
    ammtIndex= incremnetBtnSelect.get()
    axisIndex=axisBtnSelect.get()
    if axisIndex == 0: # xaxis
        selectedAmmount=Xinc[ammtIndex]
        if float(selectedAmmount)<0 and Xpos+float(selectedAmmount)<0:
            selectedAmmount=str(0) # Stop the X and Y axis from hitting the end
            msgbox.showwarning("X Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Min\n Pleaase dismiss and continue")
        if float(selectedAmmount)>0 and Xpos+float(selectedAmmount)>500:
            selectedAmmount=str(500)
            msgbox.showwarning("X Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled-> Set to Max\n Pleaase dismiss and continue")
        selectedAxis=Axis[axisIndex]
        tempString=selectedAxis+selectedAmmount+'>'
        #print(tempString)
    if axisIndex == 1: #yaxis
        selectedAmmount=Yinc[ammtIndex]
        print(Ypos+float(selectedAmmount))
        if float(selectedAmmount)<0 and Ypos+float(selectedAmmount)<0:
            selectedAmmount=str(0) # Stop the X and Y axis from hitting the end
            msgbox.showwarning("Y Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Min\n Pleaase dismiss and continue")
        if float(selectedAmmount)>0 and Ypos+float(selectedAmmount)>500:
            selectedAmmount=str(500)
            msgbox.showwarning("Y Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled-> Set to Max\n Pleaase dismiss and continue")
        selectedAxis=Axis[axisIndex]
        tempString=selectedAxis+selectedAmmount+'>'
        #print(tempString)
    if axisIndex == 2: #AoAaxis
        selectedAmmount=Tinc[ammtIndex]
        if float(selectedAmmount)<0 and AoAT+float(selectedAmmount)<-20:
            selectedAmmount=str(-20) # Stop the X and Y axis from hitting the end
            msgbox.showwarning("AoAT Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Min\n Pleaase dismiss and continue")
        if float(selectedAmmount)>0 and AoAT+float(selectedAmmount)>20:
            selectedAmmount=str(20)
            msgbox.showwarning("AoAT Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled-> Set to Max\n Pleaase dismiss and continue")
        selectedAxis=Axis[axisIndex]
        tempString=selectedAxis+selectedAmmount+'>'
        #print(tempString)
    if axisIndex == 3: #yaxis
        selectedAmmount=Binc[ammtIndex]
        if float(selectedAmmount)<0 and AoAB+float(selectedAmmount)<-20:
            selectedAmmount=-str(-20) # Stop the X and Y axis from hitting the end
            msgbox.showwarning("AoAB Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Min\n Pleaase dismiss and continue")
        if float(selectedAmmount)>0 and AoAB+float(selectedAmmount)>20:
            selectedAmmount=str(20)
            msgbox.showwarning("AoAB Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled-> Set to Max\n Pleaase dismiss and continue")
        selectedAxis=Axis[axisIndex]
        tempString=selectedAxis+selectedAmmount+'>'
        #print(tempString)
    if axisIndex ==4: # Axis has a middle increment X and Y
        selectedAmmountLast=Yinc[ammtIndex]
        selectedAmmountmiddle=Xinc[ammtIndex]
        if float(selectedAmmountLast)<0 and Xpos+float(selectedAmmountLast)<0:
            selectedAmmountLast=0 # Stop the X and Y axis from hitting the end
            msgbox.showwarning("X Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Min\n Pleaase dismiss and continue")
        if float(selectedAmmountLast)>0 and Xpos+float(selectedAmmountLast)>500: ##### update max distance once the design is finished
            selectedAmmountLast=500
            msgbox.showwarning("X Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Max\n Pleaase dismiss and continue")
        if float(selectedAmmountmiddle)<0 and Ypos+float(selectedAmmountmiddle)<0:
            selectedAmmountmiddle=0 # Stop the X and Y axis from hitting the end
            msgbox.showwarning("Y Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Min\n Pleaase dismiss and continue")
        if float(selectedAmmountmiddle)>0 and Ypos+float(selectedAmmountmiddle)>500:
            selectedAmmountmiddle=500
            msgbox.showwarning("X Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Min\n Pleaase dismiss and continue")
        tempString=Axis[axisIndex]+selectedAmmountmiddle+Axis[axisIndex+1]+selectedAmmountLast+'>'
        #print(tempString)
    if axisIndex ==5:
        selectedAmmountLast=Binc[ammtIndex]
        selectedAmmountmiddle=Tinc[ammtIndex]
        if float(selectedAmmountLast)<0 and AoAT+float(selectedAmmountLast)<-20:
            selectedAmmountLast=-20 # Stop the X and Y axis from hitting the end
            msgbox.showwarning("AoAB Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Min\n Pleaase dismiss and continue")
        if float(selectedAmmountLast)>0 and AoAT+float(selectedAmmountLast)>20: ##### update max distance once the design is finished
            selectedAmmountLast=20
            msgbox.showwarning("AoAB Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Max\n Pleaase dismiss and continue")
        if float(selectedAmmountmiddle)<0 and AoAB+float(selectedAmmountmiddle)<-20:
            selectedAmmountmiddle=-20 # Stop the X and Y axis from hitting the end
            msgbox.showwarning("AoAT Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Min\n Pleaase dismiss and continue")
        if float(selectedAmmountmiddle)>0 and AoAB+float(selectedAmmountmiddle)>20:
            selectedAmmountmiddle=20
            msgbox.showwarning("AoAT Move Error", "Warning:\n Move requested is outside of range of motion\n Error handled->Set to Max\n Pleaase dismiss and continue")
        tempString=Axis[axisIndex+1]+selectedAmmountmiddle+Axis[axisIndex+2]+selectedAmmountLast+'>'
        #print(tempString)
    GcodeSend(None)
  
  
  
def scenariobInit(event):
    global file, scenarioCommands
    scenarioCommands=[]
    header=[]
    rows=[]
    #print(portCbo.current()) debug
    if portCbo.current() == -1:
        msgbox.showwarning("Error 1", "Warning:\n Set Your Port Before Clicking Init.")
        return
    # Disable the button 
    scenarioInitBtn['state'] = tk.DISABLED
    # Begin Read code
    isFilePathSet=bool(filePath.get()) # if the currently set path is empty 
    # if error -> Display message -> "Could not open the CSV File\n Check the File Path!"-> Re enable the init button
    #else we are going to read the file
    if isFilePathSet == True:
        # in here w go to the requested file set in the path menu
        try:
            file = open(filePath,"r")
            csvreader = csv.reader(file)
        except FileNotFoundError:
            msgbox.showwarning("Error 1", "Warning:\n File Not Found!! Check Your Set Path")
            scenarioInitBtn['state'] = tk.NORMAL
            return
        else:
            pass
    else:
        # here we are going to the local path contined in this file
        script_path = os.path.abspath(__file__)  # i.e. /path/to/dir/foobar.py
        script_dir = os.path.split(script_path)[0]  # i.e. /path/to/dir/
        rel_path =r'Scenariodata.csv'
        abs_file_path = os.path.join(script_dir, rel_path)
        #print(abs_file_path) Debug
        # open the file
        try:
            file = open(abs_file_path,"r")
            csvreader = csv.reader(file)
        except FileNotFoundError:
            msgbox.showwarning("Error 2", "Warning:\n File Not Found!! Check Your Set Path")
            scenarioInitBtn['state'] = tk.NORMAL
            return
        else:
            pass
    # begin reading the file
    header=next(csvreader)
    i=0
    for row in csvreader:
        rows.append(row) # read one row at a time and shove it into the array

        # read the # of Rows -> if the Number of rows is >7 then -> 
        if i > 7:
            # display disp message -> "More senarios found than able to display\n Be careful that the data you want is in the correct format!"
            msgbox.showwarning("More Rows dtected than can be used", "Warning:\n More Rows detected Than can be used!\n Check formatting\n Program Will Continue")
            i=i+1
    file.close() # close he file  we are done with it and it occupies a thread. 
    # Translate the data into G code and store it in te varible scenariocommands
    i=0
    for row in rows:
        tempvarX=rows[i][1] # Zero position is a label
        tempvarY=rows[i][2] 
        tempvarAT=rows[i][3] 
        tempvarAB=rows[i][4]
        # traslate it into strings of g-code 
        Startstring='<G '
        Endstring='>'
        X='X' # space is provided by the start string
        Y=' Y' # space is required to seperate the commands
        AoAT=' AoAT'
        AoAB=' AoAB'
        XCommand=X+tempvarX
        YCommand=Y+tempvarY
        ATCommand=AoAT+tempvarAT
        ABCommand=AoAB+tempvarAB
        FinalCommand=Startstring+XCommand+YCommand+ATCommand+ABCommand+Endstring
        scenarioCommands.append(FinalCommand) # sore the value as g code.
        i=i+1
        #print(FinalCommand) # debug
    scenarioBtn['state'] = tk.NORMAL
    
    
    
    
    
    
if __name__ == '__main__':
    APP_TITLE = 'AirFoil Control GUI'
    BAUD_RATES = (300, 1200, 2400, 4800, 9600, 19200, 38400, 57600,
                  76800, 115200, 23040, 500000, 1000000, 2000000)
    # set serial Data bit size see: https://pythonhosted.org/pyserial/pyserial_api.html
    DATABITS = (serial.FIVEBITS, serial.SIXBITS,
                serial.SEVENBITS, serial.EIGHTBITS)
    PARITY = (serial.PARITY_EVEN, serial.PARITY_ODD,
              serial.PARITY_NONE, serial.PARITY_MARK, serial.PARITY_SPACE)
    PARITY_VAL = ('Even', 'Odd', 'None', 'Mark', 'Space')
    STOPBITS = (serial.STOPBITS_ONE,
                serial.STOPBITS_ONE_POINT_FIVE, serial.STOPBITS_TWO)
    ports = {p.name: p.description for p in list_ports.comports()}
    currentPort = serial.Serial(
        port=None, baudrate=9600, timeout=0, write_timeout=0)
    portDesc = ''
    sentTexts = []
    sentTextsPtr = 0
    isEndByNL = True
    lastUpdatedBy = 2
    ico = None

    data = {}
    fname = __file__.rsplit('.', 1)[0]
    jfile = None
    try:
        jfile = open(fname+'.json')
        data = json.load(jfile)
    except FileNotFoundError as fnfe:
        pass
    if jfile:
        jfile.close()

    ### Root Function #######################################################################
    root = tk.Tk()
    root.configure(bg='#222222')
    style = ttk.Style(root)
    root.tk.call('source','C://Users//ecslogon//Documents//PlatformIO\Projects//WSU_AIRFOIL_PROJECT//Python_GUI//Sun-Valley-ttk-theme-master//sun-valley.tcl')
    root.tk.call("set_theme", "dark") 
    filePath = tk.StringVar()
    root.title(APP_TITLE) # PaneWindow
    root.geometry('1000x960')
    root.resizable(False,False)
    try:
        ico = tk.PhotoImage(file=fname+'.png')
    except:
        pass
    if ico:
        root.iconphoto(False, ico)
    root.protocol("WM_DELETE_WINDOW", exitRoot)

    autoscrollVar = tk.BooleanVar()
    showTimestampVar = tk.BooleanVar()
    showSentTextVar = tk.BooleanVar()
    dispHexVar = tk.BooleanVar()
    rowSplit = 12
    internalData= BooleanVar(root,False)
    oneshot=tk.BooleanVar(root,False)
    directionVar=tk.IntVar(root,1)
    recv=IntVar()
    Xstr=''
    Ystr=''
    Tstr=''
    Bstr=''
    AXstr=''
    AYstr=''
    ATstr=''
    ABstr=''
    SXstr=''
    SYstr=''
    STstr=''
    SBstr=''
    Xpos=0.00
    Ypos=0.00
    AoAT=0.00
    AoAB=0.00
    AX=0
    AY=0
    AT=0
    AB=0
    SX=0
    SY=0
    ST=0
    SB=0
    # Grid Initilization 
    # lock the space between the input bar and the text box *Input bar
    tk.Grid.rowconfigure(root, 0, weight=1)
    # Expand the text box as the sreen expands
    tk.Grid.rowconfigure(root, 1, weight=1)
    tk.Grid.rowconfigure(root, 2, weight=1)
    tk.Grid.rowconfigure(root, 3, weight=1)
    tk.Grid.rowconfigure(root, 4, weight=1)
    tk.Grid.rowconfigure(root, 5, weight=1) 
    tk.Grid.rowconfigure(root, 6, weight=1) # Start bottom Rows : Send Box
    tk.Grid.rowconfigure(root, 7, weight=1) # Serial Text box
    tk.Grid.rowconfigure(root, 8, weight=1)  # Select port stuff 
    tk.Grid.rowconfigure(root, 9, weight=1)
    tk.Grid.rowconfigure(root, 10, weight=1)
    tk.Grid.rowconfigure(root, 11, weight=1)
    tk.Grid.rowconfigure(root, 12, weight=1)
    tk.Grid.rowconfigure(root, 13, weight=1)
    tk.Grid.rowconfigure(root, 14, weight=1)
    tk.Grid.rowconfigure(root, 15, weight=1)
    tk.Grid.rowconfigure(root, 16, weight=1)
    tk.Grid.rowconfigure(root, 17, weight=1)
    tk.Grid.rowconfigure(root, 18, weight=1)
    tk.Grid.rowconfigure(root, 19, weight=1)
    tk.Grid.rowconfigure(root, 20, weight=1)

    tk.Grid.columnconfigure(root, 0, weight=1)
    tk.Grid.columnconfigure(root, 1, weight=1)
    tk.Grid.columnconfigure(root, 2, weight=1)
    tk.Grid.columnconfigure(root, 3, weight=1)
    tk.Grid.columnconfigure(root, 4, weight=1)
    tk.Grid.columnconfigure(root, 5, weight=1)
    tk.Grid.columnconfigure(root, 6, weight=1)
    tk.Grid.columnconfigure(root, 7, weight=1)
    tk.Grid.columnconfigure(root, 8, weight=1)
 
 # Send Text Menu
    txText = ttk.Entry(root)
    txText.grid(row=0+rowSplit, column=0, columnspan=6,
                padx=4, pady=8, sticky=tk.N+tk.EW)
    txText.bind('<Up>', upKeyCmd)
    txText.bind('<Down>', downKeyCmd)
    txText.bind('<Button-3>', showTxTextMenu)
    
 ## Begin Movment mode 
    # Enter Button Code
    
    # Radio Button Code 
    incremnetBtnSelect= tk.IntVar()
    axisBtnSelect= tk.IntVar()
    scenariobBtnSelect= tk.IntVar()
    directionBtnSelect=tk.IntVar()
    
 # initializing the choice, i.e. Python
    Increment = [(".05",0),
            (".1", 1),
            ("1", 2),
            ("10", 3),
            ("100", 4)]
    Direction = [("+",0),
            ("-", 1)]
    Axis = [("X Axis", 0),
         ("Y Axis", 1),
         ("AoA Top", 2),
         ("AoA Bottom", 3),
         ("Move X&Y",4),
         ("Move AoAT&B",5)]
    Scenario =[("Scenario 0",0),
            ("Scenario 1",1),
            ("Scenario 2",2),
            ("Scenario 3",3),
            ("Scenario 4",4),
            ("Scenario 5",5)]
    incrementFrame = ttk.LabelFrame(root, text='''Ammount''')
    incrementFrame.grid(row=0,column=0,rowspan=2)
    directionFrame = ttk.LabelFrame(root, text='Direction')
    directionFrame.grid(row=3,column=0,rowspan=1)
    axisFrame = ttk.LabelFrame(root, text='Axis Select')
    axisFrame.grid(row=0,column=1,rowspan=2)
    scenarioFrame = ttk.LabelFrame(root, text='Scenario Select')
    scenarioFrame.grid(row=0,column=3, rowspan=2)
    for Increment, val in Increment:
        incrementRadiobtn=ttk.Radiobutton(incrementFrame, 
                    text=Increment,
                    width=10,
                    variable=incremnetBtnSelect, 
                    value=val)
        incrementRadiobtn.grid(row=1+val,column=0, padx=4, pady=2,sticky=tk.NW )
    for Direction, val in Direction:
        directionRadioBtn=ttk.Radiobutton(directionFrame, 
                    text=Direction,
                    width=10,
                    variable=directionBtnSelect, 
                    value=val)
        directionRadioBtn.grid(row=1+val,column=0, padx=4, pady=2,sticky=tk.NW )
    for Axis, val in Axis:
        radioBtn2=ttk.Radiobutton(axisFrame, 
                    text=Axis,
                    width=15,
                    variable=axisBtnSelect, 
                    value=val)
        radioBtn2.grid(row=+val,column=0, padx=5, pady=2,sticky=tk.NW )
    for Scenario, val in Scenario:
          ScenarioBtn2=ttk.Radiobutton(scenarioFrame, 
                        text=Scenario,
                        width=15,
                        variable=scenariobBtnSelect, 
                        value=val)
          ScenarioBtn2.grid(row=1+val,column=0, padx=5, pady=2,sticky=tk.NW )
    # End Radio button code 
    moveBtn = ttk.Button(root, width=20, text='Move', command=lambda:moveButton(None))
    moveBtn.grid(row=0, column=2, padx=4, pady=4,ipady=15, ipadx=15)
    homeBtn = ttk.Button(root, width=20, text='Home Selected axis', command= lambda:homeAxis(None))
    homeBtn.grid(row=1, column=2, padx=4, pady=4,ipady=15, ipadx=15)
    homeAllBtn = ttk.Button(root, width=20, text='Home All Axis', command= lambda:homeAll(None))
    homeAllBtn.grid(row=0, column=6, padx=4, pady=4,ipady=15, ipadx=15)
    stopBtn = ttk.Button(root, width=20, text='E STOP',
                        command=lambda: Stop(None))
    stopBtn.grid(row=1, column=6, padx=4, pady=4,ipady=20, ipadx=20)
    scenarioBtn = ttk.Button(root, width=20, text='Move to Senario',
                        state=tk.DISABLED, command= lambda:scenarioMove(None))
    scenarioBtn.grid(row=1, column=4, padx=4, pady=4,ipady=15, ipadx=15)
    scenarioInitBtn = ttk.Button(root, width=20, text='Scenario Init.', command=lambda:scenariobInit(None) )
    scenarioInitBtn.grid(row=0, column=4, padx=4, pady=4,ipady=15, ipadx=15)
    
    XacellSliderFrame = ttk.LabelFrame(root, text='''X Axis: Acceleration MM/s^2''')
    XacellSliderFrame.grid(row=3,column=4,columnspan=5,ipadx=5,ipady=0)
    XacellerationSlider = TickScale(XacellSliderFrame, from_=0, to=200,length=400,tickinterval=20,orient=HORIZONTAL)
    XacellerationSlider.grid(row=0, column=1, columnspan=4,ipadx=5,ipady=1)
    XspeedSliderFrame= ttk.LabelFrame(root, text='''X Axis: Speed MM/S''')
    XspeedSliderFrame.grid(row=3,column=1,columnspan=3,ipadx=5,ipady=0)
    XspeedSlider = TickScale(XspeedSliderFrame, from_=0, to=200,length=400,tickinterval=20,orient=HORIZONTAL)
    XspeedSlider.grid(row=0, column=1, columnspan=4,ipadx=5,ipady=0)
    YacellSliderFrame = ttk.LabelFrame(root, text='''Y Axis: Acceleration MM/s^2''')
    YacellSliderFrame.grid(row=4,column=4,columnspan=5,ipadx=5,ipady=0)
    YacellerationSlider = TickScale(YacellSliderFrame, from_=0, to=200,length=400,tickinterval=20,orient=HORIZONTAL)
    YacellerationSlider.grid(row=0, column=1, columnspan=4,ipadx=5,ipady=0)
    YspeedSliderFrame= ttk.LabelFrame(root, text='''Y Axis Speed MM/S''')
    YspeedSliderFrame.grid(row=4,column=1,columnspan=3,ipadx=5,ipady=0)
    YspeedSlider = TickScale(YspeedSliderFrame, from_=0, to=200,length=400,tickinterval=20,orient=HORIZONTAL)
    YspeedSlider.grid(row=0, column=1, columnspan=4,ipadx=5,ipady=0)
    TacellSliderFrame = ttk.LabelFrame(root, text='''Angle of Attack Top: Acceleration Degree/s^2''')
    TacellSliderFrame.grid(row=5,column=4,columnspan=5,ipadx=5,ipady=0)
    TacellerationSlider = TickScale(TacellSliderFrame, from_=0, to=200,length=400,tickinterval=20,orient=HORIZONTAL)
    TacellerationSlider.grid(row=0, column=1, columnspan=4,ipadx=5,ipady=0)
    TspeedSliderFrame= ttk.LabelFrame(root, text='''Angle of Attack Top: Speed Degree/S''')
    TspeedSliderFrame.grid(row=5,column=1,columnspan=3,ipadx=5,ipady=0)
    TspeedSlider = TickScale(TspeedSliderFrame, from_=0, to=200,length=400,tickinterval=20,orient=HORIZONTAL)
    TspeedSlider.grid(row=0, column=1, columnspan=4,ipadx=5,ipady=0)
    BacellSliderFrame = ttk.LabelFrame(root, text='''Angle of Attack Bottom: Acceleration Degree/s^2''')
    BacellSliderFrame.grid(row=6,column=4,columnspan=5,ipadx=5,ipady=0)
    BacellerationSlider = TickScale(BacellSliderFrame, from_=0, to=200,length=400,tickinterval=20,orient=HORIZONTAL)
    BacellerationSlider.grid(row=0, column=1, columnspan=4,ipadx=5,ipady=0)
    BspeedSliderFrame= ttk.LabelFrame(root, text='''Angle of Attack Bottom: Speed Degree/S''')
    BspeedSliderFrame.grid(row=6,column=1,columnspan=3,ipadx=5,ipady=0)
    BspeedSlider = TickScale(BspeedSliderFrame, from_=0, to=200,length=400,tickinterval=20,orient=HORIZONTAL)
    BspeedSlider.grid(row=0, column=1, columnspan=4,ipadx=5,ipady=0)
    #ScenarioBtn = tk.Button(root, width=20,height=8, text='Move to Senario',
    #					state=tk.DISABLED, command= moveButton)
    #ScenarioBtn.grid(row=4, column=4, padx=4, pady=4, rowspan=2)
 
 
 ## End Movment UI Code 
    sendBtn = ttk.Button(root, width=25, text='Send',
                        state=tk.DISABLED, command=lambda: sendCmd(None))
    sendBtn.grid(row=0+rowSplit, column=6, padx=4, pady=4, sticky=tk.NE)

    rxText = tkscroll.ScrolledText(
        root, height=20, state=tk.DISABLED, font=('Courier', 10), wrap=tk.WORD)
    rxText.grid(row=1+rowSplit,column=0, columnspan=8, padx=4, sticky=tk.NSEW)
    rxText.bind('<Button-3>', showRxTextMenu)

    autoscrollCbt = ttk.Checkbutton(
        root, text='Autoscroll', variable=autoscrollVar, onvalue=True, offvalue=False)
    autoscrollCbt.grid(row=2+rowSplit, column=0, padx=12, pady=4,ipadx=19, sticky=tk.SW)
    di = data.get('autoscroll')
    if di != None:
        autoscrollVar.set(di)

    showTimestampCbt = ttk.Checkbutton(
        root, text='Show timestamp', variable=showTimestampVar, onvalue=True, offvalue=False)
    showTimestampCbt.grid(row=2+rowSplit, column=1, padx=12, pady=4, ipadx=19,  sticky=tk.SW)
    di = data.get('showtimestamp')
    if di != None:
        showTimestampVar.set(di)

    showSentTextCbt = ttk.Checkbutton(
        root, text='Show sent text', variable=showSentTextVar, onvalue=True, offvalue=False)
    showSentTextCbt.grid(row=2+rowSplit, column=2, padx=12, pady=4, sticky=tk.SW)
    di = data.get('showsenttext')
    if di != None:
        showSentTextVar.set(di)

    portCbo = ttk.Combobox(root, width=10)
    portCbo.grid(row=2+rowSplit, column=3, padx=4, pady=4, sticky=tk.SE)
    portCbo.bind('<<ComboboxSelected>>', changePort)
    portCbo['values'] = sorted(ports)
    if len(ports) > 0:
        portCbo['state'] = 'readonly'
        portCbo.set('Select port')
    else:
        portCbo['state'] = tk.DISABLED
        portCbo.set('No port')

    lineEndingCbo = ttk.Combobox(root, width=10, state='readonly')
    lineEndingCbo.grid(row=2+rowSplit, column=4, padx=4, pady=4, sticky=tk.SE)
    lineEndingCbo['values'] = (
        'No line ending', 'Newline', 'Carriage return', 'Both CR & NL')
    di = data.get('lineending')
    if di != None:
        lineEndingCbo.current(di)
    else:
        lineEndingCbo.current(0)

    baudrateCbo = ttk.Combobox(root, width=12, state='readonly')
    baudrateCbo.grid(row=2+rowSplit, column=5, padx=4, pady=4,ipadx=12,sticky=tk.SE)
    baudrateCbo['values'] = list(str(b) + ' baud' for b in BAUD_RATES)
    baudrateCbo.bind('<<ComboboxSelected>>', changeBaudrate)
    di = data.get('baudrateindex')
    if di != None:
        baudrateCbo.current(di)
        currentPort.baudrate = BAUD_RATES[di]
    else:
        baudrateCbo.current(4)  # 9600 baud
        currentPort.baudrate = BAUD_RATES[4]

    clearBtn = ttk.Button(
        root, width=12, text='Clear output', command=clearOutputCmd)
    clearBtn.grid(row=2+rowSplit, column=6, padx=4, pady=4, sticky=tk.SE)

    txTextMenu = ttk.Menubutton(txText)
    txTextMenu.menu = tk.Menu(txTextMenu, tearoff=0)
    txTextMenu['menu'] = txTextMenu.menu
    txTextMenu.menu.add_command(label='Cut', accelerator='Ctrl+X',
                           command=lambda: txText.event_generate('<<Cut>>'))
    txTextMenu.menu.add_command(label='Copy', accelerator='Ctrl+C',
                           command=lambda: txText.event_generate('<<Copy>>'))
    txTextMenu.menu.add_command(label='Paste', accelerator='Ctrl+V',
                           command=lambda: txText.event_generate('<<Paste>>'))

    #rxTextMenu = tk.Menu(rxText, tearoff=0)
    rxTextMenu = ttk.Menubutton(rxText)
    rxTextMenu.menu = tk.Menu(rxTextMenu, tearoff=0)
    #rxTextMenu['menu'] = rxTextMenu.menu
    rxTextMenu.menu.add_command(label='Copy', accelerator='Ctrl+C',
                           command=lambda: rxText.event_generate('<<Copy>>'))
    rxTextMenu.menu.add_separator()
    rxTextMenu.menu.add_command(label='Close active port', command=closePort)
    rxTextMenu.menu.add_separator()
    rxTextMenu.menu.add_checkbutton(label='Display in hexadecimal code',
                               onvalue=True, offvalue=False, variable=dispHexVar)
    rxTextMenu.menu.add_separator()
    rxTextMenu.menu.add_command(label='Port setting', command=setting)
    rxTextMenu.menu.add_separator()
    rxTextMenu.menu.add_command(label='Scenario File Path', command=filePathSettings)
    rxTextMenu.menu.add_separator()
    rxTextMenu.menu.add_checkbutton(label='Enable Light Mode',
                               onvalue=True, offvalue=False, command=change_theme)
    rxTextMenu.menu.add_separator()
    rxTextMenu.menu.add_command(label='About', command=showAbout)

    listPortsPolling()

    root.update()
    sw = root.winfo_screenwidth()
    sh = root.winfo_screenheight()
    rw = root.winfo_width()
    rh = root.winfo_height()
    root.minsize(rw, 233)
    root.geometry(f'{rw}x{rh}+{int((sw-rw)/2)}+{int((sh-rh)/2)-30}')
    wtx =\
        """
************************************************************************************
* Welcome to Python GUI Serial Monitor as an alternative to Arduino Serial Monitor *
* Use up and down arrow keys to select previously-entered text from history list.  *
* Right click on output console for additional menus.                              *
************************************************************************************
"""
    writeConsole(wtx, 2)

    di = data.get('displayhex')
    if di != None:
        dispHexVar.set(di)
    di = data.get('databits')
    if di != None:
        currentPort.bytesize = di
    di = data.get('parity')
    if di != None:
        currentPort.parity = di
    di = data.get('stopbits')
    if di != None:
        currentPort.stopbits = di
    di = data.get('portindex')
    if di != None and di != -1 and data.get('portlist') == ports:
        portCbo.current(di)
        changePort(None)

    root.mainloop()
