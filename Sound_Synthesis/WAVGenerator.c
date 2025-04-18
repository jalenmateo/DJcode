#include "WAVGenerator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>   // For M_PI and cos
#include <string.h> // For strcmp in sound mapping
#include "soundwaves.h" // For sound generation functions and constants
#include "tokensParser.h"     // For Pattern and PlayCommand structs

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

// --- Sound Name to Function Mapping ---

// Define a type for the sound generation functions we'll be mapping to.
// Note: This assumes a consistent signature. Functions like generate_crash
// might need special handling if they don't take frequency.
typedef void (*SoundFunc)(int16_t*, int, float);
typedef void (*SoundFuncNoFreq)(int16_t*, int); // For sounds like crash, rest

// Function to get the appropriate sound generation function based on name
// Returns NULL if the name is not recognized.
void* get_sound_function(const char* sound_name, float* frequency, int* requires_freq) {
    *requires_freq = 1; // Assume frequency is needed by default

    if (strcmp(sound_name, "BOOM") == 0) { *frequency = BOOM_FREQ; return generate_boom; }
    if (strcmp(sound_name, "TSST") == 0) { *frequency = TSST_FREQ; return generate_tsst; }
    if (strcmp(sound_name, "CLAP") == 0) { *frequency = CLAP_FREQ; return generate_clap; }
    if (strcmp(sound_name, "DUN") == 0) { *frequency = BOOM_FREQ; return generate_floortom; } // Using BOOM_FREQ as base for floortom
    if (strcmp(sound_name, "DING") == 0) { *frequency = DING_FREQ; return generate_ding; }
    if (strcmp(sound_name, "DIDING") == 0) { *frequency = DING_FREQ; return generate_diding; }
    if (strcmp(sound_name, "DIDIDING") == 0) { *frequency = DING_FREQ; return generate_dididing; }
    // Sounds that don't require frequency
    if (strcmp(sound_name, "CRASH") == 0) { *requires_freq = 0; return generate_crash; }
    if (strcmp(sound_name, "REST") == 0) { *requires_freq = 0; return generate_rest; }

    // Add other sounds here...

    fprintf(stderr, "Warning: Unknown sound name '%s'\n", sound_name);
    *requires_freq = 0;
    return generate_rest; // Default to rest if unknown
}


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


// --- Main Application Logic ---

int main(int argc, char *argv[]) {
    printf("DJ Code WAV Generator\n");

    const char* token_filename = "Sound_Synthesis/tokens.txt"; // hardcoded path for now but can change later on
    const char* output_filename = "DJcode_Beats.wav";

    // --- 1. Parse the tokens file ---
    Pattern patterns[MAX_PATTERNS];
    PlayCommand play_sequence[MAX_PLAY_COMMANDS];
    int num_patterns = 0;
    int num_play_commands = 0;

    printf("Parsing token file: %s\n", token_filename);
    int parse_result = parse_tokens_file(token_filename, patterns, &num_patterns, play_sequence, &num_play_commands);

    if (parse_result != 0) {
        fprintf(stderr, "Failed to parse token file (Error code: %d).\n", parse_result);
        return 1;
    }
    printf("Parsed %d patterns and %d play commands.\n", num_patterns, num_play_commands);

    // --- 2. Calculate total audio length ---
    long long total_beats = 0;
    for (int i = 0; i < num_play_commands; i++) {
        // Find the pattern corresponding to the play command
        int pattern_found = 0;
        for (int j = 0; j < num_patterns; j++) {
            if (strcmp(play_sequence[i].pattern_name, patterns[j].name) == 0) {
                total_beats += (long long)play_sequence[i].loop_count * patterns[j].num_sounds;
                pattern_found = 1;
                break;
            }
        }
        if (!pattern_found) {
            fprintf(stderr, "Error: Pattern '%s' specified in PLAY command not found.\n", play_sequence[i].pattern_name);
            return 1;
        }
    }

    if (total_beats == 0) {
        printf("No beats to generate. Exiting.\n");
        return 0;
    }

    size_t total_samples = total_beats * SAMPLES_PER_BEAT;
    printf("Total beats: %lld, Total samples: %zu\n", total_beats, total_samples);

    // --- 3. Allocate buffer ---
    int16_t *buffer = (int16_t *)calloc(total_samples, sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "Buffer allocation failed for %zu samples.\n", total_samples);
        return 1;
    }
    printf("Allocated buffer for %zu samples.\n", total_samples);

    // --- 4. Generate Audio ---
    printf("Generating audio...\n");
    size_t current_sample_index = 0;
    int16_t temp_buffer[SAMPLES_PER_BEAT]; // Buffer for a single beat

    for (int i = 0; i < num_play_commands; i++) {
        // Find the pattern
        Pattern* current_pattern = NULL;
        for (int j = 0; j < num_patterns; j++) {
            if (strcmp(play_sequence[i].pattern_name, patterns[j].name) == 0) {
                current_pattern = &patterns[j];
                break;
            }
        }
        // We already checked for missing patterns earlier, but double-check
        if (!current_pattern) continue;

        printf("Playing pattern '%s' %d times...\n", current_pattern->name, play_sequence[i].loop_count);

        for (int loop = 0; loop < play_sequence[i].loop_count; loop++) {
            for (int sound_idx = 0; sound_idx < current_pattern->num_sounds; sound_idx++) {
                const char* sound_name = current_pattern->sounds[sound_idx];
                float frequency = 0;
                int requires_freq = 0;
                void* func_ptr = get_sound_function(sound_name, &frequency, &requires_freq);

                if (func_ptr) {
                    memset(temp_buffer, 0, sizeof(temp_buffer)); // Clear temp buffer

                    if (requires_freq) {
                        ((SoundFunc)func_ptr)(temp_buffer, SAMPLES_PER_BEAT, frequency);
                    } else {
                        ((SoundFuncNoFreq)func_ptr)(temp_buffer, SAMPLES_PER_BEAT);
                    }

                    // Mix into the main buffer if within bounds
                    if (current_sample_index < total_samples) {
                         mix_in(buffer, temp_buffer, current_sample_index, SAMPLES_PER_BEAT);
                    } else {
                        fprintf(stderr, "Warning: Exceeded calculated buffer size during generation.\n");
                        goto generation_end; // Exit loops if buffer overflow
                    }
                } else {
                     // Function not found (already warned in get_sound_function), effectively a rest
                     // No need to mix anything
                }

                current_sample_index += SAMPLES_PER_BEAT;
            } // End sounds loop
        } // End loops loop
    } // End play commands loop

generation_end:
    printf("Audio generation complete.\n");

    // --- 5. Write WAV file ---
    WavHeader header;
    // Use constants from soundwaves.h if available, otherwise define defaults
    #ifndef DEFAULT_NUM_CHANNELS
    #define DEFAULT_NUM_CHANNELS 1
    #endif
    initWavHeader(&header, SAMPLE_RATE, BIT_DEPTH, DEFAULT_NUM_CHANNELS);

    printf("Writing WAV file: %s\n", output_filename);
    int result = writeWavFile(output_filename, &header, buffer, total_samples);

    free(buffer);

    if (result == 0) {
        printf("Successfully created %s\n", output_filename);
        return 0;
    } else {
        fprintf(stderr, "Failed to write WAV file (Error code: %d).\n", result);
        return 1;
    }
}
// #endif // WAV_GENERATOR_TEST_MAIN // Keep this commented out or remove if no longer needed
