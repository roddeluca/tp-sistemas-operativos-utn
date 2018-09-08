/*
 * serializador.c
 *
 *  Created on: 19/05/2014
 *      Author: utnso
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ahora las nuestras */
#include "serializado.h"
#include "../estructurasComunes.h"
#include "../sockets/sockets.h"

static void serializar_operacion(u_int32_t operacion, t_buffer* buffer);
static t_buffer* serializar_rta_ok(void);
static t_buffer* serializar_segmentoCreado(t_puntero* pSegmento);
static t_buffer* serializar_buffer(t_solicitudBytes* solicitud);
static t_buffer* serializar_segmentation_fault(void);
static t_buffer* serializar_memory_overload(void);

static t_puntero* consola_deserializar_segmentoCreado(t_buffer* stream);
static t_buffer* consola_deserializar_bytesSolicitados(t_buffer* stream);
static t_buffer* copiar_buffer(t_buffer* stream);
static t_buffer* consola_deserializar_segmentationFault(t_buffer* stream);
static t_buffer* consola_deserializar_memoryOverload(t_buffer* stream);

static t_solicitudBytes* deserializar_solicitudBytes(t_buffer* stream);
static t_almacenarBytes* deserializar_almacenarBytes(t_buffer* stream);
static void concretar_handshake(t_buffer* stream, u_int32_t* operacion);
static void cambiar_procesoActivo(t_buffer* stream, t_idPrograma* id);
static u_int32_t* deserealizar_crearSegmento(t_buffer* stream);

static t_puntero* consola_deserializar_segmentoCreado(t_buffer* stream)
{
	t_puntero* dirLogica = malloc(sizeof(t_puntero));
	memcpy(dirLogica, stream->data, stream->size);
	return dirLogica;
}

static t_buffer* consola_deserializar_bytesSolicitados(t_buffer* stream)
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

static t_buffer* consola_deserializar_segmentationFault(t_buffer* stream)
{
	return copiar_buffer(stream);
}

static t_buffer* consola_deserializar_memoryOverload(t_buffer* stream)
{
	return copiar_buffer(stream);
}

t_mensaje* consola_deserializar(t_buffer* stream)
{
	t_mensaje* operacion = malloc(sizeof(t_mensaje));
	char* nueStreamData;

	memcpy(&operacion->idOperacion, stream->data, sizeof(u_int32_t));
	//modifico el stream eliminando el tipo de operacion que hay que realizar
	stream->size -= sizeof(u_int32_t);
	nueStreamData = malloc(stream->size);
	memcpy(nueStreamData, stream->data + sizeof(u_int32_t), stream->size);
	free(stream->data);
	stream->data = nueStreamData;

	switch (operacion->idOperacion)
	{
	case OK:
		break;
	case segmentoCreado:
		operacion->estructura = consola_deserializar_segmentoCreado(stream);
		break;
	case bytesSolicitados:
		operacion->estructura = consola_deserializar_bytesSolicitados(stream);
		break;
	case segmentation_fault:
		operacion->estructura = consola_deserializar_segmentationFault(stream);
		break;
	case memory_overload:
		operacion->estructura = consola_deserializar_memoryOverload(stream);
		break;
	}
	return operacion;
}

t_mensaje* deserealizar_operacion(t_buffer* stream, t_conexion* stConexion)
{
	t_mensaje* operacion = malloc(sizeof(t_mensaje));
	char* nueStreamData;

	memcpy(&operacion->idOperacion, stream->data, sizeof(u_int32_t));
	//modifico el stream eliminando el tipo de operacion que hay que realizar
	stream->size -= sizeof(u_int32_t);
	nueStreamData = malloc(stream->size);
	memcpy(nueStreamData, stream->data + sizeof(u_int32_t), stream->size);
	free(stream->data);
	stream->data = nueStreamData;

	switch (operacion->idOperacion)
	{
	case handshake:
		concretar_handshake(stream, &stConexion->tipoConexion);
		break;
	case solicitar:
		operacion->estructura = deserializar_solicitudBytes(stream);
		break;
	case almacenar:
		operacion->estructura = deserializar_almacenarBytes(stream);
		break;
	case cambioProcesoActivo:
		cambiar_procesoActivo(stream, &stConexion->idPrograma);
		break;
	case crearSegmento:
		operacion->estructura = deserealizar_crearSegmento(stream);
		break;
	case destruirSegmentos:
		break;
	}

	return operacion;
}

static u_int32_t* deserealizar_crearSegmento(t_buffer* stream)
{
	u_int32_t* tamanio = malloc(stream->size);
	memcpy(tamanio, stream->data, sizeof(u_int32_t));

	return tamanio;
}

