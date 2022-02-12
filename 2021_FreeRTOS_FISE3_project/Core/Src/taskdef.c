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

#define DEBUG 1

UART_HandleTypeDef huart1;

/* Binary Semaphore Handles */
SemaphoreHandle_t SemB_1a, SemB_1b, SemB_3, SemB_4;

/* Mutex Semaphore Handles */
SemaphoreHandle_t MutexI2C;

/* Queue Handles */
QueueHandle_t xQueueT1a_T2a, xQueueT2a_T3, xQueueT2a_T4, xQueueT1b_T2a,
		xQueueT1b_T2b, xQueueT2b_T3, xQueueT2b_T4;

/* Data structure for task 1a output */
typedef struct {
	double acc[3];
	double gyro[3];
} message_capteur;

/* Data structure for task 1b output */
typedef struct {
	double P; //Pressure
	double T; //Temperature
} message_capteur2;

/* Data structure for task 2a output */
typedef struct {
	double pitch;
	double roll;
	double yaw;
} message_calcul;

/* Data structure for task 2b output */
typedef struct {
	double A; //Altitude
	double T; //Temperature
} message_calcul2;

/* Callback function Timer 1 */
void vCallbackT1(TimerHandle_t xTimer) {
	xSemaphoreGive(SemB_1a);
}

/* Callback function Timer 2 */
void vCallbackT2(TimerHandle_t xTimer) {
	xSemaphoreGive(SemB_1b);
}

/* Callback function Timer 3 */
void vCallbackT3(TimerHandle_t xTimer) {
	xSemaphoreGive(SemB_3);
	xSemaphoreGive(SemB_4);
}

/* Initialization Task */
void vTaskInit(void *pvParameters) {
	TimerHandle_t Timer1, Timer2, Timer3;
	BaseType_t xTimer1Started, xTimer2Started, xTimer3Started;

	// Create timers
	Timer1 = xTimerCreate("Timer1", pdMS_TO_TICKS(10), pdTRUE, (void*) 0,
			vCallbackT1); // 100 Hz
	Timer2 = xTimerCreate("Timer2", pdMS_TO_TICKS(20), pdTRUE, (void*) 0,
			vCallbackT2); // 50 Hz
	Timer3 = xTimerCreate("Timer3", pdMS_TO_TICKS(200), pdTRUE, (void*) 0,
			vCallbackT3); // 5 Hz

	// Create binary semaphores

	SemB_1a = xSemaphoreCreateBinary();
	SemB_1b = xSemaphoreCreateBinary();
	SemB_3 = xSemaphoreCreateBinary();
	SemB_4 = xSemaphoreCreateBinary();

	// Semaphores check

	if (SemB_1a == NULL || SemB_1b == NULL || SemB_3 == NULL || SemB_4 == NULL) {
		printf("Failed to create binary semaphores\r\n");
		exit(1);
	}

	/* Create a mutex type semaphore. */

	MutexI2C = xSemaphoreCreateMutex();

	if (MutexI2C == NULL) {
		printf("Failed to create mutex semaphores\r\n");
		exit(1);
	}

#if DEBUG
	vQueueAddToRegistry(SemB_1a, "SemB_1a");
	vQueueAddToRegistry(SemB_1b, "SemB_1b");
	vQueueAddToRegistry(SemB_3, "SemB_3");
	vQueueAddToRegistry(SemB_4, "SemB_4");
	vQueueAddToRegistry(MutexI2C, "MutexI2C");
#endif

	// Create tasks

	xTaskCreate(vTask1a, "task 1a", 1000, NULL, 3, NULL);
	xTaskCreate(vTask1b, "task 1b", 1000, NULL, 3, NULL);
	xTaskCreate(vTask2a, "task 2a", 1000, NULL, 2, NULL);
	xTaskCreate(vTask2b, "task 2b", 1000, NULL, 2, NULL);
	xTaskCreate(vTask3, "task 3", 1000, NULL, 1, NULL);
	xTaskCreate(vTask4, "task 4", 1000, NULL, 1, NULL);

	// Create queues

	xQueueT1a_T2a = xQueueCreate(1, sizeof(message_capteur));

	if (xQueueT1a_T2a == NULL) {
		printf("Failed to create queue T1a_T2a\n\r");
		exit(1);
	}

	xQueueT2a_T3 = xQueueCreate(1, sizeof(message_calcul));

	if (xQueueT2a_T3 == NULL) {
		printf("Failed to create queue T2a_T3\n\r");
		exit(1);
	}

	xQueueT2a_T4 = xQueueCreate(1, sizeof(message_calcul));

	if (xQueueT2a_T4 == NULL) {
		printf("Failed to create queue T2a_T4\n\r");
		exit(1);
	}

	xQueueT1b_T2a = xQueueCreate(1, sizeof(double*));

	if (xQueueT1b_T2a == NULL) {
		printf("Failed to create queue T1b_T2a\n\r");
		exit(1);
	}

	xQueueT1b_T2b = xQueueCreate(1, sizeof(message_capteur2));

	if (xQueueT1b_T2b == NULL) {
		printf("Failed to create queue T1b_T2b\n\r");
		exit(1);
	}

	xQueueT2b_T3 = xQueueCreate(1, sizeof(message_calcul2));

	if (xQueueT2b_T3 == NULL) {
		printf("Failed to create queue T2b_T3\n\r");
		exit(1);
	}

	xQueueT2b_T4 = xQueueCreate(1, sizeof(message_calcul2));

	if (xQueueT2b_T4 == NULL) {
		printf("Failed to create queue T2b_T4\n\r");
		exit(1);
	}

#if DEBUG
	vQueueAddToRegistry(xQueueT1a_T2a, "xQueueT1a_T2a");
	vQueueAddToRegistry(xQueueT2a_T3, "xQueueT2a_T3");
	vQueueAddToRegistry(xQueueT2a_T4, "xQueueT2a_T4");
	vQueueAddToRegistry(xQueueT1b_T2a, "xQueueT1b_T2a");
	vQueueAddToRegistry(xQueueT1b_T2b, "xQueueT1b_T2b");
	vQueueAddToRegistry(xQueueT2b_T3, "xQueueT2b_T3");
	vQueueAddToRegistry(xQueueT2b_T4, "xQueueT2b_T4");
#endif

	// Start of timers
	if ((Timer1 != NULL) && (Timer2 != NULL) && (Timer3 != NULL)) {
		xTimer1Started = xTimerStart(Timer1, 0);
		xTimer2Started = xTimerStart(Timer2, 0);
		xTimer3Started = xTimerStart(Timer3, 0);
	}

	if ((xTimer1Started == pdPASS) && (xTimer2Started == pdPASS)
			&& (xTimer3Started == pdPASS)) {
		printf("Timers running\n\r");
	}
	// Delete the initialization task
	vTaskDelete(NULL);
}

