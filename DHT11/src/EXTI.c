#include "stm32l476xx.h"

void EXTI_Setup(){
	//Enable SYSCFG CLK
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	//Select output bits
	SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13_Msk;
	SYSCFG->EXTICR[3] |= (2 << SYSCFG_EXTICR4_EXTI13_Pos);

	//Enable interrupt
	EXTI->IMR1 |= EXTI_IMR1_IM13;

	//Enable Falling Edge
	EXTI->FTSR1 |= EXTI_FTSR1_FT13;

	//Enable NVIC**
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	//Set Priority of External interrupt & Internal interrupt
	NVIC_SetPriority(EXTI15_10_IRQn,0);
	NVIC_SetPriority(SysTick_IRQn,1);
}

int init_button(GPIO_TypeDef* gpio, int button_pin)
{
	//Enable AHB2 Clock
	if(gpio==GPIOC)
	{
		RCC->AHB2ENR |=RCC_AHB2ENR_GPIOCEN;
	}
	else
	{
		//Error! Add other cases to suit other GPIO pins
		return -1;
	}
	//Set GPIO pins to input mode (00)
	//First Clear bits(&) then set bits (|)
	gpio->MODER &= ~(0b11<<(2*button_pin));
	gpio->MODER |= (0b00<<(2*button_pin));

	return 0;
}
