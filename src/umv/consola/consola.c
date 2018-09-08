/*
 * consola.c
 *
 *  Created on: 27/04/2014
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <commons/string.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

/*ahora nuestra libreria*/
#include "consola.h"
#include "../umvMain.h"
#include "../estructurasComunes.h"

static t_puntero strTo_t_puntero(char* string);
static bool validarVariableEsInt(char* unParam);
/* verifica que un string este formado solo por digitos*/
static bool bfnIsDigitString(char* string);

void* pfnConsola()
{
	char szConsola[iMAX_LEN];
	char** pszConsola;

	//puts("Bienvenido a la consola UMV");
	while (1)
	{
		printf("Ingrese una instruccion: ");
		fgets(szConsola, iMAX_LEN, stdin);
		pszConsola = pszfnGenerarArgumentos(szConsola);
		//valido que no se haya apretado enter sin escribir nada
		if (NULL != pszConsola[0])
		{
			/*
			int i;
			printf("El comando es: %s\n", szfnComando(pszConsola));
			//si el comando tene parametros, imprime cuales son
			if (ifnParametros(pszConsola))
			{
				puts("Los parametros son:");
				for (i = 1; pszConsola[i] != '\0'; i++)
				{
					printf("\t%s\n", szfnParametroNum(pszConsola, i));
				}
			}
			*/

			if ((string_equals_ignore_case(szfnComando(pszConsola), "retardo") && (ifnParametros(pszConsola) == 1)))
				fnConsolaRetardo(pszConsola);
			else if ((string_equals_ignore_case(szfnComando(pszConsola), "sb"))&& (ifnParametros(pszConsola) != 0))
				fnConsolaSolicBytes(pszConsola);
			else if (string_equals_ignore_case(szfnComando(pszConsola), "dump"))
				fnConsolaDump(pszConsola);
			else if ((string_equals_ignore_case(szfnComando(pszConsola), "compactar")) && (ifnParametros(pszConsola) == 0))
				fnConsolaCompactar(pszConsola);
			else if ((string_equals_ignore_case(szfnComando(pszConsola), "alg")) && (ifnParametros(pszConsola) == 1))
				fnConsolaAlgoritmo(pszConsola);
			else if ((string_equals_ignore_case(szfnComando(pszConsola), "eb")) && (ifnParametros(pszConsola) != 0))
				fnConsolaEnviarBytes(pszConsola);
			else if ((string_equals_ignore_case(szfnComando(pszConsola), "cs")) && (ifnParametros(pszConsola) == 2))
				fnConsolaCrearSegmento(pszConsola);
			else if ((string_equals_ignore_case(szfnComando(pszConsola), "ds")) && (ifnParametros(pszConsola) == 1))
				fnConsolaDestruirSegmentos(pszConsola);
			else
				puts("Error: comando no valido o cantidad de parametros incorrecta");
		}
	}
	pthread_exit(0);
}

char** pszfnGenerarArgumentos(char* szString)
{
	char** pszOriginal;

	szString[strlen(szString) - 1] = '\0';
	FnReempCaracterPorBlanco(szString, '\t');
	pszOriginal = string_split(szString, " ");

	return pszOriginal;
}

char *szfnComando(char** pszConsola)
{
	return pszConsola[0];
}

char *szfnParametroNum(char** pszConsola, int iNum)
{
	return pszConsola[iNum];
}

void fnConsolaRetardo(char **pszConsola)
{
	char* szParametro = szfnParametroNum(pszConsola, 1);
	//valido que el parametro sea numerico
	if (!bfnIsDigitString(szParametro))
	{
		puts("Error: el parametro tiene que ser numerico");
		return;
	}
	RETARDO = atoi(szParametro);
	//imprimo informacion util
	printf("El retardo se ha configurado en %s ms\n", szParametro);
	return;
}

void FnReempCaracterPorBlanco(char* szSting, char cCaracter)
{
	int i;
	for (i = 0; szSting[i] != '\0'; i++)
	{
		if (cCaracter == szSting[i])
			szSting[i] = ' ';
	}
}

int ifnParametros(char** pszConsola)
{
	int iParam;
	for (iParam = 0; pszConsola[iParam] != NULL ; iParam++)
		;
	iParam--;
	return iParam;
}

