#ifndef EXTI_H_
#define EXTI_H_

#include "stm32f4xx.h"
#include <stdint.h>

/* Global flag set by EXTI interrupt */
extern volatile uint8_t adxl_int_flag;

/* Initialize EXTI for ADXL345 INT1 */
void adxl_exti_init(void);

#endif /* EXTI_H_ */
