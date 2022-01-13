#include "stm32g0xx.h"

int a = 0;
int b = 0;

int main(void)
{
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~(0b11 << GPIO_MODER_MODE8_Pos);
	GPIOA->MODER |= 0b10 << GPIO_MODER_MODE8_Pos; 	// PA8: Alternate function
	GPIOA->AFR[1] |= 2 << GPIO_AFRH_AFSEL8_Pos; 	// PA8: AF2 (TIM1_CH1)

	TIM1->CCER = TIM_CCER_CC1E;			// CC1: Compare enabled
	TIM1->PSC = 15;
	TIM1->ARR = 0xFFFF;
	TIM1->CR1 = TIM_CR1_CEN;
	for(;;) {
		TIM1->CNT = 0;
		while(TIM1->CNT < 3000){
			a++;
		}
		while(TIM1->CNT < 10000){
					b++;
		}
		}
	}

