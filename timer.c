// Timer support

// Written by Bernie Roehl, March 2017

#include <lpc17xx.h>
#include "type.h"
#include "timer.h"

extern uint32_t SystemCoreClock;

void timer_setup(void) {
	// timer 0 counts in microseconds
	LPC_SC->PCONP |= 0x02;  // enable power to timer 0
	LPC_TIM0->PR = ((SystemCoreClock / 4) / 1000000) - 1;  // assuming default PCLKSEL0 == 0, i.e. divide by 4
	LPC_TIM0->TCR = 1;      // start timer
}

uint32_t timer_read(void) {
	return LPC_TIM0->TC;
}
