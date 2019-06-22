#include "stdlib.h"
#include "string.h"
#include "picprog.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_rcc.h"
#include "main.h"
#include "usbd_cdc_if.h"



extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_tim3_ch3;
extern DMA_HandleTypeDef hdma_tim3_ch4_up;
extern DMA_HandleTypeDef hdma_tim3_ch1_trig;
extern CRC_HandleTypeDef hcrc;
uint16_t tents,tenth,tdly;
uint32_t terar,terab,tpint;

uint16_t myPeriod;
uint16_t myHigh, myLow, myClock;
uint8_t *buf_clock, *buf0_clock, *buf1_clock;
uint8_t *buf_array, *buf0_array, *buf1_array;
uint16_t *buf_array_16;
uint8_t *flash_buffer;
uint8_t volatile fb_start,fb_len,fb_lock;
uint8_t *packet;
uint8_t *usb_buffer;
uint16_t usb_len;
uint8_t seq;
uint8_t* datax;

uint16_t offset;
uint8_t buf,packet_len;
volatile uint8_t i_cc1,i_cc3,i_cc4;

void PB5_input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ICSPDAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ICSPDAT_GPIO_Port, &GPIO_InitStruct);
    hdma_tim3_ch3.Instance->CCR |= DMA_IT_TC;
    hdma_tim3_ch4_up.Instance->CCR &= ~DMA_IT_TC;
}	

void PB5_af(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ICSPDAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(ICSPDAT_GPIO_Port, &GPIO_InitStruct);
    hdma_tim3_ch3.Instance->CCR &= ~DMA_IT_TC;
    hdma_tim3_ch4_up.Instance->CCR |= DMA_IT_TC;
}


uint32_t get_timer2_freq()
{
	uint32_t pFLatency;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	uint32_t pclk1_freq=HAL_RCC_GetPCLK1Freq();
	HAL_RCC_GetClockConfig (&RCC_ClkInitStruct, &pFLatency);
	if(RCC_ClkInitStruct.APB1CLKDivider==1) return pclk1_freq;
	else return pclk1_freq*2;
}

void delay_us(uint32_t us)
{
	HAL_GPIO_WritePin(GPIOE, LD5_Pin, GPIO_PIN_SET);
//	uint32_t val=htim2.Instance->CNT+us;
	htim2.Instance->EGR |= TIM_EGR_UG;
	for(int i=0;i<1;i++);
	while(htim2.Instance->CNT<us);
	HAL_GPIO_WritePin(GPIOE, LD5_Pin, GPIO_PIN_RESET);
}

