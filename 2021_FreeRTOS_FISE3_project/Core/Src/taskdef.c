/*
 * taskdef.c
 *
 *  Created on: Jan 20, 2021
 *      Author: Alexandre Humbert - Julien Chavas - florentgoutailler
 */


#include "stm32f429i_discovery_lcd.h"
#include <stdio.h>
#include <stdlib.h>

#include "myLib.h"
#include "main.h"
#include <math.h>
#include "taskdef.h"
#include "i2c.h"


UART_HandleTypeDef huart1;
// Handle
SemaphoreHandle_t SemB_1a;
SemaphoreHandle_t SemB_1b;
SemaphoreHandle_t SemB_3;
SemaphoreHandle_t SemB_4;

SemaphoreHandle_t MutexI2C;

QueueHandle_t xQueueT1a_T2a, xQueueT2a_T3, xQueueT2a_T4, xQueueT1b_T2a, xQueueT1b_T2b, xQueueT2b_T3, xQueueT2b_T4;

typedef struct
{
	double acc[3];
	double gyro[3];
}message_capteur;

typedef struct
{
	double P;
	double T;
}message_capteur2;

typedef struct
{
	double pitch;
	double roll;
	double yaw;
}message_calcul;

typedef struct
{
	double A;
	double T;
}message_calcul2;

void  vCallbackT1(TimerHandle_t xTimer){
	xSemaphoreGive(SemB_1a);
}

void  vCallbackT2(TimerHandle_t xTimer){
	xSemaphoreGive(SemB_1b);
}


void vCallbackT3(TimerHandle_t xTimer){
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



	//---------------------------------------
	// TIMER - END
	//---------------------------------------

	//---------------------------------------
	// SEMAPHORE - START
	//---------------------------------------

	// Création des sémaphores binaires

	SemB_1a = xSemaphoreCreateBinary();
	SemB_1b = xSemaphoreCreateBinary();
	SemB_3  = xSemaphoreCreateBinary();
	SemB_4  = xSemaphoreCreateBinary();

	// Vérification des seémaphores

	if(SemB_1a==NULL || SemB_1b==NULL || SemB_3==NULL || SemB_4==NULL)
	{
		printf("Erreur création sémaphore  !\r\n");
		exit(1);
	}

	/* Create a mutex type semaphore. */

	MutexI2C = xSemaphoreCreateMutex();

	if( MutexI2C == NULL )
	{
		printf("Erreur création sémaphore  !\r\n");
		exit(1);
	 }

	// Debug

	vQueueAddToRegistry(SemB_1a, "SemB_1a");
	vQueueAddToRegistry(SemB_1b, "SemB_1b");
	vQueueAddToRegistry(SemB_3, "SemB_3");
	vQueueAddToRegistry(SemB_4, "SemB_4");
	vQueueAddToRegistry(MutexI2C, "MutexI2C");

	//---------------------------------------
	// SEMAPHORE - END
	//---------------------------------------


	//---------------------------------------
	// TACHE - START
	//---------------------------------------

	xTaskCreate(vTask1a, "task 1a", 1000, NULL, 3, NULL);
	xTaskCreate(vTask1b, "task 1b", 1000, NULL, 3, NULL);
	xTaskCreate(vTask2a, "task 2a", 1000, NULL, 2, NULL);
	xTaskCreate(vTask2b, "task 2b", 1000, NULL, 2, NULL);
	xTaskCreate(vTask3, "task 3", 1000, NULL, 1, NULL);
	xTaskCreate(vTask4, "task 4", 1000, NULL, 1, NULL);

	//---------------------------------------
	// TACHE - END
	//---------------------------------------

	// Creation Queue

	xQueueT1a_T2a = xQueueCreate( 1, sizeof( message_capteur ) );

	if( xQueueT1a_T2a == NULL )
	{
	  printf("Erreur creation queue\n\r");
	  exit(1);
	}

	xQueueT2a_T3 = xQueueCreate( 1, sizeof( message_calcul ) );

	if( xQueueT2a_T3 == NULL )
	{
	  printf("Erreur creation queue\n\r");
      exit(1);
	}

	xQueueT2a_T4 = xQueueCreate( 1, sizeof( message_calcul ) );

	if( xQueueT2a_T4 == NULL )
	{
	  printf("Erreur creation queue\n\r");
	  exit(1);
	}

	xQueueT1b_T2a = xQueueCreate( 1, sizeof( double* ) );

	if( xQueueT1b_T2a == NULL )
	{
	  printf("Erreur creation queue\n\r");
	  exit(1);
	}

	xQueueT1b_T2b = xQueueCreate( 1, sizeof( message_capteur2 ) );

	if( xQueueT1b_T2b == NULL )
	{
	  printf("Erreur creation queue\n\r");
	  exit(1);
	}

	xQueueT2b_T3 = xQueueCreate( 1, sizeof( message_calcul2 ) );

	if( xQueueT2b_T3 == NULL )
	{
	  printf("Erreur creation queue\n\r");
	  exit(1);
	}

	xQueueT2b_T4 = xQueueCreate( 1, sizeof( message_calcul2 ) );

	if( xQueueT2b_T4 == NULL )
	{
	  printf("Erreur creation queue\n\r");
	  exit(1);
	}

	//Debug

	 vQueueAddToRegistry(xQueueT1a_T2a,"xQueueT1a_T2a");
	 vQueueAddToRegistry(xQueueT2a_T3,"xQueueT2a_T3");
	 vQueueAddToRegistry(xQueueT2a_T4,"xQueueT2a_T4");
	 vQueueAddToRegistry(xQueueT1b_T2a,"xQueueT1b_T2a");
	 vQueueAddToRegistry(xQueueT1b_T2b,"xQueueT1b_T2b");
	 vQueueAddToRegistry(xQueueT2b_T3,"xQueueT2b_T3");
	 vQueueAddToRegistry(xQueueT2b_T4,"xQueueT2b_T4");
	// Demarrage des timers
	if( ( Timer1 != NULL ) && ( Timer2 != NULL ) && ( Timer3 != NULL ) ){
		xTimer1Started = xTimerStart( Timer1, 0 );
		xTimer2Started = xTimerStart( Timer2, 0 );
		xTimer3Started = xTimerStart( Timer3, 0 );
	}

	if( ( xTimer1Started == pdPASS ) && ( xTimer2Started == pdPASS ) && ( xTimer3Started == pdPASS ) )
	 {
		printf("Timers running\n\r");
	 }
	// Destruction de la tache d'initialisation
	vTaskDelete(NULL);
}


