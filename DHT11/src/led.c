#include "stm32l476xx.h"

int init_led(GPIO_TypeDef* gpio, int led_pin){
	//Enable AHB2 Clock
	if(gpio==GPIOA)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	}
	else if(gpio==GPIOB)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	}
	else
	{
		//Error! Add other cases to suit other GPIO pins
		return -1;
	}

	//Set GPIO pins to output mode (01)
	//First Clear bits(&) then set bits (|)
	gpio->MODER &= ~(0b11<<(2*led_pin));
	gpio->MODER |= (0b01<<(2*led_pin));

	return 0;
}
