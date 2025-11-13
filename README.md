# MorseAI Workshop: From Taps to Text 🧠

Welcome to the **Morse Code AI Workshop**!

This repository contains all the code you need to build, train, and deploy a tiny Neural Network that can decode your Morse code taps in real-time on an STM32 microcontroller.

The project is broken down into three main parts, which you can follow along in order.

---

## 🎯 Workshop Flow

### 1. Developing the Model 🧠

Before we can run a Neural Network, we need to build one! This step is done in **Python** using Google Colab. We'll load a dataset of Morse code timings, build a simple NN, and train it to recognize the letters **A** through **J**.

**File:** `MorseAI_Workshop_Notebook.ipynb`  
**What it does:** This interactive notebook guides you through all 10 steps of the AI development process, from loading data to training the model and exporting the final weights for our C code.

---

### 2. Encoding the Morse Signals 💻

This is the "hardware" part of our project. These files set up the STM32's button, timer, and UART (serial port). Their job is to listen for your button presses and "encode" them into a sequence of timings.

**`Core/Src/morse_encode.h`**  
*What it is:* The "header" file. Like a table of contents for our hardware driver, telling other files what functions are available (like `TIM2_Init()` or `sendString()`).

**`Core/Src/morse_encode.c`**  
*What it is:* The "implementation" file. Low-level code that configures the hardware and handles the button press interrupts.

**`Core/Src/main.c`**  
*What it is:* The heart of the application. Initializes the system and contains the main `while(1)` loop. Acts as the "conductor" that waits for a signal from `morse_encode.c` and then decides what to do.

---

### 3. Decoding the Signals 🚀

This is where the magic happens! These files contain the Neural Network model trained in Step 1, "hard-coded" in C. This code takes the timing sequence from the Encode step and runs inference to decode it into a letter.

**`Core/Src/morse_decode.h`**  
*What it is:* The header file for the neural network. Defines the `run_inference()` function so `main.c` can call it.

**`Core/Src/morse_decode.c`**  
*What it is:* The Neural Network itself! Contains all the math (like `dense_layer1()`, `relu()`, `softmax()`) that performs the AI prediction.

**`Core/Src/morse_nn_model.h`**  
*What it is:* The model's "brain." Holds the weights and biases exported from your Python notebook.

---

## 💡 How to Use This Repository

The most important file for you to edit is `main.c`.

Inside `Core/Src/main.c`, you'll find a **switch** that lets you choose the workshop mode. This is the easiest way to see what's going on.

Look for these lines:

```c
/*
 * -----------------------------------------------------------------------------
 * WORKSHOP CONFIGURATION
 * -----------------------------------------------------------------------------
 *
 * Change the WORKSHOP_MODE define to switch between behaviors.
 */

// *** CHANGE THIS LINE TO SWITCH MODES ***
#define WORKSHOP_MODE MODE_ENCODE_AND_DECODE
// #define WORKSHOP_MODE MODE_ENCODE_ONLY
