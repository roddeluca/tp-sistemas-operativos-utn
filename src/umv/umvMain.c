/*
 * umvMain.c
 *
 *  Created on: 17/04/2014
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include <arpa/inet.h> //inet_addr
#include <semaphore.h>
//#include <bits/semaphore.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <commons/log.h>

/*ahora nuestras librerias*/
#include "umvMain.h"
#include "consola/consola.h"
#include "sockets/sockets.h"
#include "serializado/serializado.h"

static t_puntero generarDireccionLogica(void);


int main(int argc, char *argv[])
{
	t_config* stConfig;
	if (argc < 3) {
		printf("Faltan parametros de entrada\n");
		return -1;
	}
	else{
		logLevel = atoi(argv[2]);
		stConfig = config_create(argv[1]);
	}
	t_log_level level = logLevel;
	logUmv = log_create("umvMain.log", "UMV", false, level);

	logConsolaUmv = log_create("dump.log", "UMV", false, level);


	CANT_MEM = config_get_int_value(stConfig, "CANT_MEM");

	sprintf(bufferLogUmv,"Cantidad de memoria asignada: %lu",CANT_MEM);
	log_info(logUmv,bufferLogUmv);

	RETARDO = config_get_int_value(stConfig, "RETARDO");

	sprintf(bufferLogUmv,"Retardo configurado: %d",RETARDO);
	log_info(logUmv,bufferLogUmv);

	if(string_equals_ignore_case(config_get_string_value(stConfig, "ALG_MEM"), "FF"))
	{
		ALG_MEM = FF;
		log_info(logUmv,"Algoritmo seleccionado: First-Fit");

	}else if(string_equals_ignore_case(config_get_string_value(stConfig, "ALG_MEM"), "wF"))
	{
		ALG_MEM = WF;
		log_info(logUmv,"Algoritmo seleccionado: Worst-Fit");

	}
	config_destroy(stConfig);

	pInicioMemoria = malloc(CANT_MEM);

	sprintf(bufferLogUmv,"El puntero de inicio a memoria es: %p", pInicioMemoria);
	log_info(logUmv,bufferLogUmv);

	log_info(logUmv,"--------------------------------------");

	pSegmentos = list_create();
	pthread_mutex_init(&mutex, NULL );

	pthread_t consolaId;	// Id del hilo consola
	pthread_attr_t consolaAttr;	// Atributos del hilo consola

	pthread_attr_init(&consolaAttr); // obtener los atributos predeterminados
	pthread_create(&consolaId, &consolaAttr, pfnConsola, NULL );

	//INICIO DEL CODIGO DEL MAIN

	if (EXIT_FAILURE == socket_escucharConexiones())
	{
		log_error(logUmv,"Error: se ha dejado de escuchar a las conexiones");

	}

	pthread_join(consolaId, NULL );

	free(pInicioMemoria);
	return EXIT_SUCCESS;
}
//FIN DEL CODIGO DEL MAIN

void fnCompactar()
{
	log_debug(logUmv,"Compactando la memoria");

	int iSeg, tamanioSegmento, iMoviendo;
	char *pMoviendo;
	char *pCompactado = pInicioMemoria;
	tipoSegmento * seg;

	pthread_mutex_lock(&mutex);
	for (iSeg = 0; iSeg != list_size(pSegmentos); iSeg++)
	{
		seg = list_get(pSegmentos, iSeg);

		tamanioSegmento = seg->tamanio;
		pMoviendo = (char*) seg->dirFisica;

		for (iMoviendo = 0; iMoviendo != tamanioSegmento; iMoviendo++)
		{
			*pCompactado = *pMoviendo;
			//incremento el puntero actual y el puntero a la nueva ubicacion
			pMoviendo++;
			pCompactado++;
		}
		seg->dirFisica = (t_puntero) (pCompactado - tamanioSegmento);
	}
	pthread_mutex_unlock(&mutex);
}

// devuelve direccion de memoria para un segmento, o NULL si no hay suficiente espacio - worst fit
char *buscarEspacioWF(u_int32_t tamanioNuevoSegmento)
{
	log_debug(logUmv,"Busca espacio de memoria libre por el metodo Worst-Fit");

	int mayorTamanio, tamanioLista, tamanioNodo, i, posicion, cantidad;
	mayorTamanio = posicion = 0;
	char *dirFisicaNodo;
	char *pMayorTamanio;
	char* pMoviendo = pInicioMemoria;
	tipoSegmento *nodo;
	tamanioLista = list_size(pSegmentos);

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
		if (cantidad > mayorTamanio)
		{
			mayorTamanio = cantidad; // tamaño de segmento libre mas grande
			pMayorTamanio = pMoviendo - cantidad;
		}
		pMoviendo += tamanioNodo;
		posicion += tamanioNodo;
	}
	/*
	 * Caso exepcional en que la cantidad de memoria entre el ultimo segmento y el fin de la memoria es > al mayor tamanio
	 * encontrado.
	 * Tambien vale para el caso en que no hayan segmentos en la lista de segmentos.
	 */
	if (CANT_MEM - posicion >= mayorTamanio)
	{
		mayorTamanio = CANT_MEM - posicion; // tamaño de segmento libre mas grande
		pMayorTamanio = pMoviendo;
	}
	if (mayorTamanio >= tamanioNuevoSegmento)
		return pMayorTamanio;

	return NULL ; //si no se encontro ningun espacio, se retorna NULL
}

