/*
 * sockets.h
 *
 *  Created on: 11/05/2014
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include "../estructurasComunes.h"
#include "commons/log.h"

/* STRUCTS */

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

t_log_level levelSocket;
t_log* logSocketUmv;

/* FUNCIONES */
int socket_escucharConexiones();
void *pfnControladorConexion(void *pCliente);
t_socket_server *socket_crearServidor(u_int32_t puerto);
void socket_listen(t_socket_server* server);
t_socket_cliente *socket_accept(t_socket_server* server);
t_buffer *socket_recv(t_socket_cliente* cliente); //recive mensajes de tamanio variable
void socket_send(t_socket_cliente* cliente, t_buffer* buffer); //envia mensajes de tamanio variable

void socket_serverDestroy(t_socket_server* server);
void socket_clienteDestroy(t_socket_cliente* cliente);
void socket_bufferDestroy(t_buffer* buffer);


/* VARIABLES EL ARCHIVO DE CONFIGURACION */
#define PORT 2030


#endif /* SOCKETS_H_ */
