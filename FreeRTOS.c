/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "sht31.h"
#include "queue.h"
#include "semphr.h"

extern UART_HandleTypeDef huart2;

void timerWakeTask(void *argument);
void sensorTask(void *argument);
void extiTask(void *argument);
void uartTask(void *argument);

/* Queue for UART messages */
QueueHandle_t uartQueue;

/* Binary semaphore for EXTI button */
SemaphoreHandle_t extiSemaphore;

/* Task notifications for TIM3 wakeup */
TaskHandle_t timerTaskHandle;

/* Task handles */
TaskHandle_t sensorTaskHandle;
TaskHandle_t extiTaskHandle;
TaskHandle_t uartTaskHandle;

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook(void) {
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	 to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
	 task. It is essential that code added to this hook function never attempts
	 to block in any way (for example, call xQueueReceive() with a block time
	 specified, or call vTaskDelay()). If the application makes use of the
	 vTaskDelete() API function (as this demo application does) then it is also
	 important that vApplicationIdleHook() is permitted to return to its calling
	 function, because it is the responsibility of the idle task to clean up
	 memory allocated by the kernel to any task that has since been deleted. */

	/* Turn LED ON (active mode indicator) */
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

void MX_FREERTOS_Init(void) {

	/* UART queue: store up to 10 messages, each 64 bytes */
	uartQueue = xQueueCreate(10, sizeof(char*));

	/* EXTI semaphore */
	extiSemaphore = xSemaphoreCreateBinary();

	/* Timer wakeup task (High priority) */
	xTaskCreate(timerWakeTask, "timer", 256, NULL, 5, &timerTaskHandle);

	/* Sensor task (Medium-2 priority) */
	xTaskCreate(sensorTask, "sensor", 256, NULL, 4, &sensorTaskHandle);

	/* EXTI task (Medium-1 priority) */
	xTaskCreate(extiTask, "exti", 256, NULL, 3, &extiTaskHandle);

	/* UART printing task (Medium priority) */
	xTaskCreate(uartTask, "uart", 256, NULL, 2, &uartTaskHandle);

}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void timerWakeTask(void *argument) {
	for (;;) {

		/* Wait for TIM3 interrupt notification */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

		/* Turn LED OFF (active mode indicator) */
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

	//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

		/* Wake sensor task */
		xTaskNotifyGive(sensorTaskHandle);
	}
}

void sensorTask(void *argument) {
	static char msg[64];
	char *pMsg = msg;

	for (;;) {
		/* Wait for task notification by TIM3 */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);

		int temp, hum;

		/* Measure temperature and humidity */
		if (sht31_read(&temp, &hum) == HAL_OK) {
			sprintf(msg, "T=%d H=%d\r\n", temp, hum);
		} else {
			sprintf(msg, "SHT31 error\r\n");
		}

		xQueueSend(uartQueue, &pMsg, 0);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

	}
}

void extiTask(void *argument) {
	static char msg[] = "Button pressed!\r\n"; // static - lifetime through out the program
	char *pMsg = msg;

	for (;;) {
		/* Wait for EXTI semaphore */
		xSemaphoreTake(extiSemaphore, portMAX_DELAY);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

		xQueueSend(uartQueue, &pMsg, 0);        // send pointer

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	}
}

void uartTask(void *argument) {
	char *pMsg;

	for (;;) {
		/* Block until a message arrives */
		if (xQueueReceive(uartQueue, &pMsg, portMAX_DELAY) == pdTRUE) {

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);

			/* Prints the Queue */
			HAL_UART_Transmit(&huart2, (uint8_t*) pMsg, strlen(pMsg),
			HAL_MAX_DELAY);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);

		}
	}
}

void PreSleepProcessing(uint32_t expectedIdleTime) {
	// Mark sleep entry
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);

	HAL_SuspendTick();

	// Enter sleep
	__WFI();

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
}

void PostSleepProcessing(uint32_t expectedIdleTime) {
	// Mark wakeup
	HAL_ResumeTick();

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
}

/* USER CODE END Application */

