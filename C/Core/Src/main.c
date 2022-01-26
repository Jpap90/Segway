#include "stm32g0xx.h"

int c = 0;

int main(void)
{
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~(0b11 << GPIO_MODER_MODE4_Pos);
	GPIOA->MODER |= 0b01 << GPIO_MODER_MODE4_Pos;

	GPIOA->MODER &= ~(0b11 << GPIO_MODER_MODE0_Pos);
	GPIOA->MODER |= 0b01 << GPIO_MODER_MODE0_Pos;
	//zum einschalten: GPIOA->ODR |= 1 << 0;
	//zum auschalten: GPIOA->ODR &= ~(1 << 0);

	GPIOA->MODER &= ~(0b11 << GPIO_MODER_MODE1_Pos);
	GPIOA->MODER |= 0b01 << GPIO_MODER_MODE1_Pos;
	//zum einschalten: GPIOA->ODR |= 1 << 1;
	//zum auschalten: GPIOA->ODR &= ~(1 << 1);

	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~(0b11 << GPIO_MODER_MODE8_Pos);
	GPIOA->MODER |= 0b10 << GPIO_MODER_MODE8_Pos; 	// PA8: Alternate function
	GPIOA->AFR[1] |= 2 << GPIO_AFRH_AFSEL8_Pos; 	// PA8: AF2 (TIM1_CH1)

	RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
	TIM1->CCMR1 = 0 << TIM_CCMR1_CC1S_Pos |			// CC1: output
       		       0b110 << TIM_CCMR1_OC1M_Pos;		// CC1: PWM mode 1
	TIM1->CCER = TIM_CCER_CC1E;						// CC1: Compare enabled
	TIM1->PSC = 0;
	TIM1->ARR = 800;								// 1/20 kHz @ 16 MHz
	TIM1->CCR1 = 400;						// Tastverhaeltnis 25 %
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CR1 = TIM_CR1_CEN;

	RCC->APBENR1 |= RCC_APBENR1_TIM3EN;
	TIM3->PSC = 15;
	TIM3->ARR = 0xFFFF; // 0,5 s @ 16 MHz
	TIM3->CR1 = TIM_CR1_CEN;
	for(;;) {
		GPIOA->ODR |= 1 << 4;			//Handshake = 1
		while(TIM3->CNT < 3000){
			c = TIM3->CNT;
		}
		GPIOA->ODR &= ~(1 << 4);		//Handshake = 0
		while(TIM3->CNT < 10000){
			c = TIM3->CNT;
		}
		TIM3->CNT = 0;
	}
}
