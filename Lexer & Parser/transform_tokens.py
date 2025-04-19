input_file = "tokens.txt"
output_file = "formatted_patterns.txt"

with open(input_file, "r") as f:
    lines = [line.strip() for line in f if line.strip()]  # strip and skip empty lines

output = []
i = 0
current_pattern = None
sounds = []
in_main = False

while i < len(lines):
    tokens = lines[i].split()

    if tokens[0] == "MAIN":
        # Flush any ongoing pattern block
        if current_pattern:
            output.append(current_pattern)
            for s in sounds:
                output.append(s)
            output.append("END\n")
            current_pattern = None
            sounds = []
        #output.append("MAIN")
        in_main = True
        i += 1
        continue

    if in_main and tokens[0] == "PLAY":
        # Expect PATTERN and NUMBER and LOOP in the next 2 lines
        if i + 2 < len(lines) and lines[i + 1].startswith("PATTERN") and lines[i + 2].startswith("NUMBER") and lines[i+3].startswith("LOOP"):
            pattern_number = lines[i + 2].split()[1]
            loop_number = lines[i+4].split()[1]
            output.append(f"PLAY pattern{pattern_number} LOOP {loop_number}")
            i += 3
            continue
    if not in_main and tokens[0] == "PATTERN":
        if i + 1 < len(lines) and lines[i + 1].startswith("NUMBER"):
            # Extract pattern number from next line
            pattern_number = lines[i + 1].split()[1]
            # If we already had a pattern, save its sounds
            if current_pattern:
                output.append(current_pattern)
                for s in sounds:
                    output.append(f"{s}")
                output.append("END\n")  # append END with an empty line
                sounds = []

            current_pattern = f"PATTERN pattern{pattern_number}"
            i += 2  # skip NUMBER line too
            continue

    if tokens[0] == "INSTRUMENT_SOUND" and len(tokens) > 1:
        sounds.append(tokens[1].upper())

    i += 1


with open(output_file, "w") as f:
    f.write("\n".join(output))

print(f"Formatted patterns written to {output_file}")
