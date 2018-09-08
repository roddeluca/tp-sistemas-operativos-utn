/*
 * umvMain.h
 *
 *  Created on: 21/04/2014
 *      Author: utnso
 */

#ifndef _UMVMAIN_H_
#define _UMVMAIN_H_

#include <commons/collections/list.h>
#include "sockets/sockets.h"
#include "estructurasComunes.h"
#include "serializado/serializado.h"
#include "commons/log.h"

#define rowLen 100
/* STRUCTS */

typedef struct struct_segmento{
		t_idPrograma idPrograma;
		u_int32_t tamanio;  //cantidad en bytes
		t_puntero dirLogica;
		t_puntero dirFisica;
} tipoSegmento;

enum { WF, FF };

/* VARIABLES */
char* pInicioMemoria; //donde inicia el gran malloc
t_list* pSegmentos; //lista en la que se guardara la tabla de segmentos
pthread_mutex_t mutex;
long unsigned int CANT_MEM; //longitud en bytes del gran malloc
u_int32_t RETARDO;
u_int32_t ALG_MEM;
t_log_level logLevel;
t_log *logUmv;
t_log *logConsolaUmv;
char bufferLogUmv[80];


/* FUNCIONES */

void fnCompactar();
char *buscarEspacio(u_int32_t tamanioNuevoSegmento);
void fnAlmacenarBytes(t_idPrograma idProg, char* base, u_int32_t tamanio, u_int32_t offset, char* buffer);
t_buffer* solicitarBytes(t_idPrograma idProg, t_solicitudBytes* solicitud);
t_buffer* almacenarBytes(t_idPrograma idProg, t_almacenarBytes* stAlmacenar);
t_buffer* generarSegmento(t_idPrograma pid, u_int32_t* tamanio);
int escucharConexiones();
void *pfnControladorConexion(void *socket_escucha);
t_buffer* eliminarSegmentos(t_idPrograma idPrograma);

char *buscarEspacioFF(u_int32_t tamanioNuevoSegmento);
char *buscarEspacioWF(u_int32_t tamanioNuevoSegmento);

//funciones para archivo de configuracion

void fnLevantarConfiguracion(void);



#endif /* _UMVMAIN_H_ */


