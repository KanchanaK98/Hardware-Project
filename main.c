/*
 * ATmega16_DHT11_Project_File.c
 *
 * Author-Kanchana Kariyawasam
 */ 
#define F_CPU 8000000UL	
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "LCD16x2_4bit.h"
#define DHT11_PIN 6

uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;

//// DHT11 code////
void Request()						/* Microcontroller send start pulse or request */
{
	DDRD = 0x0F;
	DDRD |= (1<<DHT11_PIN);
	PORTD &= ~(1<<DHT11_PIN);		/* set to low pin */
	_delay_ms(20);					/* wait for 20ms */
	PORTD |= (1<<DHT11_PIN);		/* set to high pin */
}

void Response()						/* receive response from DHT11 */
{
	DDRD &= ~(1<<DHT11_PIN);
	while(PIND & (1<<DHT11_PIN));
	while((PIND & (1<<DHT11_PIN))==0);
	while(PIND & (1<<DHT11_PIN));
}

uint8_t Receive_data()							/* receive data */
{	
	for (int q=0; q<8; q++)
	{
		while((PIND & (1<<DHT11_PIN)) == 0);	/* check received bit 0 or 1 */
		_delay_us(30);
		if(PIND & (1<<DHT11_PIN))				/* if high pulse is greater than 30ms */
		c = (c<<1)|(0x01);						/* then its logic HIGH */
		else									/* otherwise its logic LOW */
		c = (c<<1);
		while(PIND & (1<<DHT11_PIN));
	}
	return c;
}

////// LDR code /////
void InitADC()
{
	ADMUX=(1<<REFS0);									// For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);	// Prescalar div factor =128
}

uint16_t ReadADC(uint8_t ch)
{
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	ADMUX&=0b11100000;
	ADMUX|=ch;

	//Start Single conversion
	ADCSRA|=(1<<ADSC);

	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!

	ADCSRA|=(1<<ADIF);

	return(ADC);
}

