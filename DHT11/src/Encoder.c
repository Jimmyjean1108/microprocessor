#include "stm32l476xx.h"

int init_Encoder(GPIO_TypeDef* gpio, int CLK, int DT)
{
	//Enable AHB2 Clock
	if(gpio == GPIOA)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	}
	else if(gpio == GPIOB)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	}
	else if(gpio == GPIOC)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	}
	else
	{
		//Error! Add other cases to suit other GPIO pins
		return -1;
	}
	//Set GPIO pins to intput mode (00)
	//First Clear bits(&) then set bits (|)
	gpio -> MODER &= ~(0b11 << (2*DT));
	gpio -> MODER |= (0b00 << (2*DT));
	gpio -> MODER &= ~(0b11 << (2*CLK));
	gpio -> MODER |= (0b00 << (2*CLK));

	return 0;
}
