#ifndef __FAL_DEF_H
#define __FAL_DEF_H

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

typedef enum
{
  FAL_OK       = 0x00U,
  FAL_ERROR    = 0x01U,
  FAL_BUSY     = 0x02U,
  FAL_TIMEOUT  = 0x03U
} FAL_StatusTypeDef;

#endif
