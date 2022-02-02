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

void  vCallbackT1(TimerHandle_t xTimer){
	printf("Timer1");
}

void  vCallbackT2(TimerHandle_t xTimer){
	printf("Timer2");
}


void vCallbackT3(TimerHandle_t xTimer){
	printf("Timer3");
}

void vTaskInit(void *pvParameters ){
	TimerHandle_t Timer1, Timer2, Timer3;
	BaseType_t xTimer1Started, xTimer2Started, xTimer3Started;

	// Creation des timers
	Timer1 = xTimerCreate("Timer1", 10, pdTRUE, ( void * ) 0, vCallbackT1 );
	Timer2 = xTimerCreate("Timer2", 20, pdTRUE, ( void * ) 0, vCallbackT2 );
	Timer3 = xTimerCreate("Timer3", 200, pdTRUE, ( void * ) 0, vCallbackT3 );

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

	// Destruction de la tache
	vTaskDelete(NULL);
}








