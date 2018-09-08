/*
 * sockets.c
 *
 *  Created on: 11/05/2014
 *      Author: utnso
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> //inet_addr
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/*ahora nuestras librerias*/
#include "../estructurasComunes.h"
#include "../umvMain.h"
#include "sockets.h"
#include "../serializado/serializado.h"

/*librerias a mover*/
#include <commons/log.h>

t_socket *socket_crearServidor(t_puntero puerto)
{
	int socket_escucha;
	struct sockaddr_in *server = malloc(sizeof(struct sockaddr_in));
	t_socket* nueSocket = malloc(sizeof(t_socket));

	//Creo socket
	socket_escucha = socket(AF_INET, SOCK_STREAM, 0);
	int optval = 1;
	setsockopt(socket_escucha, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	if (socket_escucha == -1)
	{
		log_error(logSocketUmv,"No se pudo crear el socket");

		exit(EXIT_FAILURE);
	}
	//puts("Socket creado");

	//Preparo la estructura sockaddr_in
	memset(server, 0, sizeof(*server)); //inicializa con ceros la estructura
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = htonl(INADDR_ANY );
	server->sin_port = htons(PORT); //puerto de escucha asignado

	//Bind
	if (bind(socket_escucha, (struct sockaddr *) server, sizeof(*server)) < 0)
	{
		perror("Error: fallo el bind");
		exit(EXIT_FAILURE);
	}
	//puts("Se hizo el bind");
	nueSocket->desc = socket_escucha;
	nueSocket->my_addr = server;
	return nueSocket;
}

void socket_listen(t_socket_server* server)
{
	listen(server->desc, SOMAXCONN);
}

t_socket_cliente* socket_accept(t_socket_server* server)
{
	t_socket_cliente *sock_cliente = malloc(sizeof(t_socket_cliente));
	t_socket *cliente = malloc(sizeof(t_socket));
	struct sockaddr_in *remote_adress = malloc(sizeof(struct sockaddr_in));
	int fdCliente;

	int tamanioSock = sizeof(struct sockaddr_in);

	fdCliente = accept(server->desc, (struct sockaddr *) remote_adress,
			(socklen_t*) &tamanioSock);
	if (fdCliente < 0)
	{
		perror("Error: fallo el accept");
		exit(EXIT_FAILURE);
	}

	cliente->desc = fdCliente;
	cliente->my_addr = remote_adress;

	sock_cliente->socket_serv = server;
	sock_cliente->socket = cliente;

	return sock_cliente;
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
		log_info(logSocketUmv,"Desconexion programada");
	}
	else if (tamanioRecv == -1)
	{
		log_error(logSocketUmv,"Desconexion inesperada");
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

void socket_bufferDestroy(t_buffer* buffer)
{
	free(buffer->data);
	free(buffer);
}

int socket_escucharConexiones()
{
	logSocketUmv = log_create("conexiones.log","Conexiones",false,logLevel);

	t_socket_server* server = socket_crearServidor(PORT);
	t_socket_cliente* cliente;

	//Listen
	socket_listen(server);

	//Accept a las conexiones entrantes
	log_debug(logSocketUmv,"Esperando conexiones entrantes...");

	cliente = socket_accept(server);
	while (cliente->socket->desc)
	{
		//puts("Connexion aceptada");

		pthread_t nuevaConexion_thread;

		if (pthread_create(&nuevaConexion_thread, NULL, pfnControladorConexion,
				(void*) cliente) < 0)
		{
			log_error(logSocketUmv,"Error: no se pudo crear el hilo de la conexion");

			return EXIT_FAILURE;
		}
		cliente = socket_accept(server);
	}

	if (cliente->socket->desc < 0)
	{
		log_error(logSocketUmv,"Error: fallo el accept");

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void *pfnControladorConexion(void *pCliente)
{
	log_info(logSocketUmv,"Se creo el hilo");

	t_conexion stConexion;
	//Obtengo el descriptor del socket
	t_socket_cliente* cliente = (t_socket_cliente*) pCliente;
	t_buffer *buffer;
	t_buffer *buffer_respuesta;
	//Recivo handshake del cliente

	log_info(logSocketUmv,"--------------------------------------");

	buffer = socket_recv(cliente);
	if (buffer->size <= 0) // si se desconecto el cliente
	{
		log_info(logSocketUmv,"Se desconecto el cliente.");
		log_info(logSocketUmv,"Se cierra el hilo.");
		socket_clienteDestroy(cliente);
		socket_bufferDestroy(buffer);
		pthread_exit(0);
	}

	t_mensaje* stMsgDeserializado = deserealizar_operacion(buffer, &stConexion);
	if (stMsgDeserializado->idOperacion != handshake)
	{
		log_info(logSocketUmv,"Primero se debe realizar el handshake.");
		log_info(logSocketUmv,"Se desconecto el cliente.");
		log_info(logSocketUmv,"Se cierra el hilo.");
		socket_clienteDestroy(cliente);
		destroy_mensaje(stMsgDeserializado);
		socket_bufferDestroy(buffer);
		pthread_exit(0);
	}
	else
	{
		switch (stConexion.tipoConexion)
		{
		case cpu:
			log_info(logSocketUmv,"Se ha conectado un CPU.");
			break;
		case kernel:
			log_info(logSocketUmv,"Se ha conectado un kernel.");
			break;
		default:
			log_error(logSocketUmv,"No se reconoce el tipo de cliente.");

			socket_bufferDestroy(buffer);
			socket_clienteDestroy(cliente);
			destroy_mensaje(stMsgDeserializado);
			pthread_exit(0);
		}
		buffer_respuesta = serializar_respuesta(OK, &buffer); //paso &buffer para q no tire error
		//el handshake ya se hace directamente en la deserializacion;
	}
	destroy_mensaje(stMsgDeserializado);
	socket_bufferDestroy(buffer);
	socket_send(cliente, buffer_respuesta);	//respondo que se realizo el handshake
	socket_bufferDestroy(buffer_respuesta);

	log_info(logSocketUmv,"--------------------------------------");

	buffer = socket_recv(cliente);

	//Recivo mensajes del cliente
	while (buffer->size > 0)
	{

		t_mensaje* stMsgDeserializado = deserealizar_operacion(buffer,
				&stConexion);

		/* Suspendo la respuesta del mensaje RETARDO milisegundos */
		usleep(RETARDO * 1000);

		switch (stMsgDeserializado->idOperacion)
		{
		case almacenar:
			buffer_respuesta = almacenarBytes(stConexion.idPrograma,
					stMsgDeserializado->estructura);
			log_debug(logSocketUmv,"Se almacenaron los bytes");
			break;
		case cambioProcesoActivo:
			buffer_respuesta = serializar_respuesta(OK, &buffer); //paso &buffer para q no tire error
			log_debug(logSocketUmv,"Se hizo el cambio de proceso");
			break; //el cambio de proceso activo ya se hace directamente en la deserializacion;
		}

		if (stConexion.tipoConexion == kernel)
			switch (stMsgDeserializado->idOperacion)
			{
			case crearSegmento:
				buffer_respuesta = generarSegmento(stConexion.idPrograma,
						stMsgDeserializado->estructura);
				log_debug(logSocketUmv,"Se creo el segmento");
				break;
			case destruirSegmentos:
				buffer_respuesta = eliminarSegmentos(stConexion.idPrograma);
				log_debug(logSocketUmv,"Se eliminaron los segmentos");
				break;
			}

		if (stConexion.tipoConexion == cpu)
			switch (stMsgDeserializado->idOperacion)
			{
			case solicitar:
				buffer_respuesta = solicitarBytes(stConexion.idPrograma,
						stMsgDeserializado->estructura);
				log_debug(logSocketUmv,"Se solicitaron los bytes");
				break;
			}

		socket_send(cliente, buffer_respuesta);
		destroy_mensaje(stMsgDeserializado);
		socket_bufferDestroy(buffer);
		socket_bufferDestroy(buffer_respuesta);

		log_info(logSocketUmv,"--------------------------------------");
		buffer = socket_recv(cliente); //recivo el siguiente mensaje
	}
	socket_bufferDestroy(buffer);
	socket_clienteDestroy(cliente);
	log_info(logSocketUmv,"Se desconecto el cliente.");
	log_info(logSocketUmv,"Se cierra el hilo.");
	pthread_exit(0);
}

void socket_send(t_socket_cliente* cliente, t_buffer* buffer)
{

	send(cliente->socket->desc, buffer->data, buffer->size, 0);

}

# define KEYS 6
#define PATH_CONFIG "config.cfg"
