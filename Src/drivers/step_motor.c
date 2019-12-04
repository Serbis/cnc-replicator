#include "step_motor.h"
#include "dwt_delay.h"

/**
  * @brief  Инициализирует экземпляр драйвера шагового мотора
  * @param  hmotor указатель на стуктуру состояния драйвера
  * @param  init структура инициализатор драйвера
  * @retval всегда возвращается FAL_OK
  */
FAL_StatusTypeDef FAL_StepMotor_Init(StepMotor_HandleTypeDef *hmotor, StepMotor_InitTypeDef init)
{
	hmotor->__PortPower = init.PortPower;
	hmotor->__PinPower = init.PinPower;
	hmotor->__PortDirection = init.PortDirection;
	hmotor->__PinDirection = init.PinDirection;
	hmotor->__PortStep = init.PortStep;
	hmotor->__PinStep = init.PinStep;
	hmotor->__RisingDelay = init.RisingDelay;
	hmotor->__FallingDelay = init.FallingDelay;
	hmotor->__PowerOffFront = init.PowerOffFront;
	hmotor->__DirectionForfardFront = init.DirectionForfardFront;
	hmotor->__AccessMutex = xSemaphoreCreateMutex();
	hmotor->__State = STEP_MOTOR_STATE_OFF;

	HAL_GPIO_WritePin(hmotor->__PortDirection, hmotor->__PinDirection, GPIO_PIN_RESET);

	return FAL_OK;
}

/**
  * @brief  Выполняет изменение режима питания обмоток
  * @param  hmotor указатель на стуктуру состояния драйвера
  * @param  mode режим питания
  * @retval Возвращает FAL_OK в случае успеха или FAL_BUSY если не драйвер находится в заблоировнном состоянии или
  * 		FAL_ERROR если имеет место состояние аварийного останова
  */
FAL_StatusTypeDef FAL_StepMotor_TurnPower(StepMotor_HandleTypeDef *hmotor, StepMotor_PowerMode mode)
{
	if (xSemaphoreTake(hmotor->__AccessMutex, ( TickType_t ) 10))
	{
		if (hmotor->__State == STEP_MOTOR_STATE_ALARM_STOP)
		{
			xSemaphoreGive(hmotor->__AccessMutex);

			return FAL_ERROR;
		}

		if (mode == STEP_MOTOR_POWER_OFF) {
			hmotor->__State = STEP_MOTOR_STATE_OFF;
			HAL_GPIO_WritePin(hmotor->__PortPower, hmotor->__PinPower, hmotor->__PowerOffFront);
		} else {
			hmotor->__State = STEP_MOTOR_STATE_IDLE;
			if (hmotor->__PowerOffFront == GPIO_PIN_RESET)
				HAL_GPIO_WritePin(hmotor->__PortPower, hmotor->__PinPower, GPIO_PIN_SET);
			else
				HAL_GPIO_WritePin(hmotor->__PortPower, hmotor->__PinPower, GPIO_PIN_RESET);
		}


		xSemaphoreGive(hmotor->__AccessMutex);

		return FAL_OK;
	}

	return FAL_BUSY;
}

/**
  * @brief  Выполняет аварийный останов мотора, а именно симает питание с обмоток и выстываляет соотвествующий флаг
  *  		состояния. Данный возов гарантирует, что с начала момента его работы, ротор мотора сдвинется не более чем на
  *  		один шаг, до явного вызова фунации FAL_StepMotor_OutFromAlarmStop. Может вызываться из прерывания.
  * @param  hmotor указатель на стуктуру состояния драйвера
  * @retval всегда возвращается FAL_OK
  */
FAL_StatusTypeDef FAL_StepMotor_AlarmStop(StepMotor_HandleTypeDef *hmotor)
{
	UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	HAL_GPIO_WritePin(hmotor->__PortPower, hmotor->__PinPower, hmotor->__PowerOffFront);
	hmotor->__State = STEP_MOTOR_STATE_ALARM_STOP;
	taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

	return FAL_OK;
}

/**
  * @brief  Выполняет выход из режима аварийного останова
  * @param  hmotor указатель на стуктуру состояния драйвера
  * @retval Возвращает FAL_OK в случае успеха или FAL_BUSY если не драйвер находится в заблоировнном состоянии
  */
