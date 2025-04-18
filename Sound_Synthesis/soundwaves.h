#ifndef SOUNDWAVES_H
#define SOUNDWAVES_H

#include <stdint.h>
#include <stddef.h>

// Audio configuration constants
#define SAMPLE_RATE 44100
#define BIT_DEPTH 16
#define MAX_AMPLITUDE 30000

// Musical timing constants
#define BEATS_PER_MEASURE 4
#define MS_PER_BEAT 500
#define SAMPLES_PER_BEAT ((SAMPLE_RATE * MS_PER_BEAT) / 1000)

// Musical note frequencies
#define D2 73.42f
#define G2 98.00f
#define A2 110.00f
#define B2 123.47f
#define D3 146.83f
#define G3 196.00f
#define A3 220.00f
#define B3 246.94f
#define Cs4 277.18f
#define D4 293.66f
#define E4 329.63f
#define Fs4 369.99f
#define G4 392.00f
#define A4 440.00f
#define B4 493.88f

// Frequency constants for various sounds. Feel free to change by ear haha.
#define BOOM_FREQ 70.0f      // Low kick drum frequency
#define TSST_FREQ 7000.0f    // High-hat frequency center
#define CLAP_FREQ 2500.0f    // Hand clap frequency center
#define DING_FREQ 900.0f     // Triangle bell frequency

// Function declarations for drum sounds
void generate_boom(int16_t *buffer, int num_samples, float frequency);
void generate_tsst(int16_t *buffer, int num_samples, float frequency);
void generate_clap(int16_t *buffer, int num_samples, float frequency);
void generate_crash(int16_t *buffer, int num_samples);
void generate_rest(int16_t *buffer, int num_samples);
void generate_floortom(int16_t *buffer, int num_samples, float base_freq);

// Function declarations for triangle sounds
void generate_ding(int16_t *buffer, int num_samples, float frequency);
void generate_diding(int16_t *buffer, int num_samples, float frequency);
void generate_dididing(int16_t *buffer, int num_samples, float frequency);

// Function declarations for musical chord progression. Added these so we can play around with sounds other than drum and triangle. See if we want to use/mix these.
void play(int16_t *buffer, size_t buffer_size, float freq, float duration, int measure, float beat);
void DM(int16_t *buffer, size_t buffer_size, float duration, int measure, float beat);
void AM1st(int16_t *buffer, size_t buffer_size, float duration, int measure, float beat);
void Bm1st(int16_t *buffer, size_t buffer_size, float duration, int measure, float beat);
void GM2nd(int16_t *buffer, size_t buffer_size, float duration, int measure, float beat);

#endif // SOUNDWAVES_H