void vTask1a(void *pvParameters) {
	message_capteur message;

	while (1) {
		xSemaphoreTake(SemB_1a, portMAX_DELAY); // Wait for Timer 1

		xSemaphoreTake(MutexI2C, portMAX_DELAY); // Start of critical section I2C

		/* Measure acceleration and angular velocity */
		MeasureA(&hi2c3, message.acc);
		MeasureG(&hi2c3, message.gyro);

		/* Send data to task 2a */
		if ( xQueueSend( xQueueT1a_T2a, ( void * ) &message, 0) != pdPASS) {
			printf("Failed to send data\n\r");
		}

		xSemaphoreGive(MutexI2C); // End of critical section I2C
	}
}

void vTask1b(void *pvParameters) {
	double mag[3];
	double bias[3] = { 0, 0, 0 };
	double scale[3] = { 1, 1, 1 };

	int32_t Tfine;

	message_capteur2 message;
	while (1) {
		xSemaphoreTake(SemB_1b, portMAX_DELAY); // Wait for Timer 2

		xSemaphoreTake(MutexI2C, portMAX_DELAY); // Start of critical section I2C

		/* Measure magnetic field, temperature and pressure */
		MeasureM(&hi2c3, mag, bias, scale);
		MeasureT_BMP280(&hi2c3, &message.T, &Tfine);
		MeasureP(&hi2c3, &message.P, &Tfine);

		/* Send magnetic field to task 2a */
		if ( xQueueSend( xQueueT1b_T2a, ( void * ) mag, portMAX_DELAY) != pdPASS) {
			printf("Failed to send data\n\r");
		}

		/* Send temperature and pressure to task 2b */
		if ( xQueueSend(xQueueT1b_T2b, (void* ) &message,
				portMAX_DELAY) != pdPASS) {
			printf("Failed to send data\n\r");
		}

		xSemaphoreGive(MutexI2C); // End of critical section I2C
	}
}

