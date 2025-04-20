# Paths
LEXER_DIR = Lexer_Parser
SOUND_DIR = Sound_Synthesis

# Files
DJCODE_INPUT = $(LEXER_DIR)/test.dj
TOKENS_OUTPUT = $(LEXER_DIR)/tokens.txt

# Default target
all: lexer transform parse soundgen

# Step 1: Compile lexer (lexer.l + main.c)
lexer: $(LEXER_DIR)/lex.yy.c $(LEXER_DIR)/main.c
	gcc -o $(LEXER_DIR)/lexer $(LEXER_DIR)/lex.yy.c $(LEXER_DIR)/main.c

# Step 2: Run lexer on DJcode input file to generate tokens.txt
transform: lexer
	./$(LEXER_DIR)/lexer $(DJCODE_INPUT) > $(TOKENS_OUTPUT)
	cd $(LEXER_DIR) && python3 transform_tokens.py
# Step 3: Run semantic parser + .wav generation
parse:
	cd $(LEXER_DIR) && python3 parser.py

# Generate lex.yy.c from lexer.l
$(LEXER_DIR)/lex.yy.c: $(LEXER_DIR)/lexer.l
	cd $(LEXER_DIR) && flex lexer.l

# Build the dj_generator binary
soundgen: transform
	@echo "Building sound generator..."
	$(CC) $(SOUND_DIR)/WAVGenerator.c \
	      $(SOUND_DIR)/tokensParser.c \
	      $(SOUND_DIR)/soundwaves.c \
	      -o $(SOUND_DIR)/dj_generator \
	      -DWAV_GENERATOR_STANDALONE_MAIN -lm -Wall -ansi -Werror -pedantic
	@echo "Running dj_generator..."

	cd $(SOUND_DIR) && ./dj_generator

# Clean build files
clean:
	rm -f $(LEXER_DIR)/lexer $(LEXER_DIR)/lex.yy.c $(TOKENS_OUTPUT) output.wav $(SOUND_DIR)/test_dj_generator
