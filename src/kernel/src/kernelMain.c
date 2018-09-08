#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <strings.h>
#include <pthread.h>


#include <serializar.h>
#include <protocolo.h>
#include "kernelMain.h"
#include "plp/PlanificadorPLP.h"
#include "pcp/PCP.h"
#include "consola/FuncionesConsola.h"

#define BACKLOG 2
#define TIMEOUT 10



int main(int argc, char *argv[]) {

	int * serverSocket;
	pidGeneral = 1000;
	int bufsize = 1024;
	struct pollfd * stMiPoll = malloc(sizeof(struct pollfd));
	pthread_t plp;
	pthread_t pcp;
	pthread_t consolaId;

	pthread_attr_t plpAttr;
	pthread_attr_t pcpAttr;
	pthread_attr_t consolaAttr;

	if (argc < 3) {
		printf("Faltan parametros de entrada\n");
		return -1;
	}
	else {
		pConfiguracion = config_create(argv[1]);
		logLevel = atoi(argv[2]);
	}

	t_log_level loglvl = logLevel;
	sprintf(bufferMainLog, "Kernel_Main.log");
	loggerMain = log_create(bufferMainLog, "Kernel", false, loglvl);


	log_info(loggerMain, "Inicio Kernel");
	inicializador();

	pthread_attr_init(&plpAttr);
	pthread_create(&plp, &plpAttr, iniciar_plp, NULL);
	log_debug(loggerMain, "Hilo PLP creado");

	pthread_attr_init(&pcpAttr);
	pthread_create(&pcp, &pcpAttr, iniciar_pcp, NULL);
	log_debug(loggerMain, "Hilo PCP creado");

	pthread_attr_init(&consolaAttr);
	pthread_create(&consolaId, &consolaAttr, iniciar_hiloconsola, NULL);


	//pongo a escuchar al kernel
	serverSocket = iniciarServidor();
	inicializar_estructura_poll(stMiPoll, *serverSocket);
	log_debug(loggerMain, "Inicio poll");

	puts("Escuchando...");

	while(1) {

	poll(stMiPoll, BACKLOG,TIMEOUT);

		if (stMiPoll->revents == POLLIN)
		{
			int socketPrograma = *socket_server_accept(&stMiPoll->fd);
			char* archivoAnsisop = recibir(socketPrograma,bufsize);
			if(strlen(archivoAnsisop) >0) {
				t_prog* programa = nuevo_programa(archivoAnsisop,socketPrograma);

				sprintf(bufferMainLog, "Nuevo programa con pid %d",programa->programa->pcb->pid);
				log_debug(loggerMain, bufferMainLog);

				pthread_mutex_lock(&mutex);
				list_add(lista_de_programas,programa);
				pthread_mutex_unlock(&mutex);
				sem_post(&nuevoProgramaSem);
		}
		//stMiPoll->revents = 0;
		}
	}
	puts("sale");
	free(serverSocket);
	config_destroy(pConfiguracion);
	return EXIT_SUCCESS;
}

