import serial
from PIL import Image, ImageTk
import tkinter as tk

# --- CONFIG ---
PORT = "/dev/ttyACM0"        # change to your serial port (Linux ex: "/dev/ttyUSB0")
BAUD = 921600        # match your Arduino baud rate
WIDTH = 128          # your OLED width
HEIGHT = 128          # your OLED height
SCALE = 7              # how much to scale pixels in the viewer
OFFSET = 0  # pixels
# ---------------

def read_frame(ser):
    """
    Reads one ASCII frame (buffer dump) from serial and returns it as a 2D list of 0/1 pixels.
    """
    pixels = []
    while True:
        line = ser.readline().decode(errors="ignore").strip()
        if not line:
            break
        row = [1 if c == "#" else 0 for c in line]
        row = row[-OFFSET:] + row[:-OFFSET]  # rotate right by OFFSET

        # --- FIX: make sure row is exactly WIDTH long ---
        if len(row) < WIDTH:
            row += [0] * (WIDTH - len(row))   # pad missing pixels
        elif len(row) > WIDTH:
            row = row[:WIDTH]  # trim extra pixels

        pixels.append(row)

    # --- FIX: make sure we always have HEIGHT rows ---
    while len(pixels) < HEIGHT:
        pixels.append([0] * WIDTH)
    print(f"Got frame: {len(pixels)}x{len(pixels[0])}")
    return pixels


def pixels_to_image(pixels):
    h = len(pixels)
    w = len(pixels[0]) if h > 0 else 0
    img = Image.new("1", (w, h))
    for y, row in enumerate(pixels):
        for x, val in enumerate(row):
            img.putpixel((x, y), 255 if val else 0)
    return img.resize((w*SCALE, h*SCALE), Image.NEAREST)

def update():
    global imgtk
    pixels = read_frame(ser)
    print("Frame sample:", "".join("#" if p else "." for p in pixels[0][:32]))
    if pixels:
        img = pixels_to_image(pixels)
        imgtk = ImageTk.PhotoImage(img)
        canvas.create_image(0, 0, anchor="nw", image=imgtk)
    root.after(50, update)  # schedule next update

# --- Serial setup ---
ser = serial.Serial(PORT, BAUD, timeout=1)

# --- Tkinter GUI ---
root = tk.Tk()
root.title("OLED Viewer")
canvas = tk.Canvas(root, width=WIDTH*SCALE, height=HEIGHT*SCALE, bg="black")
canvas.pack()

update()
root.mainloop()
