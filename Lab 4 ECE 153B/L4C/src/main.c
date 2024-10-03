/*
 * ECE 153B - Spring 2023
 * Lab: 5A
 */
 
#include "stm32l476xx.h"
#include "SPI.h"
#include "UART.h"
#include "SysClock.h"
#include "SysTimer.h"
#include "accelerometer.h"

#include <stdio.h>



int main(void){
	
	double x,y,z;
	
	System_Clock_Init();   // System Clock = 80 MHz
	SysTick_Init();
	UART2_GPIO_Init();
	UART2_Init();
	USART_Init(USART2);
	
	SPI1_GPIO_Init();
	SPI1_Init();
	initAcc();
	
	while(1) {
		readValues(&x, &y, &z);
		printf("Acceleration: %.2f, %.2f, %.2f\r\n", x, y, z);
		delay(1000);
	}
}
