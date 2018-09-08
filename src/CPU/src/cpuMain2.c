/*
 * cpuMain.c
 *
 *  Created on: 17/04/2014
 *      Author: utnso
 */
 
 
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <parser/parser.h>
#include <parser/sintax.h>
#include <parser/metadata_program.h>
#include <commons/log.h>
#include "sys/socket.h"
#include "stdbool.h"
#include <protocolo.h>
#include "cpuMain2.h"
#include "conexiones/conexionesCpu.h"
#include "funcionesAnsisop/funcionesAnsisop.h"
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <commons/log.h>

static void signal_handler(int signo);

int main(int argc, char **argv){

	

	if (argc < 3) {
		printf("Faltan parametros de entrada\n");

		return -1;
	} else {
		parametrosCpu = malloc(sizeof(stParametrosCPU));
		
		configuracion_de_archivo(&parametrosCpu,argv[1]);
		lvl = atoi(argv[2]);
	}
	
	logCpu = log_create("CpuMain.log","CPU",false,lvl);

	log_info(logCpu,"Inicio CPU");

	conectarAUmv(parametrosCpu -> szIP_UMV, parametrosCpu -> iPuertoUMV);
	conectarAKernel(parametrosCpu -> szIpKernel,parametrosCpu -> iPuertoKernel);

	bloqueEtiquetas = malloc(0);

	recibirPrimerMensajeKernel();

	recibirPcb();

	while (1) {

		inicioContexto = pcb.cursorStack - pcb.tamanioContexto*5;

		actualizarContexto();

		ejecutarRafaga ();

		if(estadoRafaga != entradaSalidaKernel && estadoRafaga != waitPcbBloqueado && estadoRafaga != exception){
			enviarPcbAKernel(estadoRafaga);
		}

		recibirPcb();
	}

	return EXIT_SUCCESS;

}

void recibirPrimerMensajeKernel () {
	t_buffer* buffer =  socket_recv(socketKernel);
	t_mensaje* mensaje = deserializar_cpu(buffer);

	destroy_buffer(buffer);

	if (mensaje -> idOperacion == primerMsj) {
		diccionarioVariables = dictionary_create();

		t_SaludoCpu primerMensaje;

		primerMensaje = *(t_SaludoCpu *)mensaje->estructura;

		quantumDeEjecucion = primerMensaje.iEjecuciones;
		retardoDeEjecucion = primerMensaje.iDelay;

		sprintf(bufferLog,"Recibo quantum:%d y retardo:%d",quantumDeEjecucion,retardoDeEjecucion);
		log_info(logCpu,bufferLog);

		t_buffer *bufferOk = serializar_cpu(okCpu,buffer);

		enviarAKernel(bufferOk);

	} else {
		log_error(logCpu,"Fallo al recibir el primer mensaje");
	}

	cpu_free_mensaje(mensaje);
}

void recibirPcb () {
	t_buffer* buffer =  socket_recv(socketKernel);
	t_mensaje* mensaje = deserializar_cpu(buffer);

	destroy_buffer(buffer);

	pcb = *(t_pcb*)mensaje -> estructura;

	sprintf(bufferLog,"Recibo nuevo programa:%d",pcb.pid);
	log_info(logCpu,bufferLog);

	t_buffer *bufferUmv = umv_serializar(cambioProcesoActivo,&pcb.pid);

	int i = send(socketUmv,bufferUmv -> data, bufferUmv -> size,0);

	if(i > 0) {
		destroy_buffer(socket_recv(socketUmv));
	}

	generarBloqueEtiquetas();

	if(pcb.tamanioContexto > 0){
		inicioContexto = pcb.cursorStack - pcb.tamanioContexto*5;

		actualizarContexto();
	}
}

void ejecutarRafaga () {

	log_info(logCpu,"ejecutar Rafaga Cpu");

	int i;
	estadoRafaga = notificarPorFinRafaga;
	for (i = 0; puedeSeguirEjecutando (i); i++) {

		char *lineaCodigo = traerProximaInstruccion();
		if(lineaCodigo[strlen(lineaCodigo)-1] == '\n'){
			lineaCodigo[strlen(lineaCodigo)-1] = '\0';
		}

		sprintf(bufferLog,"linea de codigo:[%s]",lineaCodigo);
		log_info(logCpu,bufferLog);

		pcb.programCounter ++;
		analizadorLinea (lineaCodigo, &funciones, &funcionesKernel);

		usleep(retardoDeEjecucion * 1000);
	}

	signal(SIGUSR1, signal_handler);
}

bool puedeSeguirEjecutando (int i) {
	return ((i < quantumDeEjecucion) &&
			(estadoRafaga != notificarPorFinPrograma) &&
			(estadoRafaga != entradaSalidaKernel)&&
			(estadoRafaga != exception)&&
			(estadoRafaga != waitPcbBloqueado));
}

void enviarPcbAKernel (int operacionId) {

	log_info(logCpu,"enviar Pcb a Kernel");

	t_buffer *buffer = serializar_cpu(operacionId,&pcb);

	enviarAKernel(buffer);
}

char* traerProximaInstruccion () {

	log_info(logCpu,"obtener proxima linea de codigo del indice de codigo");

	t_solicitudBytes solicitarBytes;
	solicitarBytes.base = pcb.indiceCodigo;
	solicitarBytes.offset = 8 * pcb.programCounter;
	solicitarBytes.tamanio = 8;

	t_buffer *buffer = umv_serializar(solicitar,&solicitarBytes);
	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	if(mensajeDeserializado -> idOperacion == bytesSolicitados){
		t_buffer *buffer = mensajeDeserializado -> estructura;
		char * nuevaLinea = malloc(buffer->size);
		memcpy(nuevaLinea,buffer->data,buffer->size);

		umv_destroy_mensaje(mensajeDeserializado);

		return proximaLineaDeCodigo (nuevaLinea);
	} else {
		log_error(logCpu,"error al solicitar indice de codigo");

		excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);

		umv_destroy_mensaje(mensajeDeserializado);
	return NULL;
	}
}

