/*
 * serializador.h
 *
 *  Created on: 19/05/2014
 *      Author: utnso
 */

#ifndef SERIALIZADOR_H_
#define SERIALIZADOR_H_

#include "../estructurasComunes.h"

typedef struct solicitudBytes
{
	t_puntero base;
	u_int32_t offset;
	u_int32_t tamanio;
}t_solicitudBytes;

typedef struct almacenarBytes
{
	t_puntero base;
	u_int32_t offset;
	u_int32_t tamanio;
	char* buffer;
}t_almacenarBytes;

t_mensaje* 	consola_deserializar(t_buffer* stream);
t_mensaje* 	deserealizar_operacion(t_buffer* stream, t_conexion* conexion);
void		destroy_mensaje(t_mensaje* operacion);
void 		consola_destroy_mensaje(t_mensaje* mensaje);
t_buffer* 	serializar_respuesta(u_int32_t respuesta, void* estructura);

#endif /* SERIALIZADOR_H_ */
