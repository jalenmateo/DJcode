#include "soundwaves.h"
#include <math.h>
#include <stdlib.h>

// Math constants
#define PI 3.14159265358979323846
#define TWO_PI (2.0 * PI)

// Helper function to generate a sine wave sample
static float sine_wave(float phase) {
    return sinf(phase);
}

// Helper function to generate a triangle wave sample
static float triangle_wave(float phase) {
    float normalized_phase = fmodf(phase, TWO_PI) / TWO_PI;
    return 2.0f * fabsf(2.0f * (normalized_phase - floorf(normalized_phase + 0.5f))) - 1.0f;
}

// Helper function to generate white noise
static float white_noise() {
    return (float)rand() / RAND_MAX * 2.0f - 1.0f;
}

// Helper function to apply exponential decay envelope
static float apply_decay(float sample, int current_sample, int total_samples, float decay_factor) {
    return sample * powf(decay_factor, (float)current_sample / total_samples);
}

// Helper function to apply musical note envelope
static float apply_note_envelope(float sample, float percent_through_note) {
    float amplitude_multiplier = 1.0f;
    
    if (percent_through_note < 0.25f) {
        amplitude_multiplier = (percent_through_note / 0.25f);
    }
    else if (percent_through_note < 0.5f) {
        amplitude_multiplier = (1.0f - ((percent_through_note - 0.25f) / 0.25f)) * 0.8f + 0.2f;
    }
    else {
        amplitude_multiplier = 1.0f - percent_through_note;
    }
    
    return sample * amplitude_multiplier;
}

// DRUM SOUNDS

void generate_boom(int16_t *buffer, int num_samples, float frequency) {
    float phase = 0.0f;
    float phase_step = TWO_PI * frequency / SAMPLE_RATE;
    float decay_factor = 0.001f; // Fast decay for kick drum
    
    for (int i = 0; i < num_samples; i++) {
        float sample = sine_wave(phase);
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE);
        phase += phase_step;
    }
}

void generate_tsst(int16_t *buffer, int num_samples, float frequency) {
    float decay_factor = 0.0001f; // Very fast decay for hi-hat
    
    for (int i = 0; i < num_samples; i++) {
        float sample = white_noise();
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE * 0.7f); // Slightly reduced amplitude for high frequencies
    }
}

void generate_clap(int16_t *buffer, int num_samples, float frequency) {
    float decay_factor = 0.01f; // Slower decay for clap reverb
    
    for (int i = 0; i < num_samples; i++) {
        float sample = white_noise();
        // Simple band-pass simulation by mixing noise with a sine wave
        sample = (sample * 0.7f + sine_wave(TWO_PI * frequency * i / SAMPLE_RATE) * 0.3f);
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE * 0.8f);
    }
}

void generate_crash(int16_t *buffer, int num_samples) {
    float decay_factor = 0.05f; // Slow decay for crash
    
    for (int i = 0; i < num_samples; i++) {
        float sample = white_noise();
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE * 0.6f);
    }
}

void generate_rest(int16_t *buffer, int num_samples) {
    for (int i = 0; i < num_samples; i++) {
        buffer[i] = 0;
    }
}

// TRIANGLE SOUNDS

void generate_ding(int16_t *buffer, int num_samples, float frequency) {
    float phase = 0.0f;
    float phase_step = TWO_PI * frequency / SAMPLE_RATE;
    float decay_factor = 0.01f;
    
    for (int i = 0; i < num_samples; i++) {
        float sample = triangle_wave(phase);
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE);
        phase += phase_step;
    }
}

void generate_diding(int16_t *buffer, int num_samples, float frequency) {
    int half_samples = num_samples / 2;
    
    // First 'di'
    generate_ding(buffer, half_samples, frequency);
    
    // Second 'ding' (slightly higher pitch)
    generate_ding(buffer + half_samples, num_samples - half_samples, frequency * 1.1f);
}

void generate_dididing(int16_t *buffer, int num_samples, float frequency) {
    int third_samples = num_samples / 3;
    
    // First 'di'
    generate_ding(buffer, third_samples, frequency);
    
    // Second 'di' (slightly higher pitch)
    generate_ding(buffer + third_samples, third_samples, frequency * 1.1f);
    
    // Final 'ding' (back to original pitch)
    generate_ding(buffer + (2 * third_samples), num_samples - (2 * third_samples), frequency);
}

// MUSICAL CHORD FUNCTIONS

void play(int16_t *buffer, size_t buffer_size, float freq, float duration, int measure, float beat) {
    float current_beat = BEATS_PER_MEASURE * measure + beat;
    int start_index = (int)(current_beat * SAMPLES_PER_BEAT);
    int num_samples = (int)(duration * SAMPLES_PER_BEAT);
    int end_index = start_index + num_samples;
    
    if (end_index > buffer_size) {
        end_index = buffer_size;
    }
    
    float phase = 0.0f;
    float phase_step = TWO_PI * freq / SAMPLE_RATE;
    
    for (int i = start_index; i < end_index; i++) {
        float percent_through_note = (float)(i - start_index) / num_samples;
        float sample = sine_wave(phase);
        sample = apply_note_envelope(sample, percent_through_note);
        
        // Mix with existing buffer content
        int32_t current_val = buffer[i];
        int32_t new_val = (int32_t)(sample * 3000); // Original amplitude from example
        int32_t mixed_val = current_val + new_val;
        
        // Clamp to 16-bit range
        if (mixed_val > 32767) mixed_val = 32767;
        if (mixed_val < -32768) mixed_val = -32768;
        
        buffer[i] = (int16_t)mixed_val;
        phase += phase_step;
    }
}

void DM(int16_t *buffer, size_t buffer_size, float duration, int measure, float beat) {
    play(buffer, buffer_size, D4, duration, measure, beat);
    play(buffer, buffer_size, Fs4, duration, measure, beat);
    play(buffer, buffer_size, A4, duration, measure, beat);
}

void AM1st(int16_t *buffer, size_t buffer_size, float duration, int measure, float beat) {
    play(buffer, buffer_size, Cs4, duration, measure, beat);
    play(buffer, buffer_size, E4, duration, measure, beat);
    play(buffer, buffer_size, A4, duration, measure, beat);
}

void Bm1st(int16_t *buffer, size_t buffer_size, float duration, int measure, float beat) {
    play(buffer, buffer_size, D4, duration, measure, beat);
    play(buffer, buffer_size, Fs4, duration, measure, beat);
    play(buffer, buffer_size, B4, duration, measure, beat);
}

void GM2nd(int16_t *buffer, size_t buffer_size, float duration, int measure, float beat) {
    play(buffer, buffer_size, D4, duration, measure, beat);
    play(buffer, buffer_size, G4, duration, measure, beat);
    play(buffer, buffer_size, B4, duration, measure, beat);
}
