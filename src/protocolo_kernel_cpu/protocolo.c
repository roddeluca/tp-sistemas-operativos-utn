/*
 *
 * 
 * protocolo.c
 *
 *  Created on: 20/06/2014
 *      Author: utnso
 */
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <parser/parser.h>
#include <parser/sintax.h>
#include <parser/metadata_program.h>

#include "protocolo.h"

static t_buffer* 	buffer_new();
static t_buffer* 	serializar_pcb(t_pcb* pcb);
static t_pcb* 		deserializar_pcb(t_buffer* buffer);
static void 		serializar_operacion(u_int32_t operacion, t_buffer* buffer);
static t_buffer* 	serializar_rta_ok();
static char* 		deserializar_string(t_buffer* buffer);
static t_buffer* 	serializar_string(char* string);
static t_valor_variable* 		deserializar_valorVariable(t_buffer* buffer);


static t_buffer* cpu_serializar_asignarVariableCompartida(t_variable_compartida* varComp);
static t_buffer* cpu_serializar_obtenerValorCompartida(t_nombre_compartida varComp);
static t_buffer* cpu_serializar_valorVariable(t_valor_variable* valor);
static t_buffer* cpu_serializar_signal(t_nombre_semaforo varComp);
static t_buffer* cpu_serializar_entradaSalida(t_entradaSalida* stDisp);
static t_buffer* cpu_serializar_exception(char* textoError);
static t_buffer* cpu_serializar_wait(t_MsjWait* stWait);



static t_SaludoCpu* cpu_deserializar_primerMsj(t_buffer* buffer);

static t_buffer* kernel_serializar_rta_continuar();
static t_buffer* kernel_serializar_rta_terminar();
static t_buffer* kernel_serializar_rta_bloquear();
static t_buffer* kernel_serializar_primerMsj(t_SaludoCpu* saludo);
static t_buffer* kernel_serializar_valorVarCompartida(t_valor_variable* valor);



static t_nombre_semaforo		kernel_deserializar_signal(t_buffer* buffer);
static t_variable_compartida* 	kernel_deserializar_asignarVarCompartida(t_buffer* buffer);
static t_nombre_compartida 		kernel_deserializar_obtenerValorCompartida(t_buffer* buffer);
static t_MsjWait * 				kernel_deserializar_wait(t_buffer* buffer);
static t_entradaSalida*			kernel_deserializar_entradaSalida(t_buffer* buffer);
static char* 					kernel_deserializar_exception(t_buffer* buffer);

t_buffer* serializar_kernel(u_int32_t operacion, void* estructura)
{
	t_buffer* buffer;

	switch (operacion)
	{
	case okKernel:
		buffer = serializar_rta_ok();
		break;
	case continuar:
		buffer = kernel_serializar_rta_continuar();
		break;
	case terminar:
		buffer = kernel_serializar_rta_terminar();
		break;
	case bloquear:
		buffer = kernel_serializar_rta_bloquear();
		break;
	case primerMsj:
		buffer = kernel_serializar_primerMsj(estructura);
		break;
	case atendeProceso:
		buffer = serializar_pcb(estructura);
		break;
	case valorCompartida:
		buffer = kernel_serializar_valorVarCompartida(estructura);
		break;
	}

	serializar_operacion(operacion, buffer);

	return buffer;
}

t_buffer* serializar_cpu(u_int32_t operacion, void* estructura)
{
	t_buffer* buffer;

	switch (operacion)
	{
	case obtenerValorVarCompartida:
		buffer = cpu_serializar_obtenerValorCompartida(estructura);
		break;
	case asignarValorVarCompartida:
		buffer = cpu_serializar_asignarVariableCompartida(estructura);
		break;
	case imprimirKernel:
		buffer = cpu_serializar_valorVariable(estructura);
		break;
	case imprimirTextoKernel:
		buffer = serializar_string(estructura);
		break;
	case entradaSalidaKernel:
		buffer = cpu_serializar_entradaSalida(estructura);
		break;
	case waitKernel:
		buffer = cpu_serializar_wait(estructura);
		break;
	case signalKernel:
		buffer = cpu_serializar_signal(estructura);
		break;
	case notificarPorFinRafaga:
		buffer = serializar_pcb(estructura);
		break;
	case notificarPorFinPrograma:
		buffer = serializar_pcb(estructura);
		break;
	case exception:
		buffer = cpu_serializar_exception(estructura);
		break;
	case okCpu:
		buffer = serializar_rta_ok();
		break;
	}

	serializar_operacion(operacion, buffer);

	return buffer;
}

