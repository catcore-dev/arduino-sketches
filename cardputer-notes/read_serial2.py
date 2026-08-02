import serial, time, sys

# Try multiple baud rates
baud_rates = [115200, 9600, 57600, 230400]

for baud in baud_rates:
    print(f"\nTrying {baud} baud...")
    try:
        ser = serial.Serial('COM6', baud, timeout=2)
        # Reset device
        ser.dtr = False
        ser.rts = False
        time.sleep(0.2)
        ser.dtr = True
        ser.rts = True
        time.sleep(2)
        ser.reset_input_buffer()
        
        for i in range(5):
            data = ser.read(1024)
            if data:
                print(f"  GOT DATA at {baud}: {repr(data)}")
                ser.close()
                sys.exit(0)
            time.sleep(0.5)
        ser.close()
    except Exception as e:
        print(f"  Error at {baud}: {e}")

print("\nNo data at any baud rate.")
