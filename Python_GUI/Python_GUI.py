# https://wiki.python.org/moin/TkInter
# https://towardsdatascience.com/top-10-python-gui-frameworks-for-developers-adca32fbe6fc
# from tkinter import *

# root = Tk()
# # Creating my label widget

# def myClick():
#     myLabel =Label(root, text="look i made a button do somthing")
#     myLabel.pack()

# mylabel1 = Label(root, text="hello world")
# mylabel2= Label(root, text="My Name")
# mybutton = Button(root, text= " Push Me", command = myClick(), padx=10,pady=10)
# # Putting it on to the screen
# mylabel1.grid(row=0,column=0)
# mylabel2.grid(row=1,column=0)
# mybutton.grid(row=2,column=0)
# # creating an event loop
# root.mainloop()

######################################################################################
# This tutorial is provided by WeeW-Stack
# To get the details of this Tutorial you can take a look on the following link:
# https: // youtu.be/tBgTSdzkSGM
# Enjoy
######################################################################################

# from tkinter import *
# import serial.tools.list_ports
# import threading
# import signal
# import time

# # Dealing with interrupt issue at thread level


# def signal_handler(signum, frame):
#     sys.exit()


# # Dealing with interrupt issue at thread level
# signal.signal(signal.SIGINT, signal_handler)


# # Class to manage the dynamic animation on the GUI
# class Graphics():
#     pass

# # function that will toggle the Pin


# def togglePin():

#     if toggle_Pin_btn["text"] in "Pin High":
#         print("Pin HIGH")
#         pin_on = "1C13\n"
#         ser.write(pin_on.encode())
#         toggle_Pin_btn["text"] = "Pin Low"

#     else:
#         print("Pin LOW")
#         pin_off = "0C13\n"
#         ser.write(pin_off.encode())
#         toggle_Pin_btn["text"] = "Pin High"

# # function To get / stop the ADC data


# def getADC():
#     global serialData, graph
#     if get_ADC_btn["text"] in "Start ADC":
#         get_ADC_btn["text"] = "Stop ADC"
#         print("ADC ON")
#         pin_on = "AA001\n"
#         ser.write(pin_on.encode())
#         serialData = True
#         t1 = threading.Thread(target=readSerial)
#         t1.deamon = True
#         t1.start()

#     else:
#         print("ADC Off")
#         serialData = False
#         pin_off = "AA000\n"
#         ser.write(pin_off.encode())
#         get_ADC_btn["text"] = "Start ADC"
#         graph.canvas.itemconfig(
#             graph.text, text="---")

# # Function to initialize the GUI


# def connect_menu_init():
#     global root, connect_btn, refresh_btn, graph, toggle_Pin_btn, get_ADC_btn
#     root = Tk()
#     root.title("Serial communication")

#     # the size as we have new Gui
#     root.geometry("800x600")

#     root.config(bg="white")

#     port_lable = Label(root, text="Available Port(s): ", bg="white")
#     port_lable.grid(column=1, row=2, pady=20, padx=10)

#     port_bd = Label(root, text="Baude Rate: ", bg="white")
#     port_bd.grid(column=1, row=3, pady=20, padx=10)

#     refresh_btn = Button(root, text="R", height=2,
#                          width=10, command=update_coms)
#     refresh_btn.grid(column=3, row=2)

#     connect_btn = Button(root, text="Connect", height=2,
#                          width=10, state="disabled", command=connexion)
#     connect_btn.grid(column=3, row=4)
#     baud_select()
#     update_coms()

#     graph = Graphics()

#     graph.canvas = Canvas(root, width=300, height=300,
#                           bg="white", highlightthickness=0)
#     graph.canvas.grid(row=6, columnspan=5)

#     # part to hide the canvas until needed
#     graph.canvas.grid_remove()

#     # Dynamic update
#     graph.outer = graph.canvas.create_arc(
#         10, 10, 290, 290, start=90, extent=100, outline="#f11", fill="#f11", width=2)
#     # Static
#     graph.canvas.create_oval(
#         75, 75, 225, 225, outline="#f11", fill="white", width=2)
#     # Dynamic update
#     graph.text = graph.canvas.create_text(
#         150, 150, anchor=E, font=("Helvetica", "20"), text="---")
#     # Static
#     graph.canvas.create_text(
#         175, 150, anchor=CENTER, font=("Helvetica", "20"), text="mV")

#     # Part for the toggle Pin + adding it to global variable
#     toggle_Pin_btn = Button(root, text="Pin High", height=2,
#                             width=10, command=togglePin)
#     toggle_Pin_btn.grid(column=1, row=5)
#     toggle_Pin_btn.grid_remove()
#     # 1 btn to start / stop reading the ADC data
#     get_ADC_btn = Button(root, text="Start ADC", height=2,
#                          width=10, command=getADC)
#     get_ADC_btn.grid(column=2, row=5)
#     get_ADC_btn.grid_remove()


