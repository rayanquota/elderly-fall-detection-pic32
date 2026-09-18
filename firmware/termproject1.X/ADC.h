#ifndef ADC_H
#define ADC_H

#include <stdint.h>   // for uint16_t, uint8_t

void ADC_Init(void);
uint16_t ADC_Read(uint8_t channel);   // read from AN[channel]

#endif
