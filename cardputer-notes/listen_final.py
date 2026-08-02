import serial, time, sys

# Open with DTR=False, RTS=False = both OFF/HIGH = chip running
print("Opening COM6 (DTR=OFF, RTS=OFF = chip running)...", flush=True)
try:
    ser = serial.Serial()
    ser.port = 'COM6'
    ser.baudrate = 115200
    ser.timeout = 0.3
    ser.dtr = False
    ser.rts = False
    ser.open()
    
    # Flush any stale boot data
    time.sleep(0.5)
    ser.reset_input_buffer()
    
    print("Listening for 30 seconds. Press keys on Cardputer NOW!", flush=True)
    print("=" * 50, flush=True)
    
    all_output = []
    start = time.time()
    while time.time() - start < 30:
        data = ser.read(4096)
        if data:
            text = data.decode('utf-8', errors='replace')
            print(text, end='', flush=True)
            all_output.append(text)
            start = time.time()  # extend on data
    
    ser.close()
    
    if all_output:
        print("\n" + "=" * 50, flush=True)
        print("CAPTURED OUTPUT:", flush=True)
        print("".join(all_output), flush=True)
    else:
        print("\nNo data received at all.", flush=True)
    
except Exception as e:
    print(f"Error: {e}", flush=True)
    import traceback
    traceback.print_exc()
