# Detect OS
ifeq ($(OS),Windows_NT)
    EXEC_SUFFIX := .exe
    SLASH := \\
    PYTHON := python
    DEL := del /Q
else
    EXEC_SUFFIX :=
    SLASH := /
    PYTHON := python3
    DEL := rm -f
endif

# Paths
LEXER_DIR := Lexer_Parser
SOUND_DIR := Sound_Synthesis

# Files
LEXER := $(LEXER_DIR)$(SLASH)lexer$(EXEC_SUFFIX)
DJCODE_INPUT := $(LEXER_DIR)$(SLASH)test.dj
TOKENS_OUTPUT := $(LEXER_DIR)$(SLASH)tokens.txt
GENERATOR := dj_generator$(EXEC_SUFFIX)

# Default target
all: lexer transform parse soundgen

# Step 1: Generate lex.yy.c from lexer.l
$(LEXER_DIR)$(SLASH)lex.yy.c: $(LEXER_DIR)$(SLASH)lexer.l
	cd $(LEXER_DIR) && flex lexer.l

# Step 2: Compile lexer (lexer.l + main.c)
lexer: $(LEXER_DIR)$(SLASH)lex.yy.c $(LEXER_DIR)$(SLASH)main.c
	gcc -o $(LEXER) $(LEXER_DIR)$(SLASH)lex.yy.c $(LEXER_DIR)$(SLASH)main.c -Wall -ansi -Werror -pedantic

# Step 3: Run lexer to generate tokens.txt
transform: lexer
	$(LEXER) $(DJCODE_INPUT) > $(TOKENS_OUTPUT)
	cd $(LEXER_DIR) && $(PYTHON) transform_tokens.py

# Step 4: Run parser
parse:
	cd $(LEXER_DIR) && $(PYTHON) parser.py

# Step 5: Compile and run the sound generator
soundgen:
	@echo "Building sound generator..."
	gcc $(SOUND_DIR)$(SLASH)WAVGenerator.c \
		$(SOUND_DIR)$(SLASH)tokensParser.c \
		$(SOUND_DIR)$(SLASH)soundwaves.c \
		-o $(SOUND_DIR)$(SLASH)$(GENERATOR) \
		-DWAV_GENERATOR_STANDALONE_MAIN -lm -Wall -ansi -Werror -pedantic
	@echo "Running sound generator..."
ifeq ($(OS),Windows_NT)
	cmd /C "cd $(SOUND_DIR) && $(GENERATOR)"
else
	cd $(SOUND_DIR) && ./$(GENERATOR)
endif
# Clean generated files
clean:
	$(DEL) $(LEXER) $(LEXER_DIR)$(SLASH)lex.yy.c $(TOKENS_OUTPUT) output.wav $(SOUND_DIR)$(SLASH)$(GENERATOR)