tokens = ['PATTERN', 'NUMBER', 'COLON', 'INSTRUMENT', 'INSTRUMENT_SOUND']


VALID_SOUNDS = {
    "Drum": {"boom", "clap"},
    "Triangle": {"ting"}
}


def parse_program(lines):
    i = 0
    i = parse_pattern_block(lines, i)
    if i < len(lines):
        raise SyntaxError(f"Unexpected token at end: {lines[i]}")
    print("✅ DJcode parsed successfully.")

def parse_pattern_block(lines, i):
    while i < len(lines):
        i = parse_named_pattern(lines, i)
    return i

def parse_named_pattern(lines, i):
    if not lines[i].startswith("PATTERN"):
        raise SyntaxError("Expected PATTERN")
    print(f"{lines[i]}")
    i += 1

    if i >= len(lines) or not lines[i].startswith("NUMBER"):
        raise SyntaxError("Expected NUMBER")
    print(f"{lines[i]}")
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

if __name__ == "__main__":
    with open("tokens.txt") as f:
        lines = [line.strip() for line in f if line.strip()]  # removes blanks
    try:
        parse_program(lines)
    except SyntaxError as e:
        print(f"❌ Syntax error: {e}")