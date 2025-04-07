#include "WAVGenerator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // For M_PI and cos

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Function Implementations ---

void initWavHeader(WavHeader *header, int32_t sample_rate, int16_t bits_per_sample, int16_t num_channels) {
    if (!header) return;

    // RIFF Chunk
    strncpy(header->riff, "RIFF", 4);
    // flength will be calculated later based on data size
    strncpy(header->wave, "WAVE", 4);

    // Format Chunk ("fmt ")
    strncpy(header->fmt, "fmt ", 4);
    header->chunk_size = 16; // Standard size for PCM
    header->format_tag = 1;  // PCM
    header->num_chans = num_channels;
    header->srate = sample_rate;
    header->bits_per_samp = bits_per_sample;
    header->bytes_per_samp = (bits_per_sample / 8) * num_channels;
    header->bytes_per_sec = sample_rate * header->bytes_per_samp;

    // Data Chunk
    strncpy(header->data, "data", 4);
    // dlength will be calculated later based on data size

    // Initialize lengths to 0, they need to be set before writing
    header->flength = 0;
    header->dlength = 0;
}

void addSineWave(short int *buffer, size_t buffer_size, float freq, int duration_ms, int start_time_ms, int32_t sample_rate, int16_t amplitude) {
    if (!buffer || freq <= 0 || duration_ms <= 0 || sample_rate <= 0 || amplitude <= 0) {
        return; // Invalid parameters
    }

    // Calculate start and end indices in samples
    size_t start_sample = (size_t)(((long long)start_time_ms * sample_rate) / 1000);
    size_t duration_samples = (size_t)(((long long)duration_ms * sample_rate) / 1000);
    size_t end_sample = start_sample + duration_samples;

    // Ensure we don't write past the buffer
    if (end_sample > buffer_size) {
        end_sample = buffer_size;
        // Optional: Log a warning or error here if truncation occurs
    }
    if (start_sample >= buffer_size) {
        return; // Start time is beyond the buffer
    }

    for (size_t i = start_sample; i < end_sample; i++) {
        // Calculate time within the note in samples
        size_t sample_in_note = i - start_sample;

        // Calculate percentage through the note for envelope shaping
        float percent_through_note = (float)sample_in_note / duration_samples;
        float amplitude_multiplier = 1.0f;

        // Envelope logic exactly matching original example
        if (percent_through_note < 0.25f)
        {
            amplitude_multiplier = (percent_through_note / 0.25f);
        }
        else if (percent_through_note < 0.5f)
        {
            // Original logic: (1 - ((percent_through_note - 0.25) / 0.25)) * 0.8 + 0.2;
            // Simplified: (1 - (percent_through_note - 0.25) * 4) * 0.8 + 0.2
            // Further simplified: (1 - 4*percent + 1) * 0.8 + 0.2
            // Further simplified: (2 - 4*percent) * 0.8 + 0.2
            // Further simplified: 1.6 - 3.2*percent + 0.2
            // Further simplified: 1.8 - 3.2*percent
            amplitude_multiplier = 1.8f - 3.2f * percent_through_note;
        }
        else
        {
            // Original logic: 1 - percent_through_note;
            amplitude_multiplier = 1.0f - percent_through_note;
        }

        // Calculate the sample value using cosine wave
        // Original used 'i' directly, assuming it represents time steps scaled by sample rate implicitly
        // double time_in_seconds = (double)i / sample_rate; // This is technically correct but differs from original
        // Match original calculation:
        double value = cos((2.0 * M_PI * freq * i) / sample_rate);
        // double value = cos(2.0 * M_PI * freq * time_in_seconds); // Removed duplicate/incorrect line

        // Apply amplitude and envelope, then scale to 16-bit range
        // Add to existing buffer value to allow mixing
        int32_t current_val = buffer[i]; // Use 32-bit intermediate to prevent overflow
        int32_t new_val = (int32_t)(value * amplitude * amplitude_multiplier);
        int32_t mixed_val = current_val + new_val;

        // Clamp to 16-bit range
        if (mixed_val > 32767) mixed_val = 32767;
        if (mixed_val < -32768) mixed_val = -32768;

        buffer[i] = (short int)mixed_val;
    }
}

// --- TODO: Implement specific sound functions ---
// void addBoom(...) { /* Generate bass drum sound */ }
// void addTsss(...) { /* Generate hi-hat like sound (e.g., filtered noise) */ }
// void addBang(...) { /* Generate snare-like sound */ }


int writeWavFile(const char *filename, WavHeader *header, const short int *buffer, size_t buffer_sample_count) {
    if (!filename || !header || !buffer || buffer_sample_count == 0) {
        return -1; // Invalid arguments
    }

    FILE *fp = fopen(filename, "wb"); // Use "wb" for binary writing
    if (!fp) {
        perror("Error opening WAV file for writing");
        return -1;
    }

    // Calculate final header values based on actual data
    header->dlength = buffer_sample_count * header->bytes_per_samp;
    header->flength = header->dlength + sizeof(WavHeader) - 8; // -8 for RIFF and flength itself

    // Write the header
    size_t written = fwrite(header, 1, sizeof(WavHeader), fp);
    if (written != sizeof(WavHeader)) {
        fprintf(stderr, "Error writing WAV header.\n");
        fclose(fp);
        return -2;
    }

    // Write the audio data
    written = fwrite(buffer, header->bytes_per_samp, buffer_sample_count, fp);
    if (written != buffer_sample_count) {
        fprintf(stderr, "Error writing WAV data.\n");
        fclose(fp);
        return -2;
    }

    fclose(fp);
    return 0; // Success
}


