/*
 * serializar.h
 *
 *  Created on: 19/05/2014
 *      Author: utnso
 */
#include <stdint.h>

#ifndef SERIALIZAR_H_
#define SERIALIZAR_H_

/* STRUCTS */

typedef u_int32_t t_puntero;

typedef struct solicitudBytes
{
	t_puntero base;
	uint32_t offset;
	uint32_t tamanio;
}t_solicitudBytes;

typedef struct almacenarBytes
{
	t_puntero base;
	uint32_t offset;
	uint32_t tamanio;
	char* buffer;
}t_almacenarBytes;

typedef struct t_buffer
{
	uint32_t size;
	char* data;
}t_buffer;

typedef struct t_operacion
{
	int idOperacion;
	void* estructura;
}t_mensaje;

typedef uint32_t t_idPrograma;

enum{ kernel, cpu };
enum{ handshake, solicitar, almacenar, cambioProcesoActivo, crearSegmento, destruirSegmentos };
enum{ OK, segmentoCreado, bytesSolicitados, segmentation_fault, memory_overload };


/* FUNCIONES */

t_mensaje* 	umv_deserializar(t_buffer* stream);

t_buffer* 	umv_serializar(uint32_t operacion, void* estructura);

void umv_destroy_mensaje(t_mensaje* mensaje);

void 		destroy_buffer(t_buffer* buffer);


// inicializar Struct end

#endif /* SERIALIZAR_H_ */