int main(void)
{	
	
	int period = 2000; //timing for stepper motor
	
	DDRC=0xFF;
	DDRA=0x00;
	char data[5];
	lcdinit();					/* initialize LCD */
	lcd_clear();				/* clear LCD */
	lcd_gotoxy(0,0);			/* enter column and row position */
	lcd_print("Humidity =");
	lcd_gotoxy(0,1);
	lcd_print("Temp = ");

	///// LDR code ///////
	uint16_t adc_result;
   

   	DDRA&=~(1<<0);
	uint16_t limit=700;
	
	//Initialize ADC
	InitADC();
	
	int curtain_is_open = 0;
	///////////////////////
    while(1)
	{	
		
		
		adc_result=ReadADC(0);
		////////// LDR code /////
	


		Request();				/* send start pulse */
		Response();				/* receive response */
		I_RH=Receive_data();	/* store first eight bit in I_RH */
		D_RH=Receive_data();	/* store next eight bit in D_RH */
		I_Temp=Receive_data();	/* store next eight bit in I_Temp */
		D_Temp=Receive_data();	/* store next eight bit in D_Temp */
		CheckSum=Receive_data();/* store next eight bit in CheckSum */
		
		
		int val_RH=1;     //if RH value is not okay
		if((I_RH>=80)&&(I_RH<=85))
		{
			val_RH=0;    //if RH value is okay
		}
		
		int val_Temp=1;     //if Temp value is not okay
		if((I_Temp>=28)&&(I_Temp<=32))
		{
			val_Temp=0;    //if Temp value is okay
		}
		
		
		
		if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
		{
			lcd_gotoxy(0,0);
			lcd_print("Error");
		}
		
		else
		{	
			itoa(I_RH,data,10);		//itoa () function in C language converts int data type to string data type
			lcd_gotoxy(11,0);
			lcd_print(data);
			lcd_print(".");
			
			itoa(D_RH,data,10);
			lcd_print(data);
			lcd_print("%");

			itoa(I_Temp,data,10);
			lcd_gotoxy(6,1);
			lcd_print(data);
			lcd_print(".");
			
			itoa(D_Temp,data,10);
			lcd_print(data);
			lcddata(0xDF);
			lcd_print("C ");
			
			itoa(CheckSum,data,10);
			lcd_print(data);
			lcd_print(" ");

			
			
			
			//checking temp<28 and humidity is okay
			if((I_Temp<28)&&(val_RH==0))     
			{
					if(adc_result>limit) 
					{		// in day
					
					_delay_ms(100); 
					PORTC = 0x10;		//switch on fan heater
					if(I_Temp<20)
					{
						PORTC = 0x11;	//switch on fan heater and busser
					}
					
					if(I_Temp<28 && curtain_is_open==0)
					{
						if(adc_result>limit)
						{		// in day
							
							for(int i=0;i<1;i++)		/* Rotate Stepper Motor clockwise with Half step sequence; Half step angle 3.75 */
							{								//If we want to rotate again we should write i<2
								
								PORTD = 0x08;
								_delay_ms(period);
								PORTD = 0x04;
								_delay_ms(period);
								PORTD = 0x02;
								_delay_ms(period);
								PORTD = 0x01;
								_delay_ms(period);
								
							}
							
							
							curtain_is_open = 1;
							
						}
						
						
					}
					
					
					
					}
					if(adc_result<limit)
					{
						PORTC = 0x30;		 //switch on high wattage bulb and fan heater 
						if(I_Temp<20)
						{
							PORTC = 0x31;	//switch on high wattage bulb,fan heater and busser
						}
						 
					}
				
			} 
			else if((I_Temp>32)&&(val_RH==0))
			{
				PORTC=0xC0;     	//Switch on exhaust fan & water motor to spray water
				if(I_Temp>40)
				{
					PORTC=0xC1;		//switch on exhaust fan,water motor to spray water & busser
				}
				

			}
				
			
			//new code line begin hear
			
			if((I_RH<80)&&(val_Temp==0))
			{
				PORTC = 0x02;  //switch on humidity fire machine
				if(I_RH<70)
				{
					PORTC = 0x03;  //switch on humidity fire machine and busser
				}
				
			}
			else if((I_RH>85)&&(val_Temp==0))
			{
				PORTC = 0x40;  //switch on exhaust fan
				if(I_RH>89)
				{
					PORTC = 0x41;  //switch on exhaust fan and busser
				}
				
			}
			
			
			//new code line end
			
			
			//if temp and Rh both are not normal
			if((val_Temp==1)&&(val_RH==1))    //if temp and Rh both are not normal
			{
				if((I_RH<80)&&(I_Temp<28))
				{
					if(adc_result>limit)
					{		// in day
						
						_delay_ms(100);
						PORTC = 0x12;		//switch on fan heater and humidity fire
						if((I_Temp<20)||(I_RH<70))
						{
							PORTC = 0x13;	//switch on fan heater, humidity fire and busser
						}
						
						if(I_Temp<28 && curtain_is_open==0)
						{
							if(adc_result>limit)
							{		// in day
								
								for(int i=0;i<1;i++)		/* Rotate Stepper Motor clockwise with Half step sequence; Half step angle 3.75 */
								{								//If we want to rotate again we should write i<2
									
									PORTD = 0x08;
									_delay_ms(period);
									PORTD = 0x04;
									_delay_ms(period);
									PORTD = 0x02;
									_delay_ms(period);
									PORTD = 0x01;
									_delay_ms(period);
									
								}
								
								
								curtain_is_open = 1;
								
							}
							
							
						}
						
						
						
					}
					if(adc_result<limit)
					{
						PORTC = 0x32;		 //switch on high wattage bulb, humidity fire and fan heater
						if((I_Temp<20)||(I_RH<70))
						{
							PORTC = 0x33;	//switch on high wattage bulb, fan heater, humidity fire and busser
						}
						
					}
					
				}
				else if((I_RH<80)&&(I_Temp>32))
				{
					PORTC=0xC2;     	//Switch on exhaust fan,  humidity fire & water motor to spray water
					if((I_Temp>40)||(I_RH<70))
					{
						PORTC=0xC3;		//switch on exhaust fan, humidity fire water motor to spray water & busser
					}
					
					
				}
				else if((I_RH>85)&&(I_Temp>32))
				{
					PORTC=0xC0;     	//Switch on exhaust fan & water motor to spray water
					if((I_Temp>40)||(I_RH>89))
					{
						PORTC=0xC1;		//switch on exhaust fan,water motor to spray water & busser
					}
					
				}
				else if((I_RH>85)&&(I_Temp<28))
				{
					if(adc_result>limit)
					{		// in day
						
						_delay_ms(100);
						PORTC = 0x50;		//switch on fan heater and exhaust fan
						if((I_Temp<20)||(I_RH>89))
						{
							PORTC = 0x51;	//switch on fan heater, exhaust fan and busser
						}
						
						if(I_Temp<28 && curtain_is_open==0)
						{
							if(adc_result>limit)
							{		// in day
								
								for(int i=0;i<1;i++)		/* Rotate Stepper Motor clockwise with Half step sequence; Half step angle 3.75 */
								{								//If we want to rotate again we should write i<2
									
									PORTD = 0x08;
									_delay_ms(period);
									PORTD = 0x04;
									_delay_ms(period);
									PORTD = 0x02;
									_delay_ms(period);
									PORTD = 0x01;
									_delay_ms(period);
									
								}
								
								
								curtain_is_open = 1;
								
							}
							
							
						}
						
						
						
					}
					if(adc_result<limit)
					{
						PORTC = 0x70;		 //switch on high wattage bulb, exhaust fan and fan heater
						if((I_Temp<20)||(I_RH>89))
						{
							PORTC = 0x71;	//switch on high wattage bulb, exhaust fan fan heater and busser
						}
						
					}
					
				}
				
				
				
			}
			
			if(I_Temp>=28 && curtain_is_open == 1)
			{
				for (int k = 0; k < 1; k++)
				{
					PORTD = 0x01;
					_delay_ms(period);
					PORTD = 0x02;
					_delay_ms(period);
					PORTD = 0x04;
					_delay_ms(period);
					PORTD = 0x08;
					_delay_ms(period);
					//PORTB = 0x0C;
					//_delay_ms(period);
				}
				curtain_is_open = 0;
			}
			
			
			if((val_Temp==0)&&(val_RH==0))    // when temp and humidity is in the situation
			{
				
				_delay_ms(100);
				
				PORTC=0x00;	//To off very component in port c
				
			}
			
			
		

			
/////////////////////////////////////		
		}
				
	_delay_ms(50);
	}	
}