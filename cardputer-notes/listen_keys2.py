import serial, time, sys

print("Opening COM6 with reset control...", flush=True)

try:
    ser = serial.Serial()
    ser.port = 'COM6'
    ser.baudrate = 115200
    ser.timeout = 0.5
    
    # Open with DTR low (reset held)
    ser.dtr = False
    ser.rts = False
    ser.open()
    time.sleep(0.3)
    
    # Release reset
    ser.dtr = True
    ser.rts = True
    time.sleep(0.5)
    ser.reset_input_buffer()
    
    print("Device booting... waiting for bootloader + firmware", flush=True)
    time.sleep(2)
    
    # Read any boot/firmware messages
    initial = ser.read(4096)
    if initial:
        print(f"[BOOT]: {initial.decode('utf-8', errors='replace')}", flush=True)
    
    print("\nReady! Press keys on Cardputer now (listening 30s)...", flush=True)
    
    start = time.time()
    while time.time() - start < 30:
        data = ser.read(4096)
        if data:
            text = data.decode('utf-8', errors='replace')
            print(text, end='', flush=True)
            start = time.time()  # reset timeout on data
    
    ser.close()
    print("\n\nDone.", flush=True)
except Exception as e:
    print(f"Error: {e}", flush=True)
