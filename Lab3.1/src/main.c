#include "stm32l476xx.h"
#include "helper_functions.h"
#include "led_button.h"
#include "7seg.h"
//Define pins for 7seg
#define SEG_gpio GPIOB
#define DIN_pin 3
#define CS_pin 4
#define CLK_pin 5



int init_7seg(GPIO_TypeDef* gpio, int DIN, int CS, int CLK)
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
	else
	{
		//Error! Add other cases to suit other GPIO pins
		return -1;
	}
	//Set GPIO pins to output mode (01)
	//First Clear bits(&) the set bits (|)
	gpio->MODER &= ~(0b11 << (2*DIN));
	gpio->MODER |= (0b01 << (2*DIN));
	gpio->MODER &= ~(0b11 << (2*CS));
	gpio->MODER |= (0b01 << (2*CS));
	gpio->MODER &= ~(0b11 << (2*CLK));
	gpio->MODER |= (0b01 << (2*CLK));

	//Close display test
	send_7seg(gpio, DIN, CS, CLK, SEG_ADDRESS_DISPLAY_TEST, 0x00);
	return 0;
}

int fib(int n){

    if(n==0)
        return 0;

    if(n==1)
        return 1;

    return (fib(n-1)+fib(n-2));

}

void send_7seg(GPIO_TypeDef* gpio, int DIN, int CS, int CLK, int address, int data)
{
	//The payload to send
	int payload = ((address&0xFF)<<8)|(data&0xFF);

	//Start the sending cycles
	//16 data-bits + 1 CS signal
	int total_cycles = 16+1;

	for(int a=1;a<=total_cycles;a++)
	{
		//Reset CLK when enter
		reset_gpio(gpio,CLK);

		//Set DIN according to data except for last cycles(CS)
		if(((payload>>(total_cycles-1-a))&0x1) && a!= total_cycles)
		{
			set_gpio(gpio, DIN);
		}
		else
		{
			reset_gpio(gpio, DIN);
		}
		 //Set CS at last cycle
		if(a == total_cycles)
		{
			set_gpio(gpio, CS);
		}
		else
		{
			reset_gpio(gpio, CS);
		}

		//Set CLK when leaving (seg set data at rising edge)
		set_gpio(gpio, CLK);
	}

	return;
}

void set_gpio(GPIO_TypeDef* gpio, int pin)
{
	gpio->BSRR |= (1 << pin);
}

void reset_gpio(GPIO_TypeDef* gpio, int pin)
{
	gpio->BRR |= (1 << pin);
}

void delay_without_interrupt(int msec)
{
	int loop_cnt= 500*msec;
	while(loop_cnt)
	{
		loop_cnt--;
	}
	return ;
}

int main()
{
	if(init_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin) != 0)
	{
		//Fail to init 7seg
		return -1;
	}
	//Set Decode Mode to non-decode mode
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DECODE_MODE, 0xFF);
	//Set Scan Limit to digit 0 only
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SCAN_LIMIT, 0x00);
	//Wakeup 7seg
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SHUTDOWN, 0x01);
	//Set brightness
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_ITENSITY, 0x0F);

	int SEG_DATA_NON_DECODE_LOOP[17]=
	{
		SEG_DATA_NON_DECODE_0,
		SEG_DATA_NON_DECODE_1,
		SEG_DATA_NON_DECODE_2,
		SEG_DATA_NON_DECODE_3,
		SEG_DATA_NON_DECODE_4,
		SEG_DATA_NON_DECODE_5,
		SEG_DATA_NON_DECODE_6,
		SEG_DATA_NON_DECODE_7,
		SEG_DATA_NON_DECODE_8,
		SEG_DATA_NON_DECODE_9,
		SEG_DATA_NON_DECODE_0,
		SEG_DATA_NON_DECODE_A,
		SEG_DATA_NON_DECODE_B,
		SEG_DATA_NON_DECODE_C,
		SEG_DATA_NON_DECODE_D,
		SEG_DATA_NON_DECODE_E,
		SEG_DATA_NON_DECODE_F
	};
	int SEG_ADDRESS_DIGIT_LOOP[8]=
	{
		SEG_ADDRESS_DIGIT_0,
		SEG_ADDRESS_DIGIT_1,
		SEG_ADDRESS_DIGIT_2,
		SEG_ADDRESS_DIGIT_3,
		SEG_ADDRESS_DIGIT_4,
		SEG_ADDRESS_DIGIT_5,
		SEG_ADDRESS_DIGIT_6,
		SEG_ADDRESS_DIGIT_7
	};
	//Non-decode mode set decode mode to 0x00
	/*send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_2, SEG_DATA_NON_DECODE_LOOP[0]);
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_3, SEG_DATA_NON_DECODE_LOOP[1]);
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_4, SEG_DATA_NON_DECODE_LOOP[1]);
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_5, SEG_DATA_NON_DECODE_LOOP[4]);
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_6, SEG_DATA_NON_DECODE_LOOP[0]);
	while(1)
	{
		send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_0, SEG_DATA_NON_DECODE_LOOP[4]);
		send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_1, SEG_DATA_NON_DECODE_LOOP[9]);
		delay_without_interrupt(2000);
		send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_0, SEG_DATA_NON_DECODE_LOOP[9]);
		send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_1, SEG_DATA_NON_DECODE_LOOP[0]);
		delay_without_interrupt(2000);
	}*/
	//Decode mode set decode mode to 0xFF
	//send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_7, SEG_DATA_DECODE_H);
	//send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_6, SEG_DATA_DECODE_E);
	//send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_5, SEG_DATA_DECODE_L);
	//send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_4, SEG_DATA_DECODE_P);
	//send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_3, SEG_DATA_DECODE_9);
	//send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_2, SEG_DATA_DECODE_4);
	//send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_1, SEG_DATA_DECODE_8);
	//send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_0, SEG_DATA_DECODE_7);

	/*int current = 0;
	while(1)
	{
		//Write to digit 0
		send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DIGIT_0, SEG_DATA_NON_DECODE_LOOP[current]);
		current = (current+1)%17;
		delay_without_interrupt(1000);
	}*/

	int count=0;
	int x,m,n;
	for(int a=0; fib(a)<10000; a++)
	{
		x=fib(a);
		while(x>0){
			m=x%10;
			send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SCAN_LIMIT, count);
			send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, count+1, m);
			x=x/10;
			count++;
		}
		count=0;
		delay_without_interrupt(1000);
	}
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SCAN_LIMIT, 1);
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DECODE_MODE, 0xFC);
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, 2, 0x01);
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, 1, SEG_DATA_NON_DECODE_1);

	return 0;
}
