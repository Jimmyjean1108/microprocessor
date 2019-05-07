#include "stm32l476xx.h"
#include "helper_functions.h"
#include "7seg.h"
#include "keypad.h"

//Define pins for 7seg
#define SEG_gpio GPIOC
#define DIN_pin 5
#define CS_pin 6
#define CLK_pin 8
#define INTENSITY 0x0A

//Define pins for keypad
#define COL_gpio GPIOA
#define COL_pin 5 //5 6 7 8
#define ROW_gpio GPIOB
#define ROW_pin 3 //3 4 5 6


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
		return 1; //no touch �1
	else
		return 0; //touch �0
}


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

int init_keypad(GPIO_TypeDef* row_gpio, GPIO_TypeDef* col_gpio, int row_pin, int col_pin){
	//Enable AHB2 Clock
	if(row_gpio == GPIOA || col_gpio == GPIOA){
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	}
	if(row_gpio == GPIOB || col_gpio == GPIOB){
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	}
	//First Clear bits(&) then set bits (|)
	for(int a=0; a<4; a++)
	{
		//Set GPIO pins to output mode (01)
		col_gpio -> MODER &= ~(0b11 << (2*(col_pin+a)));
		col_gpio -> MODER |= (0b01 << (2*(col_pin+a)));
		//Set GPIO pins to very high speed mode (11)
		col_gpio -> OSPEEDR &= ~(0b11 << (2*(col_pin+a)));
		col_gpio -> OSPEEDR |= (0b11 << (2*(col_pin+a)));
		//Set GPIO pins to open drain mode (1)
		col_gpio -> OTYPER &= ~(0b1 << (col_pin+a));
		col_gpio -> OTYPER |= (0b1 << (col_pin+a));
		//Set Output to high
		set_gpio(col_gpio, col_pin+a);
	}
	//First Clear bits(&) then set bits(|)
	for(int a=0; a<4; a++)
	{
		//Set GPIO pins to input mode (00)
		row_gpio -> MODER &= ~(0b11 << (2*(row_pin+a)));
		row_gpio -> MODER |= (0b00 << (2*(row_pin+a)));
		//Set GPIO pins to Pull-Down mode (10)
		row_gpio -> PUPDR &= ~(0b11 << (2*(row_pin+a)));
		row_gpio -> PUPDR |= (0b10 << (2*(row_pin+a)));
	}
	return 0;
}

int check_keypad_input_one(GPIO_TypeDef* row_gpio, GPIO_TypeDef* col_gpio, int row_pin, int col_pin, int x, int y)
{
	int cycles = 400;
	//Set Column to push-pull mode
	col_gpio -> OTYPER &= ~(1 << (col_pin+y));
	//Count the total number of time it is pressed in a certain period
	int cnt = 0;
	for(int a=0; a<cycles; a++){
		cnt += read_gpio(row_gpio, row_pin+x);
	}
	//Set Column back to open drain mode
	col_gpio -> OTYPER |= (1 << (col_pin+y));
	//return if the key is pressed(1) or not(0)
	return (cnt > (cycles*0.7));
}

const int keypad[4][4]={
		{1,2,3,10},
		{4,5,6,11},
		{7,8,9,12},
		{15,0,14,13}
};

