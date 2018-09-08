/*
 * conexionesCpu.c
 *
 *  Created on: 26/06/2014
 *      Author: utnso
 */


#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>

#include "conexionesCpu.h"
#include "../cpuMain2.h"

int conectarAOtroProceso (char *ip,uint32_t puerto) {

	int32_t resultadoConexion;
	struct sockaddr_in socketDestino;

	int sockt = socket(AF_INET, SOCK_STREAM, 0);

	memset(&socketDestino, 0, sizeof(socketDestino));
	socketDestino.sin_family = AF_INET;
	socketDestino.sin_addr.s_addr = inet_addr(ip);
	socketDestino.sin_port = htons(puerto);

	resultadoConexion = connect(sockt, (struct sockaddr *) &socketDestino,sizeof(struct sockaddr));

	if(resultadoConexion < 0){
		return -1;
	} else {
		return sockt;
	}
}

void conectarAUmv (char* ipUmv, uint32_t puertoUmv) {
	socketUmv = conectarAOtroProceso(ipUmv, puertoUmv);

	if(socketUmv==-1)
		log_error(logCpu,"Fallo al conectar con UMV");

	while(socketUmv == -1){
		socketUmv = conectarAOtroProceso(ipUmv, puertoUmv);
	}
	int tipoHandshake = cpu;
	t_buffer *buffer = umv_serializar(handshake, &tipoHandshake);

	enviaYRecibeAOtroProceso(buffer,socketUmv);
}

void conectarAKernel(char* ipKernel, uint32_t puertoKernel) {
	socketKernel = conectarAOtroProceso(ipKernel, puertoKernel);
	if(socketKernel==-1)
			log_error(logCpu,"Fallo al conectar con KERNEL");

	while(socketKernel == -1){
		socketKernel = conectarAOtroProceso(ipKernel, puertoKernel);
	}
}

t_buffer* socket_recv (int sockt){
	t_buffer* nueBuffer = malloc(sizeof(t_buffer));
	char* bufferData = NULL;
	int tamanioBuffer = 0;
	int	tamanioRecv = 0;

	while (tamanioRecv == tamanioBuffer) {
		tamanioBuffer += 10;
		if (bufferData == NULL) {
			bufferData = malloc(tamanioBuffer);
		} else {
			bufferData = realloc(bufferData, tamanioBuffer);
		}
		tamanioRecv = recv(sockt, bufferData, tamanioBuffer, MSG_PEEK);
	}

	recv(sockt, bufferData, tamanioRecv, 0);

	if (tamanioRecv == 0) {
		log_error(logCpu,"se desconecto el servidor");
	} else if(tamanioRecv == -1) {
		log_error(logCpu,"fallo el recv");
		exit(-1);
	}

	nueBuffer->data = bufferData;
	nueBuffer->size = tamanioRecv;

	return nueBuffer;
}

t_buffer* enviaYRecibeAOtroProceso (t_buffer *buffer,uint32_t sockt){
	int32_t resultadoEnvio = send(sockt,buffer -> data,buffer -> size,0);

	if(resultadoEnvio < 0){
		log_error(logCpu,"error en el envio");
	} else {
	}

	destroy_buffer(buffer);

	return socket_recv(sockt);
}

void enviarAKernel (t_buffer *buffer) {
	int32_t resultadoEnvio = send(socketKernel,buffer -> data,buffer -> size,0);

	if(resultadoEnvio < 0){
		log_error(logCpu,"error en el envio kernel");
	} else {
	}

	destroy_buffer(buffer);

}


