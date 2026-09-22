# EXTI\_I2C\_FreeRTOS 



### Documentation is in progress!



void MX\_FREERTOS\_Init(void) {



&#x09;/\* UART queue: store up to 10 messages, each 64 bytes \*/

&#x09;uartQueue = xQueueCreate(10, sizeof(char\*));



&#x09;/\* EXTI semaphore \*/

&#x09;extiSemaphore = xSemaphoreCreateBinary();



&#x09;/\* Timer wakeup task (High priority) \*/

&#x09;xTaskCreate(timerWakeTask, "timer", 256, NULL, 5, \&timerTaskHandle);



&#x09;/\* Sensor task (Medium-2 priority) \*/

&#x09;xTaskCreate(sensorTask, "sensor", 256, NULL, 4, \&sensorTaskHandle);



&#x09;/\* EXTI task (Medium-1 priority) \*/

&#x09;xTaskCreate(extiTask, "exti", 256, NULL, 3, \&extiTaskHandle);



&#x09;/\* UART printing task (Medium priority) \*/

&#x09;xTaskCreate(uartTask, "uart", 256, NULL, 2, \&uartTaskHandle);

}