t_mensaje* deserializar_kernel(t_buffer* buffer)
{
	t_mensaje* operacion = malloc(sizeof(t_mensaje));
	char* nueStreamData;

	memcpy(&operacion->idOperacion, buffer->data, sizeof(int));
	//modifico el stream eliminando el tipo de operacion que hay que realizar
	buffer->size -= sizeof(int);
	nueStreamData = malloc(buffer->size);
	memcpy(nueStreamData, buffer->data + sizeof(int), buffer->size);
	free(buffer->data);
	buffer->data = nueStreamData;

	switch (operacion->idOperacion)
	{
	case obtenerValorVarCompartida:
		operacion->estructura = kernel_deserializar_obtenerValorCompartida(
				buffer);
		break;
	case asignarValorVarCompartida:
		operacion->estructura = kernel_deserializar_asignarVarCompartida(buffer);
		break;
	case imprimirKernel:
		operacion->estructura = deserializar_valorVariable(buffer);
		break;
	case imprimirTextoKernel:
		operacion->estructura = deserializar_string(buffer);
		break;
	case entradaSalidaKernel:
		operacion->estructura = kernel_deserializar_entradaSalida(buffer);
		break;
	case waitKernel:
		operacion->estructura = kernel_deserializar_wait(buffer);
		break;
	case signalKernel:
		operacion->estructura = kernel_deserializar_signal(buffer);
		break;
	case notificarPorFinRafaga:
		operacion->estructura = deserializar_pcb(buffer);
		break;
	case notificarPorFinPrograma:
		operacion->estructura = deserializar_pcb(buffer);
		break;
	case exception:
		operacion->estructura = kernel_deserializar_exception(buffer);
		break;
	}
	return operacion;
}

t_mensaje* deserializar_cpu(t_buffer* buffer)
{
	t_mensaje* operacion = malloc(sizeof(t_mensaje));
	char* nueStreamData;

	memcpy(&operacion->idOperacion, buffer->data, sizeof(int));
	//modifico el stream eliminando el tipo de operacion que hay que realizar
	buffer->size -= sizeof(int);
	nueStreamData = malloc(buffer->size);
	memcpy(nueStreamData, buffer->data + sizeof(int), buffer->size);
//	free(buffer->data);
	buffer->data = nueStreamData;

	switch (operacion->idOperacion)
	{
	case okKernel:
		break;
	case continuar:
		break;
	case terminar:
		break;
	case bloquear:
		break;
	case primerMsj:
		operacion->estructura = cpu_deserializar_primerMsj(buffer);
		break;
	case atendeProceso:
		operacion->estructura = deserializar_pcb(buffer);
		break;
	case valorCompartida:
		operacion->estructura = deserializar_valorVariable(buffer);
		break;
	}
	return operacion;
}


static t_SaludoCpu* cpu_deserializar_primerMsj(t_buffer* buffer)
{
	int desplazamiento = 0, size;
	t_SaludoCpu* saludo = malloc(sizeof(t_SaludoCpu));

	memcpy(&saludo->iEjecuciones, buffer->data, size = sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&saludo->iDelay, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;

	buffer->size = desplazamiento;
	return saludo;
}

static t_buffer* kernel_serializar_valorVarCompartida(t_valor_variable* valor)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(t_valor_variable);
	buffer->data = malloc(buffer->size);
	memcpy(buffer->data, valor, buffer->size);
	return buffer;
}

static t_variable_compartida* kernel_deserializar_asignarVarCompartida(t_buffer* buffer)
{
	t_variable_compartida* varComp = malloc(sizeof(t_variable_compartida));
	varComp->nombre = string_duplicate(buffer->data);
	memcpy(&varComp->valor , buffer->data + strlen(varComp->nombre) + 1,
			sizeof(t_valor_variable));
	return varComp;
}

static t_nombre_compartida kernel_deserializar_obtenerValorCompartida(
		t_buffer* buffer)
{
	return string_duplicate(buffer->data);
}

static t_valor_variable* deserializar_valorVariable(t_buffer* buffer)
{
	t_valor_variable* valorVar = malloc(sizeof(t_valor_variable));
	memcpy(valorVar, buffer->data, sizeof(t_valor_variable));
	return valorVar;
}

static char* deserializar_string(t_buffer* buffer)
{
	return string_duplicate(buffer->data);
}

static t_buffer* cpu_serializar_wait(t_MsjWait* stWait)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int desplazamiento = 0,
		size = 0;
	stWait->semaforo_long = strlen(stWait->semaforo) + 1;
	t_pcb* pcb = &stWait->pcb;
	buffer->data = malloc(sizeof(u_int32_t) + stWait->semaforo_long + sizeof(t_pcb));

	memcpy(buffer->data, &stWait->semaforo_long , size = sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, stWait->semaforo,
			size = stWait->semaforo_long);
	desplazamiento += size;

	memcpy(buffer->data + desplazamiento, &pcb->pid, size = sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->segmentoCodigo, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->segmentoStack, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->cursorStack, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->indiceCodigo, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->indiceEtiqueta, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->programCounter, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->tamanioContexto, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->tamanioEtiqueta, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->cantidad_de_funciones, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->cantidad_de_etiquetas, size =
			sizeof(u_int32_t));
	desplazamiento += size;

	buffer->size = desplazamiento;

	return buffer;
}

