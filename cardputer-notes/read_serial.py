import serial, time, sys

try:
    ser = serial.Serial('COM6', 115200, timeout=1)
    print('Serial COM6 opened, waiting 8s for boot messages...', flush=True)
    all_data = b''
    for i in range(16):
        data = ser.read(1024)
        if data:
            all_data += data
            print(f'Read {i+1}: {repr(data)}', flush=True)
        time.sleep(0.5)
    if all_data:
        print(f'Total received: {repr(all_data)}', flush=True)
        print(f'Decoded: {all_data.decode("utf-8", errors="replace")}', flush=True)
    else:
        print('No data received in 8 seconds', flush=True)
    ser.close()
except Exception as e:
    print(f'Error: {e}', flush=True)
    sys.exit(1)