FAL_StatusTypeDef FAL_StepMotor_OutFromAlarmStop(StepMotor_HandleTypeDef *hmotor)
{
	if (xSemaphoreTake(hmotor->__AccessMutex, ( TickType_t ) 10)) {
		if (hmotor->__State == STEP_MOTOR_STATE_ALARM_STOP) {
			hmotor->__State = STEP_MOTOR_STATE_IDLE;
		}

		xSemaphoreGive(hmotor->__AccessMutex);

		return FAL_OK;
	}

	return FAL_BUSY;
}

/**
  * @brief  Выполняет изменение направление вращения ротора
  * @param  hmotor указатель на стуктуру состояния драйвера
  * @param  direction направление вращения
  * @retval Возвращает FAL_OK в случае успеха или FAL_BUSY если не драйвер находится в заблоировнном состоянии или
  * 		FAL_ERROR если имеет место состояние аварийного останова
  */
FAL_StatusTypeDef FAL_StepMotor_SerDirection(StepMotor_HandleTypeDef *hmotor, StepMotor_Direction direction)
{
	if (xSemaphoreTake(hmotor->__AccessMutex, ( TickType_t ) 10)) {
			if (hmotor->__State == STEP_MOTOR_STATE_ALARM_STOP) {
				xSemaphoreGive(hmotor->__AccessMutex);

				return FAL_ERROR;
			}


			if (direction == STEP_MOTOR_DIRECTION_FORWARD) {
				HAL_GPIO_WritePin(hmotor->__PortDirection, hmotor->__PinDirection, hmotor->__DirectionForfardFront);
			} else {
				if (hmotor->__DirectionForfardFront == GPIO_PIN_RESET)
					HAL_GPIO_WritePin(hmotor->__PortDirection, hmotor->__PinDirection, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(hmotor->__PortDirection, hmotor->__PinDirection, GPIO_PIN_RESET);
			}


			xSemaphoreGive(hmotor->__AccessMutex);

			return FAL_OK;
		}

		return FAL_BUSY;
}

/**
  * @brief  Выполняет вращение ротора на заданное количество шагов. Следует обратить внимание на то, что данный метод
  * 		явлется блокирующим и он не отдаст управление пока мотор не отрабатает заданное количество шагов или не
  * 		произойдет событие аварийного останова
  * @param  hmotor указатель на стуктуру состояния драйвера
  * @param  steps количество шагов
  * @param  counter счетчик фактического количества шагов проденных ротором. Может принимать значение NULL
  * @retval Возвращает FAL_OK в случае успеха или FAL_BUSY если не драйвер находится в заблоировнном состоянии или
  * 		FAL_ERROR если драйвер находится не в состоянии STEP_MOTOR_STATE_IDLE или при отработе шагов возинкла
  * 		ситуация аварийного останова
  */
FAL_StatusTypeDef FAL_StepMotor_Run(StepMotor_HandleTypeDef *hmotor, uint32_t steps, uint32_t* counter)
{
	if (xSemaphoreTake(hmotor->__AccessMutex, ( TickType_t ) 10)) {
		if (hmotor->__State != STEP_MOTOR_STATE_IDLE) {
			xSemaphoreGive(hmotor->__AccessMutex);

			return FAL_ERROR;
		}

		hmotor->__State = STEP_MOTOR_STATE_WORK;

		uint32_t risingDelay = hmotor->__RisingDelay;
		uint32_t fallingDelay = hmotor-> __FallingDelay;

		for (uint32_t i = 0; i < steps; i++) {
			if (hmotor->__State == STEP_MOTOR_STATE_ALARM_STOP) {
				HAL_GPIO_WritePin(hmotor->__PortDirection, hmotor->__PinDirection, GPIO_PIN_RESET);

				xSemaphoreGive(hmotor->__AccessMutex);

				return FAL_ERROR;
			}

			HAL_GPIO_WritePin(hmotor->__PortStep, hmotor->__PinStep, GPIO_PIN_SET);
			DWT_Delay(risingDelay);
			HAL_GPIO_WritePin(hmotor->__PortStep, hmotor->__PinStep, GPIO_PIN_RESET);
			DWT_Delay(fallingDelay);

			if (counter != NULL)
				*counter++;
		}

		hmotor->__State = STEP_MOTOR_STATE_IDLE;

		xSemaphoreGive(hmotor->__AccessMutex);

		return FAL_OK;
	}

	return FAL_BUSY;
}
