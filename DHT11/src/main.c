/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2004-2013 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include "STM32l476xx.h"
#include "dht11.h"
#include "7seg.h"

#define DHT11_gpio GPIOB
#define DHT11_pin 6

//Define pins for 7seg
#define SEG_gpio GPIOC //PC port
#define DIN_pin 5//��瞼�祁甄蜆祁捌PC5
#define CS_pin 6//繡礙簧�祁並甄開�� PC6
#define CLK_pin 8//穡繳翹�� PC8

//Define pins for led(default use on-board led PA5)
#define LED_gpio GPIOA
#define LED_pin 5

//Define pins for motor
#define Mot_gpio GPIOA //PA port
#define Enable_pin 1//簞穡繒FEnable
#define Mot_pin1 7//簞穡繒F簡�一穡瞻礎穫
#define Mot_pin2 0//簞穡繒F簡�二穡瞻礎穫

//Define pins for button (default use  on-board button PC13)
#define BUTTON_gpio GPIOC
#define BUTTON_pin 13

#define Ro_gpio GPIOB //PB port
#define RoAPin 1 //CLK_PB1
#define RoBPin 2 //DT_PB2

struct DHT11_Dev dev;

volatile uint32_t msTicks;                      /* counts 1ms timeTicks       */

int globalCounter = 90;
int level = 5;
int flag = 0;
int Last_RoB_Status = 0;
int Current_RoB_Status = 0;

int state=1;
int last_state=2;

//button_press_cycle_per_second (How many button press segments in a second)
int button_press_cycle_per_second = 20;
//Use to state how many cycles to check per button_press_cycle
int debounce_cycles = 50;
//Use to state the threshold when we consider a button press
int debounce_threshold = 35;
/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
  msTicks++;
}

/*----------------------------------------------------------------------------
  delays number of tick Systicks (happens every 1 ms)
 *----------------------------------------------------------------------------*/
void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks){
	  if(state != last_state)
		  break;
  };
}

void delay_without_interrupt(float msec){
	int loop_cnt= 1000*msec;
	while(loop_cnt)
	{
		loop_cnt--;
	}
	return ;
}

void FPU_init(){
	//Setup FPU
	SCB -> CPACR |= (0xF << 20);
	__DSB();
	__ISB();
}

void EXTI15_10_IRQHandler(){
	if(EXTI->PR1&EXTI_PR1_PIF13_Msk){
		for(int a=0; a<button_press_cycle_per_second; a++){
				//Simple Debounce without interrupt
				int pos_cnt = 0;
				for(int a=0; a<debounce_cycles; a++){
					//If button press add count
					if(read_gpio(BUTTON_gpio, BUTTON_pin)==0){
						pos_cnt++;
					}
					delay_without_interrupt(1000/(button_press_cycle_per_second*debounce_cycles));
				}
				//Check if need to change state
				if(pos_cnt>debounce_threshold){
					if(state==1){
						state=2;
						reset_gpio(Mot_gpio,Enable_pin);
					}
					else{
						state=1;
						reset_gpio(Mot_gpio,Enable_pin);
					}
				}
				else{
				}
			}
		EXTI->PR1 = EXTI_PR1_PIF13_Msk;
	}
}

void rotaryDeal(){
	Last_RoB_Status = read_gpio(Ro_gpio, RoBPin);
	while(!(read_gpio(Ro_gpio, RoAPin))){
		Current_RoB_Status =read_gpio(Ro_gpio, RoBPin);
		flag = 1;
	}
	if(flag==1){
		flag = 0;
		if ((Last_RoB_Status == 0) && (Current_RoB_Status == 1)){
			if(globalCounter < 100){
				globalCounter = globalCounter + 10;
				level++;
				display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin, level, num_digits(level));
			}
			else
				display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin, level, num_digits(level));
		}
		if ((Last_RoB_Status == 1) && (Current_RoB_Status == 0)){
			if(globalCounter >40){
				globalCounter = globalCounter - 10;
				level--;
				display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin, level, num_digits(level));
			}
			else
				display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin, level, num_digits(level));
		}
	}
}


int DHT11_init(struct DHT11_Dev* dev, GPIO_TypeDef* gpio, uint16_t pin) {

	//Initialise TIMER2
	timer_init(TIM2,4, 100000-1);
	timer_start(TIM2);

	//Initialise GPIO DHT11
	if(gpio==GPIOA)
		{
			RCC->AHB2ENR |=RCC_AHB2ENR_GPIOAEN;
		}
		if(gpio==GPIOB)
		{
			RCC->AHB2ENR |=RCC_AHB2ENR_GPIOBEN;
		}
		else
		{
			return -1;
		}
	gpio->MODER &= ~(0b11<<(2*DHT11_pin));
	gpio->MODER |= (0b01<<(2*DHT11_pin));
	gpio->OSPEEDR &= ~(0b11 << (2*DHT11_pin));
	gpio->OSPEEDR |= (0b11 << (2*DHT11_pin));
	gpio->OTYPER &= ~(0b1 << (DHT11_pin));
	gpio->OTYPER |= (0b1 << (DHT11_pin));
	gpio->PUPDR &= ~(0b11 << (2*DHT11_pin));
	gpio->PUPDR |= (0b00 << (2*DHT11_pin));

	return 0;
}

