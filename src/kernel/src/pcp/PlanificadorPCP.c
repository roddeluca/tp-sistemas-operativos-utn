/*
 * PlanificadorPCP.c
 *
 *  Created on: 01/05/2014
 *      Author: Dani
 */


/*
 ●Planificará según el algoritmo Round Robin a los procesos encolados en la cola de
 READY.

 ●Moverá a la cola de READY todos los procesos que hayan terminado su ráfaga de CPU y
 concluído su entrada/salida en función de la prioridad de cada uno.

 ●Enviará a Ejecutar los procesos que esten en READY a los procesadores disponibles,
 informando al cpu el quantum asignado.

 ●Moverá a la cola de EXIT todos los procesos que hayan concluído su ejecución.

 ●Recibirá de cada CPU la información actual de cada PCB en ejecución y enviará a cada
 CPU libre al primer elemento de la cola de READY.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <commons/log.h>

#include "PCP.h"
#include "../soporte/ConexionConCPU.h"
#include "../kernelMain.h"

void workPCP()
{
	int iResultado = R_OK;
	char szBuffer[80];
	pid_t procID;
	pthread_t tid_CPU;
	pthread_attr_t cpuAttr;

	t_log * logger = NULL;

	procID= getpid();
	//printf("ingrese nivel de loggeo:.\n");
	//gets(szBuffer);
	sprintf(szBuffer,"LogPlanifCortoPlazo_main_%d.log",procID);

	logger = log_create(szBuffer, PROGRAMAPCP, false, logLevel);

	sprintf(szBuffer, "INICIO programa %s.", PROGRAMAPCP);
	log_info(logger, szBuffer);
	listaExec = list_create();


	/* -------------------------------------------------------------------------- *
	 * HILO DE CONEXION DE CPUS. CUANDO UN CPU SE CONECTA LE GENERA UN NUEVO HILO *
	 * -------------------------------------------------------------------------- */

	pthread_attr_init(&cpuAttr);

	if (pthread_create(&tid_CPU, &cpuAttr, escucharConexionesCPU, NULL ) == R_OK)
	{
		strcpy(szBuffer,
				"Generacion Instancia \"socket_escucharConexiones\" OK.");
		log_debug(logger, szBuffer);
		sprintf(szBuffer, "\tSe asigno instancia con id [%lu].", tid_CPU);
		log_debug(logger, szBuffer);
	}
	else
	{
		strcpy(szBuffer, "Fallo en generacion de Instancia \"fnRecibeCpu\"");
		log_error(logger, szBuffer);
		iResultado = R_INSTANCIA;
	}

	/*	--------------------------	*
	 *	INICIO DEL CODIGO DEL MAIN	*
	 *	--------------------------	*/

	pthread_join(tid_CPU, NULL );

	sprintf(szBuffer, "termino funcion con valor: [%d].\n", iResultado);
	log_info(logger, szBuffer);

}