# # Function to Enable disable the connect Btn
# def connect_check(args):
#     if "-" in clicked_com.get() or "-" in clicked_bd.get():
#         connect_btn["state"] = "disable"
#     else:
#         connect_btn["state"] = "active"


# # Function to list the Baudes
# def baud_select():
#     global clicked_bd, drop_bd
#     clicked_bd = StringVar()
#     bds = ["-",
#            "300",
#            "600",
#            "1200",
#            "2400",
#            "4800",
#            "9600",
#            "14400",
#            "19200",
#            "28800",
#            "38400",
#            "56000",
#            "57600",
#            "115200",
#            "128000",
#            "256000"]
#     clicked_bd.set(bds[0])
#     drop_bd = OptionMenu(root, clicked_bd, *bds, command=connect_check)
#     drop_bd.config(width=20)
#     drop_bd.grid(column=2, row=3, padx=50)

# # Function to Get the available COMs connected to the PC


# def update_coms():
#     global clicked_com, drop_COM
#     ports = serial.tools.list_ports.comports()
#     coms = [com[0] for com in ports]
#     coms.insert(0, "-")
#     try:
#         drop_COM.destroy()
#     except:
#         pass
#     clicked_com = StringVar()
#     clicked_com.set(coms[0])
#     drop_COM = OptionMenu(root, clicked_com, *coms, command=connect_check)
#     drop_COM.config(width=20)
#     drop_COM.grid(column=2, row=2, padx=50)
#     connect_check(0)

# # Function (Thread 3) to Manage the GUI animation


# def graph_control(graph):
#     graph.canvas.itemconfig(
#         graph.outer, exten=int(359*graph.sensor/1000))
#     graph.canvas.itemconfig(
#         graph.text, text=f"{int(3.3*graph.sensor)}")

# # Function (Thread 2) to Manage Reading the UART data from MCU


# def readSerial():
#     print("thread start")
#     global serialData, graph
#     average = 0
#     sampling = 50
#     sample = 0
#     while serialData:
#         data = ser.readline()
#         if len(data) > 0:
#             try:
#                 sensor = int(data.decode('utf8'))
#                 data_sensor = int(data.decode('utf8'))
#                 average += data_sensor
#                 sample += 1
#                 if sample == sampling:
#                     sensor = int(average/sampling)
#                     average = 0
#                     sample = 0
#                 # print(sensor)
#                     graph.sensor = sensor
#                     t2 = threading.Thread(target=graph_control, args=(graph,))
#                     t2.deamon = True
#                     t2.start()

#             except:
#                 pass

# # Function to connect / Disconnect serial Com


# def connexion():
#     global ser, serialData, toggle_Pin_btn
#     if connect_btn["text"] in "Disconnect":
#         connect_btn["text"] = "Connect"
#         refresh_btn["state"] = "active"
#         drop_bd["state"] = "active"
#         drop_COM["state"] = "active"
#         toggle_Pin_btn.grid_remove()
#         # 3 Hide the ADC widgets
#         graph.canvas.grid_remove()
#         get_ADC_btn.grid_remove()

#     else:
#         connect_btn["text"] = "Disconnect"
#         refresh_btn["state"] = "disable"
#         drop_bd["state"] = "disable"
#         drop_COM["state"] = "disable"
#         port = clicked_com.get()
#         baud = clicked_bd.get()
#         # Hide the btn when the connection ends
#         try:
#             ser = serial.Serial(port, baud, timeout=0)
#         except:
#             pass
#         # Show the toggle Pin and comment The serial read + Remove
#         toggle_Pin_btn.grid()
#         # 2 Show the adc widget
#         graph.canvas.grid()
#         get_ADC_btn.grid()

# # Dealing with interrupt issue at thread level


# def close_window():
#     global root, serialData
#     serialData = False
#     time.sleep(0.25)
#     root.destroy()


# connect_menu_init()  # Start the GUI & Initialization
# root.protocol("WM_DELETE_WINDOW", close_window)  # When Closing the Window
# root.mainloop()  # Stating the main loop / Thread

import tkinter as tk
from tkinter import Button, Label, ttk as ttk
from tkinter import scrolledtext as tkscroll
from tkinter import messagebox as msgbox
from tkinter import *
import serial
import serial.tools.list_ports as list_ports
import time
import json


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
			return '\\r'
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
		txTextMenu.entryconfigure(2, state=tk.NORMAL)
	except:
		txTextMenu.entryconfigure(2, state=tk.DISABLED)
	try:
		txTextMenu.tk_popup(event.x_root, event.y_root)
	finally:
		txTextMenu.grab_release()


