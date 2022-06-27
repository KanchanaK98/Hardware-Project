/*
 * LCD16x2_4bit.c
 *
 * Author-Kanchana Kariyawasam
 */ 

#include "LCD16x2_4bit.h"
void lcdcommand(unsigned char cmnd)
{
	/* Sending upper nibble */
	LCD_DPRT = (LCD_DPRT & 0x0f)|(cmnd & 0xf0);		/* SEND COMMAND TO DATA PORT */
	LCD_DPRT &= ~ (1<<LCD_RS);						/* RS = 0 FOR COMMAND */
	LCD_DPRT |= (1<<LCD_EN);						/* EN = 1 FOR H TO L PULSE */
	_delay_us(1);									/* WAIT FOR MAKE ENABLE WIDE */
	LCD_DPRT &= ~(1<<LCD_EN);						/* EN = 0 FOR H TO L PULSE */
	_delay_us(100);									/* WAIT FOR MAKE ENABLE WIDE */
	
	/* Sending lower nibble */
	LCD_DPRT = (LCD_DPRT & 0x0f)|(cmnd << 4);		/* SEND COMMAND TO DATA PORT */
	LCD_DPRT |= (1<<LCD_EN);						/* EN = 1 FOR H TO L PULSE */
	_delay_us(1);									/* WAIT FOR MAKE ENABLE WIDE */
	LCD_DPRT &= ~(1<<LCD_EN);						/* EN = 0 FOR H TO L PULSE */
	_delay_ms(2);									/* WAIT FOR MAKE ENABLE WIDE */
}

void lcddata(unsigned char data)
{
	LCD_DPRT = (LCD_DPRT & 0x0f)|(data & 0xf0);		/* SEND DATA TO DATA PORT */
	LCD_DPRT |= (1<<LCD_RS);						/* MAKE RS = 1 FOR DATA */
	LCD_DPRT |= (1<<LCD_EN);						/* EN=0 FOR H TO L PULSE */
	_delay_us(1);									/* WAIT FOR MAKE ENABLE WIDE */
	LCD_DPRT &= ~(1<<LCD_EN);						/* EN = 0 FOR H TO L PULSE */
	_delay_us(100);									/* WAIT FOR MAKE ENABLE WIDE */
	
	LCD_DPRT = (LCD_DPRT & 0x0f)|(data << 4);		/*  */
	LCD_DPRT |= (1<<LCD_EN);						/* EN=0 FOR H TO L PULSE*/
	_delay_us(1);									/* WAIT FOR MAKE ENABLE WIDE*/
	LCD_DPRT &= ~(1<<LCD_EN);						/* EN = 0 FOR H TO L PULSE*/
	_delay_ms(2);									/* WAIT FOR MAKE ENABLE WIDE*/
}

void lcdinit()
{
	LCD_DDDR = 0xFF;
	_delay_ms(200);									/* WAIT FOR SOME TIME */
	lcdcommand(0x33);								/* Function set to 8-bit mode.*/
	_delay_ms(200);		
	lcdcommand(0x32);								/* Function set to 8-bit mode again */
	lcdcommand(0x28);								/* INIT. LCD 2 LINE, 5 X 8 MATRIX AND 4 BIT MODE*/
	lcdcommand(0x0C);								/* DISPLAY ON CURSOR OFF */
	lcdcommand(0x01);								/* LCD CLEAR */
	_delay_ms(20);
	lcdcommand(0x82);								/* SHIFT CURSOR TO WRITE */
}

void lcd_gotoxy(unsigned char x, unsigned char y)
{
	unsigned char firstcharadd[]={0x80, 0xC0};
	lcdcommand(firstcharadd[y] + x);
}

void lcd_print(char *str)
{
	unsigned char i=0;
	while (str[i] |= 0)		/* Send each char of string till the NULL */
	{
		lcddata(str[i]);
		i++;
	}
}

void lcd_clear()
{
	lcdcommand(0x01);
	_delay_ms(2);
}