/*
 * funcionesAnsisop.h
 *
 *  Created on: 26/06/2014
 *      Author: utnso
 */

#ifndef FUNCIONESANSISOP_H_
#define FUNCIONESANSISOP_H_

#include <string.h>
#include <stdio.h>
#include <parser/parser.h>
#include <parser/sintax.h>
#include <parser/metadata_program.h>

t_puntero definirVariable (t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable (t_nombre_variable identificador_variable);
t_valor_variable dereferenciar (t_puntero direccion_variable);
void asignar (t_puntero direccion_variable, t_valor_variable valor);
void irAlLabel (t_nombre_etiqueta t_nombre_etiqueta);
void llamarSinRetorno (t_nombre_etiqueta etiqueta);
void llamarConRetorno (t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void finalizar ();
void retornar (t_valor_variable retorno) ;
t_valor_variable obtenerValorCompartida (t_nombre_compartida variable);
t_valor_variable asignarValorCompartida (t_nombre_compartida variable, t_valor_variable valor);
void imprimir (t_valor_variable valor_mostrar);
void imprimirTexto (char* texto);
void entrarSalida (t_nombre_dispositivo dispositivo, int tiempo);
void waitAnsisop (t_nombre_semaforo identificador_semaforo);
void signalAnsisop (t_nombre_semaforo identificador_semaforo);

AnSISOP_funciones funciones = {
		.AnSISOP_definirVariable		= definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= dereferenciar,
		.AnSISOP_asignar				= asignar,
		.AnSISOP_irAlLabel              = irAlLabel,
		.AnSISOP_llamarSinRetorno       = llamarSinRetorno,
		.AnSISOP_llamarConRetorno       = llamarConRetorno,
		.AnSISOP_finalizar              = finalizar,
		.AnSISOP_retornar               = retornar,
		.AnSISOP_obtenerValorCompartida = obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida = asignarValorCompartida,
		.AnSISOP_imprimir				= imprimir,
		.AnSISOP_imprimirTexto			= imprimirTexto,
		.AnSISOP_entradaSalida          = entrarSalida,
};

AnSISOP_kernel funcionesKernel = {
	.AnSISOP_wait                   = waitAnsisop,
	.AnSISOP_signal                 = signalAnsisop,
};

#endif /* FUNCIONESANSISOP_H_ */
