#include "STM32l476xx.h"

void GPIO_init_AF(){
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	//Set to Alternate function mode
	GPIOA->MODER &= ~GPIO_MODER_MODE0_Msk;
	GPIOA->MODER |= (2 << GPIO_MODER_MODE0_Pos);
	//Set AFRL
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL0_Msk;
	GPIOA->AFR[0] |= (1 << GPIO_AFRL_AFSEL0_Pos);
}


void timer_enable(TIM_TypeDef *timer){
	if(timer == TIM2){
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	}
	else if(timer == TIM3){
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;
	}
}

void timer_disable(TIM_TypeDef *timer){
	if(timer == TIM2){
		RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM2EN;
	}
	else if(timer == TIM3){
		RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM3EN;
	}
}


void timer_init(TIM_TypeDef *timer, int psc, int arr){
	//fout =
	timer->PSC = (uint32_t)(psc-1);  //PreScalser
	timer->ARR = (uint32_t)(arr-1);  //Reload value
	timer->EGR |= TIM_EGR_UG;        //Reinitialize the counter

}

void timer_start(TIM_TypeDef *timer){
	timer->CR1 |= TIM_CR1_CEN;
}

void timer_stop(TIM_TypeDef *timer){
	timer->CR1 &= ~TIM_CR1_CEN;
}


void PWM_channel_init(){
	//p.883 915 920 924
	//PA0 for PWM
	//PWM mode 1
	TIM2->CCMR1 &= ~TIM_CCMR1_OC1M_Msk;
	TIM2->CCMR1 |= (6 << TIM_CCMR1_OC1M_Pos);
	//OCPreload_Enable
	TIM2->CCMR1 &= ~TIM_CCMR1_OC1PE_Msk;
	TIM2->CCMR1 |= (1 << TIM_CCMR1_OC1PE_Pos);
	//Active high for channel 1 polarity
	TIM2->CCER &= ~TIM_CCER_CC1P_Msk;
	TIM2->CCER |= (0 << TIM_CCER_CC1P_Pos);
	//Enable for channel 1 output
	TIM2->CCER &= ~TIM_CCER_CC1E_Msk;
	TIM2->CCER |= (1 << TIM_CCER_CC1E_Pos);
	//Set Compare Register
	TIM2->CCR1 = 10;
	//Set PreScaler
	TIM2->PSC = 0;
}
