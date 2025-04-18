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


void mix_in(int16_t *dest, int16_t *src, int start, int length) {
    for (int i = 0; i < length; i++) {
        int32_t mixed = dest[start + i] + src[i];
        if (mixed > 32767) mixed = 32767;
        if (mixed < -32768) mixed = -32768;
        dest[start + i] = (int16_t)mixed;
    }
}


// --- Temporary Main Function for Testing ---
#ifdef WAV_GENERATOR_TEST_MAIN

#include "soundwaves.h"

// Test pattern constants
#define NUM_MEASURES 8  // 8 measures total
#define BUFFER_SIZE_SAMPLES (NUM_MEASURES * BEATS_PER_MEASURE * SAMPLES_PER_BEAT)

int main() {
    printf("Testing DJcode sound generation with mixed layering...\n");

    WavHeader header;
    initWavHeader(&header, SAMPLE_RATE, BIT_DEPTH, DEFAULT_NUM_CHANNELS);

    int16_t *buffer = (int16_t *)calloc(BUFFER_SIZE_SAMPLES, sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "Buffer allocation failed\n");
        return 1;
    }

    int16_t temp[SAMPLES_PER_BEAT];
    memset(temp, 0, sizeof(temp));

    int measure = 0;

    // --- Measures 0-3: Chord progression with some parallel percussion ---
    for (; measure < 4; measure++) {
        for (int beat = 0; beat < 4; beat++) {
            int start = (measure * 4 + beat) * SAMPLES_PER_BEAT;

            // Parallel sounds for first two beats
            if (beat == 0 || beat == 1) {
                memset(temp, 0, sizeof(temp));
                generate_boom(temp, SAMPLES_PER_BEAT, BOOM_FREQ);
                mix_in(buffer, temp, start, SAMPLES_PER_BEAT);

                memset(temp, 0, sizeof(temp));
                generate_floortom(temp, SAMPLES_PER_BEAT, TSST_FREQ);
                mix_in(buffer, temp, start, SAMPLES_PER_BEAT);

                memset(temp, 0, sizeof(temp));
                generate_diding(temp, SAMPLES_PER_BEAT, DING_FREQ);
                mix_in(buffer, temp, start, SAMPLES_PER_BEAT);
            }

            // Chord
            DM(buffer, BUFFER_SIZE_SAMPLES, 1, measure, beat);
        }
    }

    // --- Measures 4-5: Each sound isolated (serial style) ---
    for (; measure < 6; measure++) {
        for (int beat = 0; beat < 4; beat++) {
            int start = (measure * 4 + beat) * SAMPLES_PER_BEAT;
            memset(temp, 0, sizeof(temp));

            switch (beat) {
                case 0:
                    generate_boom(temp, SAMPLES_PER_BEAT, BOOM_FREQ);
                    break;
                case 1:
                    generate_clap(temp, SAMPLES_PER_BEAT, CLAP_FREQ);
                    break;
                case 2:
                    generate_tsst(temp, SAMPLES_PER_BEAT, TSST_FREQ);
                    break;
                case 3:
                    generate_dididing(temp, SAMPLES_PER_BEAT, DING_FREQ);
                    break;
            }
            mix_in(buffer, temp, start, SAMPLES_PER_BEAT);
        }
    }

    // --- Measures 6-7: Everything layered ---
    for (; measure < 8; measure++) {
        for (int beat = 0; beat < 4; beat++) {
            int start = (measure * 4 + beat) * SAMPLES_PER_BEAT;

            generate_boom(temp, SAMPLES_PER_BEAT, BOOM_FREQ);
            mix_in(buffer, temp, start, SAMPLES_PER_BEAT);

            generate_clap(temp, SAMPLES_PER_BEAT, CLAP_FREQ);
            mix_in(buffer, temp, start, SAMPLES_PER_BEAT);

            generate_tsst(temp, SAMPLES_PER_BEAT, TSST_FREQ);
            mix_in(buffer, temp, start, SAMPLES_PER_BEAT);

            generate_diding(temp, SAMPLES_PER_BEAT, DING_FREQ);
            mix_in(buffer, temp, start, SAMPLES_PER_BEAT);

            GM2nd(buffer, BUFFER_SIZE_SAMPLES, 1, measure, beat);
        }
    }

    const char *output_filename = "djcode_parallel_mixed.wav";
    printf("Writing: %s\n", output_filename);
    int result = writeWavFile(output_filename, &header, buffer, BUFFER_SIZE_SAMPLES);

    free(buffer);

    if (result == 0) {
        printf("Successfully created %s\n", output_filename);
        return 0;
    } else {
        fprintf(stderr, "Failed to write file\n");
        return 1;
    }
}
#endif // WAV_GENERATOR_TEST_MAIN
