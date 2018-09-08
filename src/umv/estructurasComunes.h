#ifndef ESTRUCTURASCOMUNES_H_
#define ESTRUCTURASCOMUNES_H_

typedef int t_idPrograma;
typedef u_int32_t t_puntero;

typedef struct
{
	u_int32_t tipoConexion;
	t_idPrograma idPrograma;
}t_conexion;

typedef struct
{
	char* data;
	u_int32_t size;
} t_buffer;

typedef struct t_operacion
{
	u_int32_t idOperacion;
	void* estructura;
}t_mensaje;


enum{ kernel, cpu };
enum{ handshake, solicitar, almacenar, cambioProcesoActivo, crearSegmento, destruirSegmentos };
enum{ OK, segmentoCreado, bytesSolicitados, segmentation_fault, memory_overload };

#endif /* ESTRUCTURASCOMUNES_H_ */
