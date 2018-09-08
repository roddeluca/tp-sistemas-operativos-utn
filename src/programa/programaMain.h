/*
 * programaMain.h
 *
 *  Created on: 21/04/2014
 *      Author: utnso
 */

#ifndef PROGRAMAMAIN_H_
#define PROGRAMAMAIN_H_

/* definicion parametros de conexión */
#define direccionIP "127.0.0.1"
#define puerto 10000


typedef struct stParametros{
	char* ipKernel;
	int puertoKernel;
}stParametros;

typedef enum {OK=0,ErrArchi=2,ERROR=1} CODIGOS;

static const char pathFile[] = "/usr/bin/ansisop/programa.config";


char * cargar_archivo_en_memoria(FILE *file);


#endif /* PROGRAMAMAIN_H_ */
