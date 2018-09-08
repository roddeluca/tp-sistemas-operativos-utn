/*
 * PCP.h
 *
 *  Created on: 01/05/2014
 *      Author: Dani
 */

#ifndef PCP_H_
#define PCP_H_

#include <pthread.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <semaphore.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

#include <serializar.h>

#include <protocolo.h>

#include "../kernelMain.h"

#define PROGRAMAPCP "PCP"
#define VERSION "0.0.1"
/* Valores de retorno ------------------------------------------ */

#undef R_OK
#define R_OK 		 								0
#define R_DB 										-1
#define R_LOG 										-2
#define R_PARAMETRO 								-3
#define R_ID_INSTANCIA								-4
#define R_INSTANCIA									-5
#define R_SIN_INSTANCIAS							-6
#define R_MEMORIA									-7
#define R_CANDADO									-8

/* TIPO DE DATOS -------------------------------------------------*/

t_list *listaExec, *listCPU, *listSmfrs, *listDispos;

/*-----------------------------------------------------------------------------------
 * estructura de cada CPU que me sirve para la lista de CPUs Disponible y Trabajando */
typedef struct stNodoCpu
{
	struct sockaddr_in addr;

} tNodoCpu;

/* estructuras de sockets */
typedef struct
{
	int desc;
	struct sockaddr_in* my_addr;
} t_socket;
typedef t_socket t_socket_server;

typedef struct
{
	t_socket *socket;
	t_socket *socket_serv;
} t_socket_cliente;

typedef struct
{
	int tipoConexion;
	int idPrograma;
} t_conexion;

/*-----------------------------------------------------------------*/

typedef enum
{
	ocuado = 0, libre = 1
} t_estado;

typedef struct stListaCpu
{
	t_socket cpu;
	t_estado estado;
} pListaCpu;

typedef struct stProgramaRetardo
{
	t_nuevoPrograma * programa;
	int retardo;
} pProgramaRetardo;

/* FUNCIONES -----------------------------------------------------*/

void workPCP();

void * pfnControladorConexion(void*);

void moverAExit(t_nuevoPrograma *);

void recargarReady(t_pcb * elPrograma);

char * imrpimirTexto(char *, int);

char * imrpimirVariable(void *, int);

char * obtenerVariable(t_nombre_compartida);

char * asignarVariable(t_variable_compartida *);

char * fnLock(t_MsjWait *, t_nuevoPrograma *);

char * fnUnlock(char *);

int mandarADispositivo(t_entradaSalida *,t_nuevoPrograma *);

int socket_escucharConexiones(void);

//t_mensaje* deserealizar_operacionKernel(t_buffer*);

t_buffer * serializarPCB(t_nuevoPrograma *);

t_socket *socket_crearServidor(int);

void socket_listen(t_socket_server*);

t_socket_cliente* socket_accept(t_socket_server*);

t_buffer *socket_recv(t_socket_cliente*); //recibe un buffer de tamaño variable.

void socket_send(t_socket_cliente* cliente, t_buffer* buffer); //envias un socket del tamaño que sea.

//t_mensaje* deserealizar_operacionKernel(t_buffer*, t_conexion*);

void socket_clienteDestroy(t_socket_cliente* cliente);

void socket_serverDestroy(t_socket_server* server);

void liberarTMensaje(t_mensaje *);

/* 
 * VARIABLES COMPARTIDAS -----------------------------------------*/

#endif /* PCP_H_ */

