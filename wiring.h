#ifndef Wiring_h
#define Wiring_h
#include <stdio.h>
#include <stdarg.h>
#include "gpio.h"
#include "uart.h"
#include "rng.h"
#include "w2.h"
#include "pwm.h"
#include "adc.h"
#include "watchdog.h"
#include "pwr_clk_mgmt.h"

#ifdef random
	#undef random
#endif

#define digitalWrite gpio_pin_val_write
#define digitalRead gpio_pin_val_read
#define pinMode gpio_pin_configure
#define OUTPUT GPIO_PIN_CONFIG_OPTION_DIR_OUTPUT
#define INPUT GPIO_PIN_CONFIG_OPTION_DIR_INPUT
#define LOW 0
#define HIGH 1
#define delay delay_ms
#define delayMilliseconds delay_us
#define analogWrite pwm_start
#define analogRead adc_start_single_conversion_get_value
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))
#define interrupts() interrupt_control_global_enable()
#define noInterrupts() interrupt_control_global_disable()
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define bit(b) (1UL << (b))

//PWR_CLK_MGMT_PWRDWN_MODE_ACTIVE, PWR_CLK_MGMT_PWRDWN_MODE_STANDBY, PWR_CLK_MGMT_PWRDWN_MODE_REGISTER_RET
//PWR_CLK_MGMT_PWRDWN_MODE_MEMORY_RET_TMR_ON, PWR_CLK_MGMT_PWRDWN_MODE_MEMORY_RET_TMR_OFF, PWR_CLK_MGMT_PWRDWN_MODE_DEEP_SLEEP
#define sleep(mode) PWRDWN = ((PWRDWN & ~(PWRDWN_PWR_CNTL_MASK)) | mode) 
#define random rng_get_one_byte_and_turn_off
#ifdef DEBUG
	#undef DEBUG
#endif

typedef unsigned int word;
typedef uint8_t boolean;
typedef uint8_t byte;

void setup();
void loop();
void putchar(char c);
char getchar();
void serialBegin();
void wireBegin();
w2_ack_nack_val_t wireWrite8(uint8_t slave_address, uint8_t data);
uint8_t wireRead8(uint8_t slave_address, uint8_t address);
uint16_t wireRead16(uint8_t slave_address, uint8_t address);
void gpioSetup();
#define watchdogRun(p1) watchdog_start_and_set_timeout_in_ms(p1);CLKLFCTRL=1
interrupt_isr_rfirq();

void main(){
	wireBegin();
	gpioSetup();
	setup();

	while(1){loop();}
}

void gpioSetup(){
	adc_configure (ADC_CONFIG_OPTION_RESOLUTION_12_BITS|ADC_CONFIG_OPTION_REF_SELECT_VDD |ADC_CONFIG_OPTION_RESULT_JUSTIFICATION_RIGHT);
	pwm_configure(PWM_CONFIG_OPTION_PRESCALER_VAL_10 || PWM_CONFIG_OPTION_WIDTH_8_BITS);
}

void serialBegin(){

	//Set up UART pin
	gpio_pin_configure(GPIO_PIN_ID_FUNC_TXD,
					   GPIO_PIN_CONFIG_OPTION_DIR_OUTPUT |
					   GPIO_PIN_CONFIG_OPTION_OUTPUT_VAL_SET |
					   GPIO_PIN_CONFIG_OPTION_PIN_MODE_OUTPUT_BUFFER_NORMAL_DRIVE_STRENGTH);

	//Set up UART
	uart_configure_8_n_1_38400();
}

#define debugPrint(fmt, ...) do { if (DEBUG) printf_tiny(fmt, __VA_ARGS__); printf_tiny("\n\r"); } while (0)

void wireBegin(){
	//Set up I2C hardware
	gpio_pin_configure(GPIO_PIN_ID_FUNC_W2SCL,
					   GPIO_PIN_CONFIG_OPTION_DIR_INPUT |
					   GPIO_PIN_CONFIG_OPTION_PIN_MODE_INPUT_BUFFER_ON_PULL_UP_RESISTOR);

	gpio_pin_configure(GPIO_PIN_ID_FUNC_W2SDA,
					   GPIO_PIN_CONFIG_OPTION_DIR_INPUT |
					   GPIO_PIN_CONFIG_OPTION_PIN_MODE_INPUT_BUFFER_ON_PULL_UP_RESISTOR);

	w2_configure(W2_CONFIG_OPTION_ENABLE | W2_CONFIG_OPTION_MODE_MASTER | W2_CONFIG_OPTION_CLOCK_FREQ_400_KHZ | W2_CONFIG_OPTION_ALL_INTERRUPTS_ENABLE, 0);
}

void putchar(char c)
{
	uart_send_wait_for_complete(c);
}

                                                                                                                                                                                                                                            
char getchar()
{
	unsigned char retchar;
	retchar = uart_wait_for_rx_and_get();
	return retchar;
}

w2_ack_nack_val_t wireWrite8(uint8_t slave_address, uint8_t ww8data){
	
	if(w2_master_write_to(slave_address, &ww8data, 1, 0, 0) ==  W2_NACK_VAL)
	{
		return W2_NACK_VAL;
	}else{
		return W2_ACK_VAL;
	}
}

uint8_t wireRead8(uint8_t slave_address, uint8_t address){
	uint8_t wr8data = 0;

	if(w2_master_random_address_read(slave_address, &address, 1, (uint8_t*)&wr8data, 1) ==  W2_NACK_VAL)
	{
		return W2_NACK_VAL;}
	else{
		return wr8data;
	}
}

uint16_t wireRead16(uint8_t slave_address, uint8_t address){
	uint16_t wr16data = 0;

	if(w2_master_random_address_read(slave_address, &address, 1, (uint8_t*)&wr16data, 2) ==  W2_NACK_VAL)
	{
		return W2_NACK_VAL;}
	else{
		return wr16data;
	}
}
#endif