static void cambiar_procesoActivo(t_buffer* stream, t_idPrograma* id)
{
	memcpy(id, stream->data, stream->size);
}

static void concretar_handshake(t_buffer* stream, u_int32_t* operacion)
{
	memcpy(operacion, stream->data, stream->size);
}

static t_almacenarBytes* deserializar_almacenarBytes(t_buffer* stream)
{
	t_almacenarBytes* almacenar = malloc(stream->size);
	int desplazamiento = 0, size;
	memcpy(&almacenar->base, stream->data + desplazamiento, size =
			sizeof(char*));
	desplazamiento += size;
	memcpy(&almacenar->offset, stream->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&almacenar->tamanio, stream->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	almacenar->buffer = malloc(almacenar->tamanio);
	memcpy(almacenar->buffer, stream->data + desplazamiento,
			almacenar->tamanio);
	return almacenar;
}

static t_solicitudBytes* deserializar_solicitudBytes(t_buffer* stream)
{
	t_solicitudBytes* solicitud = malloc(stream->size);
	int desplazamiento = 0, size;
	memcpy(&solicitud->base, stream->data + desplazamiento, size =
			sizeof(char*));
	desplazamiento += size;
	memcpy(&solicitud->offset, stream->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&solicitud->tamanio, stream->data + desplazamiento, sizeof(u_int32_t));
	return solicitud;
}

t_buffer* serializar_respuesta(u_int32_t respuesta, void* estructura)
{
	t_buffer* buffer;
	switch (respuesta)
	{
	case OK:
		buffer = serializar_rta_ok();
		break;
	case bytesSolicitados:
		buffer = serializar_buffer(estructura);
		break;
	case segmentation_fault:
		buffer = serializar_segmentation_fault();
		break;
	case memory_overload:
		buffer = serializar_memory_overload();
		break;
	case segmentoCreado:
		buffer = serializar_segmentoCreado(estructura);
		break;
	}
	serializar_operacion(respuesta, buffer);
	return buffer;
}

static t_buffer* serializar_segmentation_fault()
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size =
			strlen(
					"Segmentation Fault: esta tratando de acceder a un sector de memoria no permitido")
					+ 1;
	buffer->data = malloc(buffer->size);
	strcpy(buffer->data,
			"Segmentation Fault: esta tratando de acceder a un sector de memoria no permitido");
	return buffer;
}

static t_buffer* serializar_memory_overload()
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size =
			strlen(
					"Memory Overload: no existe espacio suficiente para crear el segmento")
					+ 1;
	buffer->data = malloc(buffer->size);
	strcpy(buffer->data,
			"Memory Overload: no existe espacio suficiente para crear el segmento");
	return buffer;
}

static t_buffer* serializar_rta_ok()
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = 0;
	buffer->data = malloc(0);
	return buffer;
}

static void serializar_operacion(u_int32_t operacion, t_buffer* buffer)
{
	char* nueBufferData = malloc(buffer->size + sizeof(u_int32_t));
	memcpy(nueBufferData, &operacion, sizeof(u_int32_t));
	memcpy(nueBufferData + sizeof(u_int32_t), buffer->data, buffer->size);
	free(buffer->data);
	buffer->data = nueBufferData;
	buffer->size += sizeof(u_int32_t);
}

static t_buffer* serializar_buffer(t_solicitudBytes* solicitud)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->data = malloc(solicitud->tamanio);
	buffer->size = solicitud->tamanio;
	memcpy(buffer->data, (char*) solicitud->base + solicitud->offset,
			solicitud->tamanio);
	return buffer;
}

static t_buffer* serializar_segmentoCreado(t_puntero* pSegmento)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(t_puntero);
	buffer->data = malloc(buffer->size);
	memcpy(buffer->data, pSegmento, buffer->size);
	return buffer;
}

void destroy_mensaje(t_mensaje* mensaje)
{
	if (mensaje->idOperacion == almacenar)
	{
		t_almacenarBytes* alm = mensaje->estructura;
		free(alm->buffer);
	}
	if ((mensaje->idOperacion == handshake)
			|| (mensaje->idOperacion == cambioProcesoActivo) || (mensaje->idOperacion == destruirSegmentos))
	{
	}
	else
	{
		free(mensaje->estructura);
	}
	free(mensaje);
}

void consola_destroy_mensaje(t_mensaje* mensaje)
{
	if ((mensaje->idOperacion == bytesSolicitados)
			|| (mensaje->idOperacion == segmentation_fault)
			|| (mensaje->idOperacion == memory_overload))
	{
		socket_bufferDestroy(mensaje->estructura);
	}
	else if (mensaje->idOperacion == segmentoCreado)
	{
		free(mensaje->estructura);
	}
	free(mensaje);
}
