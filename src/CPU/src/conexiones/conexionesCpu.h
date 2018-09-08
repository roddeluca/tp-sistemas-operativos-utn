/*
 * conexionesCpu.h
 *
 *  Created on: 26/06/2014
 *      Author: utnso
 */

#ifndef CONEXIONESCPU_H_
#define CONEXIONESCPU_H_

#include <serializar.h>

int conectarAOtroProceso (char *ip,uint32_t puerto);
void conectarAUmv (char* ipUmv, uint32_t puertoUmv);
void conectarAKernel(char* ipKernel, uint32_t puertoKernel);
t_buffer* socket_recv (int sockt);
t_buffer* enviaYRecibeAOtroProceso (t_buffer *buffer,uint32_t sockt);
void enviarAKernel (t_buffer *buffer);


#endif /* CONEXIONESCPU_H_ */