void picprog_init()
{
	gpio_input();
	myPeriod=get_timer2_freq()/FREQ;
	myHigh=myPeriod+1;
	myLow=0;
	myClock=myPeriod/2;
	tents=100*FREQ/1000000000+1;
	tenth=250*FREQ/1000000+1;
	tdly=1*FREQ/1000000+1;
	terar=2800;
	terab=8400;
	tpint=2800;
	buf0_array=(uint8_t*)malloc(MAX_ARRAY_BUF);
	buf0_clock=(uint8_t*)malloc(MAX_ARRAY_BUF);
	buf1_array=(uint8_t*)malloc(MAX_ARRAY_BUF);
	buf1_clock=(uint8_t*)malloc(MAX_ARRAY_BUF);
	buf_clock=buf0_clock;
	buf_array=buf0_array;
	offset=0;
	buf=0;
	flash_buffer=(uint8_t*)malloc(FB_SIZE);
	packet=(uint8_t*)malloc(FB_SIZE);
	datax=(uint8_t*)malloc(MAX_ROW_SIZE);
	fb_start=0;
	fb_len=0;
	fb_lock=0;
	usb_buffer=(uint8_t*)malloc(USB_BUFFER_SIZE);
	usb_len=0;
	buf_array_16=(uint16_t*)malloc(24);
	htim2.Instance->PSC=get_timer2_freq()/1000000-1;
	htim3.Instance->ARR=myPeriod;
	htim3.Instance->CCR3=2;
	htim3.Instance->CCR4=2;
	htim3.Instance->DIER &=~TIM_DIER_UDE;
	htim3.Instance->DIER &=~TIM_DIER_TDE;
	htim3.Instance->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
	htim3.Instance->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_OC_Start(&htim3, TIM_CHANNEL_3);
	HAL_TIM_OC_Start(&htim3, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  hdma_tim3_ch3.DmaBaseAddress->IFCR  = (DMA_FLAG_GL1 << hdma_tim3_ch3.ChannelIndex);
  hdma_tim3_ch3.Instance->CCR &= ~(DMA_IT_HT|DMA_IT_TE|DMA_IT_TC);
  hdma_tim3_ch3.Instance->CPAR = (uint32_t)(&(TIM3->CCR1));
	i_cc3=0;
  hdma_tim3_ch4_up.Instance->CCR &= ~(DMA_IT_HT|DMA_IT_TE);
  hdma_tim3_ch4_up.Instance->CCR |= DMA_IT_TC;
  hdma_tim3_ch4_up.Instance->CPAR = (uint32_t)(&(TIM3->CCR2));
	i_cc4=0;
  hdma_tim3_ch1_trig.Instance->CCR &= ~(DMA_IT_HT|DMA_IT_TE);
  hdma_tim3_ch1_trig.Instance->CCR |= DMA_IT_TC;
  hdma_tim3_ch1_trig.Instance->CPAR = (uint32_t)(&(ICSPDAT_GPIO_Port->IDR));
	i_cc1=0;
	HAL_GPIO_WritePin(GPIOE, LD4_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MCLR_GPIO_Port, MCLR_Pin, GPIO_PIN_SET);
}

void write_byte(uint8_t data_byte)
{
	uint8_t x=0x80;
	for(int i=0;i<8;i++)
	{
		if(x&data_byte) buf_array[offset]=myHigh;
		else buf_array[offset]=myLow;
		buf_clock[offset++]=myClock;
		x=x>>1;
	}
	return;
}

void write_hw(uint16_t data_hw)
{
  
	uint16_t x=0x8000;
	for(int i=0;i<16;i++)
	{
		if(x&data_hw) buf_array[offset]=myHigh;
		else buf_array[offset]=myLow;
		buf_clock[offset++]=myClock;
		x>>=1;
	}
	return;
}

void write_word(uint32_t data_word)
{
  
	uint32_t x=0x80000000;
	for(int i=0;i<32;i++)
	{
		if(x&data_word) buf_array[offset]=myHigh;
		else buf_array[offset]=myLow;
		buf_clock[offset++]=myClock;
		x=x>>1;
	}
	return;
}

void write_skip(uint8_t nskip)
{
	for(int i=0;i<nskip;i++)
	{
		buf_array[offset]=myLow;
		buf_clock[offset++]=myLow;
	}
	return;
}

void load_pc(uint16_t ad)
{
	write_byte(0x80);
	write_skip(tdly);
	for(int i=0;i<7;i++)
	{
		buf_array[offset]=myLow;
		buf_clock[offset++]=myClock;
	};
	write_hw(ad);
	buf_array[offset]=myLow;
	buf_clock[offset++]=myClock;
	write_skip(tdly);
}

void write_data16(uint8_t msb, uint8_t lsb,uint8_t onlyone)
{
	if(onlyone) write_byte(0x0); else write_byte(0x02);
	write_skip(tdly);
	for(int i=0;i<7;i++)
	{
		buf_array[offset]=myLow;
		buf_clock[offset++]=myClock;
	}
	write_byte(msb);
	write_byte(lsb);
	buf_array[offset]=myLow;
	buf_clock[offset++]=myClock;
	write_skip(tdly);
}	

void write_data8(uint8_t b)
{
	write_byte(0x00);
	write_skip(tdly);
	for(int i=0;i<15;i++)
	{
		buf_array[offset]=myLow;
		buf_clock[offset++]=myClock;
	}
	write_byte(b);
	buf_array[offset]=myLow;
	buf_clock[offset++]=myClock;
	write_skip(tdly);
}	

void row_erase(uint8_t* p)
{
	offset=0;
	load_pc(p[4]*256+p[5]);
	write_byte(0xF0);
	write_skip(1);
	burst();
	while(i_cc4);
	delay_us(terar);
}
	
void erase_all(void)
{
	offset=0;
	write_byte(0x18);
	write_skip(1);
	burst();
	while(i_cc4);
	delay_us(terab);
}
	
		


void burst(void)
{
	while(i_cc4);
	HAL_GPIO_WritePin(GPIOE, LD3_Pin, GPIO_PIN_SET);

//	HAL_DMA_Start(&hdma_tim3_ch3,(uint32_t)buf_clock, (uint32_t)(&(TIM3->CCR1)), offset);

	htim3.Instance->CR1 &= ~ TIM_CR1_CEN;
	htim3.Instance->EGR |= TIM_EGR_UG;
	
 	hdma_tim3_ch3.Instance->CCR &= ~DMA_CCR_EN;
  hdma_tim3_ch3.Instance->CMAR = (uint32_t)buf_clock;
  hdma_tim3_ch3.Instance->CNDTR = offset;
  hdma_tim3_ch3.Instance->CCR |= DMA_CCR_EN;
 
//  i_cc3=1;

//	HAL_DMA_Start_IT(&hdma_tim3_ch4_up, (uint32_t)buf_array, (uint32_t)(&(TIM3->CCR2)), offset);

 	hdma_tim3_ch4_up.Instance->CCR &= ~DMA_CCR_EN;
  hdma_tim3_ch4_up.Instance->CNDTR = offset;
  hdma_tim3_ch4_up.Instance->CMAR = (uint32_t)buf_array;
  hdma_tim3_ch4_up.Instance->CCR |= DMA_CCR_EN;
  
	i_cc4=1;

__disable_irq();
	htim3.Instance->DIER |= TIM_DIER_CC3DE | TIM_DIER_CC4DE;
	htim3.Instance->CR1 |= TIM_CR1_CEN;
	__enable_irq();
	if(buf)
	{
		buf_clock=buf0_clock;
		buf_array=buf0_array;
		buf=0;
	}
	else
	{
		buf_clock=buf1_clock;
		buf_array=buf1_array;
		buf=1;
	}
	offset=0;
	i_cc1=0;
	i_cc3=0;
}



void enter_pm(void)
{
	offset=0;
	write_skip(tents);
	HAL_GPIO_WritePin(MCLR_GPIO_Port, MCLR_Pin, GPIO_PIN_RESET);
	write_skip(tenth);
	write_word(0x4d434850);
	write_skip(tdly);
	burst();
	while(i_cc4);
	HAL_Delay(5);
	return;
}

void exit_pm(void)
{
	HAL_GPIO_WritePin(MCLR_GPIO_Port, MCLR_Pin, GPIO_PIN_SET);
}

void read_data(uint16_t ad, uint16_t len, uint8_t* data)
{
	if(len==0)
	{
		data=NULL;
		return;
	}
	offset=0;
	load_pc(ad);
	if(len==2) write_byte(0xfc);
	else write_byte(0xfe);
	write_skip(tdly);
	i_cc4=1;
	for(int i=0;i<len/2;i++)
	{
		htim3.Instance->CR1 &= ~ TIM_CR1_CEN;
		htim3.Instance->EGR |= TIM_EGR_UG;
		
 	  hdma_tim3_ch3.Instance->CCR &= ~DMA_CCR_EN;
    hdma_tim3_ch3.Instance->CMAR = (uint32_t)buf_clock;
    hdma_tim3_ch3.Instance->CNDTR = offset;
    hdma_tim3_ch3.Instance->CCR |= DMA_CCR_EN;
		
 	  hdma_tim3_ch4_up.Instance->CCR &= ~DMA_CCR_EN;
    hdma_tim3_ch4_up.Instance->CNDTR = offset;
    hdma_tim3_ch4_up.Instance->CMAR = (uint32_t)buf_array;
    hdma_tim3_ch4_up.Instance->CCR |= DMA_CCR_EN;


    __disable_irq();
		htim3.Instance->CR1 |= TIM_CR1_CEN;
	  htim3.Instance->DIER |= TIM_DIER_CC3DE | TIM_DIER_CC4DE;
	  __enable_irq();
		
	  if(buf)
	  {
		  buf_clock=buf0_clock;
		  buf_array=buf0_array;
		  buf=0;
	  }
	  else
	  {
		  buf_clock=buf1_clock;
		  buf_array=buf1_array;
		  buf=1;
	  }
	  offset=0;
		for(int j=0;j<23;j++) buf_clock[offset++]=myClock;
		for(int j=0;j<tdly;j++) buf_clock[offset++]=myLow;
   	i_cc1=1;
		i_cc3=1;
 
		while(i_cc4);

	  hdma_tim3_ch3.Instance->CCR &= ~DMA_CCR_EN;
 	  hdma_tim3_ch1_trig.Instance->CCR &= ~DMA_CCR_EN;

		PB5_input();

    hdma_tim3_ch3.Instance->CMAR = (uint32_t)buf_clock;
    hdma_tim3_ch3.Instance->CNDTR = offset;

    hdma_tim3_ch1_trig.Instance->CNDTR = 24;
    hdma_tim3_ch1_trig.Instance->CMAR = (uint32_t)buf_array_16;
		
 	  hdma_tim3_ch3.Instance->CCR |= DMA_CCR_EN;
 	  hdma_tim3_ch1_trig.Instance->CCR |= DMA_CCR_EN;


		htim3.Instance->CR1 &= ~ TIM_CR1_CEN;
	  htim3.Instance->CCR1=myClock;
		htim3.Instance->EGR |= TIM_EGR_UG;
    __disable_irq();
  	htim3.Instance->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC3DE;
		htim3.Instance->CR1 |= TIM_CR1_CEN;
  	__enable_irq();

	  if(buf)
	  {
		  buf_clock=buf0_clock;
		  buf_array=buf0_array;
		  buf=0;
	  }
	  else
	  {
		  buf_clock=buf1_clock;
		  buf_array=buf1_array;
		  buf=1;
	  }
		offset=0;
		if(i!=(len/2-1))
		{
	    write_byte(0xfe);
	    write_skip(tdly);
		  i_cc4=1;
		}

		while(i_cc1);
    hdma_tim3_ch1_trig.Instance->CCR &= ~DMA_CCR_EN;
    while(i_cc3);
    hdma_tim3_ch3.Instance->CCR &= ~DMA_CCR_EN;

    PB5_af();
		
		data[2*i]=0;
		uint8_t x=0x20;
		for(int j=9;j<15;j++)
		{
			if(buf_array_16[j] & ICSPDAT_Pin) ((uint8_t *)data)[2*i]|=x;
			x>>=1;
		}
		data[2*i+1]=0;
		x=0x80;
		for(int j=15;j<23;j++)
		{
			if(buf_array_16[j] & ICSPDAT_Pin) data[2*i+1]|=x;
			x>>=1;
		}
	}
	
}
		
uint8_t fb_read()
{
	uint8_t len;
	uint32_t crc;
	if(fb_len==0) return 0;
	if(fb_len>=8)
	{
		packet_len=flash_buffer[2]+6;
		len=((packet_len+3)&0xFE);
		if(fb_len<len) return 0;
		crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)flash_buffer,len);
		if(fb_len>len) for(int k=0;k<fb_len-len;k++) flash_buffer[k]=flash_buffer[k+len];
		fb_len-=len;
		if(crc==0)
		{
			memcpy(packet,flash_buffer,packet_len);
			return len;
		} else return 1;
	}
	return 0;
}

