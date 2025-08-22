/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for AVR              (C)ChaN, 2014    */
/*----------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include "ff.h"		/* Declarations of FatFs API */

#define GPIO_TX1				GPIO9
#define GPIO_RX1				GPIO10
#define GPIO_LED				GPIO13


#define USART_BITS  			8
#define USART_SPEED 			9600

#define IRQ_PRIORITY_USART1		1

FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */

int _write(int file, char *data, int len);

int _write(int file, char *data, int len) {
    int bytes_written;

    if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
    {
        errno = EBADF;
        return -1;
    }

    for (bytes_written = 0; bytes_written < len; bytes_written++)
    {    	
        usart_send_blocking(USART1, *data);
        data++;
    }

    return bytes_written;
    return 0;
}

static void timer_setup(void)
{	
	rcc_periph_clock_enable(RCC_TIM2);
	
	rcc_periph_reset_pulse(RST_TIM2);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	timer_set_prescaler(TIM2, 24);

	timer_disable_preload(TIM2);
	timer_continuous_mode(TIM2);
}

static void usart_setup(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_TX1);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO_TX1);
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_RX1);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO_RX1);
	
	rcc_periph_clock_enable(RCC_USART1);
	usart_set_databits(USART1, USART_BITS);
	usart_set_baudrate(USART1, USART_SPEED);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_enable(USART1);
}

static void gpio_setup(void)
{
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_LED);
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_LED);
}

int main (void)
{
	UINT bw;
	FRESULT fr;

	rcc_osc_on(RCC_HSE);
	while(!rcc_is_osc_ready(RCC_HSE));
	rcc_set_sysclk_source(RCC_CFGR_SW_HSE);
	rcc_wait_for_sysclk_status(RCC_HSE);

	gpio_setup();
	usart_setup();
	timer_setup();
	
	printf("Fat FS in a Black Pill.\n");	
	
	f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */

	fr = f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS);	/* Create a file */
	if (fr == FR_OK) {
		f_write(&Fil, "It works!\r\n", 11, &bw);	/* Write data to the file */
		fr = f_close(&Fil);							/* Close the file */
		if (fr == FR_OK && bw == 11) {		
			gpio_set(GPIOA, GPIO_LED);
		}
	}

	for (;;) ;
}


