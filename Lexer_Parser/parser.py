tokens = ['PATTERN', 'NUMBER', 'COLON', 'INSTRUMENT', 'INSTRUMENT_SOUND', 'MAIN', 'PLAY', 'LOOP']


VALID_SOUNDS = {
    "Drum": {"boom", "clap", "tsst", "crash", "rest", "dun"},
    "Triangle": {"ding", "diding", "dididing"}
}


def parse_program(lines):
    i = 0
    defined_patterns = set()
    i = parse_pattern_block(lines, i,defined_patterns)
    if i < len(lines) and lines[i] == "MAIN":
        i = parse_main_block(lines, i,defined_patterns)
    if i < len(lines):
        raise SyntaxError(f"Unexpected token at end: {lines[i]}")
    print("✅ DJcode parsed successfully.")

def parse_pattern_block(lines, i,defined_patterns):
    while i < len(lines) and lines[i]!="MAIN":
        i = parse_named_pattern(lines, i,defined_patterns)
    return i

def parse_named_pattern(lines, i,defined_patterns):
    if not lines[i].startswith("PATTERN"):
        raise SyntaxError("Expected PATTERN")
    print(f"{lines[i]}")
    i += 1

    if i >= len(lines) or not lines[i].startswith("NUMBER"):
        raise SyntaxError("Expected NUMBER")
    print(f"{lines[i]}")
    pattern_number = lines[i].split()[1]  # we just passed NUMBER line
    defined_patterns.add(pattern_number)

    i += 1

    if i >= len(lines) or lines[i] != "COLON":
        raise SyntaxError("Expected COLON")
    print(f"{lines[i]}")
    i += 1

    i = parse_instrument_sequence(lines, i)
    return i

def parse_instrument_sequence(lines, i):
    while i < len(lines) and lines[i].startswith("INSTRUMENT"):
        instr_line = lines[i]
        instr_name = instr_line.split()[1]
        i = parse_instrument_sound_group(lines, i, instr_name)
    return i

def parse_instrument_sound_group(lines, i, instr_name):
    print(f"Instrument: {instr_name}")
    if instr_name not in VALID_SOUNDS:
        raise ValueError(f"❌ Error: Unknown instrument '{instr_name}'")
    i += 1

    if i >= len(lines) or not lines[i].startswith("INSTRUMENT_SOUND"):
        raise SyntaxError(f"❌ Error: Instrument '{instr_name}' declared with no sound!")

    while i < len(lines) and lines[i].startswith("INSTRUMENT_SOUND"):
        sound_token = lines[i].split()[1]
        if sound_token not in VALID_SOUNDS[instr_name]:
            raise ValueError(f"❌ '{sound_token}' is not valid for instrument '{instr_name}'")
        print(f"  Sound: {sound_token}")
        i += 1

    return i
def parse_main_block(lines,i,defined_patterns):
    if lines[i]!= "MAIN":
        raise SyntaxError("Expected MAIN")
    print("MAIN")
    i+=1

    if i>= len(lines) or lines[i]!= "COLON":
        raise SyntaxError("Expected COLON after MAIN")
    print ("COLON")
    i+=1
    while i<len(lines):
        if lines[i]!= "PLAY":
            raise SyntaxError("Expected PLAY after COLON")
        print("PLAY")
        i+=1
        if i>= len(lines) or lines[i]!= "PATTERN":
            raise SyntaxError("Expected PATTERN after PLAY")
        print("PATTERN")
        i+=1
        if i>=len(lines) or not lines[i].startswith("NUMBER"):
            raise SyntaxError("Expected NUMBER after PATTERN")
        pattern_number = lines[i].split()[1]
        print(f"NUMBER {pattern_number}")
        if pattern_number not in defined_patterns:
            raise SyntaxError(f"❌ Pattern {pattern_number} used in MAIN but not defined earlier")
        i+=1
        if i>=len(lines) or lines[i]!= "LOOP":
            raise SyntaxError("Expected LOOP")
        print("LOOP")
        i+=1
        if i>=len(lines) or not lines[i].startswith("NUMBER"):
            raise SyntaxError("Expected NUMBER after PATTERN")
        print(f"NUMBER {lines[i].split()[1]}")
        i+=1

    return i    
if __name__ == "__main__":
    with open("tokens.txt") as f:
        lines = [line.strip() for line in f if line.strip()]  # removes blanks
    try:
        parse_program(lines)
    except SyntaxError as e:
        print(f"❌ Syntax error: {e}")