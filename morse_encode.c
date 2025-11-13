/**
 * @file morse_encode.c
 * @brief Hardware driver for Morse code capture.
 *
 * Handles button interrupts, timer logic, and basic UART output.
 */

#include "morse_encode.h"
#include "main.h"
#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>

/* -------------------------- Global Variables -------------------------- */
// These variables are DEFINED here.
volatile uint8_t g_sendFlag = 0;
volatile uint8_t g_nextSignal = 0;
float g_signalSequence[MAX_SIGNALS] = {0};

/* -------------------------- System Configuration -------------------------- */
// (Your SystemClock_Config function)
void SystemClock_Config(void) {
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    RCC->CFGR &= ~RCC_CFGR_PLLSRC;
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV;
    RCC->CFGR &= ~RCC_CFGR_PLLMUL;
    RCC->CFGR |= RCC_CFGR_PLLMUL6;      // 8 MHz * 6 = 48 MHz

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    FLASH->ACR |= FLASH_ACR_LATENCY;
}

/* ------------------------------- Peripherals ------------------------------ */
// (Your GPIO_Init, EXTI_Init, TIM2_Init, USART2_Init functions)
void GPIO_Init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN;

    // PC13: Input (Button) with pull-up (active LOW)
    GPIOC->MODER &= ~GPIO_MODER_MODER13;
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR13;
    GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_0;

    // PA5: Output (LED indicator)
    GPIOA->MODER &= ~GPIO_MODER_MODER5;
    GPIOA->MODER |= GPIO_MODER_MODER5_0;
    GPIOA->BSRR = GPIO_BSRR_BS_5; // LED off

    // PA2: Alternate function (USART2 TX)
    GPIOA->MODER &= ~GPIO_MODER_MODER2;
    GPIOA->MODER |= GPIO_MODER_MODER2_1;
    GPIOA->AFR[0] &= ~(0xF << (4 * 2));
    GPIOA->AFR[0] |= (1 << (4 * 2)); // AF1 = USART2_TX
}

void EXTI_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;

    EXTI->IMR  |= EXTI_IMR_IM13;
    EXTI->RTSR |= EXTI_RTSR_TR13;
    EXTI->FTSR |= EXTI_FTSR_TR13;

    NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void TIM2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 48 - 1;       // 1 µs per tick (48 MHz / 48)
    TIM2->ARR = 0xFFFFFFFF;
    NVIC_EnableIRQ(TIM2_IRQn);
}

void USART2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    USART2->BRR = 0x1A1;       // 48 MHz / 115200
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

/* ------------------------------- Interrupts ------------------------------- */
// (Your EXTI4_15_IRQHandler and TIM2_IRQHandler functions)
void EXTI4_15_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR13) {
        EXTI->PR = EXTI_PR_PR13; // Clear interrupt

        for (volatile int i = 0; i < 1000; i++); // Debounce

        if (!(GPIOC->IDR & GPIO_IDR_13)) { // Button pressed
            TIM2->CNT = 0;
            TIM2->CR1 |= TIM_CR1_CEN;
        }
        else { // Button released
            uint32_t duration = TIM2->CNT;
            TIM2->CR1 &= ~TIM_CR1_CEN;

            if (g_nextSignal < MAX_SIGNALS)
                g_signalSequence[g_nextSignal++] = duration;

            TIM2->CNT = 0;
            TIM2->ARR = TIMEOUT_US - 1;
            TIM2->SR &= ~TIM_SR_UIF;
            TIM2->DIER |= TIM_DIER_UIE;
            TIM2->CR1 |= TIM_CR1_CEN;
        }
    }
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        TIM2->DIER &= ~TIM_DIER_UIE;
        TIM2->CR1 &= ~TIM_CR1_CEN;
        g_sendFlag = 1;
    }
}

/* ------------------------------ Data Handling ----------------------------- */

void sendChar(char c) {
    USART2->TDR = c;
    while (!(USART2->ISR & USART_ISR_TXE));
}

void sendString(char* str) {
    while (*str != '\0') {
        sendChar(*str);
        str++;
    }
}

/**
 * @brief Sends the raw timing data over UART *without* resetting.
 */
void sendRawTimings(void) {
    __disable_irq();
    GPIOA->BSRR = GPIO_BSRR_BR_5; // LED ON

    char buffer[12];
    // Send the first 4 signals (assuming NN input size is 4)
    for (uint8_t i = 0; i < 4; i++) {
        snprintf(buffer, sizeof(buffer), "%.1f", g_signalSequence[i]);
        for (char *p = buffer; *p; p++) sendChar(*p);
        if (i < 3) sendChar(',');
    }
    sendChar('\n');

    GPIOA->BSRR = GPIO_BSRR_BS_5; // LED OFF
    __enable_irq();
}

/**
 * @brief Resets the signal buffer and state.
 */
void resetState(void) {
    __disable_irq();
    g_nextSignal = 0;
    memset((void*)g_signalSequence, 0, sizeof(g_signalSequence));
    GPIOA->BSRR = GPIO_BSRR_BS_5; // Ensure LED is off
    __enable_irq();
}
