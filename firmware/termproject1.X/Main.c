// Fall Detection Demo - Negative Y Only
// Authors: Ahmad Awad and Rayan Qouta

#include <p32xxxx.h>
#include <xc.h>
#include "ADC.h"   // assumes ADC_Init() and ADC_Read(uint8_t channel) are implemented

/*** Configuration Bits ***/
#pragma config FNOSC    = FRCPLL       // Internal FRC with PLL
#pragma config FSOSCEN  = OFF
#pragma config POSCMOD  = OFF
#pragma config OSCIOFNC = OFF
#pragma config FPBDIV   = DIV_2        // PBCLK = SYSCLK/2 = 16 MHz
#pragma config FPLLIDIV = DIV_2        // 8 MHz/2 = 4 MHz
#pragma config FPLLMUL  = MUL_16       // 4 MHz x 16 = 64 MHz
#pragma config FPLLODIV = DIV_2        // 64 MHz/2 = 32 MHz SYSCLK
#pragma config FWDTEN   = OFF

/*** Demo constants ***/
#define BASELINE_SAMPLES    32          // samples to average at startup

// Threshold for negative Y fall (in ADC counts).
// Smaller = more sensitive, larger = less sensitive.
// Start around 80-120 and tune in lab.
#define FALL_Y_DELTA        10

// LEDs on PORTD
#define LED_NORMAL_MASK     0x0008      // RD3 ON for "normal"
#define LED_FALL_MASK       0x0001      // RD0 ON for "fall detected"

/*** Simple software delay (busy-wait) ***/
static void delay_soft(unsigned long cycles)
{
    volatile unsigned long i;
    for (i = 0; i < cycles; i++) {
        ; // do nothing
    }
}

int main(void)
{
    /***** Basic setup *****/
    DDPCONbits.JTAGEN = 0;          // disable JTAG so PORT pins are free

    // --- Accelerometer inputs: AN0, AN1, AN2 on PORTB ---
    TRISBbits.TRISB0 = 1;           // RB0 = input (AN0 -> X)
    TRISBbits.TRISB1 = 1;           // RB1 = input (AN1 -> Y)
    TRISBbits.TRISB2 = 1;           // RB2 = input (AN2 -> Z)

    // AN0, AN1, AN2 analog; others digital
    // bits 0-2 = 0 (analog), bits 3-15 = 1 (digital)
    AD1PCFG = 0xFFF8;

    // --- LEDs on PORTD ---
    TRISD = 0x0000;                 // PORTD as outputs
    LATD  = 0x0000;                 // all LEDs off

    // Show "normal" LED at startup
    LATD = LED_NORMAL_MASK;

    /***** Initialize ADC *****/
    ADC_Init();                     // from your ADC.c

    /***** Baseline calibration for Y axis only *****/
    // At startup, assume device is stationary and record average Y.
    unsigned long sumY = 0;

    for (int i = 0; i < BASELINE_SAMPLES; i++) {
        unsigned int ay0 = ADC_Read(1);   // AN1 -> Y
        sumY += ay0;
        delay_soft(200UL);             // short delay between samples
    }

    unsigned int baseY = (unsigned int)(sumY / BASELINE_SAMPLES);

    /***** Main loop - fall detection on negative Y only *****/
    unsigned char fallDetected = 0;

    while (1) {
        // 1) Read Y axis only
        unsigned int ay = ADC_Read(1);   // Y axis (AN1)

        // 2) Compute deviation from baseline
        int dy = (int)ay - (int)baseY;

        // We only care about NEGATIVE Y (fall in -Y direction),
        // so dy will be NEGATIVE when ay < baseY.
        if (!fallDetected) {
            if (dy < 0) {
                // magnitude of negative change
                int negDy = -dy;   // convert to positive magnitude

                // if drop in Y is large enough, latch fall
                if (negDy > FALL_Y_DELTA) {
                    fallDetected = 1;
                }
            }
        }

        // 3) Drive LEDs: exactly ONE LED ON at all times from PORTD
        if (fallDetected) {
            LATD = LED_FALL_MASK;     // RD0 ON, all others OFF
        } else {
            LATD = LED_NORMAL_MASK;   // RD3 ON, all others OFF
        }

        // 4) Slow down loop so LED changes are visible
        delay_soft(50000UL);
    }

    // never reached
    // return 0;
}
