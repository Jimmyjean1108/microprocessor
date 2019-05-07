#include "stm32l476xx.h"

int init_motor(GPIO_TypeDef* gpio, int enable, int pin1, int pin2){
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
	//Set GPIO pins to output mode (01)
	//First Clear bits(&) then set bits (|)
	gpio->MODER &= ~(0b11 << (2*enable));
	gpio->MODER |= (0b01 << (2*enable));
	gpio->MODER &= ~(0b11 << (2*pin1));
	gpio->MODER |= (0b01 << (2*pin1));
	gpio->MODER &= ~(0b11 << (2*pin2));
	gpio->MODER |= (0b01 << (2*pin2));

	reset_gpio(gpio, enable);
	reset_gpio(gpio, pin1);
	reset_gpio(gpio, pin2);
	return 0;
}

void clockwise_rotate(GPIO_TypeDef* gpio, int enable, int pin1, int pin2){
	set_gpio(gpio,enable);
	set_gpio(gpio,pin1);
	reset_gpio(gpio,pin2);
}

void counterclockwise_rotate(GPIO_TypeDef* gpio, int enable, int pin1, int pin2){
	set_gpio(gpio,enable);
	set_gpio(gpio,pin2);
	reset_gpio(gpio,pin1);
}

void stop_rotate(GPIO_TypeDef* gpio, int enable){
	reset_gpio(gpio,enable);
}
