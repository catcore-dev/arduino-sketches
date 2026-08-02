import serial, time

# Same as pio device monitor --rts 1 --dtr 0
print("Opening COM6...", flush=True)
try:
    ser = serial.Serial()
    ser.port = 'COM6'
    ser.baudrate = 115200
    ser.timeout = 0.2
    ser.dtr = False
    ser.rts = True
    ser.open()
    
    # DON'T clear buffer - read everything
    print("Reading ALL data for 35 seconds...", flush=True)
    print("Press keys on Cardputer now!", flush=True)
    print("="*50, flush=True)
    
    all_data = b''
    start = time.time()
    while time.time() - start < 35:
        data = ser.read(4096)
        if data:
            all_data += data
            # Print immediately
            try:
                print(data.decode('utf-8', errors='replace'), end='', flush=True)
            except:
                print(repr(data), end='', flush=True)
            start = time.time()  # reset idle timer
    
    ser.close()
    
    if not all_data:
        print("\nNo data at all.", flush=True)
    else:
        print(f"\n\nTotal: {len(all_data)} bytes.", flush=True)
        
except Exception as e:
    print(f"Error: {e}", flush=True)
    import traceback
    traceback.print_exc()
