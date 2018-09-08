/*
 * serializar.c
 *
 *  Created on: 19/05/2014
 *      Author: utnso
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/* ahora las nuetras */
#include "serializar.h"

/*  declaracions de funciones privadas */
static t_buffer* 	serializar_solicitarBytes(t_solicitudBytes* solicitud);
static t_buffer*	serializar_almacenarBytes(t_almacenarBytes* almacenar);
static t_buffer* 	serializar_procesoActivo(t_idPrograma* idPrograma);
static t_buffer*	serializar_crearSegmento(u_int32_t* estructura);
static t_buffer* 	serializar_destruirSegmentos(void);
static t_buffer* 	serializar_handshake(uint32_t* tipo);

static t_buffer* 	copiar_buffer(t_buffer* stream);
static t_puntero*	deserializar_segmentoCreado(t_buffer* stream);
static t_buffer* 	deserializar_bytesSolicitados(t_buffer* stream);
static t_buffer*	deserializar_segmentationFault(t_buffer* stream);
static t_buffer*	deserializar_memoryOverload(t_buffer* stream);

static void 		serializar_operacion(uint32_t operacion, t_buffer* buffer);

static t_buffer* 	serializar_solicitarBytes(t_solicitudBytes* solicitud)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->data = malloc(sizeof(t_solicitudBytes));
	int desplazamiento = 0,
		size;

	memcpy(buffer->data, &solicitud->base, size = sizeof(char*));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &solicitud->offset, size = sizeof(int));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &solicitud->tamanio, size = sizeof(int));
	desplazamiento += size;

	buffer->size = desplazamiento;
	return buffer;
}

static t_buffer*	serializar_almacenarBytes(t_almacenarBytes* almacenar)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->data = malloc(sizeof(char*)+sizeof(int)*2+almacenar->tamanio);
	int desplazamiento = 0,
		size;
 
	memcpy(buffer->data, &almacenar->base, size = sizeof(char*));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &almacenar->offset, size = sizeof(int));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &almacenar->tamanio, size = sizeof(int));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, almacenar->buffer, size = almacenar->tamanio);
	desplazamiento += size;

	buffer->size = desplazamiento;
	return buffer;
}

static t_buffer* serializar_procesoActivo(t_idPrograma* idPrograma)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->data = malloc(sizeof(t_idPrograma));

	memcpy(buffer->data, idPrograma, sizeof(t_idPrograma));
	buffer->size = sizeof(t_idPrograma);
	return buffer;
}

static t_buffer*	serializar_crearSegmento(u_int32_t* tamanio)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->data = malloc(sizeof(u_int32_t));

	memcpy(buffer->data, tamanio, sizeof(u_int32_t));

	buffer->size = sizeof(u_int32_t);
	return buffer;
}

t_buffer* 	serializar_destruirSegmentos()
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = 0;
	buffer->data = malloc(0);
	return buffer;
}

static t_buffer* 	serializar_handshake(uint32_t* tipo)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->data = malloc(sizeof(int));

	memcpy(buffer->data, tipo, sizeof(int));

	buffer->size = sizeof(int);
	return buffer;
}

t_buffer* 	umv_serializar(uint32_t operacion, void* estructura)
{
	t_buffer* buffer;
	switch (operacion)
	{
	case handshake:
		buffer = serializar_handshake(estructura);
		break;
	case solicitar:
		buffer = serializar_solicitarBytes(estructura);
		break;
	case almacenar:
		buffer = serializar_almacenarBytes(estructura);
		break;
	case cambioProcesoActivo:
		buffer = serializar_procesoActivo(estructura);
		break;
	case crearSegmento:
		buffer = serializar_crearSegmento(estructura);
		break;
	case destruirSegmentos:
		buffer = serializar_destruirSegmentos();
		break;
	}

	serializar_operacion(operacion, buffer);

	return buffer;
}

