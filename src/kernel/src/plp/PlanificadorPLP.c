/*
 * 
 * PlanificadorPLP.c
 *
 *  Created on: 29/04/2014
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <semaphore.h>

#include "PlanificadorPLP.h"
#include <serializar.h>

bool (*ordenamiento)(void*elemento1, void*elemento2);

void planificar_programas()
{

	t_log_level loglvl;
	loglvl = logLevel;

	loggerPLP = log_create("Kernel_PLP.log", "Kernel-PLP", false, loglvl);

	sprintf(bufferLogPLP, "Inicio hilo Kernel-PLP.");
	log_info(loggerPLP, bufferLogPLP);

	pthread_t hiloExit;
	pthread_attr_t hiloExitAttr;

	newProcesos = list_create();
	exitProcesos = list_create();
	ordenamiento = ordenador;

	pthread_attr_init(&hiloExitAttr);
	pthread_create(&hiloExit, &hiloExitAttr, terminarPrograma, NULL );
	log_debug(loggerMain, "Inicio hilo manejador de procesos en exit");

	while (1)
	{
		sem_wait(&nuevoProgramaSem);
		pthread_mutex_lock(&mutex);
		t_prog* prog = (t_prog*) list_remove(lista_de_programas, 0);
		pthread_mutex_unlock(&mutex);

		if (prog != NULL) {
			guardar_programa(prog);
		}
		if (list_size(newProcesos) > 0)
				ponerEnReadyProgramas();
	}
}

void liberarNuevoPrograma(t_nuevoPrograma*prog)
{
	free(prog->pcb);
	free(prog);
}

void liberoParcial(t_prog* programa)
{
	metadata_destruir(programa->metadada);
	free(programa->archivoAnsisop);
}

void liberarPrograma(t_prog*prog)
{
	metadata_destruir(prog->metadada);
	free(prog->archivoAnsisop);
	liberarNuevoPrograma(prog->programa);
	free(prog);
}

void ponerEnReadyProgramas()
{

	if (estado_multiprogramacion > 0)
	{
		pthread_mutex_lock(&mutexMultiprogramacion);
		estado_multiprogramacion--;
		pthread_mutex_unlock(&mutexMultiprogramacion);

		pthread_mutex_lock(&mutexReady);
		t_prog* nuevoProg = (t_prog*) list_remove(newProcesos, 0);
		list_add_in_index(listaReady, list_size(listaReady),
				nuevoProg->programa);
		pthread_mutex_unlock(&mutexReady);
		sem_post(&productorConsumidor);
		liberoParcial(nuevoProg);
		sprintf(bufferLogPLP, "Agrego a Ready el programa %d peso:%d",nuevoProg->programa->pcb->pid,nuevoProg->peso);
		log_debug(loggerPLP, bufferLogPLP);
	}
}

void * terminarPrograma(void * param)
{

	while (1)
	{
		sem_wait(&semaforoExit);
		int conexion = conectar_a_umv(configuracionParametros.ipUMV,configuracionParametros.puertoUMV);

		if (conexion == -1)
				log_error(loggerPLP, "Fallo conexion con UMV para eliminar segmentos");


		while (conexion == -1)
		{
			conexion = conectar_a_umv(configuracionParametros.ipUMV,configuracionParametros.puertoUMV);

		}


		pthread_mutex_lock(&mutexExit);
		t_nuevoPrograma *tempProg = (t_nuevoPrograma*) list_remove(exitProcesos,
				0);
		pthread_mutex_unlock(&mutexExit);

		int pid = tempProg->pcb->pid;
		destruir_segmento(conexion,pid);

		send(tempProg->socketProg, "Termino la ejecucion",
				strlen("Termino la ejecucion") + 1, 0);
		close(tempProg->socketProg);
		close(conexion);
		sprintf(bufferLogPLP, "Termino correctamente proceso :%d",tempProg->pcb->pid);
		log_debug(loggerPLP, bufferLogPLP);
		liberarNuevoPrograma(tempProg);


	}

	return NULL ;
}

void rechazar_programa(t_prog* prog)
{
	int conexion = conectar_a_umv(configuracionParametros.ipUMV,configuracionParametros.puertoUMV);

	if (conexion == -1)
		log_error(loggerPLP, "Fallo conexion con UMV para eliminar segmentos");

	while (conexion == -1)
	{
		conexion = conectar_a_umv(configuracionParametros.ipUMV,configuracionParametros.puertoUMV);
	}

	int pid = prog->programa->pcb->pid;

	destruir_segmento(conexion,pid);

	send(prog->programa->socketProg, "Programa rechazado",
			strlen("Programa rechazado") + 1, 0);

	close(prog->programa->socketProg);
	close(conexion);
	sprintf(bufferLogPLP, "Rechazo proceso por memory overload :%d",prog->programa->pcb->pid);
	log_debug(loggerPLP, bufferLogPLP);
	liberarPrograma(prog);
}

void aceptar_programa(t_prog* prog)
{
	int peso = calcular_peso(prog);
	prog->peso = peso;
	list_add(newProcesos, prog);
	list_sort(newProcesos, ordenamiento);
	sprintf(bufferLogPLP, "Programa aceptado %d peso:%d",prog->programa->pcb->pid,prog->peso);
	log_debug(loggerPLP, bufferLogPLP);

}

bool ordenador(void *elemento1, void * elemento2)
{

	t_prog * prog1 = (t_prog*) elemento1;
	t_prog * prog2 = (t_prog*) elemento2;

	if (prog1->peso <= prog2->peso)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void guardar_programa(t_prog*prog)
{

	int resultado = conectarUMV(prog);

	if (resultado != -1)
	{
		aceptar_programa(prog);
	}
	else
	{
		rechazar_programa(prog);
	}
}

int calcular_peso(t_prog* prog)
{

	int peso = 5 * prog->programa->pcb->cantidad_de_etiquetas
			+ 3 * prog->programa->pcb->cantidad_de_funciones
			+ prog->metadada->instrucciones_size;
	return peso;
}

int conectarUMV(t_prog* prog)
{
	int arrayDeTamano[4] =
	{ strlen(prog->archivoAnsisop)+1, sizeof(t_intructions)
			* prog->metadada->instrucciones_size,
			prog->programa->pcb->tamanioEtiqueta,configuracionParametros.stack };

	int conexion = conectar_a_umv(configuracionParametros.ipUMV,configuracionParametros.puertoUMV);

	while (conexion == -1)
	{

		conexion = conectar_a_umv(configuracionParametros.ipUMV,configuracionParametros.puertoUMV);
	}

	if (conexion >= 0)
	{

		int miTipo = kernel;
		int pid = prog->programa->pcb->pid;

		umv_destroy_mensaje(enviar_mensajes_a_umv(conexion, &miTipo, handshake));
		sprintf(bufferLogPLP, "Handshake con UMV");

		umv_destroy_mensaje(enviar_mensajes_a_umv(conexion, &pid, cambioProcesoActivo));
		sprintf(bufferLogPLP, "Cambio de proceso activo para el programa %d",pid);
		log_debug(loggerPLP, bufferLogPLP);

		int i;

		for (i = 0; i < 4; ++i)
		{
			if (arrayDeTamano[i] == 0)
			{
				prog->programa->pcb->indiceEtiqueta = 1;
			}
			else
			{

				t_mensaje* respuesta = enviar_mensajes_a_umv(conexion,
						&arrayDeTamano[i], crearSegmento);

				if (respuesta->idOperacion == memory_overload
						|| respuesta->idOperacion == segmentation_fault)
				{
					sprintf(bufferLogPLP, "Memory Overload para el proceso %d",prog->programa->pcb->pid);
					log_error(loggerPLP, bufferLogPLP);
					return -1;

				}
				else
				{

					t_almacenarBytes almacenarEnUMV;
					t_mensaje *res;
					switch (i)
					{
					case 0:
						//segmentoCodigo
						almacenarEnUMV.base =
								*(t_puntero*) respuesta->estructura;
						almacenarEnUMV.offset = 0;
						almacenarEnUMV.tamanio = arrayDeTamano[i];
						almacenarEnUMV.buffer = prog->archivoAnsisop;

						res = enviar_mensajes_a_umv(conexion, &almacenarEnUMV,almacenar);
						prog->programa->pcb->segmentoCodigo =*(t_puntero*) respuesta->estructura;

						sprintf(bufferLogPLP, "Se reservo y almacenó el segmento de codigo con tamaño:%d",arrayDeTamano[i]);
						log_debug(loggerPLP, bufferLogPLP);
						umv_destroy_mensaje(res);
						break;
					case 1:
						//Indice de codigo

						almacenarEnUMV.base =
								*(t_puntero*) respuesta->estructura;
						almacenarEnUMV.offset = 0;
						almacenarEnUMV.tamanio = arrayDeTamano[i];
						almacenarEnUMV.buffer =
								(void*) prog->metadada->instrucciones_serializado;

						res = enviar_mensajes_a_umv(conexion, &almacenarEnUMV,
								almacenar);

						prog->programa->pcb->indiceCodigo =
								*(t_puntero*) respuesta->estructura;
						sprintf(bufferLogPLP, "Se reservo y almacenó el indice de codigo con tamaño:%d",arrayDeTamano[i]);
						log_debug(loggerPLP, bufferLogPLP);
						umv_destroy_mensaje(res);
						break;
					case 2:
						//Indice de etiquetas

						almacenarEnUMV.base =
								*(t_puntero*) respuesta->estructura;
						almacenarEnUMV.offset = 0;
						almacenarEnUMV.tamanio = arrayDeTamano[i];
						almacenarEnUMV.buffer = prog->metadada->etiquetas;

						res = enviar_mensajes_a_umv(conexion, &almacenarEnUMV,
								almacenar);
						prog->programa->pcb->indiceEtiqueta =
								*(t_puntero*) respuesta->estructura;

						sprintf(bufferLogPLP, "Se reservo y almacenó el indice de etiquetas con tamaño:%d",arrayDeTamano[i]);
						log_debug(loggerPLP, bufferLogPLP);
						umv_destroy_mensaje(res);
						break;
					case 3:
						//Stack
						prog->programa->pcb->segmentoStack =
								*(t_puntero*) respuesta->estructura;
						prog->programa->pcb->cursorStack =
								prog->programa->pcb->segmentoStack;
						sprintf(bufferLogPLP, "Se reservo el segmento de stack con tamaño:%d",arrayDeTamano[i]);
						log_debug(loggerPLP, bufferLogPLP);
						break;
					default:
						break;
					}

				}
				umv_destroy_mensaje(respuesta);
			}
		}
	}else {
		sprintf(bufferLogPLP, "Error de conexion con UMV");
		log_error(loggerPLP, bufferLogPLP);
	}
	printf("Programa %d aceptado\n",prog->programa->pcb->pid);
	close(conexion);
	return 0;
}

