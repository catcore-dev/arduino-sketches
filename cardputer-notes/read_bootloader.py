import serial, time, sys

# Try to read bootloader output from ROM bootloader
print("Opening COM6 to capture bootloader output...")
try:
    ser = serial.Serial()
    ser.port = 'COM6'
    ser.baudrate = 115200
    ser.timeout = 0.5
    
    # Open with DTR/RTS low to hold the chip in reset
    ser.dtr = False
    ser.rts = False
    ser.open()
    print("  Port opened, holding reset...")
    time.sleep(0.5)
    
    # Flush any stale data
    ser.reset_input_buffer()
    
    # Release reset
    print("  Releasing reset (DTR/RTS high)...")
    ser.dtr = True
    ser.rts = True
    time.sleep(0.1)
    
    # Read bootloader output
    all_data = b''
    for i in range(20):
        data = ser.read(1024)
        if data:
            all_data += data
            print(f"  Read {i}: {repr(data)}")
        time.sleep(0.1)
    
    if all_data:
        print(f"\n=== TOTAL ({len(all_data)} bytes) ===")
        print(repr(all_data))
        try:
            print(all_data.decode('utf-8', errors='replace'))
        except:
            pass
    else:
        print("\nNo bootloader output detected")
    
    ser.close()
except Exception as e:
    print(f"Error: {e}")
