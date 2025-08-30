/*------------------------------------------------------------------------/
/  Foolproof MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2019, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------/
  Features and Limitations:

  * Easy to Port Bit-banging SPI
    It uses only four GPIO pins. No complex peripheral needs to be used.

  * Platform Independent
    You need to modify only a few macros to control the GPIO port.

  * Low Speed
    The data transfer rate will be several times slower than hardware SPI.

  * No Media Change Detection
    Application program needs to perform a f_mount() after media change.

/-------------------------------------------------------------------------*/

#ifndef	_SDMM_SUPPORT_H_
#define	_SDMM_SUPPORT_H_


#define SD_SPI_GPIO_PORT	GPIOA
#define SD_SPI_GPIO_RCC		RCC_GPIOA
#define	MOSI_GPIO			GPIO7
#define MISO_GPIO			GPIO6
#define CK_GPIO				GPIO5
#define CS_GPIO				GPIO4

#define SD_SPI_RCC 			RCC_SPI1
#define SD_SPI 				SPI1

#define SD_SPI_RST 			RST_SPI1

#define SD_TIM				TIM2
#define SD_RCC_TIM			RCC_TIM2
#define SD_RST_TIM			RST_TIM2

#define	CS_H()				{ gpio_set(SD_SPI_GPIO_PORT, CS_GPIO); } /* Set MMC CS "high" */
#define CS_L()				{ gpio_clear(SD_SPI_GPIO_PORT, CS_GPIO); }	/* Set MMC CS "low" */

static void timer_init(void)
{	
	rcc_periph_clock_enable(SD_RCC_TIM);
	
	rcc_periph_reset_pulse(SD_RST_TIM);
	timer_set_mode(SD_TIM, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	timer_set_prescaler(SD_TIM, 24);

	timer_disable_preload(SD_TIM);
	timer_continuous_mode(SD_TIM);
}

static
void dly_us (UINT n)
{
    timer_disable_counter(SD_TIM);                   
    timer_set_counter(SD_TIM, 0);                    // CNT = 0
    timer_set_period(SD_TIM, n);                     // ARR = n    
    timer_generate_event(SD_TIM, TIM_EGR_UG);        
    timer_clear_flag(SD_TIM, TIM_SR_UIF);            
    timer_enable_counter(SD_TIM);

    while (timer_get_flag(SD_TIM, TIM_SR_UIF) == 0);
    timer_disable_counter(SD_TIM);
}

static void spi_gpio_init(void) 
{
	rcc_periph_clock_enable(SD_SPI_GPIO_RCC);

	gpio_mode_setup(SD_SPI_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CS_GPIO);
	gpio_set(SD_SPI_GPIO_PORT, CS_GPIO);
	
	gpio_mode_setup(SD_SPI_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, CK_GPIO);
	gpio_set_af(SD_SPI_GPIO_PORT, GPIO_AF5, CK_GPIO);
	gpio_mode_setup(SD_SPI_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MISO_GPIO);
	gpio_set_af(SD_SPI_GPIO_PORT, GPIO_AF5, MISO_GPIO);
	gpio_mode_setup(SD_SPI_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MOSI_GPIO);
	gpio_set_af(SD_SPI_GPIO_PORT, GPIO_AF5, MOSI_GPIO);
}

static void spi_init(void) 
{
	rcc_periph_clock_enable(SD_SPI_RCC);
	rcc_periph_reset_pulse(SD_SPI_RST);

	spi_init_master(SD_SPI, SPI_CR1_BAUDRATE_FPCLK_DIV_2, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                  SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

  	spi_enable_software_slave_management(SD_SPI);
	spi_set_nss_high(SD_SPI);
	spi_enable(SD_SPI);
}

#endif