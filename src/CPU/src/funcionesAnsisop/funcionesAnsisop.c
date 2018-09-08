/*
 * funcionesAnsisop.c
 *
 *  Created on: 26/06/2014
 *      Author: utnso
 */
 
#include <string.h>
#include <stdio.h>
#include <parser/parser.h>
#include <parser/sintax.h>
#include <parser/metadata_program.h>
#include <commons/string.h>
#include <serializar.h>
#include "../cpuMain2.h"
#include "../conexiones/conexionesCpu.h"

t_puntero definirVariable (t_nombre_variable identificador_variable) {
	sprintf(bufferLog,"Definir variable:%c",identificador_variable);
	log_debug(logCpu,bufferLog);

	t_almacenarBytes almacenarBytes;
	almacenarBytes.base = pcb.segmentoStack;
	almacenarBytes.offset = pcb.cursorStack - pcb.segmentoStack;
	almacenarBytes.tamanio = 5;
	almacenarBytes.buffer = (void*)&identificador_variable;

	t_buffer *buffer = umv_serializar(almacenar ,&almacenarBytes);

	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	t_puntero *punteroVariable = malloc(sizeof(t_puntero));
	char *nombreVariable = malloc(2);
	nombreVariable [0] = identificador_variable;
	nombreVariable [1] = '\0';

	switch(mensajeDeserializado -> idOperacion){
		case OK:
			*punteroVariable = pcb.cursorStack + 1;

			dictionary_put (diccionarioVariables,nombreVariable , punteroVariable);

			pcb.tamanioContexto ++;

			pcb.cursorStack += 5;

			umv_destroy_mensaje(mensajeDeserializado);

			return *punteroVariable;

			break;
		default:
			log_error(logCpu,"error al solicitar espacio y definir una variable");

			excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);

			umv_destroy_mensaje(mensajeDeserializado);
			break;
		}

	return 0;
}

t_puntero obtenerPosicionVariable (t_nombre_variable identificador_variable) {
	sprintf(bufferLog,"Obtener variable:%c",identificador_variable);
	log_debug(logCpu,bufferLog);

	t_nombre_variable *nombreVariable = malloc(2);
	nombreVariable [0] = identificador_variable;
	nombreVariable [1] = '\0';

	t_puntero posVariable = *(t_puntero *)dictionary_get(diccionarioVariables,nombreVariable);

	return posVariable;
}

t_valor_variable dereferenciar (t_puntero direccion_variable) {
	sprintf(bufferLog,"Dereferenciar variable:%u",direccion_variable);
	log_debug(logCpu,bufferLog);

	t_solicitudBytes solicitarBytes;
	solicitarBytes.base = pcb.segmentoStack;
	solicitarBytes.offset = direccion_variable - pcb.segmentoStack;
	solicitarBytes.tamanio = 4;

	t_buffer *buffer = umv_serializar(solicitar,&solicitarBytes);

	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	if(mensajeDeserializado -> idOperacion == bytesSolicitados){

		t_valor_variable valor = *(int32_t *)(((t_buffer*)mensajeDeserializado->estructura)->data);

		umv_destroy_mensaje(mensajeDeserializado);

		return valor;
	} else {
		excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);

		umv_destroy_mensaje(mensajeDeserializado);
	}

	return 0;
}

void asignar (t_puntero direccion_variable, t_valor_variable valor) {
	sprintf(bufferLog,"asignar  variable:%u con valor:%d",direccion_variable,valor);
	log_debug(logCpu,bufferLog);

	t_almacenarBytes almacenarBytes;
	almacenarBytes.base = pcb.segmentoStack;
	almacenarBytes.offset = direccion_variable - pcb.segmentoStack;
	almacenarBytes.tamanio = 4;
	almacenarBytes.buffer = (char *)&valor;

	t_buffer *buffer = umv_serializar(almacenar ,&almacenarBytes);

	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	switch(mensajeDeserializado -> idOperacion){
		case OK:
			break;
		default:
			excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);
			break;
	}

	umv_destroy_mensaje(mensajeDeserializado);
}