// --- Temporary Main Function for Testing ---
#ifdef WAV_GENERATOR_TEST_MAIN

// --- Constants from Example ---
// Note Frequencies
const float D2 = 73.42f;
const float G2 = 98.00f;
const float A2 = 110.00f;
const float B2 = 123.47f;
const float D3 = 146.83f;
const float G3 = 196.00f;
const float A3 = 220.00f;
const float B3 = 246.94f;
const float Cs4 = 277.18f;
const float D4 = 293.66f;
const float E4 = 329.63f;
const float Fs4 = 369.99f;
const float G4 = 392.00f;
const float A4 = 440.00f;
const float B4 = 493.88f;

// Timing
#define SAMPLE_RATE DEFAULT_SAMPLE_RATE
#define NUM_MEASURES_TO_PLAY (4 * 4) // 16 measures total
#define BEATS_PER_MEASURE 4
#define MS_PER_BEAT 500
#define SAMPLES_PER_BEAT ((SAMPLE_RATE * MS_PER_BEAT) / 1000)
#define BUFFER_SIZE_SAMPLES ((size_t)NUM_MEASURES_TO_PLAY * BEATS_PER_MEASURE * SAMPLES_PER_BEAT)

// Buffer (global for simplicity in this test adaptation)
short int *g_buffer = NULL;

// --- Adapted Play Function ---
// Uses addSineWave from this file
void play(float freq, float duration_beats, int measure, float beat) {
    if (!g_buffer) return;

    int start_time_ms = (int)(((measure * BEATS_PER_MEASURE) + beat) * MS_PER_BEAT);
    int duration_ms = (int)(duration_beats * MS_PER_BEAT);
    // Match amplitude from original example (implicitly 3000)
    int16_t amplitude = 3000;

    // Use the existing addSineWave function
    addSineWave(g_buffer, BUFFER_SIZE_SAMPLES, freq, duration_ms, start_time_ms, SAMPLE_RATE, amplitude);
}

// --- Chord Functions from Example ---
// DMajor
void DM(float duration, int measure, float beat) {
  play(D4, duration, measure, beat);
  play(Fs4, duration, measure, beat);
  play(A4, duration, measure, beat);
}

// A Major 1st Inversion
void AM1st(float duration, int measure, float beat) {
  play(Cs4, duration, measure, beat);
  play(E4, duration, measure, beat);
  play(A4, duration, measure, beat);
}

// B Minor 1st Inversion
void Bm1st(float duration, int measure, float beat) {
  play(D4, duration, measure, beat);
  play(Fs4, duration, measure, beat);
  play(B4, duration, measure, beat);
}

// G Major 2nd Inversion
void GM2nd(float duration, int measure, float beat) {
  play(D4, duration, measure, beat);
  play(G4, duration, measure, beat);
  play(B4, duration, measure, beat);
}


// --- Main Test Driver ---
int main() {
    const int16_t bits_per_sample = DEFAULT_BITS_PER_SAMPLE;
    const int16_t num_channels = DEFAULT_NUM_CHANNELS;
    const char* output_filename = "test_output.wav";

    // Calculate buffer size in bytes
    size_t buffer_bytes = BUFFER_SIZE_SAMPLES * (bits_per_sample / 8) * num_channels;

    // Allocate buffer and initialize to zero
    g_buffer = (short int*)malloc(buffer_bytes);
    if (!g_buffer) {
        fprintf(stderr, "Error allocating buffer.\n");
        return 1;
    }
    memset(g_buffer, 0, buffer_bytes); // Important to clear buffer for mixing

    // Initialize header
    WavHeader header;
    initWavHeader(&header, SAMPLE_RATE, bits_per_sample, num_channels);

    // --- Generate Chord Progression (from example) ---
    printf("Generating chord progression...\n");
    int measure = 0;
    while (measure < NUM_MEASURES_TO_PLAY) {
        play(D2, 4, measure, 0); // Bass note
        play(D3, 4, measure, 0); // Octave bass
        DM(1, measure, 0);       // Chord on beat 0
        DM(1, measure, 1);       // Chord on beat 1
        DM(1, measure, 2);       // Chord on beat 2
        DM(1, measure, 3);       // Chord on beat 3
        measure++;

        play(A2, 4, measure, 0); // Bass note
        play(A3, 4, measure, 0); // Octave bass
        AM1st(1, measure, 0);
        AM1st(1, measure, 1);
        AM1st(1, measure, 2);
        AM1st(1, measure, 3);
        measure++;

        play(B2, 4, measure, 0); // Bass note
        play(B3, 4, measure, 0); // Octave bass
        Bm1st(1, measure, 0);
        Bm1st(1, measure, 1);
        Bm1st(1, measure, 2);
        Bm1st(1, measure, 3);
        measure++;

        play(G2, 4, measure, 0); // Bass note
        play(G3, 4, measure, 0); // Octave bass
        GM2nd(1, measure, 0);
        GM2nd(1, measure, 1);
        GM2nd(1, measure, 2);
        GM2nd(1, measure, 3);
        measure++;
    }
    printf("Chord progression generated.\n");

    // Write the file
    printf("Writing WAV file: %s\n", output_filename);
    size_t buffer_size_samples = BUFFER_SIZE_SAMPLES;
    int result = writeWavFile(output_filename, &header, g_buffer, buffer_size_samples);

    // Cleanup
    free(g_buffer);
    g_buffer = NULL; // Avoid dangling pointer use

    if (result == 0) {
        printf("Successfully created %s\n", output_filename);
        return 0;
    } else {
        fprintf(stderr, "Failed to create WAV file (Error code: %d)\n", result);
        return 1;
    }
}
#endif // WAV_GENERATOR_TEST_MAIN
