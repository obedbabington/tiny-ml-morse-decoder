#ifndef MORSE_DECODE_H
#define MORSE_DECODE_H

#include <stdint.h>
#include "network_parameters.h"


// Network architecture constants
#define INPUT_SIZE 4
#define HIDDEN_SIZE1 16
#define HIDDEN_SIZE2 16
#define OUTPUT_SIZE 11  // 10 letters + unclassified

// Result structure
typedef struct {
    char letter;
    float confidence;
    float inference_time;
} inference_result_t;

// Function declarations
void normalize_input(float* input, float* normalized);
void dense_layer1(float* input, float* output);
void dense_layer2(float* input, float* output);
void dense_layer3(float* input, float* output);
void softmax(float* input, uint32_t size);
void relu(float* data, uint32_t size);
inference_result_t run_inference(float* input_timing);
void sendFloat(float fval);
void sendString(char* str);
void displayResult(inference_result_t result);

#endif