void fnConsolaDump(char **pszConsola)
{
	// comando secundario(def. como un param mas): structmp or infomp or contentmp.
	//validar cada param
		if (ifnParametros(pszConsola) > 0) // valido cant de param
		{
			puts("dump no recive parametros");
		}
		else
		{
			log_info(logConsolaUmv,"\n------------DUMP------------\n------------------------------------\n");
			pthread_mutex_lock(&mutex);
			fnMostrarEstructurasMP();
			fnMostrarEspaciosLibre();
			fnMostrarContenidoMP();
			pthread_mutex_unlock(&mutex);
			log_info(logConsolaUmv,"\n------------DUMP END------------\n------------------------------------\n");
		}
}

void fnConsolaCompactar(char **pszConsola)
{
	//valido la cantidad de parametros
	if (1 == ifnParametros(pszConsola))
	{
		puts("Error: este comando no admite parametros");
		return;
	}
	else
		fnCompactar();
	puts("Se ha compactado la memoria");
}

void fnConsolaAlgoritmo(char **pszConsola)
{
	//Se filtra por el algoritmo elegido
	if (string_equals_ignore_case(szfnParametroNum(pszConsola, 1), "FF"))
	{
		ALG_MEM = FF;
		puts(
				"Se ha cambiado el algoritmo de administracion de memoria a First-Fit");
	}
	else if (string_equals_ignore_case(szfnParametroNum(pszConsola, 1), "WF"))
	{
		ALG_MEM = WF;
		puts(
				"Se ha cambiado el algoritmo de administracion de memoria a Worst-Fit");
	}
	else
		puts("El algoritmo especificado no existe");
}

void fnConsolaEnviarBytes(char **pszConsola)
{
	if (validarVariableEsInt(szfnParametroNum(pszConsola, 1))
			&& validarVariableEsInt(szfnParametroNum(pszConsola, 3))
			&& validarVariableEsInt(szfnParametroNum(pszConsola, 4)))
	{
		t_idPrograma idProg;
		t_almacenarBytes stAlmacenar;
		t_buffer* buffer;
		t_mensaje * operacion;
		if (ifnParametros(pszConsola) == 5)
		{
			idProg = atoi((szfnParametroNum(pszConsola, 1)));
			stAlmacenar.base = strTo_t_puntero((szfnParametroNum(pszConsola, 2)));
			stAlmacenar.offset = atoi((szfnParametroNum(pszConsola, 3)));
			stAlmacenar.tamanio = atoi((szfnParametroNum(pszConsola, 4)));
			stAlmacenar.buffer = szfnParametroNum(pszConsola, 5);

			//Imprimo datos utiles en pantalla.
			printf("El proceso es: %d\n", idProg);
			printf("La direccion base(virtual) de memoria es: %p\n",
					(char*) stAlmacenar.base);
			printf("El offset es de %d bytes\n", stAlmacenar.offset);

			buffer = almacenarBytes(idProg, &stAlmacenar);
			operacion = consola_deserializar(buffer);

			printf("La direccion base(fisica) de memoria es: %p\n",
					(char*) stAlmacenar.base);

			switch (operacion->idOperacion)
			{
			case OK:
				stAlmacenar.buffer[stAlmacenar.tamanio] = '\0';
				printf("El buffer \"%s\" se ha escrito correctamente \n",
						stAlmacenar.buffer);
				break;
			case segmentation_fault:
				printf("%s \n", ((t_buffer*) operacion->estructura)->data);
				break;
			default:
				puts("Error: no se entiende la operacion");
			}
			//TODO: falta guardar en archivo
		}
		socket_bufferDestroy(buffer);
		consola_destroy_mensaje(operacion);
	}
}

