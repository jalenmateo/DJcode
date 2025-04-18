#ifndef WAVGENERATOR_H
#define WAVGENERATOR_H

#include <stdint.h>
#include <stdio.h> // For FILE*

// --- Constants ---
#define DEFAULT_SAMPLE_RATE 16000
#define DEFAULT_BITS_PER_SAMPLE 16
#define DEFAULT_NUM_CHANNELS 1 // Mono

// --- Structures ---

// Standard WAV Header Structure
typedef struct {
    char riff[4];           /* "RIFF"                                  */
    int32_t flength;        /* file length in bytes                    */
    char wave[4];           /* "WAVE"                                  */
    char fmt[4];            /* "fmt "                                  */
    int32_t chunk_size;     /* size of FMT chunk in bytes (usually 16) */
    int16_t format_tag;     /* 1=PCM                                   */
    int16_t num_chans;      /* 1=mono, 2=stereo                        */
    int32_t srate;          /* Sampling rate in samples per second     */
    int32_t bytes_per_sec;  /* bytes per second = srate*num_chans*bytes_per_samp */
    int16_t bytes_per_samp; /* bytes per sample = num_chans*(bits_per_samp/8) */
    int16_t bits_per_samp;  /* Number of bits per sample               */
    char data[4];           /* "data"                                  */
    int32_t dlength;        /* data length in bytes (filelength - 44)  */
} WavHeader;

// Structure to hold WAV generation state (optional, could manage buffer internally)
// typedef struct {
//     short int *buffer;
//     size_t buffer_size;
//     size_t current_sample_count;
//     WavHeader header;
// } WavData;


// --- Function Prototypes ---

/**
 * @brief Initializes a WavHeader struct with default values.
 *
 * @param header Pointer to the WavHeader struct to initialize.
 * @param sample_rate The desired sample rate (e.g., 16000, 44100).
 * @param bits_per_sample The desired bits per sample (e.g., 16).
 * @param num_channels The number of channels (1 for mono, 2 for stereo).
 */
void initWavHeader(WavHeader *header, int32_t sample_rate, int16_t bits_per_sample, int16_t num_channels);

/**
 * @brief Adds a synthesized sound (sine wave with basic envelope) to a buffer.
 *
 * @param buffer The audio buffer to add the sound to.
 * @param buffer_size The total size of the audio buffer in samples.
 * @param freq The frequency of the sound in Hz.
 * @param duration_ms The duration of the sound in milliseconds.
 * @param start_time_ms The start time of the sound within the buffer in milliseconds.
 * @param sample_rate The sample rate of the buffer.
 * @param amplitude The peak amplitude (0-32767 for 16-bit).
 */
void addSineWave(short int *buffer, size_t buffer_size, float freq, int duration_ms, int start_time_ms, int32_t sample_rate, int16_t amplitude);

// TODO: Add functions for specific sounds like "boom", "tsss", "bang"
// void addBoom(...);
// void addTsss(...);

/**
 * @brief Writes the WAV header and audio buffer to a file.
 *
 * Calculates final header values based on buffer length before writing.
 *
 * @param filename The name of the output WAV file.
 * @param header Pointer to the configured WavHeader struct.
 * @param buffer Pointer to the audio sample buffer.
 * @param buffer_sample_count The number of samples in the buffer.
 * @return 0 on success, -1 on file open error, -2 on write error.
 */
int writeWavFile(const char *filename, WavHeader *header, const short int *buffer, size_t buffer_sample_count);


#endif // WAVGENERATOR_H