static t_MsjWait * kernel_deserializar_wait(t_buffer* buffer)
{

	t_MsjWait * MsjWait = malloc(sizeof(t_MsjWait));
	int corrimiento, size;
	corrimiento = size = 0;
	t_pcb* pcb = &MsjWait->pcb;

	memcpy(&MsjWait->semaforo_long, buffer->data, size = sizeof(u_int32_t));
	corrimiento += size;

	MsjWait->semaforo = malloc(MsjWait->semaforo_long);

	memcpy(MsjWait->semaforo, buffer->data + corrimiento,
			size = MsjWait->semaforo_long);
	corrimiento += size;

	memcpy(&pcb->pid, buffer->data  + corrimiento, size = sizeof(u_int32_t));
	corrimiento += size;
	memcpy(&pcb->segmentoCodigo, buffer->data + corrimiento, size =
			sizeof(t_puntero));
	corrimiento += size;
	memcpy(&pcb->segmentoStack, buffer->data + corrimiento, size =
			sizeof(t_puntero));
	corrimiento += size;
	memcpy(&pcb->cursorStack, buffer->data + corrimiento, size =
			sizeof(t_puntero));
	corrimiento += size;
	memcpy(&pcb->indiceCodigo, buffer->data + corrimiento, size =
			sizeof(t_puntero));
	corrimiento += size;
	memcpy(&pcb->indiceEtiqueta, buffer->data + corrimiento, size =
			sizeof(t_puntero));
	corrimiento += size;
	memcpy(&pcb->programCounter, buffer->data + corrimiento, size =
			sizeof(u_int32_t));
	corrimiento += size;
	memcpy(&pcb->tamanioContexto, buffer->data + corrimiento, size =
			sizeof(u_int32_t));
	corrimiento += size;
	memcpy(&pcb->tamanioEtiqueta, buffer->data + corrimiento, size =
			sizeof(u_int32_t));
	corrimiento += size;
	memcpy(&pcb->cantidad_de_funciones, buffer->data + corrimiento, size =
			sizeof(u_int32_t));
	corrimiento += size;
	memcpy(&pcb->cantidad_de_etiquetas, buffer->data + corrimiento, size =
			sizeof(u_int32_t));
	corrimiento += size;

	return MsjWait;
}

static t_entradaSalida* kernel_deserializar_entradaSalida(t_buffer* buffer)
{
	t_entradaSalida* stDisp = malloc(sizeof(t_entradaSalida));
	int desplazamiento = 0,
		size = 0;
	t_pcb* pcb = &stDisp->pcb;

	memcpy(&stDisp->dispositivo_long, buffer->data, size = sizeof(u_int32_t));
	desplazamiento += size;
	stDisp->dispositivo = malloc(stDisp->dispositivo_long);

	memcpy(stDisp->dispositivo, buffer->data + desplazamiento, size = stDisp->dispositivo_long);
	desplazamiento += size;
	memcpy(&stDisp->tiempo, buffer->data + desplazamiento, size = sizeof(u_int32_t));
	desplazamiento += size;

	memcpy(&pcb->pid, buffer->data + desplazamiento, size = sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->segmentoCodigo, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->segmentoStack, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->cursorStack, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->indiceCodigo, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->indiceEtiqueta, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->programCounter, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->tamanioContexto, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->tamanioEtiqueta, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->cantidad_de_funciones, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->cantidad_de_etiquetas, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));

	return stDisp;
}

static char* kernel_deserializar_exception(t_buffer* buffer)
{
	return deserializar_string(buffer);
}

static t_buffer* kernel_serializar_primerMsj(t_SaludoCpu* saludo)
{
	int desplazamiento = 0, size;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->data = malloc(sizeof(t_SaludoCpu));

	memcpy(buffer->data, &saludo->iEjecuciones, size = sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &saludo->iDelay, size =
			sizeof(u_int32_t));
	desplazamiento += size;

	buffer->size = desplazamiento;
	return buffer;
}

static t_buffer* cpu_serializar_entradaSalida(t_entradaSalida* stDisp)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int desplazamiento = 0,
		size = 0;
	stDisp->dispositivo_long = strlen(stDisp->dispositivo) + 1;
	t_pcb* pcb = &stDisp->pcb;

	buffer->data = malloc((sizeof(u_int32_t) + stDisp->dispositivo_long + sizeof(u_int32_t) + sizeof(t_pcb)));

	memcpy(buffer->data, &stDisp->dispositivo_long, size = sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, stDisp->dispositivo, size = stDisp->dispositivo_long);
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &stDisp->tiempo, size = sizeof(u_int32_t));
	desplazamiento += size;

	memcpy(buffer->data + desplazamiento, &pcb->pid, size = sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->segmentoCodigo, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->segmentoStack, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->cursorStack, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->indiceCodigo, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->indiceEtiqueta, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->programCounter, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->tamanioContexto, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->tamanioEtiqueta, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->cantidad_de_funciones, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->cantidad_de_etiquetas, size =
			sizeof(u_int32_t));
	desplazamiento += size;

	buffer->size = desplazamiento;
	return buffer;
}

