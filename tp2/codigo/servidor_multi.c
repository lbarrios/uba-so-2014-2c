#include <signal.h>
#include <errno.h>

#include "biblioteca.h"

/* Estructura que almacena los datos de una reserva. */
typedef struct
{
  int cant_personas_pos[ANCHO_AULA][ALTO_AULA];
  int cantidad_de_personas;

  int rescatistas_disponibles;
  pthread_mutex_t* m_cant_personas_pos[ANCHO_AULA][ALTO_AULA];
} t_aula;

/* Estructura que almacena los parámetros del worker */
typedef struct Params_t
{
  int socket_fd;
  t_aula* el_aula;
} Params;

void t_aula_iniciar_vacia( t_aula* un_aula )
{
  int i, j;

  for ( i = 0; i < ANCHO_AULA; i++ )
  {
    for ( j = 0; j < ALTO_AULA; j++ )
    {
      un_aula->cant_personas_pos[i][j] = 0;
      un_aula->m_cant_personas_pos[i][j] = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
      pthread_mutex_init(un_aula->m_cant_personas_pos[i][j], NULL);  //TODO: chequar que poner en vez de NULL
    }
  }

  un_aula->cantidad_de_personas = 0;
  un_aula->rescatistas_disponibles = RESCATISTAS;
}

void t_aula_ingresar( t_aula* un_aula, t_persona* alumno )
{
  un_aula->cantidad_de_personas++;
  un_aula->cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]++;
}

void t_aula_liberar( t_aula* un_aula, t_persona* alumno )
{
  un_aula->cantidad_de_personas--;
  un_aula->cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]--;
}

static void terminar_servidor_de_alumno( int socket_fd, t_aula* aula, t_persona* alumno )
{
  printf( ">> Se interrumpió la comunicación con una consola.\n" );
  close( socket_fd );
  t_aula_liberar( aula, alumno );
  exit( -1 );
}


t_comando intentar_moverse( t_aula* el_aula, t_persona* alumno, t_direccion dir )
{
  int fila = alumno->posicion_fila;
  int columna = alumno->posicion_columna;
  alumno->salio = direccion_moverse_hacia( dir, &fila, &columna );
  ///char buf[STRING_MAXIMO];
  ///t_direccion_convertir_a_string(dir, buf);
  ///printf("%s intenta moverse hacia %s (%d, %d)... ", alumno->nombre, buf, fila, columna);
  bool entre_limites = ( fila >= 0 ) && ( columna >= 0 ) &&
                       ( fila < ALTO_AULA ) && ( columna < ANCHO_AULA );

  bool pudo_moverse = false;

  if(entre_limites || alumno->salio)
  {
    if(!alumno->salio)
    {
      pthread_mutex_lock(el_aula->m_cant_personas_pos[fila][columna]);
      int personas_en_casilla = el_aula->cant_personas_pos[fila][columna];
      if(personas_en_casilla < MAXIMO_POR_POSICION)
      {
        el_aula->cant_personas_pos[fila][columna]++;
        pthread_mutex_unlock(el_aula->m_cant_personas_pos[fila][columna]);
        pthread_mutex_lock(el_aula->m_cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]);  // chequar orden
        el_aula->cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]--;
        pthread_mutex_unlock(el_aula->m_cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]);
        alumno->posicion_fila = fila;
        alumno->posicion_columna = columna;
        pudo_moverse = true;
      }
      else
      {
        pthread_mutex_unlock(el_aula->m_cant_personas_pos[fila][columna]);
      }
    }
    else
    {
      pthread_mutex_lock(el_aula->m_cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]);  // chequar orden
      el_aula->cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]--;
      pthread_mutex_unlock(el_aula->m_cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]);
    }
  }
  return pudo_moverse;
}

void colocar_mascara( t_aula* el_aula, t_persona* alumno )
{
  printf( "Esperando rescatista. Ya casi %s!\n", alumno->nombre );
  alumno->tiene_mascara = true;
}