void irAlLabel (t_nombre_etiqueta t_nombre_etiqueta) {
	sprintf(bufferLog,"Ir a la etiqueta:%s",t_nombre_etiqueta);
	log_debug(logCpu,bufferLog);

	pcb.programCounter = metadata_buscar_etiqueta(t_nombre_etiqueta,bloqueEtiquetas,pcb.tamanioEtiqueta);

}

void llamarSinRetorno (t_nombre_etiqueta etiqueta) {

	sprintf(bufferLog,"Llamar sin retornor a la etiqueta:%s",etiqueta);
	log_debug(logCpu,bufferLog);

	dictionary_clean(diccionarioVariables);

	char *nuevosPunteros = malloc(sizeof(t_puntero)*2);
	t_puntero iniContexto = pcb.cursorStack - 5*pcb.tamanioContexto;
	memcpy(nuevosPunteros,&iniContexto,sizeof(t_puntero));
	memcpy(nuevosPunteros + sizeof(t_puntero),&(pcb.programCounter),sizeof(t_puntero));

	pcb.tamanioContexto = 0;

	t_almacenarBytes almacenarBytes;
	almacenarBytes.base = pcb.segmentoStack;
	almacenarBytes.offset = pcb.cursorStack - pcb.segmentoStack;
	almacenarBytes.tamanio = sizeof(t_puntero)*2;
	almacenarBytes.buffer = nuevosPunteros;

	t_buffer *buffer = umv_serializar(almacenar,&almacenarBytes);

	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	if(mensajeDeserializado->idOperacion != OK){
		log_error(logCpu,"falla al almacenar valores para retornar");

		excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);
	} else {
		irAlLabel(etiqueta);

		pcb.cursorStack += 8;
	}

	umv_destroy_mensaje(mensajeDeserializado);

}

void llamarConRetorno (t_nombre_etiqueta etiqueta, t_puntero donde_retornar) {

	sprintf(bufferLog,"Llamar con retorno a la etiqueta:%s para retornar en:%d",etiqueta,donde_retornar);
	log_debug(logCpu,bufferLog);

	dictionary_clean(diccionarioVariables);

	t_puntero iniContexto = pcb.cursorStack - 5*pcb.tamanioContexto;
	char *nuevosPunteros = malloc(sizeof(t_puntero)*3);

	memcpy(nuevosPunteros,&iniContexto,sizeof(t_puntero));
	memcpy(nuevosPunteros + sizeof(t_puntero),&(pcb.programCounter),sizeof(t_puntero));
	memcpy(nuevosPunteros + sizeof(t_puntero)*2,&donde_retornar,sizeof(t_puntero));

	pcb.tamanioContexto = 0;

	t_almacenarBytes almacenarBytes;
	almacenarBytes.base = pcb.segmentoStack;
	almacenarBytes.offset = pcb.cursorStack - pcb.segmentoStack;
	almacenarBytes.tamanio = sizeof(t_puntero)*3;
	almacenarBytes.buffer = nuevosPunteros;

	t_buffer *buffer = umv_serializar(almacenar,&almacenarBytes);

	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	if(mensajeDeserializado->idOperacion != OK){
		log_error(logCpu,"Fallo en almacenar valores de retorno");

		excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);
	} else {
		irAlLabel(etiqueta);
		pcb.cursorStack += 12;
	}

	umv_destroy_mensaje(mensajeDeserializado);
}

