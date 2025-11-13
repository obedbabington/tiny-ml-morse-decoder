/**
 * @file morse_decode.c
 * @brief Morse code decoder running embedded neural network inference.
 */
#include "main.h"
#include "morse_decode.h"
#include "morse_encode.h" // We need this for sendString, sendFloat, and resetState
#include <math.h>
#include <stdio.h>
#include <string.h>

/*-------------------------- Layer Buffers ----------------------------------*/
// Buffers to hold the intermediate results (activations)
static float layer1_output[HIDDEN_SIZE1];
static float layer2_output[HIDDEN_SIZE2];
static float layer3_output[OUTPUT_SIZE];
/*---------------------------------------------------------------------------*/

/*-------------------------- Helper Functions -------------------------------*/
// (Your relu, softmax, normalize functions)

void normalize_input(float* input, float* normalized) {
    for (int i = 0; i < INPUT_SIZE; i++) {
        normalized[i] = (input[i] - input_mean[i]) / input_scale[i];
    }
}

void relu(float* data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        if (data[i] < 0) data[i] = 0;
    }
}

void softmax(float* input, uint32_t size) {
    float max_val = input[0];
    float sum = 0.0f;
    for (uint32_t i = 1; i < size; i++) {
        if (input[i] > max_val) max_val = input[i];
    }
    for (uint32_t i = 0; i < size; i++) {
        input[i] = expf(input[i] - max_val);
        sum += input[i];
    }
    for (uint32_t i = 0; i < size; i++) {
        input[i] /= sum;
    }
}

/*-------------------------- Layer Functions --------------------------------*/
// (Your dense_layer1, dense_layer2, dense_layer3 functions)

void dense_layer1(float* input, float* output) {
    for (int i = 0; i < HIDDEN_SIZE1; i++) {
        float sum = biases1[i];
        for (int j = 0; j < INPUT_SIZE; j++) {
            sum += input[j] * weights1[j * HIDDEN_SIZE1 + i];
        }
        output[i] = sum;
    }
    relu(output, HIDDEN_SIZE1);
}

void dense_layer2(float* input, float* output) {
    for (int i = 0; i < HIDDEN_SIZE2; i++) {
        float sum = biases2[i];
        for (int j = 0; j < HIDDEN_SIZE1; j++) {
            sum += input[j] * weights2[j * HIDDEN_SIZE2 + i];
        }
        output[i] = sum;
    }
    relu(output, HIDDEN_SIZE2);
}

void dense_layer3(float* input, float* output) {
    for (int i = 0; i < OUTPUT_SIZE; i++) {
        float sum = biases3[i];
        for (int j = 0; j < HIDDEN_SIZE2; j++) {
            sum += input[j] * weights3[j * OUTPUT_SIZE + i];
        }
        output[i] = sum;
    }
}

/*-------------------------- Inference Routine ------------------------------*/

inference_result_t run_inference(float* input_timing) {
    inference_result_t result;
    float normalized_input[INPUT_SIZE];

    __disable_irq();
    TIM2->CNT = 0;
    TIM2->CR1 |= TIM_CR1_CEN;

    // Neural network forward pass
    normalize_input(input_timing, normalized_input);
    dense_layer1(normalized_input, layer1_output);
    dense_layer2(layer1_output, layer2_output);
    dense_layer3(layer2_output, layer3_output);
    softmax(layer3_output, OUTPUT_SIZE);

    // Stop timing
    result.inference_time = TIM2->CNT;
    TIM2->CR1 &= ~TIM_CR1_CEN;
    __enable_irq();

    // Identify most probable class
    float max_prob = layer3_output[0];
    int max_idx = 0;
    for (int i = 1; i < OUTPUT_SIZE; i++) {
        if (layer3_output[i] > max_prob) {
            max_prob = layer3_output[i];
            max_idx = i;
        }
    }

    result.letter = (max_idx < 10) ? ('A' + max_idx) : '?'; // A-J, else '?'
    result.confidence = max_prob;

    return result;
}
/*---------------------------------------------------------------------------*/

/*-------------------------- UART Utility Functions -------------------------*/
void sendFloat(float fval) {
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%.1f", fval);
    sendString(buffer);
}
/*---------------------------------------------------------------------------*/

/*-------------------------- Display Result ---------------------------------*/
void displayResult(inference_result_t result) {
    sendString("Predicted Letter: ");
    sendChar(result.letter);

    sendString("\nConfidence: ");
    sendFloat(result.confidence * 100.0f); // Show as percentage
    sendString(" %");

    sendString("\nInference Speed: ");
    sendFloat(result.inference_time);
    sendString(" us");

    sendString("\n---------------------------------------\n");

    // IMPORTANT: Reset the state for the next letter
    resetState();
}
/*---------------------------------------------------------------------------*/
