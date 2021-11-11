from tkinter import *

root = Tk()
# Creating my label widget

def myClick():
    myLabel =Label(root, text="look i made a button do somthing")
    myLabel.pack()

mylabel1 = Label(root, text="hello world")
mylabel2= Label(root, text="My Name")
mybutton = Button(root, text= " Push Me", command = myClick(), padx=10,pady=10)
# Putting it on to the screen
mylabel1.grid(row=0,column=0)
mylabel2.grid(row=1,column=0)
mybutton.grid(row=2,column=0)
# creating an event loop 
root.mainloop()