void finalizar () {
	log_debug(logCpu,"Finalizar");

	if (pcb.cursorStack - (pcb.segmentoStack + (5*pcb.tamanioContexto)) == 0){
		estadoRafaga = notificarPorFinPrograma;
		return;
	}

	t_solicitudBytes solicitarBytes;
	solicitarBytes.base = pcb.segmentoStack;
	solicitarBytes.offset = pcb.cursorStack - (pcb.segmentoStack + (5*pcb.tamanioContexto) + 8);

	solicitarBytes.tamanio = 8;

	t_buffer *buffer = umv_serializar(solicitar,&solicitarBytes);

	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	if(mensajeDeserializado->idOperacion == bytesSolicitados){
		memcpy(&inicioContexto,((t_buffer*)mensajeDeserializado->estructura)->data,4);
		memcpy(&pcb.programCounter,((t_buffer*)mensajeDeserializado->estructura)->data + 4,4);

		sprintf(bufferLog,"Almacenar inicio de contexto:%d program counter:%d",inicioContexto,pcb.programCounter);
		log_debug(logCpu,bufferLog);

		pcb.cursorStack -= (5*pcb.tamanioContexto + 8);

		actualizarContexto ();

	} else {
		excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);
	}

	umv_destroy_mensaje(mensajeDeserializado);
}

void retornar (t_valor_variable retorno) {

	sprintf(bufferLog,"Retornar valor:%d",retorno);
	log_debug(logCpu,bufferLog);

	t_solicitudBytes solicitarBytes;
	solicitarBytes.base = pcb.segmentoStack;
	solicitarBytes.offset = pcb.cursorStack - (pcb.segmentoStack + 5*pcb.tamanioContexto + 12);
	solicitarBytes.tamanio = 12;

	t_buffer *buffer = umv_serializar(solicitar,&solicitarBytes);
	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	if(mensajeDeserializado->idOperacion == bytesSolicitados){

		t_puntero *donde_retornar = malloc(sizeof(t_puntero));

		memcpy(&inicioContexto,((t_buffer*)mensajeDeserializado->estructura)->data,4);
		memcpy(&pcb.programCounter,((t_buffer*)mensajeDeserializado->estructura)->data + 4 ,4);
		memcpy(donde_retornar,((t_buffer*)mensajeDeserializado->estructura)->data + 8 ,4);

		sprintf(bufferLog,"Almacenar inicio de contexto:%d program counter:%d posicion de retorno:%p",inicioContexto,pcb.programCounter,donde_retornar);
		log_debug(logCpu,bufferLog);

		pcb.cursorStack -= (5*pcb.tamanioContexto + 12);

		asignar(*donde_retornar,retorno);

		free(donde_retornar);

		actualizarContexto ();

		if(pcb.cursorStack == pcb.segmentoStack){
			estadoRafaga = notificarPorFinPrograma;
		}
	} else {
		log_error(logCpu,"Fallo al solicitar espacio para almacenar retorno");

		excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);
	}

	umv_destroy_mensaje(mensajeDeserializado);
}

t_valor_variable obtenerValorCompartida (t_nombre_compartida variable) {

	sprintf(bufferLog,"Obtener variable compartida:%s",variable);
	log_debug(logCpu,bufferLog);

	t_buffer *buffer = serializar_cpu(obtenerValorVarCompartida,variable);

	buffer = enviaYRecibeAOtroProceso(buffer,socketKernel);

	t_mensaje *mensaje = deserializar_cpu(buffer);

	destroy_buffer(buffer);

	int valorVariable;

	if(mensaje -> idOperacion == valorCompartida) {
		valorVariable = *(int*)(mensaje -> estructura);
	} else {
		log_error(logCpu,"Variable compartida no encontrada");
		estadoRafaga = notificarPorFinPrograma;
	}

	cpu_free_mensaje(mensaje);
	return valorVariable;
}

t_valor_variable asignarValorCompartida (t_nombre_compartida variable, t_valor_variable valor) {

	sprintf(bufferLog,"Obtener variable compartida:%s y su valor:%d",variable,valor);
	log_debug(logCpu,bufferLog);

	t_variable_compartida variableCompartida;
	variableCompartida.nombre = variable;
	variableCompartida.valor = valor;

	t_buffer *buffer = serializar_cpu(asignarValorVarCompartida,&variableCompartida);

	buffer = enviaYRecibeAOtroProceso(buffer,socketKernel);

	t_mensaje *mensaje = deserializar_cpu(buffer);

	destroy_buffer(buffer);

	if(mensaje -> idOperacion != continuar) {
		estadoRafaga = notificarPorFinPrograma;

		log_error(logCpu,"Variable compartida no encontrada");
	}

	cpu_free_mensaje(mensaje);

	return valor;
}