int DHT11_read(struct DHT11_Dev* dev) {

	//Initialisation
	uint8_t i, j, temp;
	uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	//Generate START condition
	//o
	DHT11_gpio->MODER &= ~(0b11<<(2*DHT11_pin));
	DHT11_gpio->MODER |= (0b01<<(2*DHT11_pin));
	DHT11_gpio->OSPEEDR &= ~(0b11 << (2*DHT11_pin));
	DHT11_gpio->OSPEEDR |= (0b11 << (2*DHT11_pin));
	DHT11_gpio->OTYPER &= ~(0b1 << (DHT11_pin));
	DHT11_gpio->OTYPER |= (0b1 << (DHT11_pin));
	DHT11_gpio->PUPDR &= ~(0b11 << (2*DHT11_pin));
	DHT11_gpio->PUPDR |= (0b00 << (2*DHT11_pin));

	//dev->gpio->MODER |= GPIO_MODER_MODER6_0;

	//Put LOW for at least 18ms
	reset_gpio(DHT11_gpio, DHT11_pin);

	//wait 18ms
	TIM2->CNT = 0;
	while((TIM2->CNT) <= 18000);

	//Put HIGH for 20-40us
	set_gpio(DHT11_gpio, DHT11_pin);

	//wait 40us
	TIM2->CNT = 0;
	while((TIM2->CNT) <= 40);
	reset_gpio(DHT11_gpio, DHT11_pin);
	//End start condition

	//io();
	//Input mode to receive data
	DHT11_gpio -> MODER &= ~(0b11 << (2*DHT11_pin));
	DHT11_gpio -> MODER |= (0b00 << (2*DHT11_pin));

	//DHT11 ACK
	//should be LOW for at least 80us
	//while(!read_gpio(dev->gpio, dev->pin));
	TIM2->CNT = 0;
	while(!read_gpio(DHT11_gpio, DHT11_pin)) {
		if(TIM2->CNT > 100)
			return DHT11_ERROR_TIMEOUT;
	}

	//should be HIGH for at least 80us
	//while(read_gpio(dev->gpio, dev->pin));
	TIM2->CNT = 0;
	while(read_gpio(DHT11_gpio, DHT11_pin)) {
		if(TIM2->CNT > 100)
			return DHT11_ERROR_TIMEOUT;
	}

	//Read 40 bits (8*5)
	for(j = 0; j < 5; ++j) {
		for(i = 0; i < 8; ++i) {

			//LOW for 50us
			//while(!read_gpio(dev->gpio, dev->pin));
			TIM2->CNT = 0;
			while(!read_gpio(DHT11_gpio, DHT11_pin)) {
				if(TIM2->CNT > 60)
					return DHT11_ERROR_TIMEOUT;
			}

			//Start counter
			TIM2->CNT = 0;

			//HIGH for 26-28us = 0 / 70us = 1
			while(read_gpio(DHT11_gpio, DHT11_pin));
			/*while(!read_gpio(dev->gpio, dev->pin)) {
				if(TIM2->CNT > 100)
					return DHT11_ERROR_TIMEOUT;
			}*/

			//Calc amount of time passed
			temp = TIM2->CNT;

			//shift 0
			data[j] = data[j] << 1;

			//if > 30us it's 1
			if(temp > 40)
				data[j] = data[j]+1;
		}
	}

	//verify the Checksum
	if(data[4] != (data[0] + data[2]))
		return DHT11_ERROR_CHECKSUM;

	//set data
	dev->temparature = data[2];
	dev->humidity = data[0];

	return DHT11_SUCCESS;
}

