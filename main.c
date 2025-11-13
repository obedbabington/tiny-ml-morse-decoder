/**
 * @file main.c
 * @brief Main application for the Morse Code workshop.
 *
 * This file initializes the system and runs one of two modes based
 * on the WORKSHOP_MODE define:
 * 1. MODE_ENCODE_ONLY: Captures button presses, prints the raw timings.
 * 2. MODE_ENCODE_AND_DECODE: Captures timings, prints them, then runs
 * the neural network and prints the prediction.
 */

#include "main.h"
#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>

#include "morse_encode.h"
#include "morse_decode.h"

/*
 * -----------------------------------------------------------------------------
 * WORKSHOP CONFIGURATION
 * -----------------------------------------------------------------------------
 *
 * Change the WORKSHOP_MODE define to switch between behaviors.
 *
 * MODE_ENCODE_ONLY:
 * - Just captures the button presses and prints the raw timings.
 * - Useful for collecting data or showing how the timer works.
 *
 * MODE_ENCODE_AND_DECODE:
 * - Captures timings, prints them, AND feeds them to the neural network
 * to get a predicted letter.
 * - Shows the full end-to-end system.
 */

#define MODE_ENCODE_ONLY       1
#define MODE_ENCODE_AND_DECODE 2

// *** CHANGE THIS LINE TO SWITCH MODES ***
#define WORKSHOP_MODE MODE_ENCODE_AND_DECODE
// #define WORKSHOP_MODE MODE_ENCODE_ONLY

/* -------------------------------------------------------------------------- */


int main(void) {
    // --- 1. Initialize all hardware peripherals ---
    SystemClock_Config();
    GPIO_Init();
    USART2_Init();
    EXTI_Init();
    TIM2_Init();

    // --- 2. Send a welcome message over UART ---
    sendString("\n\n--- Morse Code NN Workshop Initialized ---\n");

    #if WORKSHOP_MODE == MODE_ENCODE_ONLY
        sendString("Mode: ENCODE ONLY\n");
        sendString("Tap a Morse letter (e.g., 'A' = dot, dash). Timings will be printed.\n");
    #elif WORKSHOP_MODE == MODE_ENCODE_AND_DECODE
        sendString("Mode: ENCODE AND DECODE\n");
        sendString("Tap a Morse letter. Timings and NN prediction will be printed.\n");
    #endif
    sendString("---------------------------------------\n");


    // --- 3. Main Application Loop ---
    while (1) {
        // Wait for the sendFlag to be set by the timer interrupt
        // (meaning the user has finished tapping a letter)
        if (g_sendFlag) {

        /* * This is the core logic for the workshop.
         * We use compiler directives to change the code that runs.
         */

        // --- MODE 1: Just show the raw timings ---
        #if WORKSHOP_MODE == MODE_ENCODE_ONLY
            sendString("Raw Timings: ");
            sendRawTimings(); // Print the timings
            resetState();     // Clear the buffer for the next letter
            sendString("---------------------------------------\n");

        // --- MODE 2: Show raw timings AND the NN prediction ---
        #elif WORKSHOP_MODE == MODE_ENCODE_AND_DECODE
            // 1. Show the raw data
            sendString("Raw Timings: ");
            sendRawTimings(); // Print the timings (doesn't reset)

            // 2. Run the NN inference on that data
            inference_result_t result = run_inference(g_signalSequence);

            // 3. Display the NN result (this function also resets the state)
            displayResult(result);
        #endif

            g_sendFlag = 0; // Clear the flag
        }

        // Wait for the next interrupt
        __WFI();
    }
}