int num_digits(int x)
{
	if(x==0)
	{
		return 1;
	}
	int res = 0;
	while(x)
	{
		res++;
		x/=10;
	}
	return res;
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

int display_number(GPIO_TypeDef* gpio, int DIN, int CS, int CLK, int num, int num_digs)
{
	
	for(int i = 1;i<=num_digs;i++)
	{
		send_7seg(gpio, DIN, CS, CLK, i, num % 10);
		num /= 10;
	}
	for(int i = num_digs+1;i<=8;i++)
	{
		num/=10;
		send_7seg(gpio, DIN, CS, CLK, i, 15);//blank
	}
	if(num!=0)
		return -1;
	return 0;
}

int main()
{
	//#ifdef lab_keypad_single_key
	if(init_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin) != 0)
	{
		//Fail to init 7seg
		return -1;
	}
	//Set Decode Mode to non-decode mode
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_DECODE_MODE, 0xFF);
	//Set Scan Limit to digit 0 only
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SCAN_LIMIT, 0x07);
	//Wakeup 7seg
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_SHUTDOWN, 0x01);
	//Set brightness
	send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, SEG_ADDRESS_ITENSITY, 0x05);

	if(init_keypad(ROW_gpio, COL_gpio, ROW_pin, COL_pin) != 0)
	{
		//Fail to init keypad
		return -1;
	}
	int B=0;
	int check;
	int check_state;
	double equ[1000];
	int k=0;
	int flag=1;
	restart:
	while(flag)
	{
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<4;j++)
			{
				if(check_keypad_input_one(ROW_gpio, COL_gpio, ROW_pin, COL_pin, i, j))
				{
					int A=keypad[i][j];
					delay_without_interrupt(100);
					if(A<10)
					{
						B=B*10+A;
						if(B>999)
							B/=10;
						display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin,B, num_digits(B));
						check_state=1;
					}
					else if(A<14){
						if(check_state==1){
							check+=1;
							check_state=0;
						}
						equ[k]=B;
						equ[k+1]=A;
						display_number(SEG_gpio, DIN_pin, CS_pin, CLK_pin,0, 0);
						B=0;
						k+=2;
						check-=1;
					}
					else if(A==15){
						equ[k]=B;
						B=0;

						display_number(SEG_gpio,DIN_pin,CS_pin,CLK_pin,0,0);
						delay_without_interrupt(200);

						if(check==0)
							flag=0;
						else{
							send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, 2, 10);
							send_7seg(SEG_gpio, DIN_pin, CS_pin, CLK_pin, 1, 1);
						}

					}
					else{
						B=0;
						k=0;
						check=0;
						display_number(SEG_gpio,DIN_pin,CS_pin,CLK_pin,0,0);
					}
				}
			}
		}
		
	}

	for(int j=1;j<=k-1;j+=2){
		if(equ[j]==13){
			equ[j-1]=equ[j-1]/equ[j+1];
			for(int m=j+2;m<=k;m++)
				equ[m-2]=equ[m];
			k-=2;
			j-=2;
		}
	}

	for(int j=1;j<=k-1;j+=2){
		if(equ[j]==12){
			equ[j-1]=equ[j-1]*equ[j+1];
			for(int m=j+2;m<=k;m++)
				equ[m-2]=equ[m];
			k-=2;
			j-=2;
		}
	}

	for(int j=1;j<=k-1;j+=2){
		if(equ[j]==11){
			equ[j-1]=equ[j-1]-equ[j+1];
			for(int m=j+2;m<=k;m++)
				equ[m-2]=equ[m];
			k-=2;
			j-=2;
		}
	}

	for(int j=1;j<=k-1;j+=2){
		if(equ[j]==10){
			equ[j-1]=equ[j-1]+equ[j+1];
			for(int m=j+2;m<=k;m++)
				equ[m-2]=equ[m];
			k-=2;
			j-=2;
		}
	}

	int ANS=equ[0]*1000;
	int DP=4;
	for (int i = 1; i <= 3; i++)
	{
		if(ANS%10==0){
			ANS=ANS/10;
			DP--;
		}
	}

	int num_with_DP=ANS;
	for (int i = 1; i < DP; i++)
	{
		num_with_DP/=10;
	}
	num_with_DP=num_with_DP%10;

	if (ANS<0)
	{
		ANS*=-1;
		num_with_DP*=-1;
		display_number(SEG_gpio,DIN_pin,CS_pin,CLK_pin,ANS,num_digits(ANS));
		send_7seg(SEG_gpio,DIN_pin,CS_pin,CLK_pin,DP,num_with_DP|0b10000000);
		send_7seg(SEG_gpio,DIN_pin,CS_pin,CLK_pin,num_digits(ANS)+1,10);
	}
	else{
		display_number(SEG_gpio,DIN_pin,CS_pin,CLK_pin,ANS,num_digits(ANS));
		send_7seg(SEG_gpio,DIN_pin,CS_pin,CLK_pin,DP,num_with_DP|0b10000000);		
	}
	flag=1;
	goto restart;

	return 0;
}