static void 	serializar_operacion(uint32_t operacion, t_buffer* buffer)
{
	char* nueBufferData = malloc(buffer->size + sizeof(int));
	memcpy(nueBufferData, &operacion, sizeof(int));
	memcpy(nueBufferData + sizeof(int), buffer->data , buffer->size);
	free(buffer->data);
	buffer->data = nueBufferData;
	buffer->size += sizeof(int);
}

void destroy_buffer(t_buffer* buffer)
{
	free(buffer->data);
	free(buffer);
}

// inicializar Struct begin

t_solicitudBytes* crearSolicitarBytesStruct (t_puntero base,uint32_t offset,uint32_t tamanio){
	t_solicitudBytes *solicitudBytes = malloc(sizeof(t_solicitudBytes));

	solicitudBytes -> base = base;
	solicitudBytes -> offset = offset;
	solicitudBytes -> tamanio = tamanio;

	return solicitudBytes;
}

t_almacenarBytes* crearAlmacenarBytesStruct (t_puntero base,uint32_t offset,uint32_t tamanio,char* buffer){
	t_almacenarBytes *almacenarBytes = malloc(sizeof(t_almacenarBytes));

	almacenarBytes -> base = base;
	almacenarBytes -> offset = offset;
	almacenarBytes -> tamanio = tamanio;
	almacenarBytes -> buffer = malloc(tamanio);

	memcpy(almacenarBytes->buffer, buffer, tamanio);

	return almacenarBytes;
}

t_mensaje* umv_deserializar(t_buffer* stream)
{
	t_mensaje* operacion = malloc(sizeof(t_mensaje));
	char* nueStreamData;

	memcpy(&operacion->idOperacion, stream->data, sizeof(int));
	//modifico el stream eliminando el tipo de operacion que hay que realizar
	stream->size -= sizeof(int);
	nueStreamData = malloc(stream->size);
	memcpy(nueStreamData, stream->data + sizeof(int), stream->size);
	free(stream->data);
	stream->data = nueStreamData;

	switch (operacion->idOperacion)
	{
	case OK:
		break;
	case segmentoCreado:
		operacion->estructura = deserializar_segmentoCreado(stream);
		break;
	case bytesSolicitados:
		operacion->estructura = deserializar_bytesSolicitados(stream);
		break;
	case segmentation_fault:
		operacion->estructura = deserializar_segmentationFault(stream);
		break;
	case memory_overload:
		operacion->estructura = deserializar_memoryOverload(stream);
		break;
	}
	return operacion;
}

static t_puntero* deserializar_segmentoCreado(t_buffer* stream)
{
	t_puntero* dirLogica = malloc(sizeof(t_puntero));
	memcpy(dirLogica, stream->data, stream->size);
	return dirLogica;
}

static t_buffer* deserializar_bytesSolicitados(t_buffer* stream)
{
	return copiar_buffer(stream);
}

static t_buffer*	deserializar_segmentationFault(t_buffer* stream)
{
	return copiar_buffer(stream);
}

static t_buffer*	deserializar_memoryOverload(t_buffer* stream)
{
	return copiar_buffer(stream);
}

static t_buffer* copiar_buffer(t_buffer* stream)
{
	t_buffer* nueBuffer = malloc(sizeof(t_buffer));
	nueBuffer->data = malloc(stream->size);
	nueBuffer->size = stream->size;
	memcpy(nueBuffer->data, stream->data, nueBuffer->size);
	return nueBuffer;
}

void umv_destroy_mensaje(t_mensaje* mensaje)
{
	if ((mensaje->idOperacion == bytesSolicitados)
			|| (mensaje->idOperacion == segmentation_fault)
			|| (mensaje->idOperacion == memory_overload))
	{
		destroy_buffer(mensaje->estructura);
	}
	else if (mensaje->idOperacion == segmentoCreado)
	{
		free(mensaje->estructura);
	}
	free(mensaje);
}