void inicializador() {

	int i;

	estado_multiprogramacion = config_get_int_value(pConfiguracion,"MULTIPROGRAMACION");

	sprintf(bufferMainLog, "Grado de multiprogramacion:%d",estado_multiprogramacion);
	log_debug(loggerMain, bufferMainLog);

	lista_de_programas = list_create();
	listaReady = list_create();

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutexReady,NULL);
	pthread_mutex_init(&mutexExit,NULL);
	pthread_mutex_init(&mutexMultiprogramacion,NULL);


	sem_init(&productorConsumidor, 1, 0);
	sem_init(&semaforoExit,1,0);
	sem_init(&nuevoProgramaSem,1,0);



	/* ------------------------------------ *
	 *  CREACION HILOS DE DISPOSITIVOS I/O  *
	 * ------------------------------------ */

	char** io = config_get_array_value(pConfiguracion,"ID_HIO");
	int cantidadIO;

	for (cantidadIO = 0; io[cantidadIO] != '\0'; cantidadIO++);

	pthread_t hilosIO[cantidadIO];
	pthread_attr_t attrHilosIO[cantidadIO];

	listDispos = list_create();

	char** retardoIO = config_get_array_value(pConfiguracion,"HIO");
	for (i = 0; io[i] != '\0'; i++) {

		tIO * unIO = malloc(sizeof(tIO));
		strcpy(unIO->szDispositivo, io[i]);
		unIO->iRetardo = atoi(retardoIO[i]);
		sem_init(&(unIO->semaforo), 1, 0);
		pthread_mutex_init(&unIO->mutex,NULL);
		unIO->progEncolados = list_create();

		list_add(listDispos, unIO);

		pthread_attr_init(&attrHilosIO[i]);
		pthread_create(&hilosIO[i], &attrHilosIO[i], fnDispositivos, unIO);

		sprintf(bufferMainLog, "Nuevo dispositivo %s retardo:%s",unIO->szDispositivo,retardoIO[i]);
		log_debug(loggerMain, bufferMainLog);
	}



	/*	---------	*
	 *	SEMAFOROS	*
	 *	---------	*/
	semaforos = dictionary_create();
	listSmfrs = list_create();

	char** semaforosTemp = config_get_array_value(pConfiguracion,"SEMAFOROS");
	char** valorSemaforos = config_get_array_value(pConfiguracion,"VALOR_SEMAFORO");

	for (i = 0; semaforosTemp[i] != '\0'; ++i)
	{
		tSemaforo * unSemaforo = (tSemaforo *) malloc(sizeof(tSemaforo));
		strcpy(unSemaforo->szSemaforo, semaforosTemp[i]);
		unSemaforo->iSemaforo = atoi(valorSemaforos[i]);
		pthread_mutex_init(&unSemaforo->mutex, NULL );
		unSemaforo->progEncolados = list_create();

		list_add(listSmfrs, unSemaforo);

		t_valor_variable* valor = malloc(sizeof(t_valor_variable));
		*valor = atoi(valorSemaforos[i]);
		dictionary_put(semaforos, semaforosTemp[i], valor);

		sprintf(bufferMainLog, "Nuevo semaforo %s valor:%s",unSemaforo->szSemaforo,valorSemaforos[i]);
		log_debug(loggerMain, bufferMainLog);

	}

	/*	-------------------------	*
	 *	DICCIONARIO DE VARIABLES 	*
	 *	------------------------	*/

	 variables_compartidas = dictionary_create();
	 char** variables = config_get_array_value(pConfiguracion,"ID_VARIABLE");
	 char** valorVariables = config_get_array_value(pConfiguracion,"VARIABLE");
	 for (i = 0; variables[i] != '\0'; i++)
	 {
		 tVarComp* variableComp = malloc(sizeof(tVarComp));
		  variableComp->valor = atoi(valorVariables[i]);
		   pthread_mutex_init(&(variableComp->mutex), NULL);
		  dictionary_put(variables_compartidas, variables[i], variableComp);

		sprintf(bufferMainLog, "Nuevo variable compartida %s valor:%s",variables[i],valorVariables[i]);
		log_debug(loggerMain, bufferMainLog);
	 }

	configuracionParametros.stack = config_get_int_value(pConfiguracion,"STACK");

	strcpy(configuracionParametros.ipUMV,config_get_string_value(pConfiguracion,"IP_UMV"));
	configuracionParametros.puertoUMV = config_get_int_value(pConfiguracion,"PUERTO_UMV");

	configuracionParametros.iPuertoCPU = config_get_int_value(pConfiguracion, "PUERTO_CPU");

	configuracionParametros.iQuantum = config_get_int_value(pConfiguracion, "QUANTUM");
	configuracionParametros.iRetardoQ = config_get_int_value(pConfiguracion, "RETARDO");

}

