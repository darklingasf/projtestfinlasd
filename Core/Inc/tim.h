#ifndef TIM_H_
#define TIM_H_

#include "stm32f4xx.h"
#include <stdint.h>

/* Initialize TIM2 for 20 Hz sampling */
void tim2_init(void);

/* Enable / disable sampling timer */
void tim2_start(void);
void tim2_stop(void);
extern volatile uint8_t oled_update_flag;


#endif /* TIM_H_ */
