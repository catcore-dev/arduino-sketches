import serial, time, sys

# Method 2: Open port and read immediately, no DTR/RTS manipulation
print("Opening COM6 (no DTR/RTS manipulation)...")
try:
    ser = serial.Serial('COM6', 115200, timeout=0.1)
    print(f"  DTR={ser.dtr}, RTS={ser.rts}")
    time.sleep(0.1)
    
    all_data = b''
    # Quick reads to catch boot message
    for i in range(100):
        data = ser.read(1024)
        if data:
            all_data += data
            print(f"  Read {i}: {repr(data)}")
        time.sleep(0.01)
    
    if all_data:
        print(f"\n=== TOTAL ({len(all_data)} bytes) ===")
        print(all_data.decode('utf-8', errors='replace'))
    else:
        print("\nNo data. Let me try toggling DTR manually...")
        
        # Try with DTR toggling (manual reset)
        ser.dtr = False
        time.sleep(0.3)
        ser.dtr = True
        time.sleep(0.5)
        ser.reset_input_buffer()
        
        for i in range(50):
            data = ser.read(1024)
            if data:
                all_data += data
                print(f"  Post-reset read {i}: {repr(data)}")
            time.sleep(0.02)
        
        if all_data:
            print(f"\n=== TOTAL ({len(all_data)} bytes) ===")
            print(all_data.decode('utf-8', errors='replace'))
        else:
            print("Still no data after manual reset")
    
    ser.close()
except Exception as e:
    print(f"Error: {e}")
    import traceback
    traceback.print_exc()
