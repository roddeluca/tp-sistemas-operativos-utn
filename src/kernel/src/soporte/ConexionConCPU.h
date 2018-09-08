/*
 * ConexionConCPU.h
 *
 *  Created on: 15/06/2014
 *      Author: utnso
 */

#ifndef CONEXIONCONCPU_H_
#define CONEXIONCONCPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#include "../pcp/PCP.h"

#undef PROGRAMA
#define PROGRAMA "ConexionConCpu"

t_socket_cliente* socket_accept(t_socket_server* server);
t_socket *socket_crearServidor(int puerto);
void* escucharConexionesCPU(void* argumentos);


#endif /* CONEXIONCONCPU_H_ */