void fnConsolaSolicBytes(char **pszConsola) //guardar en arch
{
	t_solicitudBytes solicitud;
	t_idPrograma idProg;
	t_buffer* buffer;
	t_mensaje * operacion;

	if ((ifnParametros(pszConsola) > 3) && (ifnParametros(pszConsola) < 6))
	{
		int flag = 1;
		if (validarVariableEsInt(szfnParametroNum(pszConsola, 1))
				&& validarVariableEsInt(szfnParametroNum(pszConsola, 2))
				&& validarVariableEsInt(szfnParametroNum(pszConsola, 3))
				&& validarVariableEsInt(szfnParametroNum(pszConsola, 4)))
		{
			if (ifnParametros(pszConsola) == 5)

			{
				flag = 0;
			}
			idProg = atoi((szfnParametroNum(pszConsola, 1)));
			solicitud.base = strTo_t_puntero((szfnParametroNum(pszConsola, 2)));
			solicitud.offset = atoi((szfnParametroNum(pszConsola, 3)));
			solicitud.tamanio = atoi((szfnParametroNum(pszConsola, 4)));
			char* imprimirBytes = malloc(solicitud.tamanio + 1);

			//Imprimo datos utiles en pantalla.
			printf("El proceso es: %d\n", idProg);
			printf("La direccion base(virtual) de memoria es: %p\n",
					(char*) solicitud.base);
			printf("El offset es de %d bytes\n", solicitud.offset);
			printf("El tamaño solicitado es de %d bytes\n", solicitud.tamanio);

			buffer = solicitarBytes(idProg, &solicitud);
			operacion = consola_deserializar(buffer);

			printf("La direccion base(fisica) de memoria es: %p\n",
					(char*) solicitud.base);

			switch (operacion->idOperacion)
			{
			case bytesSolicitados:
				memcpy(imprimirBytes, ((t_buffer*) operacion->estructura)->data,
						solicitud.tamanio);
				imprimirBytes[solicitud.tamanio] = '\0';
				printf("El buffer solicitado es: \"%s\" \n", imprimirBytes);
				free(imprimirBytes);
				break;
			case segmentation_fault:
				printf("%s \n", ((t_buffer*) operacion->estructura)->data);
				break;
			default:
				puts("Error: no se entiende la operacion");
			}
			if (flag == 0)
			{ //TODO: falta terminar aca
				FILE *fp;
				char nombreF[60] =
						"contenido posicion solicitada memoria principal.txt";
				fp = fopen(nombreF, "w");

				fclose(fp);
				puts(
						"Archivo guardado con el nombre \"contenido posicion solicitada memoria principal.txt\"");
			}
		}
	}
	else
	{
		puts("cantidad de parametros incorrectos");
	}
	socket_bufferDestroy(buffer);
	consola_destroy_mensaje(operacion);
}

void fnConsolaCrearSegmento(char **pszConsola) //validar params
{
	t_idPrograma pid;
	u_int32_t tamanio;
	t_buffer* buffer;
	t_mensaje * operacion;

	pid = atoi((szfnParametroNum(pszConsola, 1)));
	tamanio = atoi((szfnParametroNum(pszConsola, 2)));
	buffer = generarSegmento(pid, &tamanio);
	operacion = consola_deserializar(buffer);
	//Imprimo datos utiles en pantalla.
	printf("El proceso es: %d\n", pid);
	printf("El tamaño solicitado es de %d bytes\n",
			tamanio);

	switch (operacion->idOperacion)
	{
	case segmentoCreado:
		printf("El segmento se creo en la direccion: %p \n",
				(char*) *(t_puntero*) operacion->estructura);
		break;
	case memory_overload:
		printf("%s\n", ((t_buffer*) operacion->estructura)->data);
		break;
	default:
		puts("Error: no se entiende la operacion");
	}
	socket_bufferDestroy(buffer);
	consola_destroy_mensaje(operacion);
}

static bool validarVariableEsInt(char* unParam)
{
	if (bfnIsDigitString(unParam))
	{
		return true;
	}
	else
	{
		puts("tipo de parametro invalido");
		return false;
	}
}

void fnConsolaDestruirSegmentos(char **pszConsola)
{
	if (validarVariableEsInt(szfnParametroNum(pszConsola, 1)))
	{
		t_idPrograma idPrograma;
		idPrograma = atoi(szfnParametroNum(pszConsola, 1));
		socket_bufferDestroy(eliminarSegmentos(idPrograma)); //libero automaticamente la memoria que crea el buffer de retorno de eliminarSegmentos
		puts("Segmentos eliminados");
	}
	else
	{
		return;
	}
}

static bool bfnIsDigitString(char string[])
{
	int iCaracter;
	//valido que el parametro sea en digitos.
	for (iCaracter = 0; string[iCaracter] != '\0'; iCaracter++)
	{
		if (isdigit (string[iCaracter] ))
		{
			return true;
		}
	}
	return false;
}