char* proximaLineaDeCodigo (char* instruccion) {

	log_info(logCpu,"obtener proxima linea de codigo del codigo literal ");

	t_solicitudBytes solicitarBytes;
	solicitarBytes.base = pcb.segmentoCodigo;
	memcpy(&solicitarBytes.offset,instruccion,sizeof(int));
	memcpy(&solicitarBytes.tamanio,instruccion + sizeof(int),sizeof(int));

	sprintf(bufferLog,"linea codigo pc:%d offset:%d tamanio:%d",pcb.programCounter,solicitarBytes.offset,solicitarBytes.tamanio);
	log_info(logCpu,bufferLog);

	t_buffer *buffer = umv_serializar(solicitar,&solicitarBytes);
	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	if(mensajeDeserializado -> idOperacion == bytesSolicitados){
		t_buffer *buffer = mensajeDeserializado -> estructura;

		char * linea = malloc(buffer->size + 1);
		memcpy(linea,buffer->data,buffer->size);

		linea [solicitarBytes.tamanio] = '\0';

		umv_destroy_mensaje(mensajeDeserializado);

		return linea;
	} else {
		log_error(logCpu,"error al solicitar codigo literal");

		excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);

		umv_destroy_mensaje(mensajeDeserializado);

		return NULL;
	}
}

void configuracion_de_archivo(stParametrosCPU ** parametros, char* archivo) {

		FILE *fdConfiguracion=fopen(archivo,"r");
		if(fdConfiguracion==NULL) {
		}

		if(cpu_levantar_configuracion(&fdConfiguracion,parametros)!=0) {
		}

		fclose(fdConfiguracion);
}

void actualizarContexto (){
	log_info(logCpu,"actualizar contexto y variables");

	dictionary_clean(diccionarioVariables);

	int desplazamiento = pcb.cursorStack - inicioContexto;

	pcb.tamanioContexto = desplazamiento/5;

	t_solicitudBytes solicitarBytes;
	solicitarBytes.base = pcb.segmentoStack;
	solicitarBytes.offset = inicioContexto - pcb.segmentoStack;
	solicitarBytes.tamanio = pcb.tamanioContexto*5;

	t_buffer *buffer = umv_serializar(solicitar,&solicitarBytes);

	buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

	t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

	destroy_buffer(buffer);

	if(mensajeDeserializado->idOperacion == bytesSolicitados){
		int i;

			for(i = 0 ; i < solicitarBytes.tamanio ; i+=4){
				t_nombre_variable *nombreVariable = malloc(2);
				nombreVariable [0] = ((t_buffer*)mensajeDeserializado->estructura)->data [i];
				nombreVariable [1] = '\0';

				i ++;

				int *puntero = malloc(sizeof(t_puntero));

				*puntero = solicitarBytes.base + solicitarBytes.offset + i;

				dictionary_put(diccionarioVariables,nombreVariable,puntero);
			}

	} else {
		log_error(logCpu,"falla al actualizar el contexto");

		excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);
	}

	umv_destroy_mensaje(mensajeDeserializado);
}

void generarBloqueEtiquetas () {
	log_info(logCpu,"generar bloque de etiquetas");

	if(pcb.tamanioEtiqueta > 0){
		t_solicitudBytes solicitudBytes;
		solicitudBytes.base = pcb.indiceEtiqueta;
		solicitudBytes.offset = 0;
		solicitudBytes.tamanio = pcb.tamanioEtiqueta;

		t_buffer *buffer = umv_serializar(solicitar,&solicitudBytes);

		buffer = enviaYRecibeAOtroProceso(buffer,socketUmv);

		t_mensaje *mensajeDeserializado = umv_deserializar(buffer);

		destroy_buffer(buffer);

		if (mensajeDeserializado->idOperacion == bytesSolicitados) {
				char *punteroNuevaInstruccion = NULL;
				int offset = 0;

				bloqueEtiquetas = realloc(bloqueEtiquetas,pcb.tamanioEtiqueta);

				memcpy(bloqueEtiquetas,((t_buffer*) mensajeDeserializado->estructura)->data,pcb.tamanioEtiqueta);

				while (offset < pcb.tamanioEtiqueta) {
					punteroNuevaInstruccion = bloqueEtiquetas + offset;

					int pc;
					memcpy(&pc,
							punteroNuevaInstruccion + strlen(punteroNuevaInstruccion)
									+ sizeof(char), sizeof(t_puntero_instruccion));
					offset += strlen(punteroNuevaInstruccion) + sizeof(t_puntero_instruccion)
							+ 1;
				}

		} else {
			log_error(logCpu,"falla al generar bloque de etiquetas");

			excepcionSolicitud(((t_buffer*)mensajeDeserializado->estructura)->data);
		}

		umv_destroy_mensaje(mensajeDeserializado);
	}
}

static void signal_handler(int signo) {
	printf("signal:%d",signo);
    if(signo == SIGUSR1) {
    	printf( "Process  is passing SIGUSR1 to ...\n");

    	enviarPcbAKernel(notificarPorFinRafaga);
    	exit(0);
    }
    return;
}

void excepcionSolicitud (t_buffer *estructura) {

	log_info(logCpu,"notificar por excepcion de una solicitud");

	estadoRafaga = exception;

	t_buffer *buffer = serializar_cpu(exception,estructura);

	enviarAKernel(buffer);
}


