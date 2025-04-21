#include "WAVGenerator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>   
#include <string.h> 
#include "soundwaves.h" 
#include "tokensParser.h"     


void initWavHeader(WavHeader *header, int32_t sample_rate, int16_t bits_per_sample, int16_t num_channels) {
    if (!header) return;

    /* RIFF Chunk DO NOT CHANGE THIS*/
    strncpy(header->riff, "RIFF", 5);
    /* flength will be calculated later based on data size. DO NOT CHANGE*/
    strncpy(header->wave, "WAVE", 5);

    /* Format Chunk ("fmt ") DO NOT CHANGE THIS*/
    strncpy(header->fmt, "fmt ", 5);
    header->chunk_size = 16; /* Standard size for PCM*/
    header->format_tag = 1;  /* PCM*/
    header->num_chans = num_channels; /* can change. 1 is mono 2 is stereo ...*/
    header->srate = sample_rate; /* can change*/
    header->bits_per_samp = bits_per_sample; /* can change*/
    header->bytes_per_samp = (bits_per_sample / 8) * num_channels;
    header->bytes_per_sec = sample_rate * header->bytes_per_samp;

    /* Data Chunk. */
    strncpy(header->data, "data", 5);
    /* dlength will be calculated later based on data size*/

    /* Initialize lengths to 0, they need to be set before writing*/
    header->flength = 0;
    header->dlength = 0;
}

SoundFunction get_sound_function(const char* sound_name, float* frequency) {
    SoundFunction result;
    result.requires_freq = 1; /* Assume frequency is needed by default*/

    if (strcmp(sound_name, "BOOM") == 0) { 
        *frequency = BOOM_FREQ; 
        result.func.with_freq = generate_boom;
    }
    else if (strcmp(sound_name, "TSST") == 0) { 
        *frequency = TSST_FREQ; 
        result.func.with_freq = generate_tsst;
    }
    else if (strcmp(sound_name, "CLAP") == 0) { 
        *frequency = CLAP_FREQ; 
        result.func.with_freq = generate_clap;
    }
    else if (strcmp(sound_name, "DUN") == 0) { 
        *frequency = BOOM_FREQ; 
        result.func.with_freq = generate_floortom;
    }
    else if (strcmp(sound_name, "DING") == 0) { 
        *frequency = DING_FREQ; 
        result.func.with_freq = generate_ding;
    }
    else if (strcmp(sound_name, "DIDING") == 0) { 
        *frequency = DING_FREQ; 
        result.func.with_freq = generate_diding;
    }
    else if (strcmp(sound_name, "DIDIDING") == 0) { 
        *frequency = DING_FREQ; 
        result.func.with_freq = generate_dididing;
    }
    else if (strcmp(sound_name, "CRASH") == 0) { 
        result.requires_freq = 0; 
        result.func.no_freq = generate_crash;
    }
    else if (strcmp(sound_name, "REST") == 0) { 
        result.requires_freq = 0; 
        result.func.no_freq = generate_rest;
    }
    else {
        fprintf(stderr, "Warning: Unknown sound name '%s'\n", sound_name);
        result.requires_freq = 0;
        result.func.no_freq = generate_rest; /* Default to rest if unknown*/
    }

    return result;
}

int writeWavFile(const char *filename, WavHeader *header, const short int *buffer, size_t buffer_sample_count) {
    FILE *fp;
    size_t written;

    if (!filename || !header || !buffer || buffer_sample_count == 0) {
        return -1; /* Invalid arguments*/
    }

    fp = fopen(filename, "wb"); /* Use "wb" for binary writing*/
    if (!fp) {
        perror("Error opening WAV file for writing");
        return -1;
    }

    /* Calculate final header dlength and flenght values based on actual data*/
    header->dlength = buffer_sample_count * header->bytes_per_samp;
    header->flength = header->dlength + sizeof(WavHeader) - 8; /* -8 for RIFF and flength itself*/

    /* Write the header*/
    written = fwrite(header, 1, sizeof(WavHeader), fp);
    if (written != sizeof(WavHeader)) {
        fprintf(stderr, "Error writing WAV header.\n");
        fclose(fp);
        return -2;
    }

    /* Write the audio data*/
    written = fwrite(buffer, header->bytes_per_samp, buffer_sample_count, fp);
    if (written != buffer_sample_count) {
        fprintf(stderr, "Error writing WAV data.\n");
        fclose(fp);
        return -2;
    }

    fclose(fp);
    return 0; /* Success*/
}

void mix_in(int16_t *dest, int16_t *src, int start, int length) {
    int i;
    int32_t mixed;
    
    for (i = 0; i < length; i++) {
        mixed = dest[start + i] + src[i];
        if (mixed > 32767) mixed = 32767;
        if (mixed < -32768) mixed = -32768;
        dest[start + i] = (int16_t)mixed;
    }
}


