/*
 * PlanificadorPCP_th.c
 *
 *  Created on: 01/05/2014
 *      Author: Dani
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "PCP.h"
#include "../kernelMain.h"

void moverAExit(t_nuevoPrograma * elPrograma)
{
	/* sacar el programa de la lista de exec, actualizar el PCB y mandarlo a lista exit */

	t_nuevoPrograma * programaBuscado;

	bool bfnBuscaProg(void* param)
	{
		t_nuevoPrograma* programa = (t_nuevoPrograma*) param;
		return (programa->pcb->pid == elPrograma->pcb->pid);
	}

	pthread_mutex_lock(&mutexExec);
	programaBuscado = (t_nuevoPrograma *) list_remove_by_condition(listaExec,bfnBuscaProg);
	pthread_mutex_unlock(&mutexExec);


	if (elPrograma->pcb->pid == programaBuscado->pcb->pid)
	{
		//printf("mando a exit el programa: [%d]\n", programaBuscado->pcb->pid);
		pthread_mutex_lock(&mutexExit);
		list_add(exitProcesos, programaBuscado);
		pthread_mutex_unlock(&mutexExit);

		pthread_mutex_lock(&mutexMultiprogramacion);
		estado_multiprogramacion++;
		pthread_mutex_unlock(&mutexMultiprogramacion);

		sem_post(&semaforoExit);
		sem_post(&nuevoProgramaSem);
	}

}

void recargarReady(t_pcb * elPrograma)
{
	/*sacar del programa de exec, actualizar el pcb y cargarlo en la cola de ready*/

	t_nuevoPrograma * programaBuscado;

	bool bfnBuscaProg(void* param)
	{
		t_nuevoPrograma* programa = (t_nuevoPrograma*) param;
		return (programa->pcb->pid == elPrograma->pid);
	}

	pthread_mutex_lock(&mutexExec);
	programaBuscado = list_remove_by_condition(listaExec, bfnBuscaProg);
	pthread_mutex_unlock(&mutexExec);

	*programaBuscado->pcb = *elPrograma;

	pthread_mutex_lock(&mutexReady);
	list_add_in_index(listaReady, listaReady->elements_count, programaBuscado);
	pthread_mutex_unlock(&mutexReady);

	sem_post(&productorConsumidor);

}

char * imrpimirTexto(char * parametro, int sokete)
{
	/*mando por soscket mensaje al programa para imprimir por su consola
	 * y avisarle al CPU cuando termine*/
	int iEnvio;
	//char * pp = malloc(150);

	//printf("texto: [%s]", parametro);

	//sprintf(pp, "[%s]", (char *) parametro);

	iEnvio = send(sokete, parametro, strlen(parametro)+1, 0);
	//free(pp);
	if (iEnvio > 0)
	{
		return string_itoa(iEnvio);
	}
	else
	{
		return "exit";
	}
}

char * imrpimirVariable(void * parametro, int sokete)
{
	/*
	 * buscar variable global, mandarla a imprimir en la consola del programa y avisarle al CPU.
	 * si no existe avisarle al CPU esto y esperar PCB para terminar el programa.*/

	char * pp="";
	int iEnvio;

	//printf("%d", *(int*) parametro);

	pp=string_itoa(*(int*) parametro);

	iEnvio = send(sokete, pp, strlen(pp)+1, 0);
	free(pp);

	if (iEnvio > 0)
	{
		return string_itoa(iEnvio);
	}
	else
	{
		return "exit";
	}

}

char * obtenerVariable(t_nombre_compartida parametro)
{
	/*buscar variable global y devolverla al CPU que la solicito si existe.
	 * si no existe avisarle al CPU y esperar PCB para terminar el programa. */

	char * retorno = malloc(sizeof(int));
	bool encontrada = false;

	//printf("Paramentro: %s\n", parametro);
	tVarComp* variableCompar = (tVarComp*) dictionary_get(variables_compartidas, parametro);


	if (variableCompar != NULL )
	{
		encontrada = true;
		sprintf(retorno, "%d", variableCompar->valor);
	}

	if (encontrada == false)
	{
		retorno = realloc(retorno, 5);
		sprintf(retorno, "exit");
	}

	return retorno;

}

