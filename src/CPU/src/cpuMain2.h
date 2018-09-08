/*
 * cpuMain2.h
 *
 *  Created on: 12/06/2014
 *      Author: utnso
 */


#ifndef CPUMAIN2_H_
#define CPUMAIN2_H_


#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/log.h>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <serializar.h>
#include <protocolo.h>

enum {
	 okPCB = 0,
	 cortarCpu = 1,
};

typedef struct stParametrosCpu {
	char szIpKernel[16];
	int iPuertoKernel;
	char szIP_UMV[16];
	int iPuertoUMV;
} stParametrosCPU;

typedef enum {
	OKConfigCPU = 0,
	ErrArchiCPU = 2,
	ERRORCPU = 1
} CODIGOSCPU;

t_pcb pcb;
t_dictionary *diccionarioVariables;
char *bloqueEtiquetas;
uint32_t tamanioContexto;
t_puntero inicioContexto;

uint32_t socketUmv;
uint32_t socketKernel;
uint32_t quantumDeEjecucion;
uint32_t retardoDeEjecucion;

int estadoRafaga;

stParametrosCPU *parametrosCpu;

int lvl;
t_log *logCpu;
char bufferLog[200];

void actualizarContexto ();
void recibirPrimerMensajeKernel ();
void ejecutarRafaga ();
void enviarPcbAKernel ();
char* traerProximaInstruccion ();
char* proximaLineaDeCodigo (char* instruccion);
//void notificarPcpFinPrograma ();
//void notificarPcpFinQuantum ();
void recibirPcb ();
void configuracion_de_archivo(stParametrosCPU ** parametros, char* archivo);
void generarBloqueEtiquetas();
bool puedeSeguirEjecutando (int i);
void excepcionSolicitud ();

CODIGOSCPU cpu_cargar_estructura(char * szBuffer, stParametrosCPU ** estructura);
CODIGOSCPU cpu_levantar_configuracion (FILE **fdConfig, stParametrosCPU ** parametros);


#endif /* CPUMAIN2_H_ */

