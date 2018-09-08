/*
 * ConexionConUMV.h
 *
 *  Created on: 05/06/2014
 *      Author: utnso
 */

#ifndef CONEXIONCONUMV_H_
#define CONEXIONCONUMV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>


#include <serializar.h>


int conectar_a_umv(char* ip, int puerto);
t_mensaje * enviar_mensajes_a_umv (int socket_umv,void* estructura,int operacion);
t_buffer * recibir_umv(int socketUMV);
void destruir_segmento(int socket_umv, int pid);

#endif /* CONEXIONCONUMV_H_ */
