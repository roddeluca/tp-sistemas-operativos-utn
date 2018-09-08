/*
 * FuncionesConsola.c
 *
 *  Created on: 06/07/2014
 *      Author: utnso
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <commons/string.h>
#include <commons/collections/list.h>

#include "FuncionesConsola.h"
#include "../plp/PlanificadorPLP.h"
#include "../kernelMain.h"
#include "../pcp/PCP.h"

void iniciar_consola()
{

	char consolaInput[iMAX_LEN];

	printf("inicia consola\n");

	while (1)
	{
		printf("Ingrese el comando listas para ver los planificadores:\n");

		gets(consolaInput);

		if (!string_equals_ignore_case(consolaInput, "\n"))
		{
			if ((string_equals_ignore_case(consolaInput, "listas")))
			{
				mostrar_listas();
			}
			else
			{
				puts("El comando no es válido\n");
			}
		}

	}
	pthread_exit(0);

}

void mostrar_listas()
{

	void mostrarProgramasPLPNew(void * prog)
	{
		t_prog * programa = (t_prog*) prog;
		printf("Pid: [%d]  Peso:[%d]->",programa->programa->pcb->pid, programa->peso);
	}

	puts("====================================\n\n");
	puts("Lista NEW:\n");

	list_iterate(newProcesos, mostrarProgramasPLPNew);
	printf("/\n");

	int i;
	char** arrayIO = config_get_array_value(pConfiguracion, "ID_HIO");
	char** arraySems = config_get_array_value(pConfiguracion, "SEMAFOROS");
	tSemaforo * semaf;
	tIO *dispo;

	void imprimirLista(void* prog)
	{
		t_nuevoPrograma* progActual = (t_nuevoPrograma*) prog;
		printf("Pid: [%d] ->", progActual->pcb->pid);
	}


	puts("====================================\n\n");
	puts("Lista Ready:\n");

	list_iterate(listaReady, imprimirLista);
	printf("/\n");

	puts("====================================\n\n");
	puts("Lista Ejecucion:\n");

	list_iterate(listaExec, imprimirLista);
	printf("/\n");

	puts("====================================\n\n");
	puts("Lista Exit:\n");

	list_iterate(exitProcesos, imprimirLista);
	printf("/\n");
	for (i = 0; arraySems[i] != '\0'; i++)
	{
		bool bfnEncuentraSem(void* param)
		{
			tSemaforo *unSem = (tSemaforo*) param;
			return strcmp(unSem->szSemaforo, arraySems[i]) == 0 ? 1 : 0;
		}

		semaf = (tSemaforo *) list_find(listSmfrs, bfnEncuentraSem);

		printf("====================================\n");
		printf("Lista programas bloqueado por el semaforo [%s]:\n",
				arraySems[i]);
		list_iterate(semaf->progEncolados, imprimirLista);
		printf("/\n");
	}
	for (i = 0; arrayIO[i] != '\0'; i++)
	{
		bool bfnEncuentraDis(void* param)
		{
			tIO *unDis = (tIO*) param;
			return strcmp(unDis->szDispositivo, arrayIO[i]) == 0 ? 1 : 0;
		}
		dispo = (tIO *) list_find(listDispos, bfnEncuentraDis);

		printf("====================================\n");
		printf("Lista programas bloqueado en [%s]:\n", arrayIO[i]);
		list_iterate(dispo->progEncolados, imprimirLista);
		printf("/\n");
	}

}