void* atendedor_de_alumno( int socket_fd, t_aula* el_aula )
{
  t_persona alumno;
  t_persona_inicializar( &alumno );

  if ( recibir_nombre_y_posicion( socket_fd, &alumno ) != 0 )
  {
    /* O la consola cortó la comunicación, o hubo un error. Cerramos todo. */
    terminar_servidor_de_alumno( socket_fd, NULL, NULL );
  }

  printf( "Atendiendo a %s en la posicion (%d, %d)\n",
          alumno.nombre, alumno.posicion_fila, alumno.posicion_columna );
  t_aula_ingresar( el_aula, &alumno );

  /// Loop de espera de pedido de movimiento.
  for ( ;; )
  {
    t_direccion direccion;

    /// Esperamos un pedido de movimiento.
    if ( recibir_direccion( socket_fd, &direccion ) != 0 )
    {
      /* O la consola cortó la comunicación, o hubo un error. Cerramos todo. */
      terminar_servidor_de_alumno( socket_fd, el_aula, &alumno );
    }

    /// Tratamos de movernos en nuestro modelo
    bool pudo_moverse = intentar_moverse( el_aula, &alumno, direccion );
    printf( "%s se movio a: (%d, %d)\n", alumno.nombre, alumno.posicion_fila, alumno.posicion_columna );
    /// Avisamos que ocurrio
    enviar_respuesta( socket_fd, pudo_moverse ? OK : OCUPADO );
    //printf("aca3\n");

    if ( alumno.salio )
    {
      break;
    }
  }

  colocar_mascara( el_aula, &alumno );
  t_aula_liberar( el_aula, &alumno );
  enviar_respuesta( socket_fd, LIBRE );
  printf( "Listo, %s es libre!\n", alumno.nombre );
  return NULL;
}

void wrap_atendedor_de_alumno( Params* params )
{
  //Params* params = ( Params* )params_ptr;
  atendedor_de_alumno( params->socket_fd, params->el_aula );
  free( params );
}

int main( void )
{
  //signal(SIGUSR1, signal_terminar);
  int socketfd_cliente, socket_servidor, socket_size;
  struct sockaddr_in local, remoto;

  /* Crear un socket de tipo INET con TCP (SOCK_STREAM). */
  if ( ( socket_servidor = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
  {
    perror( "creando socket" );
  }

  /* Crear nombre, usamos INADDR_ANY para indicar que cualquiera puede conectarse aquí. */
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = INADDR_ANY;
  local.sin_port = htons( PORT );

  if ( bind( socket_servidor, ( struct sockaddr* )&local, sizeof( local ) ) == -1 )
  {
    perror( "haciendo bind" );
  }

  /* Escuchar en el socket y permitir 5 conexiones en espera. */
  if ( listen( socket_servidor, 5 ) == -1 )
  {
    perror( "escuchando" );
  }

  t_aula el_aula;
  t_aula_iniciar_vacia( &el_aula );
  /// Aceptar conexiones entrantes.
  socket_size = sizeof( remoto );

  pthread_attr_t attr; //< Acá voy a guardar la configuración para crear el thread
  // Inicializo la configuración por defecto
  pthread_attr_init( &( attr ) );

  for ( ;; )
  {
    if ( -1 == ( socketfd_cliente =
                   accept( socket_servidor, ( struct sockaddr* ) &remoto, ( socklen_t* ) &socket_size ) ) )
    {
      printf( "!! Error al aceptar conexion\n" );
    }
    else
    {
      pthread_t tid; //< Acá voy a guardar el Thread-ID
      // Reservo memoria para el struct que contiene los parámetros que va a recibir el thread
      Params* params = ( Params* ) malloc( sizeof( Params ) );
      {
        params->socket_fd = socketfd_cliente; //< El thread recibe el descriptor de la conexión con el cliente
        params->el_aula = &el_aula; //< También recibe una referencia al aula
      }
      // Creo el thread
      pthread_create( &( tid ), //< Puntero a int donde se va a guardar el thread-id
                      &( attr ), //< Puntero a atributos
                      ( void (*))( void* ) wrap_atendedor_de_alumno, //< Puntero a función que va a correr el thread
                      ( void* ) params ); //< Puntero a un struct que contiene los parámetros
    }
  }

  // Elimino la configuración (el manual dice que es importante hacerlo)
  pthread_attr_destroy( &( attr ) );
  return 0;
}
