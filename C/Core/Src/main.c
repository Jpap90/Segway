#include "stm32g0xx.h"
volatile float vist; //global deklariert, fuer Anzeige beim Debuggen
volatile float phi; //global deklariert, fuer Anzeige beim Debuggen

float i_reg=0;		//Integrator für Cascade Control, Initialisierung

const float Ta=0.01; 	//Periodendauer der Regelschleife (10msek)
const float Ti=0.1 ;	//I-Zeitkonst. Kaskadenregler
const float Kr=2/0.216;	//Verstärkung Kaskadenregler n/Ks

//_______________________________________________________________________________________
//Function UART auslesen
int __io_getchar(void) {
	while ((USART1->ISR & USART_ISR_RXNE_RXFNE) == 0){}//solange Empfangsbit RXNE==0, wird gewartet
	return USART1->RDR; //Rueckgabe des empfangenen Bytes
}
//Function Spannung an die Motoren ausgeben u_A/u_B... gewünschte Spannung
void volt2bridge(float u_A, float u_B){
	if (u_A>0){GPIOA->ODR |= (1 << 0);}else{GPIOA->ODR &= ~(1 << 0);} //PA0 (DIRA)
	if (u_A<0){u_A*=-1.0f;} 	//absoluter Wert
	u_A *= 799/12;				//Spannung an PWM-Auflösung anpassen
	if (u_A>799){u_A=799;}		//PWM kann maximal 799 ausgeben
	short PWM = u_A;			//16-Bit Zahl
	TIM1->CCR1 = PWM;

	if (u_B>0){GPIOA->ODR |= (1 << 1);}else{GPIOA->ODR &= ~(1 << 1);} //PA1 (DIRB)
	if (u_B<0){u_B*=-1.0f;} 	//absoluter Wert
	u_B *= 799/12;				//Spannung an PWM-Auflösung anpassen
	if (u_B>799){u_B=799;}		//PWM kann maximal 799 ausgeben
	PWM = u_B;					//16-Bit Zahl, (short)
	TIM1->CCR2 = PWM;
}

