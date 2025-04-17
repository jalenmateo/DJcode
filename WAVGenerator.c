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

#include "soundwaves.h"

// Test pattern constants
#define NUM_MEASURES 8  // 8 measures total
#define BUFFER_SIZE_SAMPLES (NUM_MEASURES * BEATS_PER_MEASURE * SAMPLES_PER_BEAT)

int main() {
    printf("Testing DJcode sound generation...\n");

    // Initialize WAV header
    WavHeader header;
    initWavHeader(&header, SAMPLE_RATE, BIT_DEPTH, DEFAULT_NUM_CHANNELS);

    // Calculate buffer size and allocate
    size_t buffer_bytes = BUFFER_SIZE_SAMPLES * sizeof(int16_t);
    int16_t *buffer = (int16_t*)malloc(buffer_bytes);
    if (!buffer) {
        fprintf(stderr, "Error allocating buffer.\n");
        return 1;
    }
    memset(buffer, 0, buffer_bytes);

    printf("Generating test pattern with both musical chords and percussion...\n");

    // First 4 measures: Musical chord progression with percussion
    int measure = 0;
    while (measure < 4) {
        // D Major with boom and tsst
        play(buffer, BUFFER_SIZE_SAMPLES, D2, 4, measure, 0); // Bass
        play(buffer, BUFFER_SIZE_SAMPLES, D3, 4, measure, 0); // Octave
        DM(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 0);
        DM(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 1);
        DM(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 2);
        DM(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 3);
        generate_boom(buffer + (measure * 4 * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, BOOM_FREQ);
        generate_tsst(buffer + ((measure * 4 + 1) * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, TSST_FREQ);
        measure++;

        // A Major with clap
        play(buffer, BUFFER_SIZE_SAMPLES, A2, 4, measure, 0);
        play(buffer, BUFFER_SIZE_SAMPLES, A3, 4, measure, 0);
        AM1st(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 0);
        AM1st(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 1);
        AM1st(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 2);
        AM1st(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 3);
        generate_clap(buffer + ((measure * 4 + 2) * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, CLAP_FREQ);
        measure++;

        // B Minor with crash
        play(buffer, BUFFER_SIZE_SAMPLES, B2, 4, measure, 0);
        play(buffer, BUFFER_SIZE_SAMPLES, B3, 4, measure, 0);
        Bm1st(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 0);
        Bm1st(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 1);
        Bm1st(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 2);
        Bm1st(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 3);
        generate_crash(buffer + ((measure * 4 + 3) * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT);
        measure++;

        // G Major with triangle sounds
        play(buffer, BUFFER_SIZE_SAMPLES, G2, 4, measure, 0);
        play(buffer, BUFFER_SIZE_SAMPLES, G3, 4, measure, 0);
        GM2nd(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 0);
        GM2nd(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 1);
        GM2nd(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 2);
        GM2nd(buffer, BUFFER_SIZE_SAMPLES, 1, measure, 3);
        generate_ding(buffer + (measure * 4 * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, DING_FREQ);
        generate_diding(buffer + ((measure * 4 + 2) * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, DING_FREQ);
        measure++;
    }

    // Last 4 measures: Drum and triangle pattern
    for (int i = 0; i < 4; i++) {
        int base_beat = (measure + i) * 4;
        
        // Basic drum pattern
        generate_boom(buffer + ((base_beat + 0) * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, BOOM_FREQ);
        generate_tsst(buffer + ((base_beat + 1) * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, TSST_FREQ);
        generate_clap(buffer + ((base_beat + 2) * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, CLAP_FREQ);
        
        // Add triangle sounds on top
        if (i % 2 == 0) {
            generate_diding(buffer + ((base_beat + 3) * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, DING_FREQ);
        } else {
            generate_dididing(buffer + ((base_beat + 3) * SAMPLES_PER_BEAT), SAMPLES_PER_BEAT, DING_FREQ);
        }
    }

    // Write the WAV file
    const char* output_filename = "test_output.wav";
    printf("Writing WAV file: %s\n", output_filename);
    int result = writeWavFile(output_filename, &header, buffer, BUFFER_SIZE_SAMPLES);

    // Cleanup
    free(buffer);

    if (result == 0) {
        printf("Successfully created %s\n", output_filename);
        printf("Test pattern contains:\n");
        printf("- Measures 0-3: Musical chord progression with percussion\n");
        printf("  * D Major with boom/tsst\n");
        printf("  * A Major with clap\n");
        printf("  * B Minor with crash\n");
        printf("  * G Major with triangle sounds\n");
        printf("- Measures 4-7: Drum and triangle pattern\n");
        printf("  * Basic drum pattern (boom, tsst, clap)\n");
        printf("  * Alternating diding/dididing\n");
        return 0;
    } else {
        fprintf(stderr, "Failed to create WAV file (Error code: %d)\n", result);
        return 1;
    }
}
#endif // WAV_GENERATOR_TEST_MAIN
