#ifndef __STEP_MOTOR_H
#define __STEP_MOTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "fal_def.h"


/**
  * @brief режим подачи тока на обмотки мотора
  */
typedef enum
{
	STEP_MOTOR_POWER_OFF = 0,		/*!< Ток отуствеует		                       					*/
	STEP_MOTOR_POWER_ON				/*!< Ток подан (режим удержания)               					*/
} StepMotor_PowerMode;

/**
  * @brief направления вращения ротора котора
  */
typedef enum
{
	STEP_MOTOR_DIRECTION_FORWARD = 0,	/*!< Вращение по часовой стрелке                         		*/
	STEP_MOTOR_DIRECTION_BACKWARD 		/*!< Вращение против часовой стрелки                       		*/
} StepMotor_Direction;

/**
  * @brief  текущий статус мотора
  */
typedef enum
{
	STEP_MOTOR_STATE_OFF = 0,		/*!< На обмтках мотора отсутсвует ток                         	*/
	STEP_MOTOR_STATE_IDLE,			/*!< На обмотки мотора подан ток. Ротор в режиме удержания   	*/
	STEP_MOTOR_STATE_WORK,			/*!< Мотор находится в режиме вращения 						  	*/
	STEP_MOTOR_STATE_ALARM_STOP		/*!< Мотор в режиме аварийной остановки							*/
} StepMotor_State;

/**
  * @brief дескриптор инстанса шагового мотора
  */
typedef struct __StepMotor_HandleTypeDef
{
	GPIO_TypeDef* __PortPower;
	uint16_t __PinPower;
	GPIO_TypeDef* __PortDirection;
	uint16_t __PinDirection;
	GPIO_TypeDef* __PortStep;
	uint16_t __PinStep;
	uint32_t __RisingDelay;
	uint32_t __FallingDelay;
	GPIO_PinState __DirectionForfardFront;
	GPIO_PinState __PowerOffFront;
	SemaphoreHandle_t __AccessMutex;
	StepMotor_State __State;
} StepMotor_HandleTypeDef;

/**
  * @brief структура инициализации мотора
  */
typedef struct
{
	GPIO_TypeDef* PortPower; 				/*!< Порт управления питанием обмоток									*/

	uint16_t PinPower;						/*!< Пин управления питанием обмоток									*/

	GPIO_TypeDef* PortDirection;			/*!< Порт управления направлением вращения								*/

	uint16_t PinDirection;					/*!< Пин управления направлением вращения								*/

	GPIO_TypeDef* PortStep;					/*!< Порт управления шагом       										*/

	uint16_t PinStep;						/*!< Пин управления шагом        										*/

	uint32_t RisingDelay;					/*!< Продолженительность восходящего импульса шага в мкс				*/

	uint32_t FallingDelay;					/*!< Продолженительность нисходящего импульса шага в мкс 				*/

	GPIO_PinState DirectionForfardFront; 	/*!< Логическая величина фронта сигнала используемого для прямого
												 вращения 															*/

	GPIO_PinState PowerOffFront; 			/*!< Логическая величина фронта сигнала используемого для подачи тока на
	 	 	 	 	 	 	 	 	 	 	 	 обмотки	 														*/

} StepMotor_InitTypeDef;

FAL_StatusTypeDef FAL_StepMotor_Init(StepMotor_HandleTypeDef *hmotor, StepMotor_InitTypeDef init);
FAL_StatusTypeDef FAL_StepMotor_TurnPower(StepMotor_HandleTypeDef *hmotor, StepMotor_PowerMode mode);
FAL_StatusTypeDef FAL_StepMotor_AlarmStop(StepMotor_HandleTypeDef *hmotor);
FAL_StatusTypeDef FAL_StepMotor_OutFromAlarmStop(StepMotor_HandleTypeDef *hmotor);
FAL_StatusTypeDef FAL_StepMotor_SerDirection(StepMotor_HandleTypeDef *hmotor, StepMotor_Direction direction);
FAL_StatusTypeDef FAL_StepMotor_Run(StepMotor_HandleTypeDef *hmotor, uint32_t steps, uint32_t* counter);


#endif