void vTask1a(void *pvParameters ){
	message_capteur message;

	while(1)
	{
		xSemaphoreTake(SemB_1a, portMAX_DELAY);

        xSemaphoreTake( MutexI2C, portMAX_DELAY);

		MeasureA(&hi2c3,message.acc);
		MeasureG(&hi2c3,message.gyro);



	    if( xQueueSend( xQueueT1a_T2a, ( void * ) &message, 0) != pdPASS )
	    {
	        printf("Erreur envoi data\n\r");
	    }

	    xSemaphoreGive( MutexI2C );
	  }
}


void vTask1b(void *pvParameters ){
	double mag[3];
	double bias[3]={0,0,0};
	double scale[3]={1,1,1};

	int32_t Tfine;

	message_capteur2 message;
	while(1)
	{
		xSemaphoreTake(SemB_1b, portMAX_DELAY);

        xSemaphoreTake( MutexI2C, portMAX_DELAY);

		MeasureM(&hi2c3,mag,bias,scale);
		MeasureT_BMP280(&hi2c3,&message.T,&Tfine);
		MeasureP(&hi2c3,&message.P,&Tfine);


	    if( xQueueSend( xQueueT1b_T2a, ( void * ) mag, portMAX_DELAY) != pdPASS )
	    {
	        printf("Erreur envoi data\n\r");
	    }


	    if( xQueueSend( xQueueT1b_T2b, ( void * ) &message, portMAX_DELAY) != pdPASS )
	    {
	        printf("Erreur envoi data\n\r");
	    }

	    xSemaphoreGive( MutexI2C );
	}
}