//funcion por algoritmo first fit
char *buscarEspacioFF(u_int32_t tamanioNuevoSegmento)
{
	log_debug(logUmv,"Busca espacio de memoria libre por el metodo First-Fit");

	int i, tamanioNodo, posicion, cantidadLibre;
	posicion = 0;
	char *dirFisicaNodo;
	char *pMoviendo = pInicioMemoria;
	tipoSegmento *nodo;
	for (i = 0; (posicion != CANT_MEM) && (i < list_size(pSegmentos)); i++)
	{
		nodo = list_get(pSegmentos, i);
		dirFisicaNodo = (char*) nodo->dirFisica;
		tamanioNodo = nodo->tamanio;

		for (cantidadLibre = 0;
				(pMoviendo != dirFisicaNodo) && (posicion < CANT_MEM);
				cantidadLibre++)
		{
			posicion++;
			pMoviendo++;
		}

		if (cantidadLibre < tamanioNuevoSegmento)
		{
			pMoviendo += tamanioNodo;
			posicion += tamanioNodo;
		}
		else
		{
			pMoviendo -= cantidadLibre;
			return pMoviendo;
		}
	}
	/*
	 * si no encontro espacio entre los segmentos, se fija si el espacio entre el ultimo segmento
	 * y el fin de la memoria hay espacio. En caso de que la lista de segmentos este vacia, se fija
	 * si la cantidad de memoria disponible es mayor a la solicitada
	 */
	if (CANT_MEM - posicion >= tamanioNuevoSegmento)
		return pMoviendo;

	return NULL ; //si no se encontro ningun espacio, se retorna NULL
}

char *buscarEspacio(u_int32_t tamanioNuevoSegmento)

{
	log_debug(logUmv,"Buscando algoritmo de asignacion de memoria");
	if (ALG_MEM == WF)
		return buscarEspacioWF(tamanioNuevoSegmento);
	else if (ALG_MEM == FF)
		return buscarEspacioFF(tamanioNuevoSegmento);
	else
	{
		log_info(logUmv,"Algoritmo de asignacion de memoria no encontrado");

		return NULL ;
	}
}

t_buffer* solicitarBytes(t_idPrograma idProg, t_solicitudBytes* solicitud)
{
	log_info(logUmv,"Solicitando bytes");

	t_buffer* buffer;
	tipoSegmento* segmentoPedido;

	bool bfnSegmento(void* segmento)
	{
		tipoSegmento* unSegmento = (tipoSegmento*) segmento;
		return (unSegmento->idPrograma == idProg)
				&& (unSegmento->dirLogica == solicitud->base);
	}

	pthread_mutex_lock(&mutex);

	segmentoPedido = list_find(pSegmentos, bfnSegmento);

	if (segmentoPedido == NULL
			|| (solicitud->tamanio + solicitud->offset > segmentoPedido->tamanio))
	{

		log_error(logUmv,"Segmentation fault al solicitar bytes");

		buffer = serializar_respuesta(segmentation_fault, &buffer); //paso el segundo parametro para que no se genere error (no se usa)
		pthread_mutex_unlock(&mutex);
	}
	else
	{
		solicitud->base = segmentoPedido->dirFisica;
		buffer = serializar_respuesta(bytesSolicitados, solicitud);
		pthread_mutex_unlock(&mutex);
	}
	return buffer;
}

t_buffer* almacenarBytes(t_idPrograma idProg, t_almacenarBytes* stAlmacenar)
{
	log_info(logUmv,"Almacenando bytes");

	t_buffer* buffer;
	tipoSegmento* segmentoPedido;

	sprintf(bufferLogUmv,"el tamanio a almacenar es de: %d", stAlmacenar->tamanio);
	log_debug(logUmv,bufferLogUmv);

	sprintf(bufferLogUmv,"el puntero logico es: %p", (char *)stAlmacenar->base);
	log_debug(logUmv,bufferLogUmv);

	char imprimo[stAlmacenar->tamanio + 1];
	imprimo[stAlmacenar->tamanio] = '\0';

	sprintf(bufferLogUmv,"el buffer es %s", imprimo);
	log_debug(logUmv,bufferLogUmv);

	bool bfnSegmento(void* segmento)
	{
		tipoSegmento* unSegmento = (tipoSegmento*) segmento;
		return (unSegmento->idPrograma == idProg)
				&& (unSegmento->dirLogica == stAlmacenar->base);
	}
	pthread_mutex_lock(&mutex);
	segmentoPedido = list_find(pSegmentos, bfnSegmento);

	if (segmentoPedido == NULL
			|| (stAlmacenar->tamanio + stAlmacenar->offset
					> segmentoPedido->tamanio))
	{
		log_error(logUmv,"Segmentation fault al almacenar bytes");

		buffer = serializar_respuesta(segmentation_fault, &buffer); //paso el segundo parametro para que no se genere error (no se usa)
		pthread_mutex_unlock(&mutex);
	}
	else
	{
		memcpy((char*) segmentoPedido->dirFisica + stAlmacenar->offset,
				stAlmacenar->buffer, stAlmacenar->tamanio);
		pthread_mutex_unlock(&mutex);
		stAlmacenar->base = segmentoPedido->dirFisica;
		buffer = serializar_respuesta(OK, &buffer); //paso el segundo parametro para que no se genere error (no se usa)
	}
	return buffer;
}

