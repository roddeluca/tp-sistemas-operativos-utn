/*
 * consola.h
 *
 *  Created on: 27/04/2014
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_


/* lenght maximo (+1) que puede tener una instruccion de consola*/
#define iMAX_LEN 101

/* separa una instruccion de consola en un array de strings*/
char** pszfnGenerarArgumentos(char* szString);
/* obtiene el comando de un array de strings*/
char *szfnComando(char** pszConsola);
/* obtiene un parametro de un array de strings*/
char *szfnParametroNum(char** pszConsola, int iNum);
void FnReempCaracterPorBlanco(char* szSting, char cCaracter);
/* cuenta la cantidad de parametros que tiene una instruccion de consola*/
 int ifnParametros(char** pszConsola);


/* el hilo de la consola inicia en esta funcion */
void *pfnConsola();

/*
 * funciones que hacen validacions de una instruccion de consola.
 * llaman a su correspondiente tarea a realizar
 * */
void fnConsolaRetardo(char **pszConsola);
void fnConsolaSolicBytes(char **pszConsola);
void fnConsolaEnviarBytes(char **pszConsola);
void fnConsolaCrearSegmento(char **pszConsola);
void fnConsolaDestruirSegmentos(char **pszConsola);
void fnConsolaAlgoritmo(char **pszConsola);
void fnConsolaCompactar(char **pszConsola);
void fnConsolaDump(char **pszConsola);

void fnMostrarContenidoMP();
void fnMostrarEspaciosLibre();
void fnMostrarEstructurasMP();

#endif /* CONSOLA_H_ */
