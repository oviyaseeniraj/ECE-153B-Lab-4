#include "I2C.h"

extern void Error_Handler(void);

// Inter-integrated Circuit Interface (I2C)
// up to 100 Kbit/s in the standard mode, 
// up to 400 Kbit/s in the fast mode, and 
// up to 3.4 Mbit/s in the high-speed mode.

// Recommended external pull-up resistance is 
// 4.7 kOmh for low speed, 
// 3.0 kOmh for the standard mode, and 
// 1.0 kOmh for the fast mode
	
//===============================================================================
//                        I2C GPIO Initialization
//===============================================================================
void I2C_GPIO_Init(void) {
	// [TODO]
	RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	
	//PB7 alt function
	GPIOB -> MODER &= ~GPIO_MODER_MODE7;
	GPIOB -> MODER |= GPIO_MODER_MODE7_1;
	GPIOB -> AFR[0] |= GPIO_AFRL_AFSEL7_2;

	//PB6 alt function
	GPIOB -> MODER &= ~GPIO_MODER_MODE6;
	GPIOB -> MODER |= GPIO_MODER_MODE6_1;
	GPIOB -> AFR[0] |= GPIO_AFRL_AFSEL6_2;
	
	//output type: open-drain
	GPIOB -> OTYPER |= GPIO_OTYPER_OT7;
	GPIOB -> OTYPER |= GPIO_OTYPER_OT6;
	
	//output speed: very high
	GPIOB -> OSPEEDR |= GPIO_OSPEEDR_OSPEED7;
	GPIOB -> OSPEEDR |= GPIO_OSPEEDR_OSPEED6;
	
	//pull up mode
	GPIOB -> PUPDR &= ~GPIO_PUPDR_PUPD7;
	GPIOB -> PUPDR |= GPIO_PUPDR_PUPD7_0;
	GPIOB -> PUPDR &= ~GPIO_PUPDR_PUPD6;
	GPIOB -> PUPDR |= GPIO_PUPDR_PUPD6_0;

}
	
#define I2C_TIMINGR_PRESC_POS	28
#define I2C_TIMINGR_SCLDEL_POS	20
#define I2C_TIMINGR_SDADEL_POS	16
#define I2C_TIMINGR_SCLH_POS	8
#define I2C_TIMINGR_SCLL_POS	0

//===============================================================================
//                          I2C Initialization
//===============================================================================
void I2C_Initialization(void){
	uint32_t OwnAddr = 0x52;
	
	// [TODO]
	// 1a
	RCC -> APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

	// 1b
	RCC -> CCIPR &= ~RCC_CCIPR_I2C1SEL;
	RCC -> CCIPR |= RCC_CCIPR_I2C1SEL_0;

	// 1c
	RCC -> APB1RSTR1 |= RCC_APB1RSTR1_I2C1RST;
	RCC -> APB1RSTR1 &= ~RCC_APB1RSTR1_I2C1RST;

	// disable 12c
	I2C1 -> CR1 &= ~I2C_CR1_PE;

	// 2a: analog noise filter on 
	I2C1 -> CR1	&= ~I2C_CR1_ANFOFF;

	// 2a: digital noise filter off 
	I2C1 -> CR1 &= ~I2C_CR1_DNF;

	// 2a: error interrupts on
	I2C1 -> CR1 |= I2C_CR1_ERRIE;

	// 2a: clock stretching on
	I2C1 -> CR1 &= ~I2C_CR1_NOSTRETCH;

	// master in 7-bit addressing mode
	I2C1 -> CR2 &= ~I2C_CR2_ADD10;

	// 2a: automatic end mode and NACK generation
	I2C1 -> CR2 |= I2C_CR2_AUTOEND;
	I2C1 -> CR2 |= I2C_CR2_NACK;

	// 2b
	//I2C1 -> TIMINGR = 0; 
	// PRESC
	I2C1 -> TIMINGR |= 0x7 << I2C_TIMINGR_PRESC_POS; 
	// SCLDEL
	I2C1 -> TIMINGR |= 0x9 << I2C_TIMINGR_SCLDEL_POS; 
	// SDADEL
	I2C1 -> TIMINGR |= 0xC << I2C_TIMINGR_SDADEL_POS; 
	// SCLH
	I2C1 -> TIMINGR |= 0x27 << I2C_TIMINGR_SCLH_POS; 
	// SCLL
	I2C1 -> TIMINGR |= 0x2E <<I2C_TIMINGR_SCLL_POS; 

	// disable 
	I2C1 -> OAR1 &= ~I2C_OAR1_OA1EN;

	// 7-bit mode
	I2C1 -> OAR1 &= ~I2C_OAR1_OA1MODE;

	// address to use
	I2C1 -> OAR1 = OwnAddr << 1;

	// enable
	I2C1 -> OAR1 |= I2C_OAR1_OA1EN;

	// Enable I2C in the control register
	I2C1 -> CR1 |= I2C_CR1_PE;
	
}

