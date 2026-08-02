import serial, time, sys

print("Opening COM6, listening for keypresses...", flush=True)
print("Press keys on Cardputer now!", flush=True)

try:
    ser = serial.Serial('COM6', 115200, timeout=0.5)
    # Don't toggle DTR/RTS - device is already running
    time.sleep(0.5)
    ser.reset_input_buffer()
    
    start = time.time()
    while time.time() - start < 20:  # listen for 20 seconds
        data = ser.read(4096)
        if data:
            text = data.decode('utf-8', errors='replace')
            print(text, end='', flush=True)
            start = time.time()  # reset timeout on any data
    ser.close()
    print("\n\nDone listening.", flush=True)
except Exception as e:
    print(f"Error: {e}", flush=True)