void SystemClock_Config(int speed){
	//system clock -> MSI
	RCC->CFGR &= ~RCC_CFGR_SW_Msk;
	RCC->CFGR |= RCC_CFGR_SW_MSI;

	while(!(((RCC->CFGR & RCC_CFGR_SWS_Msk)>> RCC_CFGR_SWS_Pos)==0));

	RCC->CR &= ~RCC_CR_PLLON;//Disabe PLL
	while((RCC->CR & RCC_CR_PLLRDY) != 0); //Make sure PLL is ready (unlocked)

	//Set PLL to MSI
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC_Msk;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_MSI;

	int set_R=0, set_N=0, set_M=0;
	//Change R N M (4*N)/(2*(R+1)*(M+1))
	if(speed == 40){
		set_R = 1;
		set_N = 40;
		set_M = 0;
	}
	else if(speed == 16){
		set_R = 0;
		set_N = 8;
		set_M = 0;
	}
	else if(speed == 10){
		set_R = 0;
		set_N = 5;
		set_M = 0;
	}
	else if(speed == 6){
		set_R = 0;
		set_N = 3;
		set_M = 0;
	}
	else if(speed == 1){
		set_R = 3;
		set_N = 8;
		set_M = 3;
	}
	else{
		//Default 4 MHz
		set_R = 3;
		set_N = 8;
		set_M = 0;
	}

	//Set PLLR
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLR_Msk;
	RCC->PLLCFGR |= (set_R << RCC_PLLCFGR_PLLR_Pos);
	//Set PLLN
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_Msk;
	RCC->PLLCFGR |= (set_N << RCC_PLLCFGR_PLLN_Pos);
	//Set PLLM
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM_Msk;
	RCC->PLLCFGR |= (set_M << RCC_PLLCFGR_PLLM_Pos);

	//Enable PLLR
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;
	//Enable PLL
	RCC->CR |= RCC_CR_PLLON;

	//system clock -> PLL
	RCC->CFGR &= ~RCC_CFGR_SW_Msk;
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	while(!(((RCC->CFGR & RCC_CFGR_SWS_Msk)>>RCC_CFGR_SWS_Pos)==3));
}

/*----------------------------------------------------------------------------
  Main function
 *----------------------------------------------------------------------------*/
int main (void) {
	
  SystemClock_Config(0);
  if (SysTick_Config(4e6 / 1000)) { 			  /* SysTick 1 msec interrupts  */
    while (1);                                    /* Capture error              */
  }

  timer_enable(TIM2);
  FPU_init();

  if(init_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin) != 0)
  {
	//Fail to init 7seg
	return -1;
  }

  if(init_led(LED_gpio, LED_pin) != 0)
  {
  	//Fail to init led
  	return -1;
  }

  if(init_Encoder(Ro_gpio, RoAPin, RoBPin) != 0)
  {
	  //Fail to init Encoder
	  return -1;
  }

  init_button(BUTTON_gpio, BUTTON_pin);
  EXTI_Setup();

  //Set Decode Mode to all segments
  send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DECODE_MODE, 0xFF);
  //Set Scan Limit to digit 0 only
  send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SCAN_LIMIT, 0x07);
  //Wakeup 7seg
  send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SHUTDOWN, 0x01);
  //Set brightness
  send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_ITENSITY, 0x05);

  while(1){
	  while(state==1) {
		  if(last_state==2){
			  if(init_motor(Mot_gpio, Enable_pin, Mot_pin1, Mot_pin2) != 0)
			  {
				//Fail to init Motor
				return -1;
			  }
			  DHT11_init(&dev, DHT11_gpio, DHT11_pin);
			  last_state=1;
			  display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin, dev.temparature, num_digits(dev.temparature));
		  }
		  int res = DHT11_read(&dev);
		  //put_gpio(dev.temparature);
		  if(res == DHT11_ERROR_CHECKSUM) {
			  //printf("ERROR\n");
			  int a=0;
			  a++;
			  stop_rotate(Mot_gpio,Enable_pin);
		  }
		  else if(res == DHT11_SUCCESS) {
			  //printf("dht11 success\n");
			  //printf("T %d - H %d\n", dev.temparature, dev.humidity);
			  int a=0;
			  a++;
			  display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin, dev.temparature, num_digits(dev.temparature));
			  if(dev.temparature > 25){
				  set_gpio(LED_gpio, LED_pin);
				  counterclockwise_rotate(Mot_gpio, Enable_pin, Mot_pin1, Mot_pin2);
			  }
			  else{
				  reset_gpio(LED_gpio, LED_pin);
				  stop_rotate(Mot_gpio,Enable_pin);
			  }
		  }
		  else {
			  //printf("TIMEOUT\n");
			  int a=0;
			  a++;
			  stop_rotate(Mot_gpio,Enable_pin);
		  }
		  Delay(5000);
	  }

	  while(state==2){
		  if(last_state==1){
			  GPIO_init_AF();
			  timer_init(TIM2,2, 101); //4M/2/21
			  PWM_channel_init();
			  timer_start(TIM2);
			  last_state=2;
			  display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin, level, num_digits(level));
		  }
		  rotaryDeal();
		  TIM2->CCR1 = globalCounter;
		  set_gpio(Mot_gpio,Enable_pin);
		  reset_gpio(Mot_gpio,Mot_pin1);
	  }
  }
}
