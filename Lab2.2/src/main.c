#include "stm32l476xx.h"

#define LED_gpio GPIOB
#define LED1_pin 3
#define LED2_pin 4
#define LED3_pin 5
#define LED4_pin 6

#define BUTTON_gpio GPIOC
#define BUTTON_pin 13

int init_led(GPIO_TypeDef* gpio, int LED_pin)
{
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
	gpio->MODER &= ~(0b11<<(2*LED_pin));
	gpio->MODER |= (0b01<<(2*LED_pin));

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
void set_gpio(GPIO_TypeDef* gpio, int pin)
{
		gpio->BSRR |=( 1<< pin);
}
void reset_gpio(GPIO_TypeDef* gpio, int pin)
{
		gpio->BRR |=( 1<< pin);
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
	if(init_led(LED_gpio, LED1_pin) !=0)
	{
		return -1;
	}
	if(init_led(LED_gpio, LED2_pin) !=0)
	{
		return -1;
	}
	if(init_led(LED_gpio, LED3_pin) !=0)
	{
		return -1;
	}
	if(init_led(LED_gpio, LED4_pin) !=0)
	{
		return -1;
	}

	if(init_but(BUTTON_gpio, BUTTON_pin) !=0)
	{
			return -1;
	}

	int shift_direction =0;
	int led_data2 = 0b000011;
	int led_data3 = 0b000111;
	int led_data1 = 0b000001;
	int leds[4]={LED1_pin,LED2_pin,LED3_pin,LED4_pin};
	int state =1;
	int button_press_cycle_per_second=10;
	int debounce_cycles=100;
	int debounce_threshold=debounce_cycles*0.7;
	int last_button_state=0;
	//int a;

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
					state=state+1;
					shift_direction = 0;
					if(state == 4)
					{
						state = 1;
					}
				}
				last_button_state=0;
			}
		}

		//delay_without_interrupt(1000);
		if(state==1)
		{
			for(int a=0;a<4;a++)
			{
				if((led_data2>>(a+1))&0x1)
				{
					reset_gpio(LED_gpio,leds[a]);
				}
				else
				{
					set_gpio(LED_gpio, leds[a]);
				}


			}
			//delay_without_interrupt(1000);
			//燈號移動
			if(shift_direction==0)
			{
				led_data2=(led_data2<<1);
			}
			else
			{
				led_data2=(led_data2>>1);
			}
			//位移方向更換
			if(led_data2==0b000011 || led_data2==0b110000)
			{
				shift_direction = 1-shift_direction;
			}
			//delay_without_interrupt(1000);
		}
		if(state==2)
		{
			for(int a=0;a<4;a++)
			{
				if((led_data3>>(a+1))&0x1)
				{
					reset_gpio(LED_gpio,leds[a]);
				}
				else
				{
					set_gpio(LED_gpio, leds[a]);
				}
			}
			//delay_without_interrupt(1000);
			//燈號移動
			if(shift_direction==0)
			{
				led_data3=(led_data3<<1);
			}
			else
			{
				led_data3=(led_data3>>1);
			}
			//位移方向更換
			if(led_data3==0b000111 || led_data3==0b111000)
			{
				shift_direction = 1-shift_direction;
			}
		}
		if(state==3)
		{
			for(int a=0;a<4;a++)
			{
				if((led_data1>>(a+1))&0x1)
				{
					reset_gpio(LED_gpio,leds[a]);
				}
				else
				{
					set_gpio(LED_gpio, leds[a]);
				}
			}
			//delay_without_interrupt(1000);
			//燈號移動
			if(shift_direction==0)
			{
				led_data1=(led_data1<<1);
			}
			else
			{
				led_data1=(led_data1>>1);
			}
			//位移方向更換
			if(led_data1==0b000001 || led_data1==0b100000)
			{
				shift_direction = 1-shift_direction;
			}
		}
	}
	return 0;
}
