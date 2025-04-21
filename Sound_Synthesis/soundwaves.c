#include "soundwaves.h"
#include <stdlib.h>
#include <math.h>

#define sinf(x) ((float)sin((double)(x)))
#define powf(x,y) ((float)pow((double)(x),(double)(y)))
#define fmodf(x,y) ((float)fmod((double)(x),(double)(y)))
#define fabsf(x) ((float)fabs((double)(x)))
#define floorf(x) ((float)floor((double)(x)))

/* Math constants*/
#define PI 3.14159265358979323846
#define TWO_PI (2.0 * PI)

/* Helper function to generate a sine wave sample*/
static float sine_wave(float phase) {
    return sinf(phase);
}

/* Helper function to generate a triangle wave sample*/
static float triangle_wave(float phase) {
    float normalized_phase;
    normalized_phase = fmodf(phase, TWO_PI) / TWO_PI;
    return 2.0f * fabsf(2.0f * (normalized_phase - floorf(normalized_phase + 0.5f))) - 1.0f;
}

/* Helper function to generate white noise*/
static float white_noise(void) {
    return (float)rand() / RAND_MAX * 2.0f - 1.0f;
}

/* Helper function to apply exponential decay envelope*/
static float apply_decay(float sample, int current_sample, int total_samples, float decay_factor) {
    return sample * powf(decay_factor, (float)current_sample / total_samples);
}

/* Helper function to apply musical note envelope - currently unused but kept for future use */
static float apply_note_envelope(float sample, float percent_through_note) __attribute__((unused));
static float apply_note_envelope(float sample, float percent_through_note) {
    float amplitude_multiplier;
    
    amplitude_multiplier = 1.0f;
    
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

/* DRUM SOUNDS*/

void generate_boom(int16_t *buffer, int num_samples, float frequency) {
    float phase;
    float phase_step;
    float decay_factor;
    float sample;
    int i;
    
    phase = 0.0f;
    phase_step = TWO_PI * frequency / SAMPLE_RATE;
    decay_factor = 0.001f; /* Fast decay for kick drum*/
    
    for (i = 0; i < num_samples; i++) {
        sample = sine_wave(phase);
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE);
        phase += phase_step;
    }
}

void generate_tsst(int16_t *buffer, int num_samples, float frequency) {
    float decay_factor;
    float sample;
    int i;
    
    decay_factor = 0.0001f; /* Very fast decay for hi-hat*/
    
    for (i = 0; i < num_samples; i++) {
        sample = white_noise();
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE * 0.7f); /* Slightly reduced amplitude for high frequencies*/
    }
}

void generate_clap(int16_t *buffer, int num_samples, float frequency) {
    float decay_factor;
    float sample;
    int i;
    
    decay_factor = 0.01f; /* Slower decay for clap reverb*/
    
    for (i = 0; i < num_samples; i++) {
        sample = white_noise();
        /* Simple band-pass simulation by mixing noise with a sine wave*/
        sample = (sample * 0.5f + sine_wave(TWO_PI * frequency * i / SAMPLE_RATE) * 0.5f);
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE * 0.8f);
    }
}

void generate_crash(int16_t *buffer, int num_samples) {
    float decay_factor;
    float sample;
    int i;
    
    decay_factor = 0.05f; /* Slow decay for crash*/
    
    for (i = 0; i < num_samples; i++) {
        sample = white_noise();
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE * 0.6f);
    }
}

void generate_rest(int16_t *buffer, int num_samples) {
    int i;
    
    for (i = 0; i < num_samples; i++) {
        buffer[i] = 0;
    }
}

void generate_floortom(int16_t *buffer, int num_samples, float base_freq) {
    float phase1;
    float phase2;
    float phase_step1;
    float phase_step2;
    float decay_factor;
    float s1;
    float s2;
    float noise;
    float sample;
    int i;

    phase1 = 0.0f;
    phase2 = 0.0f;
    phase_step1 = TWO_PI * base_freq / SAMPLE_RATE;
    phase_step2 = TWO_PI * base_freq * 1.5f / SAMPLE_RATE;
    decay_factor = 0.002f;

    for (i = 0; i < num_samples; i++) {
        s1 = sine_wave(phase1);
        s2 = triangle_wave(phase2);
        noise = white_noise();

        sample = (s1 * 0.7f + s2 * 0.2f + noise * 0.1f);
        sample = apply_decay(sample, i, num_samples, decay_factor);

        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE);

        phase1 += phase_step1;
        phase2 += phase_step2;
    }
}

/* TRIANGLE SOUNDS*/

void generate_ding(int16_t *buffer, int num_samples, float frequency) {
    float phase;
    float phase_step;
    float decay_factor;
    float sample;
    int i;
    
    phase = 0.0f;
    phase_step = TWO_PI * frequency / SAMPLE_RATE;
    decay_factor = 0.01f;
    
    for (i = 0; i < num_samples; i++) {
        sample = triangle_wave(phase);
        sample = apply_decay(sample, i, num_samples, decay_factor);
        buffer[i] = (int16_t)(sample * MAX_AMPLITUDE);
        phase += phase_step;
    }
}

void generate_diding(int16_t *buffer, int num_samples, float frequency) {
    int half_samples;
    
    half_samples = num_samples / 2;
    
    /* First 'di'*/
    generate_ding(buffer, half_samples, frequency);
    
    /* Second 'ding' (slightly higher pitch)*/
    generate_ding(buffer + half_samples, num_samples - half_samples, frequency * 1.1f);
}

void generate_dididing(int16_t *buffer, int num_samples, float frequency) {
    int third_samples;
    
    /* Split the beat into 3. Basically allows for 3 dings in one stretch. 
       An alternative can be to parallelize 3 dings into 2*buffer with 3 different start points.*/
    third_samples = num_samples / 3;
    
    /* First 'di'*/
    generate_ding(buffer, third_samples, frequency);
    
    /* Second 'di' (slightly higher pitch)*/
    generate_ding(buffer + third_samples, third_samples, frequency * 1.1f);
    
    /* Final 'ding' (back to original pitch)*/
    generate_ding(buffer + (2 * third_samples), num_samples - (2 * third_samples), frequency);
}