int main(void) {
	//GPIOs config
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN; //Clock enable Port A
	GPIOA->MODER &= ~(0b11 << (2*0));
	GPIOA->MODER |= 0b01 << (2*0); //ModeRegister: PA0 '01' (=output) setzen, DIRA
	GPIOA->MODER &= ~(0b11 << (2*1));
	GPIOA->MODER |= 0b01 << (2*1); //ModeRegister: PA1 '01' (=output) setzen, DIRB
	GPIOA->MODER &= ~(0b11 << (2*4));
	GPIOA->MODER |= 0b01 << (2*4); //ModeRegister: PA4 '01' (=output) setzen, HANDSHAKE
	GPIOA->MODER &= ~(0b11 << (2*5));
	GPIOA->MODER |= 0b00 << (2*5); //ModeRegister: PA5 '00' (=input) setzen, RESET
	GPIOA->PUPDR |= 0b01 << (2*5); //Pullup: PA5 '01' (=PullUp) setzen, RESET


	//Timer 3 config
	RCC->APBENR1 |= RCC_APBENR1_TIM3EN; //Timer 3 einschalten?
	TIM3->PSC = 15; //Prescaler auf 15+1, Zaehltakt auf 1 us
	TIM3->ARR = 0xFFFF;  //höchster Zaehlwert
	TIM3->CR1 = TIM_CR1_CEN; //Timer enable

	//U(S)ART Config Pin PA10, USART1
	    //RCC->IOPENR |= RCC_IOPENR_GPIOAEN; //Clock enable Port A, siehe oben
	GPIOA->MODER &= ~GPIO_MODER_MODE10_Msk;
	GPIOA->MODER |= (0b10 << GPIO_MODER_MODE10_Pos); //  PA10: Alternate function
	GPIOA->AFR[1] |= (1 << GPIO_AFRH_AFSEL10_Pos); //  PA10: AF1 = USART1/RX
	RCC->APBENR2 |= RCC_APBENR2_USART1EN;
	USART1->BRR = 278; // 57.6kBaud @ 16 MHz, Baud Rate Register
	USART1->CR1 = USART_CR1_RE | USART_CR1_UE;// USART EnableControl Register, Receiver Enable,

    //INIT PA8 (D9) /PA9 (D5)  =PWMA/PWMB
	GPIOA->MODER &= ~(0b11 << GPIO_MODER_MODE8_Pos);
	GPIOA->MODER |= 0b10 << GPIO_MODER_MODE8_Pos; 	// PA8: Alternate function
	GPIOA->AFR[1] |= 2 << GPIO_AFRH_AFSEL8_Pos; 	// PA8: AF2 (TIM1_CH1)
	GPIOA->MODER &= ~(0b11 << GPIO_MODER_MODE9_Pos);
	GPIOA->MODER |= 0b10 << GPIO_MODER_MODE9_Pos; 	// PA9: Alternate function
	GPIOA->AFR[1] |= 2 << GPIO_AFRH_AFSEL9_Pos; 	// PA9: AF2 (TIM1_CH2)

	RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
	TIM1->CCMR1 |= 0 << TIM_CCMR1_CC1S_Pos |	// CC1: output
				0b110 << TIM_CCMR1_OC1M_Pos;	// CC1: PWM mode 1
	TIM1->CCMR1 |= 0 << TIM_CCMR1_CC2S_Pos |	// CC2: output
				0b110 << TIM_CCMR1_OC2M_Pos;	// CC2: PWM mode 1
	TIM1->CCER |= TIM_CCER_CC1E;			// CC1: Compare enabled
	TIM1->CCER |= TIM_CCER_CC2E;			// CC2: Compare enabled
	TIM1->PSC = 0;							// Prescaler
	TIM1->ARR = 799;				        // 20 kHz @ 16 MHz
	TIM1->CCR1 = 0;				// Tastverhaeltnis D9/PWMA 0...799
	TIM1->CCR2 = 0;				// Tastverhaeltnis D5/PWMB 0...799
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CR1 = TIM_CR1_CEN;			//Timer enable

	//______________________________________________________________________________________________________
	//RESET auswerten: zuerst LOW, dann HIGH
	while((GPIOA->IDR & (1<<5))>0){} //warten, solange Reset auf HIGH
	while((GPIOA->IDR & (1<<5))==0){} //warten, solange Reset auf LOW
//________________________________________________________________________________
	while(1){    //Beginn Endlosschleife
		TIM3->CNT = 0;   //Tim3 auf 0 setzen

		GPIOA->ODR |= (1 << 4); //OR; von Bit 4 = Handshake setzen

		while(TIM3->CNT < 3000) {}  // 3000 us spaeter
		GPIOA->ODR &= ~(1 << 4); //AND; von Bit 4 = Handshake löschen

		uint8_t recv[8];
		for (unsigned int i = 0; i < 8; i++){
			recv[i] = __io_getchar(); //Messwerte über UART einlesen
		}
		//phi = (recv[0]+256*recv[1]-32768) / 1024.0f; //phi(t), in [°]
		//float sr=( recv[2]+256* recv[3]-32768)*0.00452;    //s(t) rechtes Rad [m]
		vist=( recv[4]+256* recv[5]-32768)/10000.0f; //v(t) rechtes Rad [m/sek]
		//float sl=( recv[6]+256* recv[7]-32768)*0.00452;    //s(t) linkes Rad [m]

float vsoll=0.5; // Vorgabe der Motorgeschwindigkeit
//_____________________________________________________________________________________
//Motor-Drehzahl-Regelung (Kaskadenregler)...benötigt vsoll(t)
		float e = vsoll - vist; 		//Regelabweichung e
		      i_reg = i_reg + e;		//Integrator des PI-Reglers
		float u = (e+(i_reg*Ta/Ti))*Kr; //Stellgröße Spannung in [V]

		volt2bridge(u, -u);	//Ausgabe an die H-Brücken (rechtes Rad, linkes Rad(das dreht sich rueckwaerts :))
		while(TIM3->CNT < 10000) {}  // 10ms spaeter
	}//Ende While...Endlosschleie

}
