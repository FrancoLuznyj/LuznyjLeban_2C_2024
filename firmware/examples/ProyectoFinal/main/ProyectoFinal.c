/*! @mainpage Proyecto Final
 *
 * @section genDesc General Description
 * 
 * Se diseña un dispositivo montable en un cinturon que mide la distancia a la que se encuentran distintos obstaculos
 * a la derecha y a la izquierda del usuario que lo porte y en funcion de los valores de dichas distancias se activan
 * dos motores. El control de los motores esta hecho de manera tal que el motor de la izquierda se activa o desactiva 
 * segun la distancia que se mida en el sensor de izquierda, y analogamente funcionan el motor y sensor de la derecha.
 * Para las distintas distancias se configuran distintos patrones de vibracion de los motores:
 * Los motores se encienden según los siguientes rangos de distancia:
 * > 30: vibración muy leve con patron de vibracion 0
 * 30-25: vibración leve con patron de vibracion 1
 * 25-20: vibración un poco menos leve con patron de vibracion 2
 * 20-15: vibración moderada con patron de vibracion 3
 * 15-10: vibración un poco más intensa con patron de vibracion 4
 * 10-5: vibración intensa con patron de vibracion 5
 * 5-0: vibración continua (muy intensa) con patron de vibracion 6
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral	    |   ESP32   	|
 * |:------------------:|:--------------|
 * |  ECHO IZQUIERDA    | 	GPIO_3	    |
 * | TRIGGER IZQUIERDA  | 	GPIO_2	    |
 * | 	   +5V 	 		| 	+5V 		|
 * | 	   GND 	 		| 	GND 		|
 * |  ECHO DERECHA 	 	| 	GPIO_1 	    |
 * | TRIGGER DERECHA    | 	GPIO_0	    |
 * | 	   +5V 	 		| 	+5V 		|
 * | 	   GND 	 		| 	GND 		|
 * | 	   +5V		 	|	+5V			|
 * | 	   GND		 	| 	GND			|
 * | 	   1,2EN	 	| 	GPIO_22		|
 * | 	   1A		 	| 	GPIO_21		|
 * | 	   2A		 	| 	GPIO_20		|
 * | 	   3,4EN	 	| 	GPIO_19		|
 * | 	   3A		 	| 	GPIO_18		|
 * | 	   4A		 	| 	GPIO_9		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 16/10/2024 | Document creation		                         |
 *
 * @author Francesca Leban (francheleban123@gmail.com)
 * @author Franco Luznyj (francoluznyj1999@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "delay_mcu.h"
#include "l293.h"
#include "hc_sr04.h"

/*==================[macros and definitions]=================================*/

/**
 * @brief DELAY_ESPERA
 * 
 * Periodo de tiempo que se bloquean las tareas de vibrar motor 1 y vibrar motor 2
 *
 */
#define DELAY_ESPERA 200

/**
 * @brief CONFIG_BLINK_PERIOD_MEDIR_DISTANCIA
 * 
 * Periodo de tiempo que transcurre entre cada medicion de distancia
 *
 */
#define CONFIG_BLINK_PERIOD_MEDIR_DISTANCIA 2000

/*==================[internal data definition]===============================*/

/**
 * @brief vibrar_motor1_task_handle
 * 
 * Variable de tipo TaskHandle_t para la tarea Vibrar_motor1_Task
 *
 */
TaskHandle_t vibrar_motor1_task_handle = NULL;

/**
 * @brief vibrar_motor2_task_handle
 * 
 * Variable de tipo TaskHandle_t para la tarea Vibrar_motor2_Task
 *
 */
TaskHandle_t vibrar_motor2_task_handle = NULL;

/**
 * @brief medir_distancia_task_handle
 * 
 * Variable de tipo TaskHandle_t para la tarea MedirDistancia
 *
 */
TaskHandle_t medir_distancia_task_handle = NULL;

/**
 * @brief distancia_derecha
 * 
 * Variable de tipo uint16_t que almacena el valor de la distancia de un obstaculo a la derecha
 *
 */
uint16_t distancia_derecha = 300;

/**
 * @brief distancia_izquierda
 * 
 * Variable de tipo uint16_t que almacena el valor de la distancia de un obstaculo a la izquierda
 *
 */
uint16_t distancia_izquierda = 300;

/*==================[internal functions declaration]=========================*/


/**
 * @brief MedirDistancia
 * 
 * Tarea que se encarga de medir distancia con el sensor HcSr04
 *
 * La tarea se encarga de inicializar los sensores HcSr04 y medir, en centimetros, la distancia de un obstaculo a la
 * izquierda y derecha
 *
 * @param pvParameter No se utiliza
 *
 */
static void MedirDistancia(void *pvParameter)
{
    while (true)
    {
        HcSr04Init(GPIO_3, GPIO_2);
        distancia_izquierda = HcSr04ReadDistanceInCentimeters();
        printf("%d\r\n",distancia_izquierda);
        HcSr04Init(GPIO_0, GPIO_1);
        distancia_derecha = HcSr04ReadDistanceInCentimeters();
        printf("%d\r\n",distancia_derecha);
        vTaskDelay(CONFIG_BLINK_PERIOD_MEDIR_DISTANCIA / portTICK_PERIOD_MS);
    }
}
        
