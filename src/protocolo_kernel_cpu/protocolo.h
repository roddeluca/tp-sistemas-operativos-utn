/*
 * protocolo.h
 *
 *  Created on: 20/06/2014
 *      Author: utnso
 */

#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

#include <serializar.h>
#include <parser/parser.h>

enum {
	obtenerValorVarCompartida = 0,
	asignarValorVarCompartida = 1,
	imprimirKernel = 2,
	imprimirTextoKernel = 3,
	entradaSalidaKernel = 4,
	waitKernel = 5,
	signalKernel = 6,
	notificarPorFinRafaga = 7,
	notificarPorFinPrograma = 8,
	exception = 9,
	okCpu = 10,
	waitPcbBloqueado = 11,
};

enum {
	continuar = 0,
	bloquear = 1,
	okKernel = 2,
	primerMsj = 3,
	atendeProceso = 4,
	valorCompartida = 5,
	terminar = 6,
};

typedef struct stSaludoCpu
{
	u_int32_t iEjecuciones;
	u_int32_t iDelay;
}t_SaludoCpu;


typedef struct stPCB{
		 u_int32_t pid;
		 t_puntero segmentoCodigo;
		 t_puntero segmentoStack;
		 t_puntero cursorStack;
		 t_puntero indiceCodigo;
		 t_puntero indiceEtiqueta;
		 u_int32_t programCounter;
		 u_int32_t tamanioContexto;
		 u_int32_t tamanioEtiqueta;
		 u_int32_t cantidad_de_funciones;
		 u_int32_t cantidad_de_etiquetas;
} t_pcb;

typedef struct stWait{
	u_int32_t semaforo_long;
	t_nombre_semaforo semaforo;
	t_pcb pcb;
}t_MsjWait;

typedef struct t_entrada_salida
{
	u_int32_t dispositivo_long;
	t_nombre_dispositivo dispositivo;
	u_int32_t  tiempo;
	t_pcb pcb;
}t_entradaSalida;

typedef u_int32_t t_puntero;

typedef struct {
	t_nombre_compartida nombre;
	t_valor_variable valor;
//Para obtenerValorKernel, asignarValorKernel,imprimirKernel,imprimirTextoKernel
} t_variable_compartida;

/*funciones que usa la cpu*/
t_buffer* serializar_cpu (u_int32_t operacion, void* estructura);
t_mensaje* deserializar_cpu(t_buffer* buffer);
void cpu_free_mensaje(t_mensaje* mensaje);


/*funciones que usa el kernel*/
t_buffer* serializar_kernel (u_int32_t operacion, void* estructura);
t_mensaje* deserializar_kernel(t_buffer* buffer);

#endif /* PROTOCOLO_H_ */
