/*
 * PlanificadorPLP.h
 *
 *  Created on: 08/06/2014
 *      Author: utnso
 */

#ifndef PLANIFICADORPLP_H_
#define PLANIFICADORPLP_H_

#include <unistd.h>
#include <commons/collections/list.h>

#include "../kernelMain.h"

char bufferLogPLP[80];
t_log * loggerPLP;

void planificar_programas();
void rechazar_programa(t_prog* prog);
void aceptar_programa(t_prog* prog);
int calcular_peso(t_prog* prog);
int conectarUMV(t_prog* prog);
void guardar_programa(t_prog*prog);
bool ordenador (void *elemento1, void * elemento2);
void * terminarPrograma(void *);
void liberarPrograma(t_prog*prog);
void liberarNuevoPrograma(t_nuevoPrograma*prog);
void liberoParcial(t_prog* programa);
void ponerEnReadyProgramas();

#endif /* PLANIFICADORPLP_H_ */
