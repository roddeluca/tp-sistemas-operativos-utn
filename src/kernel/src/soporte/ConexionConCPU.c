/*
 * ConexionConCPU.c
 *
 *  Created on: 15/06/2014
 *      Author: utnso
 *      
 *      
 */

#include "ConexionConCPU.h"

#include <semaphore.h>
#include <serializar.h>
#include <protocolo.h>

char szBuffer[80];
t_log * logger = NULL,*logCPU = NULL;


void* escucharConexionesCPU(void* argumentos) {

	pid_t procID;

	procID = getpid();
	//printf("ingrese nivel de loggeo:.\n");
	//gets(szBuffer);
	sprintf(szBuffer, "th_recibe_conexiones_cpu_%d.log", procID);

	logger = log_create(szBuffer, PROGRAMA, false, logLevel);

	sprintf(szBuffer, "INICIO programa %s.", PROGRAMA);
	log_info(logger, szBuffer);

	t_socket_server* server = socket_crearServidor(
			configuracionParametros.iPuertoCPU);
	t_socket_cliente* cliente;

	//Listen
	listen(server->desc, SOMAXCONN);

	sprintf(szBuffer, "escucho conexiones con el socket %d.", server->desc);
	log_debug(logger, szBuffer);

	cliente = socket_accept(server);
	sprintf(szBuffer, "acepto conexion del socket %d.", cliente->socket->desc);
	log_debug(logger, szBuffer);
	while (cliente->socket->desc) {

		pthread_t nuevaConexion_thread;

		if (pthread_create(&nuevaConexion_thread, NULL, pfnControladorConexion,
				(void*) cliente) < 0) {
			perror("Error: no se pudo crear el hilo de la conexion");
			return NULL ;
		}
		cliente = socket_accept(server);
		sprintf(szBuffer, "acepto conexion del socket %d.",
				cliente->socket->desc);
		log_debug(logger, szBuffer);
	}

	if (cliente->socket->desc < 0) {

		sprintf(szBuffer, "fallo el accept");
		log_debug(logger, szBuffer);
		return NULL ;
	}
	return NULL ;
}

