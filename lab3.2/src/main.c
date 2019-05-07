#include "stm32l476xx.h"
#include "helper_functions.h"
#include "led_button.h"
#include "7seg.h"
//Define pins for 7seg
#define SEG_gpio GPIOB
#define DIN_pin 3
#define CS_pin 4
#define CLK_pin 5
#define BUTTON_gpio GPIOC
#define BUTTON_pin 13


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

int init_but(GPIO_TypeDef* gpio, int LED_pin)
{
	if(gpio==GPIOC)
	{
		RCC->AHB2ENR |=RCC_AHB2ENR_GPIOCEN;
	}
	else
	{
		return -1;
	}
	gpio->MODER &= ~(0b11<<(2*LED_pin));
	gpio->MODER |= (0b00<<(2*LED_pin));

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

int read_gpio(GPIO_TypeDef* gpio, int pin)
{
	if((gpio->IDR)>>pin & 1)
		return 0;
	else
		return 1;
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
	if(init_but(BUTTON_gpio, BUTTON_pin) !=0)
	{		//Fail to init button
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
	int button_press_cycle_per_second=10;
	int debounce_cycles=100;
	int debounce_threshold=debounce_cycles*0.7;
	int n=0;
	int count=0;
	int x,m;
	int last_button_state=0;

	while(1)
	{

		for(int a=0;a<button_press_cycle_per_second;a++)
		{
			int pos_cnt=0;
			for(int a=0;a<debounce_cycles;a++)
			{
				if(read_gpio(BUTTON_gpio,BUTTON_pin)==0)
				{
					pos_cnt++;
				}
				delay_without_interrupt(1000/(button_press_cycle_per_second*debounce_cycles));
			}
			if(pos_cnt>debounce_threshold)
			{
				if(last_button_state==0)
					{
					}
				else
					{
					}
				last_button_state=1;
			}
			else
			{
				if(last_button_state==0)
					{
					}
				else
					{
						n++;
					}
				last_button_state=0;

			}
		}
		if(n==0){
			send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SCAN_LIMIT, 0);
			send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, 1, 0);
		}

		x=fib(n);
		if(x>10000){
				goto stop;
		}
		while(x>0){
			m=x%10;
			send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SCAN_LIMIT, count);
			send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, count+1, m);
			x=x/10;
			count++;
		}
		count=0;
	}
	stop:
		send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SCAN_LIMIT, 1);
		send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DECODE_MODE, 0xFC);
		send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, 2, 0x01);
		send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, 1, SEG_DATA_NON_DECODE_1);

	return 0;
}
