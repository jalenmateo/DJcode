# System Patterns: DJcode Compiler

## Architecture Overview
The compiler will follow a traditional pipeline architecture:

```mermaid
graph LR
    A[Input File (.dj/.txt)] --> B(Lexer);
    B --> C(Parser);
    C --> D(Semantic Analyzer);
    D --> E(WAV Generator);
    E --> F[Output File (.wav)];
    G[compiler.c] -- controls --> B;
    G -- controls --> C;
    G -- controls --> D;
    G -- controls --> E;
```

## Components
-   **`lexer.c`**: Reads the input DJcode file and breaks it down into a stream of tokens (e.g., KEYWORD("boom"), KEYWORD("tsss")).
-   **`parser.c`**: Takes the token stream from the lexer and builds an Abstract Syntax Tree (AST) or similar structure representing the sequence of commands.
-   **`semantic.c`**: Analyzes the AST for meaning, potentially validating command sequences or preparing data structures for the generator. (Details TBD - might be simple initially).
-   **`WAVGenerator.c`**: Takes instructions (likely derived from the AST/semantic analysis) and generates the raw audio sample data, writing it to a WAV file with the correct header. This component will contain the logic for synthesizing sounds corresponding to keywords.
-   **`compiler.c`**: The main driver program. It orchestrates the process, calling the lexer, parser, semantic analyzer, and WAV generator in sequence. Handles file I/O and command-line arguments.
-   **`Makefile`**: Defines build rules for compiling the project components.
-   **`README.md`**: Project documentation.
-   **`Log.txt` (Optional)**: For logging errors or debugging information.

## Key Technical Decisions (Initial)
-   **Language:** C (as requested).
-   **Build System:** Make.
-   **Audio Format:** Standard WAV (PCM, 16-bit mono initially, based on example). Sample rate and other parameters TBD, but the example uses 16000 Hz.
-   **Sound Generation:** Will start with basic synthesis adapted from the provided example, potentially evolving to include noise generation or simple sample playback for keywords like "boom", "tsss".
