import sys
import os
import json

# TASM v1.0 Opcodes
OPCODES = {
    'PUSH':    0x01,
    'MUI_SET': 0x02,
    'WAIT':    0x03,
    'JMP':     0x04,
    'SMOOTH':  0x05,
    'MUI_GET': 0x06,
    'SUB':     0x07,
    'JMP_POS': 0x08
}

ARG_SIZES = {
    'PUSH':    2,
    'MUI_SET': 1,
    'WAIT':    0,
    'JMP':     2,
    'SMOOTH':  1,
    'MUI_GET': 1,
    'SUB':     0,
    'JMP_POS': 2
}

def parse_line(line):
    # Remove comments
    if ';' in line:
        line = line.split(';')[0]
    line = line.strip()
    return line

def assemble(input_file, output_file, debug_file="debug_map.json"):
    lines = []
    try:
        with open(input_file, 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: File {input_file} not found.")
        sys.exit(1)

    labels = {}
    current_addr = 0
    clean_lines_data = [] # Stores (line_content, original_line_index)

    # --- PASS 1: Calculate Addresses & Map Labels ---
    print("--- TASM: Pass 1 (Mapping Labels) ---")
    
    # Pre-process lines to map source lines to current addresses
    # We need to preserve the relationship: Program Address -> Source Line Index
    
    debug_source_lines = [l.rstrip() for l in lines]
    address_to_line_map = {}

    for idx, raw_line in enumerate(lines):
        line = parse_line(raw_line)
        
        # If line is empty or just a comment or label, it doesn't take address space
        # But a label IS an address marker.
        
        if not line:
            continue

        if line.endswith(':'):
            label = line[:-1]
            labels[label] = current_addr
            print(f"  [L] Label '{label}' mapped at 0x{current_addr:02X}")
            continue

        parts = line.split()
        mnemonic = parts[0].upper()

        if mnemonic not in OPCODES:
            print(f"Error: Unknown opcode '{mnemonic}' at line {idx+1}: {raw_line}")
            sys.exit(1)

        # Record mapping: This address starts at this source line
        address_to_line_map[str(current_addr)] = idx
        
        # Store for Pass 2
        clean_lines_data.append({
            'line': line,
            'addr': current_addr,
            'parts': parts,
            'mnemonic': mnemonic
        })

        # Advance Address
        current_addr += 1 + ARG_SIZES[mnemonic]

    print(f"  Total Program Size: {current_addr} bytes")

    # --- PASS 2: Generate Machine Code ---
    print("--- TASM: Pass 2 (Generating Hex) ---")
    hex_output = []
    
    for item in clean_lines_data:
        parts = item['parts']
        mnemonic = item['mnemonic']
        opcode = OPCODES[mnemonic]
        
        hex_output.append(f"{opcode:02X}")

        args = parts[1:]
        
        if mnemonic in ['PUSH', 'JMP', 'JMP_POS']:
            if len(args) != 1:
                print(f"Error: {mnemonic} expects 1 argument.")
                sys.exit(1)
            arg = args[0]
            val = 0
            if arg in labels:
                val = labels[arg]
            else:
                try:
                    val = int(arg, 0)
                except ValueError:
                    print(f"Error: Invalid argument '{arg}'")
                    sys.exit(1)
            
            val = val & 0xFFFF
            hex_output.append(f"{(val >> 8) & 0xFF:02X}")
            hex_output.append(f"{val & 0xFF:02X}")
            
        elif mnemonic in ['MUI_SET', 'MUI_GET', 'SMOOTH']:
            if len(args) != 1:
                print(f"Error: {mnemonic} expects 1 argument.")
                sys.exit(1)
            try:
                val = int(args[0], 0)
            except ValueError:
                print(f"Error: Invalid arg '{args[0]}'")
                sys.exit(1)
            
            val = val & 0xFF
            hex_output.append(f"{val:02X}")

    # --- Write Outputs ---
    with open(output_file, 'w') as f:
        for byte in hex_output:
            f.write(byte + "\n")
            
    # Write Debug Map
    debug_data = {
        "source_code": debug_source_lines,
        "address_map": address_to_line_map
    }
    
    with open(debug_file, 'w') as f:
        json.dump(debug_data, f, indent=2)

    print(f"--- Success! Hex: {output_file}, Debug Map: {debug_file} ---")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python tasm.py <input.tasm> [output.hex]")
        sys.exit(1)
    
    input_f = sys.argv[1]
    output_f = "firmware.hex"
    if len(sys.argv) > 2:
        output_f = sys.argv[2]
        
    assemble(input_f, output_f)
