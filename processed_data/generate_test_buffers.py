import os
import csv

def extract_slice(csv_path, num_samples=100):
    values = []
    if not os.path.exists(csv_path):
        print(f"Error: {csv_path} does not exist.")
        return values
        
    with open(csv_path, 'r') as f:
        reader = csv.reader(f)
        # Skip header
        next(reader)
        for i, row in enumerate(reader):
            if i >= num_samples:
                break
            values.append(float(row[1]))
    return values

def main():
    processed_dir = "/home/sankar/github/CAN-IDS/processed_data"
    output_dir = "/home/sankar/github/CAN-IDS/stm32_wokwi"
    os.makedirs(output_dir, exist_ok=True)
    
    normal_vals = extract_slice(os.path.join(processed_dir, "normal_traffic.csv"))
    dos_vals = extract_slice(os.path.join(processed_dir, "dos_attack.csv"))
    fuzzy_vals = extract_slice(os.path.join(processed_dir, "fuzzy_attack.csv"))
    impersonation_vals = extract_slice(os.path.join(processed_dir, "impersonation_attack.csv"))
    
    header_path = os.path.join(output_dir, "test_buffers.h")
    print(f"Generating C++ test buffers in {header_path}...")
    
    with open(header_path, 'w') as f:
        f.write("#ifndef TEST_BUFFERS_H\n")
        f.write("#define TEST_BUFFERS_H\n\n")
        
        f.write(f"// Simulated normal CAN traffic slice (length: {len(normal_vals)})\n")
        f.write(f"const float normal_traffic_buffer[] = {{\n    " + ", ".join(map(str, normal_vals)) + "\n};\n\n")
        
        f.write(f"// Simulated DoS attack traffic slice (length: {len(dos_vals)})\n")
        f.write(f"const float dos_attack_buffer[] = {{\n    " + ", ".join(map(str, dos_vals)) + "\n};\n\n")
        
        f.write(f"// Simulated Fuzzy attack traffic slice (length: {len(fuzzy_vals)})\n")
        f.write(f"const float fuzzy_attack_buffer[] = {{\n    " + ", ".join(map(str, fuzzy_vals)) + "\n};\n\n")
        
        f.write(f"// Simulated Impersonation attack traffic slice (length: {len(impersonation_vals)})\n")
        f.write(f"const float impersonation_attack_buffer[] = {{\n    " + ", ".join(map(str, impersonation_vals)) + "\n};\n\n")
        
        f.write("#endif // TEST_BUFFERS_H\n")
        
    print("Buffers generated successfully.")

if __name__ == "__main__":
    main()
