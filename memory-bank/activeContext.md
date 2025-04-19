# Active Context: DJcode Compiler (Initialization)

## Current Focus
-   Implementing sound synthesis for DJcode keywords.
-   Testing and refining the generated waveforms.

## Recent Changes
-   Created `soundwaves.h` and `soundwaves.c` for dedicated sound synthesis.
-   Defined constants for sample rate (44100 Hz), bit depth (16-bit), and frequencies.
-   Implemented synthesis functions for all required sounds:
    * DRUM sounds: BOOM, TSST, CLAP, CRASH, REST, DUN (formerly FLOORTOM)
    * TRIANGLE sounds: DING, DIDING, DIDIDING
-   Updated `WAVGenerator.c` with new test pattern to demonstrate all sounds.
-   Updated system patterns documentation to reflect new components.

## Next Steps
1.  Test and refine the sound synthesis (may need to adjust frequencies, envelopes, or mixing).
2.  Define the DJcode language syntax and keywords more formally.
3.  Start implementing the Lexer (`lexer.c`).
4.  Create the `Makefile`.

## Active Decisions & Considerations
-   Using raw waveform synthesis with envelopes for distinct sounds:
    * Sine waves with fast decay for percussion (BOOM)
    * White noise with filtering for high-frequency sounds (TSST, CLAP)
    * Triangle waves for bell-like tones (DING)
    * Compound patterns for DIDING/DIDIDING using timing divisions
-   Sample rate increased to 44100 Hz for better high-frequency reproduction
-   Amplitude scaling and mixing carefully managed to prevent clipping
