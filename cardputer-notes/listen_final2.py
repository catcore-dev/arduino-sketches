import serial, time, sys

print("Opening COM6 (DTR=OFF, RTS=OFF)...", flush=True)
try:
    ser = serial.Serial()
    ser.port = 'COM6'
    ser.baudrate = 115200
    ser.timeout = 0.2
    ser.dtr = False
    ser.rts = False
    ser.open()
    
    # DON'T clear buffer - read everything including boot data
    print("Waiting for boot + keypresses (45 seconds)...", flush=True)
    print("Press keys on Cardputer NOW!", flush=True)
    print("=" * 50, flush=True)
    
    all_output = []
    start = time.time()
    while time.time() - start < 45:
        data = ser.read(4096)
        if data:
            text = data.decode('utf-8', errors='replace')
            print(text, end='', flush=True)
            all_output.append(text)
            start = time.time()  # extend on data
    
    ser.close()
    
    if all_output:
        print("\n" + "=" * 50, flush=True)
        print("TOTAL CAPTURED:", flush=True)
        print("".join(all_output), flush=True)
    else:
        print("\nNo data received.", flush=True)
    
except Exception as e:
    print(f"Error: {e}", flush=True)