/**
 * @brief Vibrar_motor1_Task
 * 
 * Tarea que se encarga de hacer vibrar el motor 1 segun el valor de distancia izquierda
 *
 * @param pvParameter No se utiliza
 *
 */
static void Vibrar_motor1_Task(void *pvParameter)
{
    while (true)
    {
        if (distancia_izquierda >= 30)
        {
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (1000) / portTICK_PERIOD_MS );
        }

        if (distancia_izquierda >= 25 && distancia_izquierda < 30)
        {
            printf("Entro en vibracion leve\n");
            
            L293SetSpeed(MOTOR_1, 50);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (2000 - DELAY_ESPERA) / portTICK_PERIOD_MS );

        }
        else if (distancia_izquierda >= 20 && distancia_izquierda < 25)
        {
            printf("Entro en vibracion un poco menos leve\n");

            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (3000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (3000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (3000 - DELAY_ESPERA) / portTICK_PERIOD_MS);

        }
        else if (distancia_izquierda >= 15 && distancia_izquierda < 20)
        {
            printf("Entro en vibracion moderada\n");

            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (2000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (2000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (2000 - DELAY_ESPERA) / portTICK_PERIOD_MS);

        }
        else if (distancia_izquierda >= 10 && distancia_izquierda < 15)
        {
            printf("Entro en vibracion un poco mas intensa\n");

            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (1000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (1000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (1000 - DELAY_ESPERA) / portTICK_PERIOD_MS);

        }
        else if (distancia_izquierda >= 5 && distancia_izquierda < 10)
        {
            printf("Entro en vibracion intensa\n");

            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (500 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (500 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 90);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_1, 0);
            vTaskDelay( (500 - DELAY_ESPERA) / portTICK_PERIOD_MS);

        }
        else if(distancia_izquierda > 0 && distancia_izquierda < 5)
        {
            printf("Entro en vibracion continua\n");

            L293SetSpeed(MOTOR_1, 100);
            vTaskDelay(250 / portTICK_PERIOD_MS);

        }
    }

    vTaskDelay( DELAY_ESPERA / portTICK_PERIOD_MS );
}

/**
 * @brief Vibrar_motor2_Task
 * 
 * Tarea que se encarga de hacer vibrar el motor 2 segun el valor de distancia derecha
 *
 * @param pvParameter No se utiliza
 *
 */
static void Vibrar_motor2_Task(void *pvParameter)
{
    while (true)
    {
        if (distancia_derecha >= 30)
        {
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (1000) / portTICK_PERIOD_MS );
        }

        if (distancia_derecha >= 25 && distancia_derecha < 30)
        {
            printf("Entro en vibracion leve\n");

            L293SetSpeed(MOTOR_2, 50);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (2000 - DELAY_ESPERA) / portTICK_PERIOD_MS );

        }
        else if (distancia_derecha >= 20 && distancia_derecha < 25)
        {
            printf("Entro en vibracion un poco menos leve\n");

            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (3000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (3000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (3000 - DELAY_ESPERA) / portTICK_PERIOD_MS);

        }
        else if (distancia_derecha >= 15 && distancia_derecha < 20)
        {
            printf("Entro en vibracion moderada\n");

            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (2000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (2000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (2000 - DELAY_ESPERA) / portTICK_PERIOD_MS);

        }
        else if (distancia_derecha >= 10 && distancia_derecha < 15)
        {
            printf("Entro en vibracion un poco mas intensa\n");

            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (1000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (1000 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (1000 - DELAY_ESPERA) / portTICK_PERIOD_MS);

        }
        else if (distancia_derecha >= 5 && distancia_derecha < 10)
        {
            printf("Entro en vibracion intensa\n");

            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (500 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (500 - DELAY_ESPERA) / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 90);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            L293SetSpeed(MOTOR_2, 0);
            vTaskDelay( (500 - DELAY_ESPERA) / portTICK_PERIOD_MS);

        }
        else if(distancia_derecha > 0 && distancia_derecha < 5)
        {
            printf("Entro en vibracion continua\n");
            
            L293SetSpeed(MOTOR_2, 100);
            vTaskDelay(250 - DELAY_ESPERA / portTICK_PERIOD_MS);

        }
    }

    vTaskDelay( DELAY_ESPERA / portTICK_PERIOD_MS );
}

/*==================[external functions definition]==========================*/
void app_main(void)
{   

//Inicializacion de L293
    L293Init();

//Creacion de tareas
    xTaskCreate(MedirDistancia, "MedirDistancia", 2048, NULL, 4, &medir_distancia_task_handle);
    xTaskCreate(Vibrar_motor1_Task, "VibrarTask", 2048, NULL, 5, &vibrar_motor1_task_handle);
    xTaskCreate(Vibrar_motor2_Task, "VibrarTask", 2048, NULL, 5, &vibrar_motor2_task_handle);
}

/*==================[end of file]============================================*/
