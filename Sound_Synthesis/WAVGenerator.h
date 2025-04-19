#ifndef WAVGENERATOR_H
#define WAVGENERATOR_H

#include <stdint.h>
#include <stdio.h> // For FILE*

#define DEFAULT_SAMPLE_RATE 16000
#define DEFAULT_BITS_PER_SAMPLE 16
#define DEFAULT_NUM_CHANNELS 1 // Mono


// Standard WAV Header Structure. Can look at https://docs.fileformat.com/audio/wav/. This is where I got this from
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

// Define a type for the sound generation functions we'll be mapping to.
// Note: This assumes a consistent signature. Functions like generate_crash
// might need special handling if they don't take frequency.
typedef void (*SoundFunc)(int16_t*, int, float);
typedef void (*SoundFuncNoFreq)(int16_t*, int); // For sounds like crash, rest

//Initializes a WavHeader struct with default values.
void initWavHeader(WavHeader *header, int32_t sample_rate, int16_t bits_per_sample, int16_t num_channels);

// Writes the WAV header and audio buffer to a file. Calculates final header values based on buffer length before writing.
// return 0 on success, -1 on file open error, -2 on write error.
int writeWavFile(const char *filename, WavHeader *header, const short int *buffer, size_t buffer_sample_count);

// Function to get the appropriate sound generation function based on name. We utilise function pointers to call the right sound generation funciton.
// We can actually allocate a buffer an array to each generate_* function output if we want to keep things static. Like if we want to make it so that users cannot tamper with the preset waveforms we provide. But I guess this way makes it more future proof.
// Returns NULL if the name is not recognized.
void* get_sound_function(const char* sound_name, float* frequency, int* requires_freq);

// Allows for mixing capabilities, like parallelizing sounds using a buffer
void mix_in(int16_t *dest, int16_t *src, int start, int length) ;

#endif // WAVGENERATOR_H