void fnMostrarContenidoMP()
{
	tipoSegmento* nodo;
	int i;
	char* buffer = malloc(0);
	log_info(logConsolaUmv, "\n----------Mostrar contenido de memoria----------\n");
	for (i = 0; i < list_size(pSegmentos); i++)
	{
		nodo = list_get(pSegmentos, i);
		char* base = (char*)nodo->dirFisica;
		buffer = realloc(buffer, nodo->tamanio + 1);
		memcpy(buffer, base, nodo->tamanio);
		buffer[nodo->tamanio] = '\0';
		int offset =(nodo->dirFisica - (t_puntero)pInicioMemoria);

		log_info(logConsolaUmv,"Contenido del segmento numero %d\n", i);
		log_info(logConsolaUmv,buffer);

		sprintf(bufferLogUmv,"\t Offset: %d\n", offset);
		log_info(logConsolaUmv,bufferLogUmv);

		sprintf(bufferLogUmv,"\t Tamanio: %d\n", nodo->tamanio);
		log_info(logConsolaUmv,bufferLogUmv);
	}
	log_info(logConsolaUmv, "\n----------Fin contenido de memoria----------\n");
	free(buffer);
}

void fnMostrarEstructurasMP()
{
	tipoSegmento* nodo;
	int i;
	log_info(logConsolaUmv,"\n-------Mostrar segmentos------------\n");
	for (i = 0; i < list_size(pSegmentos); i++)
	{
		nodo = list_get(pSegmentos, i);

		sprintf(bufferLogUmv,"Segmento numero: %d\n", i);
		log_info(logConsolaUmv,bufferLogUmv);

		sprintf(bufferLogUmv,"\t Pid: %d", nodo->idPrograma);
		log_info(logConsolaUmv,bufferLogUmv);

		sprintf(bufferLogUmv,"\t Base: %p", (char*)nodo->dirFisica);
		log_info(logConsolaUmv,bufferLogUmv);

		sprintf(bufferLogUmv,"\t Tamanio: %d", nodo->tamanio);
		log_info(logConsolaUmv,bufferLogUmv);

		sprintf(bufferLogUmv,"\t Puntero logico: %p", (char*)nodo->dirLogica);
		log_info(logConsolaUmv,bufferLogUmv);


	}
	log_info(logConsolaUmv,"\n-------Fin mostrar segmentos------------\n");
}


void fnMostrarEspaciosLibre()
{
	int tamanioLista,
		tamanioNodo,
		i,
		posicion,
		cantidad,
		contadorLibres;
	contadorLibres = posicion = 0;
	char *dirFisicaNodo;
	char* pMoviendo = pInicioMemoria;
	char * base;
	tipoSegmento *nodo;
	tamanioLista = list_size(pSegmentos);

	log_info(logConsolaUmv,"\n-----------Comienzo descripcion espacios libres-----------\n");
	for (i = 0; (posicion != CANT_MEM) && (i < tamanioLista); i++)
	{
		cantidad = 0;

		nodo = list_get(pSegmentos, i);
		dirFisicaNodo = (char*) nodo->dirFisica;
		tamanioNodo = nodo->tamanio;

		while ((pMoviendo != dirFisicaNodo) && (posicion != CANT_MEM))
		{
			posicion++;
			cantidad++; // tamaño de segmento libre
			pMoviendo++;
		}
		if (cantidad > 0)
		{
			contadorLibres++;
			base = pMoviendo - cantidad;

			sprintf(bufferLogUmv,"Espacio libre numero: %d", contadorLibres);
			log_debug(logConsolaUmv,bufferLogUmv);

			sprintf(bufferLogUmv,"\t Base: %p", base);
			log_debug(logConsolaUmv,bufferLogUmv);

			sprintf(bufferLogUmv,"\t Tamanio: %d", cantidad);
			log_debug(logConsolaUmv,bufferLogUmv);
		}
		pMoviendo += tamanioNodo;
		posicion += tamanioNodo;
	}
	//Analizo el ultimo bloque de la memoria
	cantidad = CANT_MEM - posicion;
	contadorLibres++;
	if( cantidad > 0)
	{
		sprintf(bufferLogUmv,"Espacio libre numero: %d", contadorLibres);
		log_info(logConsolaUmv,bufferLogUmv);

		sprintf(bufferLogUmv,"\t Base: %p", base);
		log_info(logConsolaUmv,bufferLogUmv);

		sprintf(bufferLogUmv,"\t Tamanio: %d", cantidad);
		log_info(logConsolaUmv,bufferLogUmv);
	}
	log_info(logConsolaUmv,"\n-----------Fin descripcion espacios libres-----------\n");

}

static t_puntero strTo_t_puntero(char* string)
{
	t_puntero num;
	char* inicioNum = string_substring(string, 0, 2);

	if(string_equals_ignore_case(inicioNum , "0x"))
		sscanf(string, "%x", &num);
	else
		num = atoi(string);
	return num;
}