def showRxTextMenu(event):
	if len(rxText.tag_ranges(tk.SEL)):
		rxTextMenu.entryconfigure(0, state=tk.NORMAL)
	else:
		rxTextMenu.entryconfigure(0, state=tk.DISABLED)
	if currentPort.isOpen():
		rxTextMenu.entryconfigure(2, state=tk.NORMAL)
	else:
		rxTextMenu.entryconfigure(2, state=tk.DISABLED)
	try:
		rxTextMenu.tk_popup(event.x_root, event.y_root)
	finally:
		rxTextMenu.grab_release()


def writeConsole(txt, upd=0):
	global isEndByNL, lastUpdatedBy
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
	ad += txt
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
	if not currentPort.is_open:
		return
	preset = time.perf_counter_ns()
	try:
		while currentPort.in_waiting > 0 and time.perf_counter_ns()-preset < 2000000:  # loop duration about 2ms
			ch = currentPort.read()  # read the current data on the port
			tm = time.strftime('%H:%M:%S.{}'.format(
				repr(time.time()).split('.')[1][:3]))
			txt = ''
			if dispHexVar.get():  # If display in Hex is true turn the chacter to it hex value
				txt += get_hexstr_of_chr(ch)
			else:  # else of the chacter is desired in a string convert the
				txt += get_str_of_chr(ch)
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
	msgbox.showinfo(APP_TITLE, 'Designed by ZulNs\n@Gorontalo, 13 April 2021')


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
	tk.Label(settingDlg, text='Parity:').grid(
		row=1, column=1, padx=0, pady=0, sticky=tk.NS+tk.E)
	tk.Label(settingDlg, text='Stop bits:').grid(
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
	tk.Button(settingDlg, text='Default', width=10, command=defaultSetting).\
		grid(row=1, column=0, padx=12, pady=0, sticky=tk.NS+tk.W)
	tk.Button(settingDlg, text='OK', width=10, command=lambda: setPort(None)).\
		grid(row=3, column=1, padx=0, pady=12, sticky=tk.S)
	cancelBtn = tk.Button(settingDlg, text='Cancel',
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


def defaultSetting():
	dataBitsCbo.set(serial.EIGHTBITS)
	parityCbo.current(PARITY.index(serial.PARITY_NONE))
	stopBitsCbo.set(serial.STOPBITS_ONE)


def setPort(event):
	currentPort.bytesize = DATABITS[dataBitsCbo.current()]
	currentPort.parity = PARITY[parityCbo.current()]
	currentPort.stopbits = STOPBITS[stopBitsCbo.current()]
	settingDlg.destroy()


def hideSetting(event):  # hide the settings menu
	settingDlg.destroy()

rowSplit = 7

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

	### Root Function ###
	root = tk.Tk()
	root.title(APP_TITLE) # PaneWindow
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

	# lock the space between the input bar and the text box *Input bar
	tk.Grid.rowconfigure(root, 0, weight=1)
	# Expand the text box as the sreen expands
	tk.Grid.rowconfigure(root, 1, weight=10)
	tk.Grid.rowconfigure(root, 2, weight=10)
	tk.Grid.rowconfigure(root, 3, weight=10)
	tk.Grid.rowconfigure(root, 4, weight=10)
	tk.Grid.rowconfigure(root, 5, weight=1) 
	tk.Grid.rowconfigure(root, 6, weight=1) # Start bottom Rows : Send Box
	tk.Grid.rowconfigure(root, 7, weight=50) # Serial Text box
	tk.Grid.rowconfigure(root, 8, weight=1)  # Select port stuff 
	tk.Grid.rowconfigure(root, 9, weight=1)
	

	tk.Grid.columnconfigure(root, 0, weight=1)
	tk.Grid.columnconfigure(root, 1, weight=1)
	tk.Grid.columnconfigure(root, 2, weight=1)
	tk.Grid.columnconfigure(root, 3, weight=50)
	tk.Grid.columnconfigure(root, 4, weight=1)
	tk.Grid.columnconfigure(root, 5, weight=1)
	tk.Grid.columnconfigure(root, 6, weight=1)

	txText = tk.Entry(root)
	txText.grid(row=0+rowSplit, column=0, columnspan=6,
				padx=4, pady=8, sticky=tk.N+tk.EW)
	txText.bind('<Up>', upKeyCmd)
	txText.bind('<Down>', downKeyCmd)
	txText.bind('<Button-3>', showTxTextMenu)
	
 
	# Radio Button Code 
	v = tk.IntVar()
	v.set(1)
 # initializing the choice, i.e. Python
	languages = [("Python",0),
			("Perl", 1),
			("Java", 2),
			("C++", 3),
			("C", 4)]
	frame = LabelFrame(root, text='Select Beverage', padx=10,pady=2, bg= '#dddddd')
	for language, val in languages:
		radioBtn=tk.Radiobutton(frame, 
					text=language,
					indicatoron = 0,
					padx = 2, 
					width=10,
					variable=v, 
					value=val)
		radioBtn.grid(row=2+val,column=0, padx=4, pady=2,sticky=tk.NW )
	frame.grid(row=0,column=0)
 
 
	# End Radio button code 
	sendBtn = tk.Button(root, width=12, text='Send',
						state=tk.DISABLED, command=lambda: sendCmd(None))
	sendBtn.grid(row=0+rowSplit, column=6, padx=4, pady=4, sticky=tk.NE)

	rxText = tkscroll.ScrolledText(
		root, height=20, state=tk.DISABLED, font=('Courier', 10), wrap=tk.WORD)
	rxText.grid(row=1+rowSplit,column=0, columnspan=7, padx=4, sticky=tk.NSEW)
	rxText.bind('<Button-3>', showRxTextMenu)

	autoscrollCbt = tk.Checkbutton(
		root, text='Autoscroll', variable=autoscrollVar, onvalue=True, offvalue=False)
	autoscrollCbt.grid(row=2+rowSplit, column=0, padx=4, pady=4, sticky=tk.SW)
	di = data.get('autoscroll')
	if di != None:
		autoscrollVar.set(di)

	showTimestampCbt = tk.Checkbutton(
		root, text='Show timestamp', variable=showTimestampVar, onvalue=True, offvalue=False)
	showTimestampCbt.grid(row=2+rowSplit, column=1, padx=4, pady=4, sticky=tk.SW)
	di = data.get('showtimestamp')
	if di != None:
		showTimestampVar.set(di)

	showSentTextCbt = tk.Checkbutton(
		root, text='Show sent text', variable=showSentTextVar, onvalue=True, offvalue=False)
	showSentTextCbt.grid(row=2+rowSplit, column=2, padx=4, pady=4, sticky=tk.SW)
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

	lineEndingCbo = ttk.Combobox(root, width=14, state='readonly')
	lineEndingCbo.grid(row=2+rowSplit, column=4, padx=4, pady=4, sticky=tk.SE)
	lineEndingCbo['values'] = (
		'No line ending', 'Newline', 'Carriage return', 'Both CR & NL')
	di = data.get('lineending')
	if di != None:
		lineEndingCbo.current(di)
	else:
		lineEndingCbo.current(0)

	baudrateCbo = ttk.Combobox(root, width=12, state='readonly')
	baudrateCbo.grid(row=2+rowSplit, column=5, padx=4, pady=4, sticky=tk.SE)
	baudrateCbo['values'] = list(str(b) + ' baud' for b in BAUD_RATES)
	baudrateCbo.bind('<<ComboboxSelected>>', changeBaudrate)
	di = data.get('baudrateindex')
	if di != None:
		baudrateCbo.current(di)
		currentPort.baudrate = BAUD_RATES[di]
	else:
		baudrateCbo.current(4)  # 9600 baud
		currentPort.baudrate = BAUD_RATES[4]

	clearBtn = tk.Button(
		root, width=12, text='Clear output', command=clearOutputCmd)
	clearBtn.grid(row=2+rowSplit, column=6, padx=4, pady=4, sticky=tk.SE)

	txTextMenu = tk.Menu(txText, tearoff=0)
	txTextMenu.add_command(label='Cut', accelerator='Ctrl+X',
						   command=lambda: txText.event_generate('<<Cut>>'))
	txTextMenu.add_command(label='Copy', accelerator='Ctrl+C',
						   command=lambda: txText.event_generate('<<Copy>>'))
	txTextMenu.add_command(label='Paste', accelerator='Ctrl+V',
						   command=lambda: txText.event_generate('<<Paste>>'))

	rxTextMenu = tk.Menu(rxText, tearoff=0)
	rxTextMenu.add_command(label='Copy', accelerator='Ctrl+C',
						   command=lambda: rxText.event_generate('<<Copy>>'))
	rxTextMenu.add_separator()
	rxTextMenu.add_command(label='Close active port', command=closePort)
	rxTextMenu.add_separator()
	rxTextMenu.add_checkbutton(label='Display in hexadecimal code',
							   onvalue=True, offvalue=False, variable=dispHexVar)
	rxTextMenu.add_separator()
	rxTextMenu.add_command(label='Port setting', command=setting)
	rxTextMenu.add_separator()
	rxTextMenu.add_command(label='About', command=showAbout)

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
* Monitor Credit: Designed by ZulNs @Gorontalo, 13 April 2021                      *
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