void send_ack()
{
	uint8_t ack[8];
	ack[0]=ID;
	ack[1]= (packet[1]==0xFF ? 0 : packet[1]+1);
	ack[2]=0; 
	ack[3]=ACK;
	ack[4]=0;
	ack[5]=0;
	uint32_t crc0=HAL_CRC_Calculate(&hcrc,(uint32_t*)ack,6);
	ack[6]=(crc0&0xFF00)>>8;
	ack[7]=crc0&0xFF;
	usb_send(ack,8);
}
	
void send_nack()
{
	uint8_t nack[8];
	nack[0]=ID;
	nack[1]= (packet[1]==0xFF ? 0 : packet[1]+1);
	nack[2]=0; 
	nack[3]=NACK;
	nack[4]=packet[4];
	nack[5]=packet[5];
	uint32_t crc0=HAL_CRC_Calculate(&hcrc,(uint32_t*)nack,6);
	nack[6]=(crc0&0xFF00)>>8;
	nack[7]=crc0&0xFF;
	usb_send(nack,8);
}
	
	
int8_t write_flash(uint8_t *p)
{
	uint16_t addr=p[4]*256+p[5];
	uint8_t len=p[2];
	offset=0;
	if(addr==0x1F80)
	{
		offset=0;
	}
	load_pc(addr);
	for(uint8_t i=0;i<len/2;i++) write_data16(p[i*2+6],p[i*2+1+6],i==(len/2-1));
	write_byte(0xE0);
	write_skip(1);
	burst();
	while(i_cc4);
	delay_us(tpint);
	if(addr&0x8000) delay_us(tpint);
	return 0;
}

