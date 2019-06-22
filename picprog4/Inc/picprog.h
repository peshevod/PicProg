#ifndef __picprog_H
#define __picprog_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "stm32f3xx_hal.h"
	 
#define MAX_ARRAY_BUF 1500
#define USB_BUFFER_SIZE 1024
#define FB_SIZE 128
#define MAX_ROW_SIZE	64
#define FREQ 1000000

#define ID						0x44

#define FLAG_ANS			0x10	 
#define FLAG_CMD			0x20
#define FLAG_ERASE		0x40
#define FLAG_CHECK		0x80
	 
#define ACK					(0x00|FLAG_ANS)
#define START				(0x01|FLAG_CMD)
#define WRITE				(0x02|FLAG_CMD)
#define ERASE_ALL		(0x03|FLAG_CMD)
#define READ				(0x04|FLAG_CMD)
#define NACK				(0x05|FLAG_ANS)
#define	DATA				(0x06|FLAG_ANS)
#define STOP				(0x07|FLAG_CMD)
	 
#define N_OF_WRITE_TRIES 5
	 

void delay_us(uint32_t us);
uint32_t get_timer2_freq(void);	 
void picprog_init(void);
void enter_pm(void);
void exit_pm(void);
void write_byte(uint8_t data_byte);
void write_skip(uint8_t nskip);
void burst(void);
void PB5_input(void);
void PB5_af(void);
void read_data(uint16_t ad, uint16_t len, uint8_t* data);
void load_pc(uint16_t ad);
void row_erase(uint8_t* p);
void erase_all(void);
uint8_t fb_read(void);
void write_data16(uint8_t msb, uint8_t lsb,uint8_t onlyone);
void write_data8(uint8_t b);
int8_t write_flash(uint8_t *p);
uint8_t test_flash(uint8_t *p);
void usb_send(uint8_t* dat,uint16_t len);
void send_ack(void);
void send_nack(void);
uint8_t* read_flash(uint8_t *p);
void send_flash(uint8_t *p);
void gpio_af(void);
void gpio_input(void);



#ifdef __cplusplus
 }
#endif
#endif /*__ picprog_H */

