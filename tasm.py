import sys
import os

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

# Instruction Argument Sizes (in bytes, excluding opcode)
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

def assemble(input_file, output_file):
    lines = []
    try:
        with open(input_file, 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: File {input_file} not found.")
        sys.exit(1)

    labels = {}
    current_addr = 0
    clean_lines = []

    # --- PASS 1: Calculate Addresses & Map Labels ---
    print("--- TASM: Pass 1 (Mapping Labels) ---")
    for line in lines:
        line = parse_line(line)
        if not line:
            continue

        # Check for Label Definition (e.g., "START:")
        if line.endswith(':'):
            label = line[:-1]
            labels[label] = current_addr
            print(f"  [L] Label '{label}' mapped at 0x{current_addr:02X}")
            continue

        parts = line.split()
        mnemonic = parts[0].upper()

        if mnemonic not in OPCODES:
            print(f"Error: Unknown opcode '{mnemonic}' at line: {line}")
            sys.exit(1)

        # Increment address
        # Opcode (1 byte) + Args size
        current_addr += 1 + ARG_SIZES[mnemonic]
        clean_lines.append(line)

    print(f"  Total Program Size: {current_addr} bytes")

    # --- PASS 2: Generate Machine Code ---
    print("--- TASM: Pass 2 (Generating Hex) ---")
    hex_output = []
    current_addr = 0
    
    for line in clean_lines:
        parts = line.split()
        mnemonic = parts[0].upper()
        opcode = OPCODES[mnemonic]
        
        # Add Opcode Byte
        hex_output.append(f"{opcode:02X}")
        current_addr += 1

        args = parts[1:]
        
        if mnemonic in ['PUSH', 'JMP', 'JMP_POS']:
            # Expecting 16-bit Argument (Number or Label)
            if len(args) != 1:
                print(f"Error: {mnemonic} expects 1 argument.")
                sys.exit(1)
            
            arg = args[0]
            val = 0
            
            # Check if it is a Label
            if arg in labels:
                val = labels[arg]
            else:
                try:
                    # Parse number (handle hex 0x or decimal)
                    val = int(arg, 0)
                except ValueError:
                    print(f"Error: Invalid argument '{arg}' for {mnemonic}")
                    sys.exit(1)
            
            # Format 16-bit (High byte, Low byte)
            val = val & 0xFFFF # Mask to 16 bits
            high_byte = (val >> 8) & 0xFF
            low_byte = val & 0xFF
            hex_output.append(f"{high_byte:02X}")
            hex_output.append(f"{low_byte:02X}")
            current_addr += 2
            
        elif mnemonic in ['MUI_SET', 'MUI_GET', 'SMOOTH']:
            # Expecting 8-bit Argument
            if len(args) != 1:
                print(f"Error: {mnemonic} expects 1 argument.")
                sys.exit(1)
            
            try:
                val = int(args[0], 0)
            except ValueError:
                print(f"Error: Invalid argument '{args[0]}' for {mnemonic}")
                sys.exit(1)
                
            val = val & 0xFF
            hex_output.append(f"{val:02X}")
            current_addr += 1
            
        elif mnemonic in ['WAIT', 'SUB']:
            # No Arguments
            pass

    # --- Write Output ---
    with open(output_file, 'w') as f:
        # Write bytes separated by newlines or spaces
        # Verilog $readmemh accepts whitespace separated
        for byte in hex_output:
            f.write(byte + "\n")
            
    print(f"--- Success! Output written to {output_file} ---")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python tasm.py <input.tasm> [output.hex]")
        sys.exit(1)
    
    input_f = sys.argv[1]
    output_f = "firmware.hex"
    if len(sys.argv) > 2:
        output_f = sys.argv[2]
        
    assemble(input_f, output_f)
