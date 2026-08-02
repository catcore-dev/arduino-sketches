import serial, time

# Same as pio device monitor --rts 1 --dtr 0:
# DTR=inactive=HIGH=False, RTS=active=LOW=True
print("Opening COM6 (DTR=False=HIGH, RTS=True=LOW)...", flush=True)
try:
    ser = serial.Serial()
    ser.port = 'COM6'
    ser.baudrate = 115200
    ser.timeout = 0.3
    ser.dtr = False  # OFF = HIGH = EN HIGH = chip running
    ser.rts = True   # ON = LOW = doesn't matter
    ser.open()
    time.sleep(0.3)
    ser.reset_input_buffer()
    
    print("Listening 30 seconds. Press keys NOW!", flush=True)
    print("="*50, flush=True)
    
    start = time.time()
    while time.time() - start < 30:
        data = ser.read(4096)
        if data:
            print(data.decode('utf-8', errors='replace'), end='', flush=True)
            start = time.time()
    
    ser.close()
    print("\nDone.", flush=True)
except Exception as e:
    print(f"Error: {e}", flush=True)
