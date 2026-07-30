import os
import re
import csv

def convert_dataset(input_path, output_path):
    print(f"Converting {os.path.basename(input_path)} to {os.path.basename(output_path)}...")
    
    first_timestamp = None
    count = 0
    
    with open(input_path, 'r') as infile, open(output_path, 'w', newline='') as outfile:
        writer = csv.writer(outfile)
        # Write header
        writer.writerow(['timestamp', 'data_value'])
        
        for line in infile:
            # Match line: Timestamp:          0.002167        ID: 0260    000    DLC: 8    05 20 00 30 ff 93 5f 35
            match = re.search(r"Timestamp:\s+([0-9.]+)\s+ID:\s+0260\s+\d+\s+DLC:\s+(\d+)\s+([0-9a-fA-F\s]+)", line)
            if match:
                ts = float(match.group(1))
                dlc = int(match.group(2))
                data_str = match.group(3).strip()
                data_bytes = data_str.split()
                
                # We need at least 7 bytes (index 6)
                if len(data_bytes) >= 7:
                    if first_timestamp is None:
                        first_timestamp = ts
                    
                    # Convert timestamp to milliseconds relative to start
                    rel_ms = int(round((ts - first_timestamp) * 1000))
                    # Extract 7th byte (index 6)
                    val = int(data_bytes[6], 16)
                    
                    writer.writerow([rel_ms, val])
                    count += 1
                    
    print(f"Finished. Extracted {count} rows.")

if __name__ == "__main__":
    dataset_dir = "/home/sankar/github/CAN-IDS/10) CAN-Intrusion Dataset"
    output_dir = "/home/sankar/github/CAN-IDS/processed_data"
    os.makedirs(output_dir, exist_ok=True)
    
    files_to_convert = {
        "Attack_free_dataset.txt": "normal_traffic.csv",
        "DoS_attack_dataset.txt": "dos_attack.csv",
        "Fuzzy_attack_dataset.txt": "fuzzy_attack.csv",
        "Impersonation_attack_dataset.txt": "impersonation_attack.csv"
    }
    
    for infile, outfile in files_to_convert.items():
        inpath = os.path.join(dataset_dir, infile)
        outpath = os.path.join(output_dir, outfile)
        if os.path.exists(inpath):
            convert_dataset(inpath, outpath)
        else:
            print(f"Error: {inpath} does not exist.")