void vTask2a(void *pvParameters ){
	message_capteur message;
	double mag[3];
	message_calcul messageT3T4;
	int i = 0;
	int j=0;

	while(1)
	{


		if( xQueueReceive( xQueueT1a_T2a, ( void * ) &message, portMAX_DELAY ) == pdPASS )
		{
			if (j == 1){ // On récupère une valeur tous les 2 calculs (T1a 100 Hz et T1b 50 Hz)
			if( xQueueReceive( xQueueT1b_T2a, ( void * ) mag, portMAX_DELAY ) == pdPASS )
			    j = 0;
				}
				else{
					j++;
				}

			Pitch(&hi2c3, message.acc, &messageT3T4.pitch);
			Roll(&hi2c3, message.acc, &messageT3T4.roll);
			Yaw2(&hi2c3, message.acc, message.gyro, mag, &messageT3T4.yaw);

			if(i == 19) // On envoie une valeur sur 20
			{
				if( xQueueSend( xQueueT2a_T3, ( void * ) &messageT3T4, portMAX_DELAY) != pdPASS )
				{
					printf("Erreur envoi data\n\r");
				}
				if( xQueueSend( xQueueT2a_T4, ( void * ) &messageT3T4, portMAX_DELAY) != pdPASS )
				{
					printf("Erreur envoi data\n\r");
				}
				i = 0;
			}
			else{
				i++;
				}

		}
	}
}

void vTask2b(void *pvParameters ){
	message_capteur2 messageRecu;

	message_calcul2 messageEnv;
	int i = 0;

	while(1)
	{
		if( xQueueReceive( xQueueT1b_T2b, ( void * ) &messageRecu, portMAX_DELAY ) == pdPASS )
		{
				messageEnv.A = 288.15/0.0065*(1 - pow(messageRecu.P/101325,1.0/5.255)); // calcul de l'altitude
				messageEnv.T = messageRecu.T;
				if(i == 9) // On envoie une valeur sur 10
				{
					if( xQueueSend( xQueueT2b_T3, ( void * ) &messageEnv, portMAX_DELAY) != pdPASS )
					{
						printf("Erreur envoi data\n\r");
					}

					if( xQueueSend( xQueueT2b_T4, ( void * ) &messageEnv, portMAX_DELAY) != pdPASS )
					{
						printf("Erreur envoi data\n\r");
					}
					i = 0;
				}
				else{
					i++;
				}
			}
		else
		{
			printf("Erreur recepetion data\n\r");
		}
	}
}

void vTask3(void *pvParameters ){
	message_calcul messageT2a;
	message_calcul2 messageT2b;
	while(1)
	{
		xSemaphoreTake(SemB_3, portMAX_DELAY);
		if( xQueueReceive( xQueueT2a_T3, ( void * ) &messageT2a, portMAX_DELAY ) != pdPASS )
		{
			printf("Erreur recepetion data RS232\n\r");
		}
		if( xQueueReceive( xQueueT2b_T3, ( void * ) &messageT2b, portMAX_DELAY ) != pdPASS )
		{
			printf("Erreur recepetion data RS232\n\r");
		}
		printf("pitch=%2.2f deg, roll=%2.2f deg/s, yaw=%2.2f deg/s,  altitude=%2.2f m, temperature=%2.2f deg \n\r",messageT2a.roll, messageT2a.pitch, messageT2a.yaw, messageT2b.A, messageT2b.T);


	}
}

void vTask4(void *pvParameters ){
	message_calcul messageT2a;
	message_calcul2 messageT2b;
	while(1)
	{
		xSemaphoreTake(SemB_4, portMAX_DELAY);
		if( xQueueReceive( xQueueT2a_T4, ( void * ) &messageT2a, portMAX_DELAY ) != pdPASS )
		{
			printf("Erreur recepetion data LCD\n\r");
		}
		if( xQueueReceive( xQueueT2b_T4, ( void * ) &messageT2b, portMAX_DELAY ) != pdPASS )
		{
			printf("Erreur recepetion data RS232\n\r");
		}
		GUI(messageT2a.roll, messageT2a.pitch,  messageT2a.yaw , messageT2b.A, messageT2b.T);


	}
}





