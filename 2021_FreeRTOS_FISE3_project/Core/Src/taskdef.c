/*
 * taskdef.c
 *
 *  Created on: Jan 20, 2021
 *      Author: florentgoutailler
 */


#include "stm32f429i_discovery_lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "taskdef.h"


UART_HandleTypeDef huart1;
// Handle
SemaphoreHandle_t SemB_1a;
SemaphoreHandle_t SemB_1b;
SemaphoreHandle_t SemB_3;
SemaphoreHandle_t SemB_4;

void  vCallbackT1(TimerHandle_t xTimer){
	printf("Timer1");
	xSemaphoreGive(SemB_1a);
}

void  vCallbackT2(TimerHandle_t xTimer){
	printf("Timer2");
	xSemaphoreGive(SemB_1b);
}


void vCallbackT3(TimerHandle_t xTimer){
	printf("Timer3");
	xSemaphoreGive(SemB_3);
	xSemaphoreGive(SemB_4);
}




void vTaskInit(void *pvParameters ){
	TimerHandle_t Timer1, Timer2, Timer3;
	BaseType_t xTimer1Started, xTimer2Started, xTimer3Started;


	//---------------------------------------
	// TIMER - START
	//---------------------------------------

	// Creation des timers
	Timer1 = xTimerCreate("Timer1", pdMS_TO_TICKS(10), pdTRUE, ( void * ) 0, vCallbackT1 );
	Timer2 = xTimerCreate("Timer2", pdMS_TO_TICKS(20), pdTRUE, ( void * ) 0, vCallbackT2 );
	Timer3 = xTimerCreate("Timer3", pdMS_TO_TICKS(100), pdTRUE, ( void * ) 0, vCallbackT3 );

	// Demarrage des timers
	if( ( Timer1 != NULL ) && ( Timer2 != NULL ) && ( Timer3 != NULL ) ){
		xTimer1Started = xTimerStart( Timer1, 0 );
		xTimer2Started = xTimerStart( Timer3, 0 );
		xTimer3Started = xTimerStart( Timer3, 0 );
	}


	if( ( xTimer1Started == pdPASS ) && ( xTimer2Started == pdPASS ) && ( xTimer3Started == pdPASS ) )
	 {
		printf("Timers running");
	 }

	//---------------------------------------
	// TIMER - END
	//---------------------------------------

	//---------------------------------------
	// SEMAPHORE - START
	//---------------------------------------

	// création des sémaphores binaires

	SemB_1a = xSemaphoreCreateBinary();
	SemB_1b = xSemaphoreCreateBinary();
	SemB_3  = xSemaphoreCreateBinary();
	SemB_4  = xSemaphoreCreateBinary();

	// vérification

	if(SemB_1a==NULL || SemB_1b==NULL || SemB_3==NULL || SemB_4==NULL)
	{
		printf("Erreur création sémaphore  !\r\n");
		exit(1);
	}

	// Debug

	vQueueAddToRegistry(SemB_1a, "SemB_1a");
	vQueueAddToRegistry(SemB_1b, "SemB_1b");
	vQueueAddToRegistry(SemB_3, "SemB_3");
	vQueueAddToRegistry(SemB_4, "SemB_4");


	//---------------------------------------
	// SEMAPHORE - END
	//---------------------------------------


	//---------------------------------------
	// TACHE - START
	//---------------------------------------

	xTaskCreate(vTask1a, "task 1a", 1000, NULL, 3, NULL);
	xTaskCreate(vTask1b, "task 1b", 1000, NULL, 3, NULL);
	xTaskCreate(vTask2a, "task 1a", 1000, NULL, 2, NULL);
	xTaskCreate(vTask2b, "task 1b", 1000, NULL, 2, NULL);
	xTaskCreate(vTask1a, "task 3", 1000, NULL, 1, NULL);
	xTaskCreate(vTask1b, "task 4", 1000, NULL, 1, NULL);

	//---------------------------------------
	// TACHE - END
	//---------------------------------------



	// Destruction de la tache
	vTaskDelete(NULL);
}


void vTask1a(void *pvParameters ){
	while(1)
	{
		xSemaphoreTake(SemB_1a, portMAX_DELAY);
	}
}

void vTask1b(void *pvParameters ){
	while(1)
	{
		xSemaphoreTake(SemB_1b, portMAX_DELAY);

	}
}

void vTask2a(void *pvParameters ){
	while(1)
	{


	}
}

void vTask2b(void *pvParameters ){
	while(1)
	{

	}
}

void vTask3(void *pvParameters ){
	while(1)
	{
		xSemaphoreTake(SemB_3, portMAX_DELAY);

	}
}

void vTask4(void *pvParameters ){
	while(1)
	{
		xSemaphoreTake(SemB_4, portMAX_DELAY);

	}
}