uint8_t test_flash(uint8_t *p)
{
	uint8_t* datay=read_flash(p);
	return !memcmp(datay,&p[6],p[2]);
}
	
void send_flash(uint8_t *p)
{
	if((p[4]*256+p[5])&0x8000) p[2]=2;
	else p[2]=64;
	uint8_t* datay=read_flash(p);
	memcpy(&p[6],datay,p[2]);
	p[1]= (p[1]==0xFF ? 0 : p[1]+1);
	p[3]=DATA;
	uint32_t crc0=HAL_CRC_Calculate(&hcrc,(uint32_t*)p,p[2]+6);
	p[p[2]+6]=(crc0&0xFF00)>>8;
	p[p[2]+7]=crc0&0xFF;
	usb_send(p,p[2]+8);
}
	
uint8_t* read_flash(uint8_t *p)
{
	uint16_t addr=p[4]*256+p[5];
	uint8_t len=p[2];
	read_data(addr, len, datax);
	return datax;
}
	
				
void usb_send(uint8_t* dat,uint16_t len)
{
	memcpy(usb_buffer+usb_len,dat,len);
	usb_len+=len;
	for(int i=0;i<10;i++)
	{
		if(CDC_Transmit_FS(dat, usb_len)==USBD_OK)
		{
			usb_len=0;
			return;
		};
		HAL_Delay(10);
	}
}

void gpio_af(void)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = ICSPCLK_Pin|ICSPDAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
	
void gpio_input(void)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = ICSPCLK_Pin|ICSPDAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
	