void imprimir (t_valor_variable valor_mostrar) {

	if(estadoRafaga == notificarPorFinPrograma){
		return;
	}

	sprintf(bufferLog,"valor a imprimir:%d",valor_mostrar);
	log_debug(logCpu,bufferLog);

	t_buffer *buffer = serializar_cpu(imprimirKernel,&valor_mostrar);

	buffer = enviaYRecibeAOtroProceso(buffer,socketKernel);

	t_mensaje *mensaje = deserializar_cpu(buffer);

	destroy_buffer(buffer);

	if(mensaje -> idOperacion == continuar) {
	} else {

		log_error(logCpu,"Fallo en la impresion");

		estadoRafaga = notificarPorFinPrograma;
	}

	cpu_free_mensaje(mensaje);
}

void imprimirTexto (char* texto) {
	sprintf(bufferLog,"%s",texto);
	log_debug(logCpu,bufferLog);

	t_buffer *buffer = serializar_cpu(imprimirTextoKernel,texto);

	buffer = enviaYRecibeAOtroProceso(buffer,socketKernel);

	t_mensaje *mensaje = deserializar_cpu(buffer);

	destroy_buffer(buffer);

	if(mensaje -> idOperacion == continuar) {
	} else {
		log_error(logCpu,"Fallo en la impresion");

		estadoRafaga = notificarPorFinPrograma;
	}

	cpu_free_mensaje(mensaje);
}

void entrarSalida (t_nombre_dispositivo dispositivo, int tiempo) {

	sprintf(bufferLog,"E/S del dispositivo:%s con tiempo:%d",dispositivo,tiempo);
	log_debug(logCpu,bufferLog);

	t_entradaSalida entradaSalida;
	entradaSalida.dispositivo = dispositivo;
	entradaSalida.tiempo = tiempo;
	entradaSalida.pcb = pcb;

	t_buffer *buffer = serializar_cpu(entradaSalidaKernel,&entradaSalida);

	estadoRafaga = entradaSalidaKernel;

	enviarAKernel(buffer);
}

void waitAnsisop (t_nombre_semaforo identificador_semaforo) {

	sprintf(bufferLog,"wait semaforo:%s",identificador_semaforo);
	log_debug(logCpu,bufferLog);

	t_MsjWait strWait;
	strWait.semaforo = identificador_semaforo;
	strWait.pcb = pcb;

	t_buffer *buffer = serializar_cpu(waitKernel,&strWait);

	buffer = enviaYRecibeAOtroProceso(buffer,socketKernel);

	t_mensaje *mensaje = deserializar_cpu(buffer);

	destroy_buffer(buffer);

	if(mensaje -> idOperacion != continuar) {
			estadoRafaga = waitPcbBloqueado;
	} else {
		log_debug(logCpu,"Se realizo el wait");
	}

	cpu_free_mensaje(mensaje);
}

void signalAnsisop (t_nombre_semaforo identificador_semaforo) {

	sprintf(bufferLog,"signal semaforo:%s",identificador_semaforo);
	log_debug(logCpu,bufferLog);

	t_buffer *buffer = serializar_cpu(signalKernel,identificador_semaforo);

	buffer = enviaYRecibeAOtroProceso(buffer,socketKernel);

	t_mensaje *mensaje = deserializar_cpu(buffer);

	destroy_buffer(buffer);

	if(mensaje -> idOperacion == terminar) {
		log_error(logCpu,"No se encontro el semaforo");

		estadoRafaga = notificarPorFinPrograma;
	} else {
		log_debug(logCpu,"Se  realizo el signal");
	}

	cpu_free_mensaje(mensaje);
}