t_socket *socket_crearServidor(int puerto) {
	int socket_escucha;
	struct sockaddr_in *server = malloc(sizeof(struct sockaddr_in));
	t_socket* nueSocket = malloc(sizeof(t_socket));

	//Creo socket
	int optval = 1;
	socket_escucha = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(socket_escucha, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	if (socket_escucha == -1) {

		sprintf(szBuffer, "No se pudo crear el socket");
		log_error(logger, szBuffer);
		exit(EXIT_FAILURE);
	}

	//Preparo la estructura sockaddr_in
	memset(server, 0, sizeof(*server)); //inicializa con ceros la estructura
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = htonl(INADDR_ANY );
	server->sin_port = htons(puerto); //puerto de escucha asignado

	//Bind
	if (bind(socket_escucha, (struct sockaddr *) server, sizeof(*server)) < 0) {

		sprintf(szBuffer, "fallo el bind");
		log_error(logger, szBuffer);
		exit(EXIT_FAILURE);
	}
	nueSocket->desc = socket_escucha;
	nueSocket->my_addr = server;
	return nueSocket;
}

t_socket_cliente* socket_accept(t_socket_server* server) {
	t_socket_cliente *sock_cliente = malloc(sizeof(t_socket_cliente));
	t_socket *cliente = malloc(sizeof(t_socket));
	struct sockaddr_in *remote_adress = malloc(sizeof(struct sockaddr_in));
	int fdCliente;

	int tamanioSock = sizeof(struct sockaddr_in);

	fdCliente = accept(server->desc, (struct sockaddr *) remote_adress,
			(socklen_t*) &tamanioSock);
	if (fdCliente < 0) {

		sprintf(szBuffer, "fallo el acept");
		log_error(logger, szBuffer);
		exit(EXIT_FAILURE);
	}

	cliente->desc = fdCliente;
	cliente->my_addr = remote_adress;

	sock_cliente->socket_serv = server;
	sock_cliente->socket = cliente;

	return sock_cliente;
}

void *pfnControladorConexion(void *pCliente) {
	//t_conexion stConexion;
	//Obtengo el descriptor del socket
	t_socket_cliente* cliente = (t_socket_cliente*) pCliente;
	t_buffer *buffer;	// *buffer_operacion;
	t_buffer *buffer_respuesta = NULL;
	t_nuevoPrograma * programa;
	int iOperacion;
	pid_t prID;

	prID = getpid();

	sprintf(szBuffer,"th_conexion_con_CPU_%d_%d.log",cliente->socket->desc,prID);
	logCPU=log_create(szBuffer,"Conexion con CPU",false,logLevel);

	sprintf(szBuffer,"Comienzo ejecucion del hilo de conexion con CPU [%d]",cliente->socket->desc);
	log_info(logCPU,szBuffer);

	t_mensaje* stMsgDeserializado;

	t_SaludoCpu UnSaludo = { configuracionParametros.iQuantum,
			configuracionParametros.iRetardoQ };

	/*HANSHAKE con este CPU*/

	buffer_respuesta = serializar_kernel(primerMsj, &UnSaludo);

	socket_send(cliente, buffer_respuesta);

	destroy_buffer(buffer_respuesta);

	buffer = socket_recv(cliente);
	if (buffer->size <= 0) // si se desconecto el cliente
			{
		socket_clienteDestroy(cliente);
		destroy_buffer(buffer);
		pthread_exit(0);
	}

	sem_wait(&productorConsumidor);

	pthread_mutex_lock(&mutexReady);
	programa = list_remove(listaReady, 0);
	pthread_mutex_unlock(&mutexReady);

	sprintf(szBuffer, "saco de ready el programa %d", programa->pcb->pid);
	log_debug(logCPU, szBuffer);

	pthread_mutex_lock(&mutexExec);
	list_add(listaExec, programa);
	pthread_mutex_unlock(&mutexExec);

	sprintf(szBuffer, "cargo en la lista de ejecucion el programa %d",
			programa->pcb->pid);
	log_debug(logCPU, szBuffer);

	buffer_respuesta = serializar_kernel(atendeProceso, programa->pcb);

	socket_send(cliente, buffer_respuesta);
	destroy_buffer(buffer_respuesta);

	buffer = socket_recv(cliente); //recivo el siguiente mensaje

	int valor;

	while (buffer->size > 0) {

		if (buffer->size > 0) {

			stMsgDeserializado = deserializar_kernel(buffer);
			char* tempVariable;
			t_entradaSalida * entradaSalida;
			t_buffer * buf;

			switch (stMsgDeserializado->idOperacion) {
			case obtenerValorVarCompartida:

				tempVariable = obtenerVariable(stMsgDeserializado->estructura);

				if (strcmp(tempVariable, "exit") == 0) {
					char * errProg = string_duplicate(
							"No existe la variable compartida\n");
					send(programa->socketProg, errProg, strlen(errProg) + 1, 0);
					free(errProg);
					iOperacion = terminar;

					sprintf(szBuffer, "variable %s no existe",
							(char *) stMsgDeserializado->estructura);
					log_debug(logCPU, szBuffer);
				}

				else {
					iOperacion = valorCompartida;
					valor = atoi(tempVariable);
					sprintf(szBuffer, "la variable %s tiene valor %d",
							(char *) stMsgDeserializado->estructura, valor);
					log_debug(logCPU, szBuffer);

				}
				buf = serializar_kernel(iOperacion, &valor);
				socket_send(cliente, buf);
				destroy_buffer(buf);
				free(tempVariable);
				break;

			case asignarValorVarCompartida:
				tempVariable = asignarVariable(stMsgDeserializado->estructura);

				if (strcmp(tempVariable, "exit") == 0) {
					char * errProg = string_duplicate(
							"No existe la variable compartida\n");
					send(programa->socketProg, errProg, strlen(errProg) + 1, 0);
					free(errProg);
					iOperacion = terminar;
					sprintf(szBuffer, "variable %s no existe",
							(char *) stMsgDeserializado->estructura);
					log_debug(logCPU, szBuffer);
				}

				else {
					iOperacion = continuar;
					valor = atoi(tempVariable);
					sprintf(szBuffer, "la variable %s ahora vale %d",
							(char *) stMsgDeserializado->estructura, valor);
					log_debug(logCPU, szBuffer);
				}
				buf = serializar_kernel(iOperacion, &valor);
				socket_send(cliente, buf);
				destroy_buffer(buf);
				free(tempVariable);

				break;

			case imprimirKernel:
				tempVariable = imrpimirVariable(stMsgDeserializado->estructura,
						programa->socketProg);

				if (strcmp(tempVariable, "exit") == 0) {
					iOperacion = terminar;
					sprintf(szBuffer, "no se pudo imprimir la variable");
					log_debug(logCPU, szBuffer);
				} else {
					iOperacion = continuar;
					sprintf(szBuffer, "se imprimio la variable");
					log_debug(logCPU, szBuffer);
				}

				buf = serializar_kernel(iOperacion, tempVariable);
				socket_send(cliente, buf);
				destroy_buffer(buf);
				free(tempVariable);

				break;

			case imprimirTextoKernel:
				tempVariable = imrpimirTexto(stMsgDeserializado->estructura,
						programa->socketProg);

				if (strcmp(tempVariable, "exit") == 0) {
					sprintf(szBuffer, "no se pudo imprimir el texto");
					log_debug(logCPU, szBuffer);
					iOperacion = terminar;
				} else {
					sprintf(szBuffer, "se imprimio el texto");
					log_debug(logCPU, szBuffer);
					iOperacion = continuar;
				}

				buf = serializar_kernel(iOperacion, tempVariable);
				socket_send(cliente, buf);
				destroy_buffer(buf);
				free(tempVariable);
				break;

			case entradaSalidaKernel:
				entradaSalida =
						(t_entradaSalida *) stMsgDeserializado->estructura;
				*programa->pcb = entradaSalida->pcb;

				iOperacion = mandarADispositivo(entradaSalida, programa);

				if (iOperacion == terminar) {

					char * errProg = string_duplicate(
							"No existe el dispositivo \n");
					send(programa->socketProg, errProg, strlen(errProg) + 1, 0);

					log_debug(logCPU, errProg);

					free(errProg);

					moverAExit(programa);
					sprintf(szBuffer, "se mueve a exit el programa %d",
							programa->pcb->pid);
					log_debug(logCPU, szBuffer);
				}

				sem_wait(&productorConsumidor);

				pthread_mutex_lock(&mutexReady);
				programa = list_remove(listaReady, 0);
				pthread_mutex_unlock(&mutexReady);

				sprintf(szBuffer, "se saca de ready el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				pthread_mutex_lock(&mutexExec);
				list_add(listaExec, programa);
				pthread_mutex_unlock(&mutexExec);

				sprintf(szBuffer, "se mueve a ejecucion el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				buffer_respuesta = serializar_kernel(atendeProceso,
						programa->pcb);

				socket_send(cliente, buffer_respuesta);
				destroy_buffer(buffer_respuesta);

				break;

			case waitKernel:
				tempVariable = fnLock(stMsgDeserializado->estructura, programa);

				if (strcmp(tempVariable, "exit") == 0) {
					char * errProg = string_duplicate(
							"No existe el semaforo\n");
					send(programa->socketProg, errProg, strlen(errProg) + 1, 0);

					log_debug(logCPU, errProg);
					free(errProg);
					iOperacion = terminar;
				} else {
					iOperacion = atoi(tempVariable);
					sprintf(szBuffer, "se hizo un wait al semaforo %s",
							((t_MsjWait *) stMsgDeserializado->estructura)->semaforo);
					log_debug(logCPU, szBuffer);
				}

				buf = serializar_kernel(iOperacion, tempVariable);
				socket_send(cliente, buf);
				destroy_buffer(buf);

				if (iOperacion != continuar) {
					sem_wait(&productorConsumidor);

					pthread_mutex_lock(&mutexReady);
					programa = list_remove(listaReady, 0);
					pthread_mutex_unlock(&mutexReady);

					sprintf(szBuffer, "se saca de ready el programa %d",
							programa->pcb->pid);
					log_debug(logCPU, szBuffer);

					pthread_mutex_lock(&mutexExec);
					list_add(listaExec, programa);
					pthread_mutex_unlock(&mutexExec);

					sprintf(szBuffer, "se mueve a ejecucion el programa %d",
							programa->pcb->pid);
					log_debug(logCPU, szBuffer);

					buffer_respuesta = serializar_kernel(atendeProceso,
							programa->pcb);

					socket_send(cliente, buffer_respuesta);
					destroy_buffer(buffer_respuesta);
				}

				free(tempVariable);
				break;

			case signalKernel:
				tempVariable = fnUnlock(stMsgDeserializado->estructura);

				if (strcmp(tempVariable, "exit") == 0) {
					char * errProg = string_duplicate(
							"No existe el semaforo\n");
					send(programa->socketProg, errProg, strlen(errProg) + 1, 0);
					log_debug(logCPU, errProg);
					free(errProg);
					iOperacion = terminar;
				} else {
					sprintf(szBuffer, "se hizo signal al semaforo %s",
							(char*) stMsgDeserializado->estructura);
					log_debug(logger, szBuffer);
					iOperacion = atoi(tempVariable);
				}

				buf = serializar_kernel(iOperacion, tempVariable);
				socket_send(cliente, buf);
				destroy_buffer(buf);

				free(tempVariable);
				break;

			case notificarPorFinRafaga:
				recargarReady(stMsgDeserializado->estructura);
				sprintf(szBuffer, "mueve a ready el programa %d",
						((t_pcb *) stMsgDeserializado->estructura)->pid);
				log_debug(logCPU, szBuffer);

				sem_wait(&productorConsumidor);

				pthread_mutex_lock(&mutexReady);
				programa = list_remove(listaReady, 0);
				pthread_mutex_unlock(&mutexReady);

				sprintf(szBuffer, "se saca de ready el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				pthread_mutex_lock(&mutexExec);
				list_add(listaExec, programa);
				pthread_mutex_unlock(&mutexExec);

				sprintf(szBuffer, "se mueve a ejecucion el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				buffer_respuesta = serializar_kernel(atendeProceso,
						programa->pcb);

				socket_send(cliente, buffer_respuesta);
				destroy_buffer(buffer_respuesta);

				break;

			case notificarPorFinPrograma:

				moverAExit(programa);

				sprintf(szBuffer, "mueve a exit el programa %d",
						((t_pcb *) stMsgDeserializado->estructura)->pid);
				log_debug(logCPU, szBuffer);

				sprintf(szBuffer, "se mueve a exit el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				sem_wait(&productorConsumidor);

				pthread_mutex_lock(&mutexReady);
				programa = list_remove(listaReady, 0);
				pthread_mutex_unlock(&mutexReady);

				sprintf(szBuffer, "se saca de ready el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				pthread_mutex_lock(&mutexExec);
				list_add(listaExec, programa);
				pthread_mutex_unlock(&mutexExec);

				sprintf(szBuffer, "se mueve a ejecucion el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				buffer_respuesta = serializar_kernel(atendeProceso,
						programa->pcb);

				socket_send(cliente, buffer_respuesta);
				destroy_buffer(buffer_respuesta);

				break;

			case exception:

				send(programa->socketProg, "Falla de Segmento\n", 19, 0);

				sprintf(szBuffer, "ocurrio una falla de segmento");
				log_debug(logCPU, szBuffer);

				moverAExit(programa);

				sprintf(szBuffer, "mueve a exit el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				sem_wait(&productorConsumidor);

				pthread_mutex_lock(&mutexReady);
				programa = list_remove(listaReady, 0);
				pthread_mutex_unlock(&mutexReady);

				sprintf(szBuffer, "mueve a ready el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				pthread_mutex_lock(&mutexExec);
				list_add(listaExec, programa);
				pthread_mutex_unlock(&mutexExec);

				sprintf(szBuffer, "mueve a ejecucion el programa %d",
						programa->pcb->pid);
				log_debug(logCPU, szBuffer);

				buffer_respuesta = serializar_kernel(atendeProceso,
						programa->pcb);

				socket_send(cliente, buffer_respuesta);
				destroy_buffer(buffer_respuesta);

				break;

			default:
				sprintf(szBuffer, "error al obtener la operacion, llego %d",
						iOperacion);
				log_error(logCPU, szBuffer);
				pthread_exit(0);
			}

		} else {
			sprintf(szBuffer, "error en la conexion cun cpu");
			log_error(logCPU, szBuffer);

		}

		liberarTMensaje(stMsgDeserializado);
		buffer = socket_recv(cliente); //recivo el siguiente mensaje

	}

	moverAExit(programa);

	sprintf(szBuffer, "mueve a exit el programa %d", programa->pcb->pid);
	log_debug(logCPU, szBuffer);

	socket_clienteDestroy(cliente);
	destroy_buffer(buffer);
	pthread_exit(0);

}

