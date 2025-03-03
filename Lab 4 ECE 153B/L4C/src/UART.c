#include "UART.h"

void UART2_Init(void) {
	//enable clock for usart2
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
	
	//peripherals independent clock config reg for usart2
	RCC->CCIPR |= RCC_CCIPR_USART2SEL_0;
}

void UART2_GPIO_Init(void) {
	RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	//set pA3 to alt function mode
	GPIOA -> MODER &= ~GPIO_MODER_MODE3_0;
	GPIOA -> MODER |= GPIO_MODER_MODE3_1;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFSEL3_0;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFSEL3_1;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFSEL3_2;
	GPIOA -> AFR[0] &= ~GPIO_AFRL_AFSEL3_3;
	
	//set pA2 to alt function mode
	GPIOA -> MODER &= ~GPIO_MODER_MODE2_0;
	GPIOA -> MODER |= GPIO_MODER_MODE2_1;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFSEL2_0;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFSEL2_1;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFSEL2_2;
	GPIOA -> AFR[0] &= ~GPIO_AFRL_AFSEL2_3;
	
	//set to very high speed
	GPIOA -> OSPEEDR |= GPIO_OSPEEDR_OSPEED2;
	GPIOA -> OSPEEDR |= GPIO_OSPEEDR_OSPEED3;
	
	//set to pull up
	GPIOA -> PUPDR &= ~GPIO_PUPDR_PUPD2;
	GPIOA -> PUPDR |= GPIO_PUPDR_PUPD2_0;
	GPIOA -> PUPDR &= ~GPIO_PUPDR_PUPD3;
	GPIOA -> PUPDR |= GPIO_PUPDR_PUPD3_0;
}

void USART_Init(USART_TypeDef* USARTx) {
	//disable usart
	USARTx -> CR1 &= ~USART_CR1_UE; 
	
	//word length:8, oversampling: 16, stop bit:1
	USARTx -> CR1 &= ~USART_CR1_M;
	USARTx -> CR1 &= ~USART_CR1_OVER8;
	USARTx -> CR2 &= ~USART_CR2_STOP;
	
	//enable transmitter and receiver
	USARTx -> CR1 |= USART_CR1_TE;
	USARTx -> CR1 |= USART_CR1_RE;
	
	//enable usart
	USARTx -> CR1 |= USART_CR1_UE;
	
	//baud rate: 9600
	USARTx -> BRR = 80000000 / 9600;
}

uint8_t USART_Read (USART_TypeDef * USARTx) {
	// SR_RXNE (Read data register not empty) bit is set by hardware
	while (!(USARTx->ISR & USART_ISR_RXNE));  // Wait until RXNE (RX not empty) bit is set
	// USART resets the RXNE flag automatically after reading DR
	return ((uint8_t)(USARTx->RDR & 0xFF));
	// Reading USART_DR automatically clears the RXNE flag 
}

void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes) {
	int i;
	// TXE is cleared by a write to the USART_DR register.
	// TXE is set by hardware when the content of the TDR 
	// register has been transferred into the shift register.
	for (i = 0; i < nBytes; i++) {
		while (!(USARTx->ISR & USART_ISR_TXE));   	// wait until TXE (TX empty) bit is set
		// Writing USART_DR automatically clears the TXE flag 	
		USARTx->TDR = buffer[i] & 0xFF;
		USART_Delay(300);
	}
	while (!(USARTx->ISR & USART_ISR_TC));   		  // wait until TC bit is set
	USARTx->ISR &= ~USART_ISR_TC;
}   

void USART_Delay(uint32_t us) {
	uint32_t time = 100*us/7;    
	while(--time);   
}