//===============================================================================
//                           I2C Start
// Master generates START condition:
//    -- Secondary address: 7 bits
//    -- Automatically generate a STOP condition after all bytes have been transmitted 
// Direction = 0: Master requests a write transfer
// Direction = 1: Master requests a read transfer
//=============================================================================== 
int8_t I2C_Start(I2C_TypeDef * I2Cx, uint32_t DevAddress, uint8_t Size, uint8_t Direction) {	
	
	// Direction = 0: Master requests a write transfer
	// Direction = 1: Master requests a read transfer
	
	uint32_t tmpreg = 0;
	
	// This bit is set by software, and cleared by hardware after the Start followed by the address
	// sequence is sent, by an arbitration loss, by a timeout error detection, or when PE = 0.
	tmpreg = I2Cx->CR2;
	
	tmpreg &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
	
	if (Direction == READ_FROM_SECONDARY)
		tmpreg |= I2C_CR2_RD_WRN;  // Read from Secondary
	else
		tmpreg &= ~I2C_CR2_RD_WRN; // Write to Secondary
		
	tmpreg |= (uint32_t)(((uint32_t)DevAddress & I2C_CR2_SADD) | (((uint32_t)Size << 16 ) & I2C_CR2_NBYTES));
	
	tmpreg |= I2C_CR2_START;
	
	I2Cx->CR2 = tmpreg; 
	
   	return 0;  // Success
}

//===============================================================================
//                           I2C Stop
//=============================================================================== 
void I2C_Stop(I2C_TypeDef * I2Cx){
	// Master: Generate STOP bit after the current byte has been transferred 
	I2Cx->CR2 |= I2C_CR2_STOP;								
	// Wait until STOPF flag is reset
	while( (I2Cx->ISR & I2C_ISR_STOPF) == 0 ); 
}

//===============================================================================
//                           Wait for the bus is ready
//=============================================================================== 
void I2C_WaitLineIdle(I2C_TypeDef * I2Cx){
	// Wait until I2C bus is ready
	while( (I2Cx->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY );	// If busy, wait
}

//===============================================================================
//                           I2C Send Data
//=============================================================================== 
int8_t I2C_SendData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size) {
	int i;
	
	if (Size <= 0 || pData == NULL) return -1;
	
	I2C_WaitLineIdle(I2Cx);
	
	if (I2C_Start(I2Cx, DeviceAddress, Size, WRITE_TO_SECONDARY) < 0 ) return -1;

	// Send Data
	// Write the first data in DR register
	// while((I2Cx->ISR & I2C_ISR_TXE) == 0);
	// I2Cx->TXDR = pData[0] & I2C_TXDR_TXDATA;  

	for (i = 0; i < Size; i++) {
		// TXE is set by hardware when the I2C_TXDR register is empty. It is cleared when the next
		// data to be sent is written in the I2C_TXDR register.
		// while( (I2Cx->ISR & I2C_ISR_TXE) == 0 ); 

		// TXIS bit is set by hardware when the I2C_TXDR register is empty and the data to be
		// transmitted must be written in the I2C_TXDR register. It is cleared when the next data to be
		// sent is written in the I2C_TXDR register.
		// The TXIS flag is not set when a NACK is received.
		while((I2Cx->ISR & I2C_ISR_TXIS) == 0 );
		I2Cx->TXDR = pData[i] & I2C_TXDR_TXDATA;  // TXE is cleared by writing to the TXDR register.
	}
	
	// Wait until TC flag is set 
	while((I2Cx->ISR & I2C_ISR_TC) == 0 && (I2Cx->ISR & I2C_ISR_NACKF) == 0);
	
	if( (I2Cx->ISR & I2C_ISR_NACKF) != 0 ) return -1;

	I2C_Stop(I2Cx);
	return 0;
}


//===============================================================================
//                           I2C Receive Data
//=============================================================================== 
int8_t I2C_ReceiveData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size) {
	int i;
	
	if(Size <= 0 || pData == NULL) return -1;

	I2C_WaitLineIdle(I2Cx);

	I2C_Start(I2Cx, DeviceAddress, Size, READ_FROM_SECONDARY); // 0 = sending data to the secondary, 1 = receiving data from the secondary.
						  	
	for (i = 0; i < Size; i++) {
		// Wait until RXNE flag is set 	
		while( (I2Cx->ISR & I2C_ISR_RXNE) == 0 );
		pData[i] = I2Cx->RXDR & I2C_RXDR_RXDATA;
	}
	
	// Wait until TCR flag is set 
	while((I2Cx->ISR & I2C_ISR_TC) == 0);
	
	I2C_Stop(I2Cx);
	
	return 0;
}
