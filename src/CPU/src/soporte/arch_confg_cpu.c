/*
 * arch_confg_cpu.c
 *
 *  Created on: 24/06/2014
 *      Author: utnso
 */
/*
 * archConfigCpu.c
 *
 *  Created on: 24/06/2014
 *      Author: utnso
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cpuMain2.h"

#define rowLenght 200


CODIGOSCPU cpu_cargar_estructura(char * szBuffer, stParametrosCPU ** estructura) {
	char * pch;
	CODIGOSCPU eReturn = OKConfigCPU;

	pch = strtok(szBuffer, " =[],\n");
	if (pch != NULL )
	{
		if (strcmp(pch, "IP_KERNEL") == OKConfigCPU)
		{
			pch = strtok(NULL, " =[],\n");
			strcpy((*estructura)->szIpKernel, pch);
		}

		else if (strcmp(pch, "PUERTO_KERNEL") == OKConfigCPU)
		{
			pch = strtok(NULL, " =[],\n");
			(*estructura)->iPuertoKernel = atoi(pch);
		}
		else if (strcmp(pch, "IP_UMV") == OKConfigCPU)
		{
			pch = strtok(NULL, " =[],\n");
			strcpy((*estructura)->szIP_UMV, pch);
		}
		else if (strcmp(pch, "PUERTO_UMV") == OKConfigCPU)
		{
			pch = strtok(NULL, " =[],\n");
			(*estructura)->iPuertoUMV = atoi(pch);
		}
		else
		{
			eReturn = ERRORCPU;
		}

	}
	return eReturn;
}

CODIGOSCPU cpu_levantar_configuracion(FILE **fdConfig, stParametrosCPU ** parametros) {
	CODIGOSCPU eReturn = OKConfigCPU;
		char szBuffer[rowLenght];

		if (fdConfig == NULL )
		{
			printf("Error al abrir el archivo\n");
			eReturn = ErrArchiCPU;
		}

		if (eReturn == OKConfigCPU)
			while (fgets(szBuffer, rowLenght, *fdConfig) != NULL && eReturn == OKConfigCPU)
				eReturn = cpu_cargar_estructura(szBuffer, parametros);


		// * esto dejalo para probarlo.. y si despues no queres imprimirlo sacalo.
		// * pero te recomiento dejar la funcion abajo por si despues hay que modificarlo.

		return eReturn;
}




