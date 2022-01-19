#include "stm32g0xx.h"

int a = 0;
int b = 0;
int c = 0;

int main(void)
{
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~(0b11 << GPIO_MODER_MODE4_Pos);
	GPIOA->MODER |= 0b01 << GPIO_MODER_MODE4_Pos;

	RCC->APBENR1 |= RCC_APBENR1_TIM3EN;
	TIM3->PSC = 15;
	TIM3->ARR = 0xFFFF; // 0,5 s @ 16 MHz
	TIM3->CR1 = TIM_CR1_CEN;
	for(;;) {
		GPIOA->ODR &= ~(0 << GPIO_ODR_OD4);			//Handshake = 1
		while(TIM3->CNT < 3000){
			a++;
			c = TIM3->CNT;
		}
		GPIOA->ODR |= 1 << GPIO_ODR_OD4;			//Handshake = 0
		while(TIM3->CNT < 10000){
			b++;
			c = TIM3->CNT;
		}
		TIM3->CNT = 0;
	}
}
