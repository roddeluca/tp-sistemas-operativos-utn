/*
 * kernelMain.h
 *
 *  Created on: 24/05/2014
 *      Author: utnso
 */

#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <parser/metadata_program.h>
#include <stdint.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>

#include <protocolo.h>
#include "soporte/ConexionConUMV.h"

#ifndef KERNELMAIN_H_
#define KERNELMAIN_H_

int logLevel;
t_log * loggerMain;
char bufferMainLog[80];

sem_t productorConsumidor,semaforoExit, nuevoProgramaSem;

t_config* pConfiguracion;
t_dictionary * variables_compartidas;
t_dictionary * semaforos;



t_list *newProcesos,*exitProcesos,*listaReady,*lista_de_programas;
pthread_mutex_t mutex;
pthread_mutex_t mutexReady;
pthread_mutex_t mutexExec;
pthread_mutex_t mutexExit;
pthread_mutex_t mutexMultiprogramacion;


int pidGeneral;
int estado_multiprogramacion;


typedef struct {
	int socketProg;
	t_pcb* pcb;

} t_nuevoPrograma;

typedef struct stParametros{
	int iRetardoQ;
	int iQuantum;
	int iPuertoCPU;
	int puertoUMV;
	char ipUMV[16];
	int stack;
}stParametros;

stParametros configuracionParametros;

typedef struct {
	t_medatada_program *metadada;
	char* archivoAnsisop;
	int peso;
	t_nuevoPrograma * programa;
} t_prog;


typedef struct stSemaforo{
	char szSemaforo[20];
	int iSemaforo;
	pthread_mutex_t mutex;
	t_list * progEncolados;
} tSemaforo;

typedef struct stDisp{
	char szDispositivo[20];
	int iRetardo;
	sem_t semaforo;
	t_list * progEncolados;
	pthread_mutex_t mutex;
} tIO;

typedef struct stVarCom{
	t_valor_variable valor;
	pthread_mutex_t mutex;
} tVarComp;

int generar_pid();

char * recibir(int socketProg, int buffer_size);

t_prog * nuevo_programa(char* codigoAnsisop, int socketPrograma);

int* iniciarServidor();

void inicializar_estructura_poll(struct pollfd* programas, int serverSocket);

int* socket_server_accept(int *socket_server);

char * cargar_archivo_en_memoria (FILE *file);

void * iniciar_plp();

void * iniciar_pcp();

void * iniciar_hiloconsola(void*);

void * 	fnSemaforos(void *);

void * fnDispositivos (void *);

void inicializador();

#endif /* KERNELMAIN_H_ */
