#include <signal.h>
#include <errno.h>

#include "biblioteca.h"

#include <unistd.h>

#define MIN_ALUMNOS_LIBRES 5
#ifdef MAXIMO_POR_POSICION
  #undef MAXIMO_POR_POSICION
#endif
#define MAXIMO_POR_POSICION MIN_ALUMNOS_LIBRES

/* Estructura que almacena los datos de una reserva. */
typedef struct
{
  int cant_personas_pos[ANCHO_AULA][ALTO_AULA];
  int cantidad_de_personas;
  pthread_mutex_t* cantidad_de_personas_mutex;

  int cantidad_de_alumnos_libres;
  int cantidad_de_alumnos_saliendo;
  pthread_mutex_t* cantidad_de_alumnos_saliendo_mutex;

  int rescatistas_disponibles;
  pthread_mutex_t* m_cant_personas_pos[ANCHO_AULA][ALTO_AULA];
} t_aula;

/* Estructura que almacena los parámetros del worker */
typedef struct Params_t
{
  int socket_fd;
  t_aula* el_aula;
} Params;

pthread_mutex_t hay_alumnos_libres_mutex;
pthread_cond_t hay_alumnos_libres;

pthread_mutex_t hay_rescatistas_libres_mutex;
pthread_cond_t hay_rescatistas_libres;

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
  un_aula->cantidad_de_alumnos_libres = 0;
  un_aula->cantidad_de_alumnos_saliendo = 0;
  un_aula->rescatistas_disponibles = RESCATISTAS;
}

void t_aula_ingresar( t_aula* un_aula, t_persona_copada* alumno )
{
  pthread_mutex_lock(un_aula->cantidad_de_personas_mutex);
  un_aula->cantidad_de_personas++;
  un_aula->cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]++;
  pthread_mutex_unlock(un_aula->cantidad_de_personas_mutex);
}

void t_aula_liberar( t_aula* un_aula, t_persona_copada* alumno )
{
  /* Pido mutex y resto 1 a la cantidad de personas */
  printf("Liberando al alumno...\n");
  pthread_mutex_lock(un_aula->cantidad_de_personas_mutex);
  un_aula->cantidad_de_personas--;
  pthread_mutex_unlock(un_aula->cantidad_de_personas_mutex);
  printf("Quedan %d personas en el aula\n",un_aula->cantidad_de_personas);
  /* Libero el mutex para la cantidad de personas */

  /* Pido mutex para la cantidad de alumnos libres y la aumento en 1 */
  pthread_mutex_lock( &hay_alumnos_libres_mutex );
  un_aula->cantidad_de_alumnos_libres++;
  pthread_mutex_unlock( &hay_alumnos_libres_mutex );
  /* Libero el mutex para la cantidad de alumnos libres */

  // Hago signal sobre la variable de condición de alumnos libres
  pthread_cond_signal( &hay_alumnos_libres );
  printf("Hay %d alumnos libres.\n",un_aula->cantidad_de_alumnos_libres);

  // Pido mutex para la cantidad de alumnos libres
  pthread_mutex_lock( &hay_alumnos_libres_mutex );
  while ( un_aula->cantidad_de_alumnos_libres < MIN_ALUMNOS_LIBRES 
    && un_aula->cantidad_de_alumnos_saliendo <= 0
    && un_aula->cantidad_de_personas != 0
  ) {
    /**
     * Espero mientras que no se cumplan ninguna de las siguientes condiciones:
     *   La cantidad de alumnos libres es al menos la mínima requerida para que puedan salir.
     *   La cantidad de personas que ya están saliendo es mayor a 0
     *   Ya no quedan personas en el aula
     */

    printf("Esperando alumnos libres (%d/5)\n", un_aula->cantidad_de_alumnos_libres);
    pthread_cond_wait( &hay_alumnos_libres, &hay_alumnos_libres_mutex );
  }

  //pthread_mutex_lock( un_aula->cantidad_de_alumnos_saliendo_mutex );
  /* Si llegué hasta aquí, significa que este alumno se encuentra en condiciones de salir,
    entonces, si no hay personas saliendo... */
  if( un_aula->cantidad_de_alumnos_saliendo <= 0 )
  {
    /* ...y la cantidad de alumnos libre es mayor o igual al mínimo (MIN_ALUMNOS_LIBRES) */
    if( un_aula->cantidad_de_alumnos_libres >= MIN_ALUMNOS_LIBRES )
    {
      // ...entonces sumo "MIN_ALUMNOS_LIBRES" a la cantidad de personas saliendo
      // Y resto "MIN_ALUMNOS_LIBRES" de la cantidad de personas libres
      printf( "Se formó un grupo de (%d) alumnos...\n", MIN_ALUMNOS_LIBRES );
      un_aula->cantidad_de_alumnos_libres -= MIN_ALUMNOS_LIBRES;
      un_aula->cantidad_de_alumnos_saliendo += MIN_ALUMNOS_LIBRES;
    }
    else
    {
      /* De lo contrario, sumo solo un alumno a la cantidad de alumnos saliendo */
      un_aula->cantidad_de_alumnos_saliendo++;
    }
  }

  printf( "Saliendo %d/5...\n", un_aula->cantidad_de_alumnos_saliendo );
  un_aula->cantidad_de_alumnos_saliendo--;
  //pthread_mutex_unlock( un_aula->cantidad_de_alumnos_saliendo_mutex );

  // Libero el mutex de alumnos libres, y doy un signal para que algun otro thread pueda comprobar la variable de condición
  pthread_mutex_unlock( &hay_alumnos_libres_mutex );
  pthread_cond_signal( &hay_alumnos_libres );

}