t_prog * nuevo_programa(char* codigoAnsisop, int socketPrograma) {

	t_prog * nuevoPrograma = malloc(sizeof(t_prog));

	nuevoPrograma->programa = malloc(sizeof(t_nuevoPrograma));
	nuevoPrograma->programa->pcb = malloc(sizeof(t_pcb));

	nuevoPrograma->programa->socketProg = socketPrograma;
	nuevoPrograma->peso = 0;

	nuevoPrograma->metadada = metadata_desde_literal(codigoAnsisop);
	nuevoPrograma->programa->pcb->pid = generar_pid();
	if (nuevoPrograma->metadada->etiquetas != NULL) {
		nuevoPrograma->programa->pcb->tamanioEtiqueta = nuevoPrograma->metadada->etiquetas_size;

	} else {
		nuevoPrograma->programa->pcb->tamanioEtiqueta = 0;
	}

	nuevoPrograma->programa->pcb->tamanioContexto = 0;
	nuevoPrograma->programa->pcb->programCounter = nuevoPrograma->metadada->instruccion_inicio;
	nuevoPrograma->programa->pcb->cantidad_de_etiquetas = nuevoPrograma->metadada->cantidad_de_etiquetas;
	nuevoPrograma->programa->pcb->cantidad_de_funciones = nuevoPrograma->metadada->cantidad_de_funciones;

	nuevoPrograma->archivoAnsisop = malloc(strlen(codigoAnsisop)+1);
	memcpy(nuevoPrograma->archivoAnsisop,codigoAnsisop,strlen(codigoAnsisop)+1);

	free(codigoAnsisop);

	return nuevoPrograma;
}

int generar_pid() {
	pidGeneral++;
	return pidGeneral;
}

void inicializar_estructura_poll(struct pollfd* stMiPoll, int serverSocket) {

	stMiPoll->fd = serverSocket;
	stMiPoll->events = POLLIN;
	stMiPoll->revents = 0;
}


/*
 *
 * Conexiones
 *
 */

int* socket_server_accept(int *socket_server) {
	int *remote_client_socket;
	struct sockaddr_in *remote_address;
	int addrlen;

	remote_client_socket = malloc(sizeof(int));
	addrlen = sizeof(struct sockaddr_in);
	remote_address = malloc(sizeof(struct sockaddr_in));

	if((*remote_client_socket = accept(*socket_server, (struct sockaddr *)remote_address, (void *) &addrlen)) == -1) {
		log_error(loggerMain, "Fallo accept");
		exit(EXIT_FAILURE);
	}

	free(remote_address);
	return remote_client_socket;
}

int* iniciarServidor() {

	struct sockaddr_in serv;

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(config_get_int_value(pConfiguracion,"PUERTO_PROG"));

	//Creo socket
	int optval = 1;
	int *serverSocket = malloc(sizeof(int));
	*serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(*serverSocket,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));

	//Bindeo
	if( bind(*serverSocket, (struct sockaddr *)&serv, sizeof(struct sockaddr)) !=0) {
		log_error(loggerMain, "Fallo el bind");
	}else {
		log_debug(loggerMain, "Se hizo el bind");

	}

	//Escucho
	if (listen(*serverSocket, BACKLOG) != 0) {
		log_error(loggerMain, "Fallo el listen");
	} else {
		log_debug(loggerMain, "Se hizo el listen");

	}
	return serverSocket;
}

char * recibir (int socketProg, int buffer_size) {

	char* bufferData = NULL;
	int tamanioBuffer = 0;
	int	tamanioRecv = 0;

	while (tamanioRecv == tamanioBuffer)
	{
		tamanioBuffer += 10;
		if (bufferData == NULL)
			bufferData = malloc(buffer_size);
		else
			bufferData = realloc(bufferData, tamanioBuffer);
			tamanioRecv = recv(socketProg, bufferData, tamanioBuffer, MSG_PEEK);
	}

	recv(socketProg, bufferData, tamanioRecv, 0);

	if (tamanioRecv == 0) {
		log_debug(loggerMain,"Programa desconectado");
		close(socketProg);

	} else if(tamanioRecv == -1) {
		log_error(loggerMain, "Fallo el recv del programa");
	}

	return bufferData;
}

/*
 *
 * Planificadores
 *
 */

void * iniciar_plp (void* argumento) {

	planificar_programas();

	return NULL;
}

void * iniciar_pcp (void* argumento) {

	workPCP();

	return NULL;
}

void * iniciar_hiloconsola(void* argumento) {
	iniciar_consola();
	return NULL;
}
