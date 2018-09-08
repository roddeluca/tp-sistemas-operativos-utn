/*
 * ConexionConUMV.c
 *
 *  Created on: 05/06/2014
 *      Author: utnso
 */


#include "ConexionConUMV.h"



int conectar_a_umv(char* ip, int puerto) {

	//printf("IP UMV:%s\n",ip);
	struct sockaddr_in dest;

	int socket_umv = socket(AF_INET, SOCK_STREAM, 0);

	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(ip);
	dest.sin_port = htons(puerto);

	//printf("Conectandose a UMV.......");
	int conexion = connect(socket_umv, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	if(conexion >= 0) {
	//	printf(" exitosa\n");
		return socket_umv;
	} else {
	//	log_error(loggerPLP,"Error al conectarse con la UMV");
		return conexion;
	}

}

t_mensaje * enviar_mensajes_a_umv (int socket_umv,void* estructura,int operacion){

	t_mensaje * respuesta;
	t_buffer* buffer = umv_serializar(operacion,estructura);
	int sendRes = send(socket_umv,buffer->data,buffer->size,0);
	if (sendRes> 0) {
		t_buffer* buffer2 = recibir_umv(socket_umv);
		respuesta = umv_deserializar(buffer2);

	} else {
	//	log_error(loggerPLP,"Error al enviarle a la UMV");
		respuesta = malloc(0);
	}
	return respuesta;
}

t_buffer *recibir_umv(int socketUMV) {

	t_buffer* nueBuffer = malloc(sizeof(t_buffer));
	char* bufferData = NULL;
	int tamanioBuffer = 0;
	int	tamanioRecv = 0;

	while (tamanioRecv == tamanioBuffer)
	{
		tamanioBuffer += 10;
		if (bufferData == NULL) {
			bufferData = malloc(tamanioBuffer);
		}
		else {
			bufferData = realloc(bufferData, tamanioBuffer);
		}
		tamanioRecv = recv(socketUMV, bufferData, tamanioBuffer, MSG_PEEK);

	}

	recv(socketUMV, bufferData, tamanioRecv, 0);

	if (tamanioRecv == 0) {
			//puts("Desconectado de UMV");
			close(socketUMV);

	} else if(tamanioRecv == -1) {
			//puts("Fallo el recv");
	}

	nueBuffer->data = bufferData;
	nueBuffer->size = tamanioRecv;
	return nueBuffer;
}

void destruir_segmento(int socket_umv, int pid) {
	int miTipo = kernel;

	umv_destroy_mensaje(enviar_mensajes_a_umv(socket_umv, &miTipo, handshake));
	//printf("Handshake.....OK\n");

	umv_destroy_mensaje(enviar_mensajes_a_umv(socket_umv, &pid, cambioProcesoActivo));
	//printf("Cambio de proceso activo.....OK\n");

	umv_destroy_mensaje(enviar_mensajes_a_umv(socket_umv, &pid, destruirSegmentos));
	//printf("Eliminando segmentos.....OK\n");
}
