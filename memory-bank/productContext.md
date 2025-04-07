# Product Context: DJcode Compiler

## Problem Solved
Creating simple beat samples or musical sequences often requires specialized software (DAWs) or complex programming libraries. DJcode aims to simplify this process by allowing users to describe sounds and rhythms using intuitive, natural-language-like keywords.

## How It Should Work
1.  A user writes a sequence of commands in a DJcode file (e.g., `boom clap tsss boom`).
2.  The user runs the DJcode compiler, providing the input file.
3.  The compiler parses the DJcode, interprets the keywords (like "boom", "tsss"), and translates them into corresponding audio data (e.g., a bass drum sound for "boom", a hi-hat for "tsss").
4.  The compiler generates a standard WAV file containing the resulting audio sequence.

## User Experience Goals
-   **Simplicity:** Easy to learn and use, even for those without extensive programming or music production experience.
-   **Expressiveness:** Allow for basic rhythmic patterns and sound combinations using keywords.
-   **Direct Output:** Generate a usable WAV file directly from the code.