char * asignarVariable(t_variable_compartida * parametro)
{
	/*buscar variable global, asignarle el valor y avisarle al CPU.
	 * si no existe avisarle al CPU y esperar PCB para terminar el programa.*/

	char * retorno = malloc(sizeof(int));

	bool encontrada = false;

	tVarComp* variableCom = (tVarComp*) dictionary_get(
			variables_compartidas, parametro->nombre);

	if (variableCom != NULL )
	{
		pthread_mutex_lock(&(variableCom->mutex));
		variableCom->valor = parametro->valor;
		pthread_mutex_unlock(&(variableCom->mutex));
		encontrada = true;
		sprintf(retorno, "%d", variableCom->valor);

	}

	if (encontrada == false)
	{
		retorno = realloc(retorno, 5);
		sprintf(retorno, "exit");
	}

	return retorno;
}

char * fnLock(t_MsjWait * parametro, t_nuevoPrograma * programa)
{
	/* controla el valor del semaforo..
	 * si es > 0 devuelvo una señal de continuar proceso
	 * si es <= 0 aviso al CPU y lo pongo en BLOCK de la lista del semaforo hasta que haya un signal */

	/*
	 * S->value--;
	 * if(S->value < 0)
	 *  {
	 *   añadir este proceso a S->list;
	 *   block();
	 *  }
	 * */

	tSemaforo * semaf;

	char * retorno = malloc(sizeof(int));

	bool bfnEncuentraSem(void* param)
	{
		tSemaforo *unSem = (tSemaforo*) param;
		return strcmp(unSem->szSemaforo, parametro->semaforo) == 0 ? 1 : 0;
	}

	//printf("semaforo que me llego: [%s]\n", parametro->semaforo);

	semaf = (tSemaforo *) list_find(listSmfrs, bfnEncuentraSem);

	if (semaf != NULL )
	{
		semaf->iSemaforo--;

		if (semaf->iSemaforo < 0 && list_size(listaExec))
		{

			bool bfnBuscaProg(void* param)
			{
				t_nuevoPrograma* prog = (t_nuevoPrograma*) param;
				return (prog->pcb->pid == programa->pcb->pid) ? 1 : 0;
			}

			pthread_mutex_lock(&mutexExec);
			list_remove_by_condition(
					listaExec, bfnBuscaProg);
			pthread_mutex_unlock(&mutexExec);

			*programa->pcb = parametro->pcb;

			pthread_mutex_lock(&(semaf->mutex));
			list_add(semaf->progEncolados, programa);
			pthread_mutex_unlock(&(semaf->mutex));


			sprintf(retorno, "%d", bloquear);
		}
		else
		{

			sprintf(retorno, "%d", continuar);
		}

	}
	else
	{
		retorno = realloc(retorno, 5);
		sprintf(retorno, "exit");
	}

	return retorno;

}

char * fnUnlock(char * parametro)
{
	/* habilito proceso de la cola de blockeados de ese semaforo y/o incremento el valor. */
	/*
	 * S->value++;
	 * if(S->value <= 0)
	 * 	{
	 * 		eliminar un proceso P de S->list;
	 * 		wakeup(P);
	 * 	}
	 * */
	tSemaforo * semaf;
	t_nuevoPrograma * programa;
	char * retorno = malloc(sizeof(int));

	bool bfnEncuentraSem(void* param)
	{
		tSemaforo *unSem = (tSemaforo*) param;
		return strcmp(unSem->szSemaforo, parametro) == 0 ? 1 : 0;
	}

	semaf = (tSemaforo *) list_find(listSmfrs, bfnEncuentraSem);

	if (semaf != NULL )
	{
		semaf->iSemaforo++;

		if (semaf->iSemaforo <= 0 && list_size(semaf->progEncolados))
		{
			pthread_mutex_lock(&(semaf->mutex));
			programa = list_remove(semaf->progEncolados, 0);
			pthread_mutex_unlock(&(semaf->mutex));

			pthread_mutex_lock(&mutexReady);
			list_add_in_index(listaReady, listaReady->elements_count, programa);
			pthread_mutex_unlock(&mutexReady);
			sem_post(&productorConsumidor);

		}
		sprintf(retorno, "%d", continuar);

	}
	else
	{
		retorno = realloc(retorno, 5);
		sprintf(retorno, "exit");
	}

	return retorno;
}