/* TEMPORARY Main Application Logic FOR TESTING */
#ifdef WAV_GENERATOR_STANDALONE_MAIN

int main(int argc, char *argv[]) {
    const char* token_filename;
    const char* output_filename;
    Pattern patterns[MAX_PATTERNS];
    PlayCommand play_sequence[MAX_PLAY_COMMANDS];
    int num_patterns;
    int num_play_commands;
    int parse_result;
    int pattern_found;
    int i, j, loop, sound_idx;
    size_t total_samples;
    size_t current_sample_index;
    int16_t *buffer;
    int16_t temp_buffer[SAMPLES_PER_BEAT]; /* Buffer for a single beat*/
    Pattern* current_pattern;
    const char* sound_name;
    float frequency;
    SoundFunction sound_func;
    WavHeader header;
    int result;
    size_t total_beats;

    printf("DJ Code WAV Generator\n");

    token_filename = "../Lexer_Parser/transformed_tokens.txt";
    output_filename = "../NEW_DJcode_Beats.wav";
    num_patterns = 0;
    num_play_commands = 0;

    printf("Parsing token file: %s\n", token_filename);
    parse_result = parse_tokens_file(token_filename, patterns, &num_patterns, play_sequence, &num_play_commands);

    if (parse_result != 0) {
        fprintf(stderr, "Failed to parse token file (Error code: %d).\n", parse_result);
        return 1;
    }
    printf("Parsed %d patterns and %d play commands.\n", num_patterns, num_play_commands);

    /* Calculate total audio length*/
    total_beats = 0;
    for (i = 0; i < num_play_commands; i++) {
        pattern_found = 0;
        for (j = 0; j < num_patterns; j++) {
            if (strcmp(play_sequence[i].pattern_name, patterns[j].name) == 0) {
                total_beats += play_sequence[i].loop_count * patterns[j].num_sounds;
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

    total_samples = total_beats * SAMPLES_PER_BEAT;
    printf("Total beats: %lu, Total samples: %lu\n", (unsigned long) total_beats, (unsigned long) total_samples);

    /* Allocate buffer*/
    buffer = (int16_t *)calloc(total_samples, sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "Buffer allocation failed for %lu samples.\n", (unsigned long)total_samples);
        return 1;
    }
    printf("Allocated buffer for %lu samples.\n", (unsigned long) total_samples);

    /* Generate Audio*/
    printf("Generating audio...\n");
    current_sample_index = 0;

    for (i = 0; i < num_play_commands; i++) {
        current_pattern = NULL;
        for (j = 0; j < num_patterns; j++) {
            if (strcmp(play_sequence[i].pattern_name, patterns[j].name) == 0) {
                current_pattern = &patterns[j];
                break;
            }
        }
        if (!current_pattern) continue;

        printf("Playing pattern '%s' %d times...\n", current_pattern->name, play_sequence[i].loop_count);

        for (loop = 0; loop < play_sequence[i].loop_count; loop++) {
            for (sound_idx = 0; sound_idx < current_pattern->num_sounds; sound_idx++) {
                sound_name = current_pattern->sounds[sound_idx];
                frequency = 0;
                sound_func = get_sound_function(sound_name, &frequency);

                memset(temp_buffer, 0, sizeof(temp_buffer)); /* Clear temp buffer*/

                if (sound_func.requires_freq) {
                    sound_func.func.with_freq(temp_buffer, SAMPLES_PER_BEAT, frequency);
                } else {
                    sound_func.func.no_freq(temp_buffer, SAMPLES_PER_BEAT);
                }

                /* Mix into the main buffer if within bounds*/
                if (current_sample_index < total_samples) {
                    mix_in(buffer, temp_buffer, current_sample_index, SAMPLES_PER_BEAT);
                } else {
                    fprintf(stderr, "Warning: Exceeded calculated buffer size during generation.\n");
                    goto generation_end;
                }

                current_sample_index += SAMPLES_PER_BEAT;
            }
        }
    }

generation_end:
    printf("Audio generation complete.\n");

    /* Write WAV file*/
    initWavHeader(&header, SAMPLE_RATE, BIT_DEPTH, DEFAULT_NUM_CHANNELS);

    printf("Writing WAV file: %s\n", output_filename);
    result = writeWavFile(output_filename, &header, buffer, total_samples);

    free(buffer);

    if (result == 0) {
        printf("Successfully created %s\n", output_filename);
        return 0;
    } else {
        fprintf(stderr, "Failed to write WAV file (Error code: %d).\n", result);
        return 1;
    }
}
#endif