void vTask2a(void *pvParameters) {
	message_capteur message;
	double mag[3];
	message_calcul messageT3T4;
	int i = 0;
	int j = 0;

	while (1) {

		/* Get data from T1a */
		if (xQueueReceive(xQueueT1a_T2a, (void*) &message,
		portMAX_DELAY) == pdPASS) {
			if (j == 1) { // Get one in two messages from T1b (T1a 100 Hz, T1b 50 Hz, T2a 100 Hz)
				if (xQueueReceive(xQueueT1b_T2a, (void*) mag,
				portMAX_DELAY) == pdPASS)
					j = 0;
			} else {
				j++;
			}

			/* Angles calculation */
			Pitch(&hi2c3, message.acc, &messageT3T4.pitch);
			Roll(&hi2c3, message.acc, &messageT3T4.roll);
			Yaw2(&hi2c3, message.acc, message.gyro, mag, &messageT3T4.yaw);

			if (i == 19) // Send one in twenty value to T3 and T4
					{
				if ( xQueueSend(xQueueT2a_T3, (void* ) &messageT3T4,
						portMAX_DELAY) != pdPASS) {
					printf("Failed to send data\n\r");
				}
				if ( xQueueSend(xQueueT2a_T4, (void* ) &messageT3T4,
						portMAX_DELAY) != pdPASS) {
					printf("Failed to send data\n\r");
				}
				i = 0;
			} else {
				i++;
			}

		}
	}
}

void vTask2b(void *pvParameters) {
	message_capteur2 messageRecu;
	message_calcul2 messageEnv;
	int i = 0;

	while (1) {
		// Get data from T2b
		if (xQueueReceive(xQueueT1b_T2b, (void*) &messageRecu,
		portMAX_DELAY) == pdPASS) {
			messageEnv.A = 288.15 / 0.0065
					* (1 - pow(messageRecu.P / 101325, 1.0 / 5.255)); // Altitude calculation with International Standard Atmosphere
			messageEnv.T = messageRecu.T;
			if (i == 9) // Send one in ten values to T3 and T4 ( T2b 50Hz and T3/T4 5Hz )
					{
				if ( xQueueSend(xQueueT2b_T3, (void* ) &messageEnv,
						portMAX_DELAY) != pdPASS) {
					printf("Failed to send data\n\r");
				}

				if ( xQueueSend(xQueueT2b_T4, (void* ) &messageEnv,
						portMAX_DELAY) != pdPASS) {
					printf("Failed to send data\n\r");
				}
				i = 0;
			} else {
				i++;
			}
		} else {
			printf("Failed to receive data\n\r");
		}
	}
}

void vTask3(void *pvParameters) {
	message_calcul messageT2a;
	message_calcul2 messageT2b;

	while (1) {
		xSemaphoreTake(SemB_3, portMAX_DELAY); // Wait for Timer T3

		/* Get data from T2a */
		if (xQueueReceive(xQueueT2a_T3, (void*) &messageT2a,
		portMAX_DELAY) != pdPASS) {
			printf("Failed to receive data RS232\n\r");
		}
		/* Get data from T2b */
		if (xQueueReceive(xQueueT2b_T3, (void*) &messageT2b,
		portMAX_DELAY) != pdPASS) {
			printf("Failed to receive data RS232\n\r");
		}
		printf(
				"pitch=%2.2f deg, roll=%2.2f deg/s, yaw=%2.2f deg/s,  altitude=%2.2f m, temperature=%2.2f deg \n\r",
				messageT2a.roll, messageT2a.pitch, messageT2a.yaw, messageT2b.A,
				messageT2b.T); // Send data on RS232

	}
}

void vTask4(void *pvParameters) {
	message_calcul messageT2a;
	message_calcul2 messageT2b;

	while (1) {
		xSemaphoreTake(SemB_4, portMAX_DELAY); // Wait for Timer T3

		/* Get data from T2a */
		if (xQueueReceive(xQueueT2a_T4, (void*) &messageT2a,
		portMAX_DELAY) != pdPASS) {
			printf("Failed to receive data LCD\n\r");
		}
		/* Get data from T2b */
		if (xQueueReceive(xQueueT2b_T4, (void*) &messageT2b,
		portMAX_DELAY) != pdPASS) {
			printf("Failed to receive data LCD\n\r");
		}
		GUI(messageT2a.roll, messageT2a.pitch, messageT2a.yaw, messageT2b.A,
				messageT2b.T); // Display data on LCD

	}
}

