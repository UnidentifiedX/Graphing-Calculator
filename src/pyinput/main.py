import tkinter as tk
import serial

root = tk.Tk()
root.title("Calculator")

expression = tk.StringVar()

ser = serial.Serial("/dev/tty.usbmodem1301", 115200)

def press(x):
    expression.set(f"Sent key: {x}")

    if (len(x) == 1):
        ser.write(x.encode())
        return
    
    match x:
        case "LEFT":
            ser.write(b'L')
        case "RIGHT":
            ser.write(b'R')
        case "BKSP":
            ser.write(b'B')
        case "CLEAR":
            ser.write(b'C')
        case "UP":
            ser.write(b'U')
        case "DOWN":
            ser.write(b'D')
        case "sin":
            ser.write(b's')
        case "cos":
            ser.write(b'c')
        case "tan":
            ser.write(b't')
        case "log":
            ser.write(b'g')
        case "pi":
            ser.write(b'p')

label = tk.Entry(root, textvariable=expression, font=("Arial", 24))
label.grid(row=0, column=0, columnspan=4)

buttons = [
    "pi", "frac", "sqrt", "pow",
    "sin", "cos", "tan", "log",
    "UP", "DOWN", "(", ")",
    "LEFT", "RIGHT", "BKSP", "CLEAR",
    "7","8","9","/",
    "4","5","6","*",
    "1","2","3","-",
    "0",".","=","+"
]

row = 1
col = 0

for b in buttons:
    tk.Button(root, text=b, width=5, height=2,
              command=lambda x=b: press(x)).grid(row=row, column=col)
    col += 1
    if col > 3:
        col = 0
        row += 1

root.mainloop()