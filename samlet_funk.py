import serial
import time
import os
import re

def parse_signal(raw_data):
    raw_values = re.findall(r'[-+]?\d+', raw_data)
    values = [int(v) for v in raw_values]  

    signal_pairs = list(zip(values[::2], values[1::2]))

    binary_data = ""
    for pulse, space in signal_pairs:
        if abs(space) > 2000:
            if abs(pulse) > 2000:
                binary_data += " header "
            else:
                binary_data += " mid "
        elif 500 <= abs(pulse) <= 850:
            if abs(space) > 900:
                binary_data += "1"
            elif abs(space) <= 900:
                binary_data += "0"

    binary_data = binary_data.strip()

    hex_data = []
    current_group = ""
    for item in binary_data.split():
        if item in {"header", "mid"}:
            if current_group:
                padding_length = (4 - len(current_group) % 4) % 4
                current_group = current_group + '0' * padding_length
                hex_group = "".join(f"{int(current_group[i:i+4], 2):X}" for i in range(0, len(current_group), 4))
                hex_data.append(hex_group)
                current_group = ""
            hex_data.append(item)
        else:
            current_group += item

    if current_group:
        padding_length = (4 - len(current_group) % 4) % 4
        current_group = current_group + '0' * padding_length
        hex_group = "".join(f"{int(current_group[i:i+4], 2):X}" for i in range(0, len(current_group), 4))
        hex_data.append(hex_group)

    hex_output = " ".join(hex_data) 
    return binary_data, hex_output


def log_and_translate_data():
    default_folder_name = f"data_logs_{time.strftime('%Y%m%d_%H%M%S')}"
    user_folder_name = input("Indtast mappenavn: ").strip()

    folder_name = user_folder_name if user_folder_name else default_folder_name
    os.makedirs(folder_name, exist_ok=True)

    port = "COM5"  
    baudrate = 115200  
    ser = serial.Serial(port, baudrate, timeout=0.1)

    buffer = ""
    last_received_time = time.time()
    timeout = 1

    capturing = False 
    raw_data = "" 

    try:
        while True:
            data = ser.readline().decode(errors='ignore').strip()
            if data:
                buffer += data + "\n"
                last_received_time = time.time()

                if not capturing:
                    if data.startswith("+"):
                        parts = data.split(",")
                        if len(parts) >= 1:
                            pulse_value = parts[0].replace("+", "").strip()
                            if pulse_value.isdigit() and int(pulse_value) > 3000:
                                capturing = True 
                                raw_data = data + "\n" 
                else:
                    if data.startswith(("+", "-")):
                        raw_data += data + "\n"
                    else:
                        capturing = False  


            if buffer and (time.time() - last_received_time > timeout):
                timestamp = time.strftime("%Y%m%d_%H%M%S")
                user_filename = input("Indtast filnavn: ").strip()
                filename = f"{user_filename}.txt" if user_filename else f"output_{timestamp}.txt"
                filepath = os.path.join(folder_name, filename)  

                with open(filepath, "w") as file:
                    file.write(buffer)
                
                print(f"Data gemt i {filepath}")

                if raw_data:
                    binary_data, hex_output = parse_signal(raw_data)
                    translation_filename = f"translated_{filename}"
                    translation_filepath = os.path.join(folder_name, translation_filename)
                    
                    with open(translation_filepath, "w") as trans_file:
                        trans_file.write(f"Binary Data: {binary_data}\n")
                        trans_file.write(f"Hexadecimal: {hex_output}\n")
                    
                    print(f"Oversat data gemt i {translation_filepath}")

                buffer = ""
                raw_data = ""
                capturing = False

    except KeyboardInterrupt:
        print("Stopper programmet...")
    finally:
        ser.close()


if __name__ == "__main__":
    log_and_translate_data()
