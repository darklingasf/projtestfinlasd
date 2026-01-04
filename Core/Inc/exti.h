#ifndef EXTI_H_
#define EXTI_H_

#include "stm32f411xe.h"
#include <stdint.h>

/* Flag raised on ADXL345 interrupt */
extern volatile uint8_t adxl_int_flag;

/* Initialize EXTI for ADXL345 INT1 (PA8 → EXTI8) */
void adxl_exti_init(void);

#endif /* EXTI_H_ */