t_buffer* generarSegmento(t_idPrograma pid, u_int32_t* tamanio)
{
	log_info(logUmv,"Generar segmento");

	t_buffer* buffer;
	int i;
	tipoSegmento *nodoLista;
	tipoSegmento *nodoAux;
	char *pFisica;
	pthread_mutex_lock(&mutex);

	/* si no hay lugar para crear el segmento, se compacta la memoria */
	if ((pFisica = buscarEspacio(*tamanio)) == NULL )
	{
		pthread_mutex_unlock(&mutex);
		fnCompactar();
		pthread_mutex_lock(&mutex);
		pFisica = buscarEspacio(*tamanio);
	}
	/* si despues de compactar la memoria, no hay lugar
	 * para crear el segmento, se notifica el error 'memory overload'
	 */
	if (pFisica == NULL )
	{
		log_error(logUmv,"Memory overload");

		buffer = serializar_respuesta(memory_overload, &i); // paso &i como parametro para que no falle, no la va a usar igual
		pthread_mutex_unlock(&mutex);
	}
	else if (tamanio == 0)//trato la creacion de un segmento de tamanio 0 como memory overload
	{

		log_error(logUmv,"Memory overload");

		buffer = serializar_respuesta(memory_overload, &i); // paso &i como parametro para que no falle, no la va a usar igual
		pthread_mutex_unlock(&mutex);
	}
	else
	{
		nodoAux = malloc(sizeof(tipoSegmento));
		nodoAux->idPrograma = pid;
		nodoAux->tamanio = *tamanio;
		nodoAux->dirLogica = generarDireccionLogica();
		nodoAux->dirFisica = (t_puntero) pFisica;
		for (i = 0; (i != list_size(pSegmentos)); i++)
		{
			nodoLista = list_get(pSegmentos, i);
			if ((nodoAux->dirFisica) < (nodoLista->dirFisica))
			{
				break;
			}
		}
		list_add_in_index(pSegmentos, i, nodoAux);
		pthread_mutex_unlock(&mutex);
		buffer = serializar_respuesta(segmentoCreado, &nodoAux->dirLogica);

		sprintf(bufferLogUmv,"Dir logica del nuevo segmento: %p",
				(char*) nodoAux->dirLogica);
		log_debug(logUmv,bufferLogUmv);

		sprintf(bufferLogUmv,"Dir logica casteada a int: %d \n", nodoAux->dirLogica);
		log_debug(logUmv,bufferLogUmv);

		sprintf(bufferLogUmv,"El tamanio del segmento es: %d \n", nodoAux->tamanio);
		log_debug(logUmv,bufferLogUmv);

	}
	return buffer;
}

t_buffer* eliminarSegmentos(t_idPrograma idPrograma)
{
	log_info(logUmv,"Eliminar segmentos");

	t_buffer* buffer;

	bool bfnSegmento(void* segmento)
	{
		tipoSegmento* unSegmento = (tipoSegmento*) segmento;
		return (unSegmento->idPrograma == idPrograma);
	}

	void fnDestruirSegmento(void* segmento)
	{
		tipoSegmento* unSegmento = (tipoSegmento*) segmento;
		free(unSegmento);
	}

	pthread_mutex_lock(&mutex);

	while (list_any_satisfy(pSegmentos, bfnSegmento))
	{
		list_remove_and_destroy_by_condition(pSegmentos, bfnSegmento,
				fnDestruirSegmento);
	}
	pthread_mutex_unlock(&mutex);
	buffer = serializar_respuesta(OK, &buffer); // paso la direccion del buffer solo para evitar un error
	return buffer;
}

static t_puntero generarDireccionLogica(void)
{
	log_info(logUmv,"Generar Direccion logica");

	t_puntero nueDir = random();

	bool bfnTieneLaDireccion(void* segmento)
	{
		tipoSegmento* unSegmento = (tipoSegmento*) segmento;
		return (unSegmento->dirLogica == nueDir);
	}

	while (list_any_satisfy(pSegmentos, bfnTieneLaDireccion))
	{
		nueDir = random();
	}

	return nueDir;
}