static void terminar_servidor_de_alumno( int socket_fd, t_aula* aula, t_persona_copada* alumno )
{
  printf( ">> Se interrumpió la comunicación con una consola.\n" );
  close( socket_fd );
  t_aula_liberar( aula, alumno );
  exit( -1 );
}


t_comando intentar_moverse( t_aula* el_aula, t_persona_copada* alumno, t_direccion dir )
{
  // Obtengo las filas y columnas actuales del alumno
  int fila = alumno->posicion_fila;
  int columna = alumno->posicion_columna;
  // Actualizo las 
  alumno->salio = direccion_moverse_hacia( dir, &fila, &columna );
  bool entre_limites = ( fila >= 0 ) && ( columna >= 0 ) &&
                       (fila < ANCHO_AULA) && (columna < ALTO_AULA);

  // Si el alumno ya salió...
  if(alumno->salio)
  {
    // Pido el mutex para el casillero en donde está parado el alumno
    pthread_mutex_lock(el_aula->m_cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]);
    // Le resto uno a la cantidad de alumnos que se encuentran parados en el casillero donde estaba parado el alumno
    el_aula->cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]--;
    // Libero el mutex para el casillero en donde estaba parado el alumno
    pthread_mutex_unlock(el_aula->m_cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]);
    // Retorno true porque el alumno se pudo mover
    return (t_comando) true;
  }

  // Si se encuentra dentro del rango del tablero...
  if(entre_limites)
  {
    pthread_mutex_t* primer_mutex;
    pthread_mutex_t* segundo_mutex;
    if( fila<alumno->posicion_fila || (fila==alumno->posicion_fila && columna<alumno->posicion_columna) )
    {
       primer_mutex = el_aula->m_cant_personas_pos[fila][columna];
       segundo_mutex = el_aula->m_cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna];
    }
    else
    {
      primer_mutex = el_aula->m_cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna];
      segundo_mutex = el_aula->m_cant_personas_pos[fila][columna];
    }

    pthread_mutex_lock(primer_mutex);
    pthread_mutex_lock(segundo_mutex);

    if(el_aula->cant_personas_pos[fila][columna] < MAXIMO_POR_POSICION)
    {
      el_aula->cant_personas_pos[fila][columna]++;
      el_aula->cant_personas_pos[alumno->posicion_fila][alumno->posicion_columna]--;
      alumno->posicion_fila = fila;
      alumno->posicion_columna = columna;
      
      pthread_mutex_unlock(segundo_mutex);
      pthread_mutex_unlock(primer_mutex);
      return (t_comando) true;
    }

    pthread_mutex_unlock(segundo_mutex);
    pthread_mutex_unlock(primer_mutex);
  }

  // Si llegué hasta aquí es que el alumno no se pudo mover
  return (t_comando) false;
}

void colocar_sombrero( t_aula* el_aula, t_persona_copada* alumno )
{
  printf( "Colocando sombrero!!!. Ya casi %s!\n", alumno->nombre );
  alumno->tiene_sombrero = true;
  usleep(5000 * 1000);
  printf( "Ya se le colocó el sombrero a %s!\n", alumno->nombre );
}


