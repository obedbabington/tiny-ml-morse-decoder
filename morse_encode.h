#ifndef MORSE_ENCODE_H
#define MORSE_ENCODE_H

#include <stdint.h>

/* ----------------------------- Configuration ----------------------------- */
#define MAX_SIGNALS 4      // Max number of signals (presses) per letter
#define TIMEOUT_US 2000000  // 2 seconds timeout (microseconds)

/* -------------------------- Global Variables ----------------------------- */
// 'extern' tells other files that these variables exist somewhere else
extern volatile uint8_t g_sendFlag;
extern volatile uint8_t g_nextSignal;
extern float g_signalSequence[MAX_SIGNALS];

/* ------------------------ Function Prototypes ---------------------------- */

// System & Peripheral Initialization
void SystemClock_Config(void);
void GPIO_Init(void);
void EXTI_Init(void);
void TIM2_Init(void);
void USART2_Init(void);

// Data Handling Functions
void sendChar(char c);
void sendString(char* str);
void sendRawTimings(void);
void resetState(void);

#endif // MORSE_ENCODE_H