int mandarADispositivo(t_entradaSalida * parametro, t_nuevoPrograma * programa)
{
	/* mando a la cola de BLOCK el proceso */
	int retorno;

	tIO * unDispo;

	bool bfnEncuentraDis(void* param)
	{
		tIO *unDis = (tIO*) param;
		return strcmp(unDis->szDispositivo, parametro->dispositivo) == 0 ? 1 : 0;
	}

	unDispo = (tIO *) list_find(listDispos, bfnEncuentraDis);

	if (unDispo != NULL )
	{
		pProgramaRetardo * pPrograma = malloc(sizeof(pProgramaRetardo));

		pPrograma->retardo = parametro->tiempo;
		pPrograma->programa = programa;
		pthread_mutex_lock(&unDispo->mutex);
		list_add(unDispo->progEncolados, pPrograma);
		pthread_mutex_unlock(&unDispo->mutex);


		bool bfnBuscaProg(void* param)
		{
			t_nuevoPrograma* prog = (t_nuevoPrograma*) param;
			return (prog->pcb->pid == programa->pcb->pid) ? 1 : 0;
		}

		pthread_mutex_lock(&mutexExec);
		list_remove_by_condition(
				listaExec, bfnBuscaProg);
		pthread_mutex_unlock(&mutexExec);

		sem_post(&(unDispo->semaforo));

		retorno = bloquear;

	}
	else
	{

		retorno = terminar;
	}
	return retorno;
}

void liberarTMensaje(t_mensaje * mensaje)
{

	t_variable_compartida * var;
	t_entradaSalida * entraSa;
	t_MsjWait * waiT;
	switch (mensaje->idOperacion) {
		case asignarValorVarCompartida:
			var = (t_variable_compartida*) mensaje->estructura;
			free(var->nombre);
			free(mensaje->estructura);
			free(mensaje);
			break;
		case entradaSalidaKernel:
			entraSa = (t_entradaSalida*) mensaje->estructura;
			free(entraSa->dispositivo);
			free(mensaje->estructura);
			free(mensaje);
			break;
		case waitKernel:
			waiT = (t_MsjWait*) mensaje->estructura;
			free(waiT->semaforo);
			free(mensaje->estructura);
			free(mensaje);
			break;
		default:
			free(mensaje->estructura);
			free(mensaje);
			break;
		}

}

t_buffer *socket_recv(t_socket_cliente* cliente)
{
	t_buffer* nueBuffer = malloc(sizeof(t_buffer));
	char* bufferData = NULL;
	int tamanioBuffer = 0;
	int tamanioRecv = 0;

	while (tamanioRecv == tamanioBuffer)
	{
		tamanioBuffer += 10;
		if (bufferData == NULL )
			bufferData = malloc(tamanioBuffer);
		else
			bufferData = realloc(bufferData, tamanioBuffer);
		tamanioRecv = recv(cliente->socket->desc, bufferData, tamanioBuffer,
				MSG_PEEK);
	}

	recv(cliente->socket->desc, bufferData, tamanioRecv, 0);

	if (tamanioRecv == 0)
	{
		puts("Cliente desconectado");
	}
	else if (tamanioRecv == -1)
	{
		puts("Fallo el recv: desconexion inesperada");
	}

	nueBuffer->data = bufferData;
	nueBuffer->size = tamanioRecv;
	return nueBuffer;
}

void socket_serverDestroy(t_socket_server* server)
{
	free(server->my_addr);
	close(server->desc);
	free(server);
}

void socket_clienteDestroy(t_socket_cliente* cliente)
{
	close(cliente->socket->desc);
	free(cliente->socket);
	free(cliente);
}

void socket_send(t_socket_cliente* cliente, t_buffer* buffer)
{
	send(cliente->socket->desc, buffer->data, buffer->size, 0);
}

void * fnDispositivos(void * parametro)
{
	tIO *unIO = (tIO *) parametro;
	pProgramaRetardo * programa;

	while (1)
	{
		sem_wait(&(unIO->semaforo));

		pthread_mutex_lock(&(unIO->mutex));
		programa = (pProgramaRetardo *) list_remove(unIO->progEncolados, 0);
		pthread_mutex_unlock(&(unIO->mutex));

		usleep(unIO->iRetardo * programa->retardo * 1000);

		printf("el programa [%d] estuvo bloqueado [%d] milisegundos.\n",
				programa->programa->pcb->pid,
				unIO->iRetardo * programa->retardo);

		pthread_mutex_lock(&mutexReady);
		list_add_in_index(listaReady, listaReady->elements_count,
				programa->programa);
		pthread_mutex_unlock(&mutexReady);

		sem_post(&productorConsumidor);

		free(programa);
	}

	return NULL ;

}

