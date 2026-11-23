import tkinter as tk
import serial

root = tk.Tk()
root.title("Calculator")

expression = tk.StringVar()

port = "/dev/tty.usbmodem11301"
ser = serial.Serial(port, 115200)

def write_to_serial(x):
    if len(x) == 1:
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
        case "sqrt":
            ser.write(b'q')
        case "frac":
            ser.write(b'f')
        case "ln":
            ser.write(b'l')
        case "abs":
            ser.write(b'a')
        case "nrt":
            ser.write(b'n')
        case "pow":
            ser.write(b'w')
        case "quit":
            ser.write(b'Q')
        case "graph":
            ser.write(b'G')
        case "mode":
            ser.write(b'M')

def press(x):
    global ser

    expression.set(f"Sent key: {x}")

    try:
        write_to_serial(x)
    except Exception as e: # when the port is not available, try to reconnect
        ser = serial.Serial(port, 115200)
        write_to_serial(x)

label = tk.Entry(
    root,
    textvariable=expression,
    font=("Arial", 24)
)
label.grid(row=0, column=0, columnspan=4)

buttons = [
    "quit", "graph", "x", "mode",
    "e", "ln", "nrt", "abs",
    "pi", "frac", "sqrt", "pow",
    "sin", "cos", "tan", "log",
    "UP", "DOWN", "(", ")",
    "LEFT", "RIGHT", "BKSP", "CLEAR",
    "7", "8", "9", "/",
    "4", "5", "6", "*",
    "1", "2", "3", "-",
    "0", ".", "=", "+"
]

row = 1
col = 0

for b in buttons:
    tk.Button(
        root,
        text=b,
        width=5,
        height=2,
        command=lambda x=b: press(x)
    ).grid(row=row, column=col)

    col += 1
    if col > 3:
        col = 0
        row += 1

# Position window in the bottom-right corner
root.update_idletasks()

window_width = root.winfo_width()
window_height = root.winfo_height()

screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()

margin = 20

x = screen_width - window_width - margin
y = screen_height - window_height - margin

root.geometry(f"+{x}+{y}")

# Bring window to the front
root.lift()
root.focus_force()
root.attributes("-topmost", True)
root.after(100, lambda: root.attributes("-topmost", False))

root.mainloop()