static t_buffer* cpu_serializar_asignarVariableCompartida(t_variable_compartida* varComp)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = strlen(varComp->nombre) + 1 + sizeof(t_valor_variable);
	buffer->data = malloc(buffer->size);
	memcpy(buffer->data, varComp->nombre, strlen(varComp->nombre) + 1);
	memcpy(buffer->data + strlen(varComp->nombre) + 1, &varComp->valor,
			sizeof(t_valor_variable));
	return buffer;
}

static t_buffer* serializar_pcb(t_pcb* pcb)
{
	int desplazamiento = 0, size;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->data = malloc(sizeof(t_pcb));

	memcpy(buffer->data, &pcb->pid, size = sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->segmentoCodigo, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->segmentoStack, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->cursorStack, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->indiceCodigo, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->indiceEtiqueta, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->programCounter, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->tamanioContexto, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->tamanioEtiqueta, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->cantidad_de_funciones, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(buffer->data + desplazamiento, &pcb->cantidad_de_etiquetas, size =
			sizeof(u_int32_t));
	desplazamiento += size;

	buffer->size = desplazamiento;
	return buffer;
}

static t_pcb* deserializar_pcb(t_buffer* buffer)
{
	int desplazamiento = 0, size;
	t_pcb * pcb = malloc(sizeof(t_pcb));

	memcpy(&pcb->pid, buffer->data, size = sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->segmentoCodigo, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->segmentoStack, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->cursorStack, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->indiceCodigo, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->indiceEtiqueta, buffer->data + desplazamiento, size =
			sizeof(t_puntero));
	desplazamiento += size;
	memcpy(&pcb->programCounter, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->tamanioContexto, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->tamanioEtiqueta, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->cantidad_de_funciones, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;
	memcpy(&pcb->cantidad_de_etiquetas, buffer->data + desplazamiento, size =
			sizeof(u_int32_t));
	desplazamiento += size;

	buffer->size = desplazamiento;

	return pcb;
}

static t_buffer* cpu_serializar_exception(char* textoError)
{
	t_buffer* buffer = serializar_string(textoError);
	return buffer;
}

static t_buffer* buffer_new()
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = 0;
	buffer->data = malloc(0);
	return buffer;
}

static t_buffer* kernel_serializar_rta_continuar()
{
	t_buffer* buffer = buffer_new();
	return buffer;
}

static t_buffer* kernel_serializar_rta_terminar()
{
	t_buffer* buffer = buffer_new();
	return buffer;
}

static t_buffer* serializar_rta_ok()
{
	t_buffer* buffer = buffer_new();
	return buffer;
}

static t_buffer* kernel_serializar_rta_bloquear()
{
	t_buffer* buffer = buffer_new();
	return buffer;
}

static void serializar_operacion(u_int32_t operacion, t_buffer* buffer)
{
	char* nueBufferData = malloc(buffer->size + sizeof(int));
	memcpy(nueBufferData, &operacion, sizeof(int));
	memcpy(nueBufferData + sizeof(int), buffer->data, buffer->size);
	free(buffer->data);
	buffer->data = nueBufferData;
	buffer->size += sizeof(int);
}

static t_buffer* cpu_serializar_obtenerValorCompartida(
		t_nombre_compartida varComp)
{
	t_buffer* buffer = serializar_string(varComp);
	return buffer;
}


static t_buffer* cpu_serializar_signal(t_nombre_semaforo varComp)
{
	t_buffer* buffer = serializar_string(varComp);
	return buffer;
}

static t_nombre_semaforo kernel_deserializar_signal(t_buffer* buffer)
{
	return deserializar_string(buffer);
}

static t_buffer* cpu_serializar_valorVariable(t_valor_variable* valor)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(t_valor_variable);
	buffer->data = malloc(buffer->size);
	memcpy(buffer->data, valor, buffer->size);
	return buffer;
}

static t_buffer* serializar_string(char* string)
{
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = strlen(string) + 1;
	buffer->data = string_duplicate(string);
	return buffer;
}

void cpu_free_mensaje(t_mensaje* mensaje)
{
	switch(mensaje->idOperacion)
	{
	case primerMsj ... valorCompartida:
		free(mensaje->estructura);
		free(mensaje);
		break;
	default:
		free(mensaje);
		break;
	}
}
