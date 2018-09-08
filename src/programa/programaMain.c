/*
 * programaMain.c
 *
 *  Created on: 17/04/2014
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
/*ahora nuestra libreria*/
#include "programaMain.h"


#define rowLenght 200

CODIGOS efnCargarEstructura(char *,stParametros *);
void fnImprimirEstructura(stParametros estructura);


int main(int argc, char *argv[]) {

	CODIGOS eReturn=OK;
	char szBuffer[rowLenght];
	stParametros parametros;
	char* archivoAnsisop;
	char package[1024];


	if ( argc > 1 )
	{
		FILE *archiConf = fopen( pathFile, "r" );
		FILE *file = fopen( argv[1], "r" );

		if (file == 0 && archiConf == 0)
		{
			printf( "No se pudo abrir el archivo\n" );
		}
		else {
			archivoAnsisop = cargar_archivo_en_memoria(file);

			while(fgets(szBuffer,rowLenght,archiConf) != NULL && eReturn == OK)
			{
				eReturn = efnCargarEstructura(szBuffer,&parametros);
			}

		}
		fnImprimirEstructura(parametros);

		fclose(file);
		fclose(archiConf);
	}

	struct sockaddr_in dest;
	int mysocket = socket(AF_INET, SOCK_STREAM, 0);

	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(parametros.ipKernel);
	dest.sin_port = htons(parametros.puertoKernel);

	void * aEnviar = malloc(strlen(archivoAnsisop)+1);

	if (aEnviar!=NULL) {
		memcpy(aEnviar,archivoAnsisop,strlen(archivoAnsisop)+1);
		free(archivoAnsisop);
	}

	puts("Conectando...\n");

	int conexion = connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));

	while (conexion == -1 ){
		conexion = connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
	}



	if (conexion == 0) {

		send(mysocket,aEnviar,strlen(aEnviar)+1,0);

		free(aEnviar);
		puts("Esperando respuesta....\n");

		int recibido = recv(mysocket,(void*)package,sizeof(package),0);
		printf("%s\n",package);

		while (recibido > 0) {
			recibido = recv(mysocket,(void*)package,sizeof(package),0);
			if (recibido <= 0) {
				printf("Se cerro la conexion\n");

			} else {
				printf("%s\n",package);
			}
		}

	} else {
		puts("Fallo la conexion!");
	}

	return 0;
}

char* cargar_archivo_en_memoria (FILE *file) {

	char *contents;
	int fileSize = 0;

	fseek(file, 0L, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	contents = malloc(fileSize+1);

	size_t size = fread(contents,1,fileSize,file);
	contents[size] = 0; // Add terminating zero.

	return contents;
}

CODIGOS efnCargarEstructura(char * szBuffer,stParametros * estructura)
{
	char * pch;
	CODIGOS eReturn = OK;

	pch = strtok (szBuffer," =[],\n");
	if(pch != NULL)
	{

		if(strcmp(pch,"IP")==OK)
        {
            pch = strtok (NULL, " =,\n");
            estructura->ipKernel=malloc(strlen(pch)+1);
            strcpy(estructura->ipKernel,pch);

        }
		else if(strcmp(pch,"PUERTO")==OK)
        {
            pch = strtok (NULL, " =,\n");
            estructura->puertoKernel=atoi(pch);
        }

	}
	return eReturn;
}

void fnImprimirEstructura(stParametros estructura)
{
	printf("IP Kernel: [%s]\n",estructura.ipKernel);
	printf("--------------------------------\n");
	printf("PUERTOKernel: [%d]\n",estructura.puertoKernel);
	printf("--------------------------------\n");
}



