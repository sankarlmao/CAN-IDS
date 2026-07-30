import os
import re
import csv

def convert_dataset(input_paths, output_path):
    names = ", ".join([os.path.basename(p) for p in input_paths])
    print(f"Converting [{names}] to {os.path.basename(output_path)}...")
    
    first_timestamp = None
    count = 0
    
    with open(output_path, 'w', newline='') as outfile:
        writer = csv.writer(outfile)
        # Write header
        writer.writerow(['timestamp', 'data_value'])
        
        for input_path in input_paths:
            with open(input_path, 'r') as infile:
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
        "normal_traffic.csv": ["Attack_free_dataset_part1.txt", "Attack_free_dataset_part2.txt"],
        "dos_attack.csv": ["DoS_attack_dataset_part1.txt", "DoS_attack_dataset_part2.txt"],
        "fuzzy_attack.csv": ["Fuzzy_attack_dataset_part1.txt", "Fuzzy_attack_dataset_part2.txt"],
        "impersonation_attack.csv": ["Impersonation_attack_dataset_part1.txt", "Impersonation_attack_dataset_part2.txt"]
    }
    
    for outfile, infiles in files_to_convert.items():
        outpath = os.path.join(output_dir, outfile)
        inpaths = [os.path.join(dataset_dir, f) for f in infiles]
        
        # Verify all input paths exist
        all_exist = True
        for p in inpaths:
            if not os.path.exists(p):
                print(f"Error: {p} does not exist.")
                all_exist = False
                break
                
        if all_exist:
            convert_dataset(inpaths, outpath)