void* atendedor_de_alumno( int socket_fd, t_aula* el_aula )
{
  t_persona_copada alumno;
  t_persona_copada_inicializar( &alumno );

  if ( recibir_nombre_copado_y_posicion( socket_fd, &alumno ) != 0 )
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
    printf("%s intentando moverse\n", alumno.nombre);
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

  printf("Esperando rescatista para poner sombrero a alumno.\n");
  // Me fijo si hay rescatistas disponibles
  pthread_mutex_lock(&hay_rescatistas_libres_mutex);
  while ( el_aula->rescatistas_disponibles <= 0 ) {
    // Mientras no haya rescatistas disponibles, espero
    pthread_cond_wait(&hay_rescatistas_libres, &hay_rescatistas_libres_mutex);
  }
  // Una vez que se que hay al menos un rescatista disponible, resto uno a la cantidad de disponibles
  el_aula->rescatistas_disponibles--;
  printf("Se desocupó un rescatista. Quedan %d rescatistas desocupados.\n",el_aula->rescatistas_disponibles);
  pthread_mutex_unlock(&hay_rescatistas_libres_mutex);

  // Le coloco el sombrero al alumno, mientras tanto el rescatista está ocupado
  colocar_sombrero( el_aula, &alumno );
  printf("Se colocó el sombrero al alumno.\n");

  // Una vez que le coloqué el sombrero, el rescatista se desocupó, así que le sumo uno a la cantidad de disponibles
  pthread_mutex_lock(&hay_rescatistas_libres_mutex);
  el_aula->rescatistas_disponibles++;
  pthread_mutex_unlock(&hay_rescatistas_libres_mutex);
  pthread_cond_signal(&hay_rescatistas_libres);
  printf("Quedan %d rescatistas desocupados.\n",el_aula->rescatistas_disponibles);

  t_aula_liberar( el_aula, &alumno );
  enviar_respuesta( socket_fd, LIBRE );
  printf( "Listo, %s es libre!\n", alumno.nombre );
 	fflush(stdout);
	return NULL;
}

/**
 * Esta función se va a encargar de llamar a la función
 * atendedor de alumno, pasandole los parámetros correspondientes.
 * Recibe un struct de parámetros y luego se tiene que encargar
 * de liberar la memoria correspondiente.
 */
void wrap_atendedor_de_alumno( Params* params )
{
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
  /// Aceptar conexiones entrantes.
  socket_size = sizeof( remoto );

  t_aula el_aula; t_aula_iniciar_vacia( &el_aula );

  pthread_attr_t attr; //< Acá voy a guardar la configuración para crear el thread
  pthread_attr_init( &( attr ) ); //< Inicializo la configuración por defecto

  // Inicializo el mutex para la variable de condición
  pthread_mutex_init( &hay_alumnos_libres_mutex, (const pthread_mutexattr_t *) NULL );
  pthread_mutex_init( &hay_rescatistas_libres_mutex, (const pthread_mutexattr_t *) NULL);
  el_aula.cantidad_de_personas_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init( el_aula.cantidad_de_personas_mutex, (const pthread_mutexattr_t *) NULL);
  el_aula.cantidad_de_alumnos_saliendo_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init( el_aula.cantidad_de_alumnos_saliendo_mutex, (const pthread_mutexattr_t *) NULL);

  // Inicializo las variables de condición
  pthread_condattr_t __condattr;
  pthread_condattr_init(&__condattr);
  pthread_cond_init( &hay_alumnos_libres, &__condattr );
  pthread_condattr_init(&__condattr);
  pthread_cond_init( &hay_rescatistas_libres, &__condattr );

  for ( ;; )
  {
    if ( -1 == ( socketfd_cliente =
                   accept( socket_servidor, ( struct sockaddr* ) &remoto, ( socklen_t* ) &socket_size ) ) )
    {
      printf( "!! Error al aceptar conexion\n" );
    }
    else
    {
      //pthread_t tid; //< Acá voy a guardar el Thread-ID
      // Reservo memoria para el struct que contiene los parámetros que va a recibir el thread
      Params* params = ( Params* ) malloc( sizeof( Params ) );
      {
        params->socket_fd = socketfd_cliente; //< El thread recibe el descriptor de la conexión con el cliente
        params->el_aula = &el_aula; //< También recibe una referencia al aula
      }
      // Creo el thread
      /*
      pthread_create( &( tid ), //< Puntero a int donde se va a guardar el thread-id
                      &( attr ), //< Puntero a atributos
                      ( void (*))( void* ) wrap_atendedor_de_alumno, //< Puntero a función que va a correr el thread
                      ( void* ) params ); //< Puntero a un struct que contiene los parámetros
      */
    }
  }

  // Elimino la configuración (el manual dice que es importante hacerlo)
  pthread_attr_destroy( &( attr ) );

  // ¿Hay que correr pthread_condattr_destroy?
  //pthread_condattr_destroy
  return 0;
}
