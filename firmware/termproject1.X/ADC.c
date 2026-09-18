#include <xc.h>
#include <p32xxxx.h>
#include "ADC.h"

// ADC Configuration
void ADC_Init(void)
{
    AD1CON1 = 0x00E0;    // manual sampling, auto conversion
    AD1CON2 = 0x0000;    // AVdd/AVss, MUXA, single sample
    AD1CON3 = 0x1F19;    // sample time & ADC clock
    AD1CHS  = 0x0000;    // default to AN0
    AD1CSSL = 0;         // no scan
    IFS1bits.AD1IF = 0;  // clear ADC interrupt flag
    AD1CON1bits.ADON = 1; // turn ADC on
}

// Read from a given analog channel (AN0 = 0, AN1 = 1, AN2 = 2, etc.)
uint16_t ADC_Read(uint8_t channel)
{
    AD1CHSbits.CH0SA = channel;  // select AN[channel]

    AD1CON1bits.SAMP = 1;        // start sampling
    for (volatile int i = 0; i < 500; i++); // small delay
    AD1CON1bits.SAMP = 0;        // end sampling, start conversion

    while (!AD1CON1bits.DONE);   // wait for conversion complete

    uint16_t value = ADC1BUF0;   // read result
    AD1CON1bits.DONE = 0;        // clear DONE flag

    return value;
}
