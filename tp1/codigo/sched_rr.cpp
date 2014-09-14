#include "sched_rr.h"
#include <iostream>
#include <algorithm>

using namespace std;

#define DEBUG_HEADER cerr << __FILE__ << ":" << __LINE__ << "(" << __FUNCTION__ << ") ";
#define DEBUG_VAR(var) cerr << #var << ":" << var << " ";

#define DEBUG1(var) DEBUG_HEADER; DEBUG_VAR(var); cerr << endl;
#define DEBUG2(var1, var2) DEBUG_HEADER; DEBUG_VAR(var1); DEBUG_VAR(var2); cerr << endl;

#define FORVECTORDEBUG1(i, f, v) for(int fd=i;fd<f;fd++){DEBUG1(v[fd]);}

SchedRR::SchedRR( vector<int> argn )
{
  cerr << "Inicializando SchedRR" << endl;
  // Round robin recibe la cantidad de cores y sus cpu_quantum por parámetro
  this->cores_count = argn[0]; DEBUG1( this->cores_count );
  // Genero la cola de quantums
  this->cores_quantums.reserve( this->cores_count );
  copy( ( ++argn.begin() ), argn.end(), this->cores_quantums.begin() ); FORVECTORDEBUG1( 0, this->cores_count, cores_quantums );
  // Asigno el tamaño de la cola de ticks
  this->cores_ticks.resize( this->cores_count );
}

SchedRR::~SchedRR()
{
}

/**
 * Este método es llamado cuando se desea cargar un proceso a la cola de ejecución.
 *
 * @param pid "process ID"
 */
void SchedRR::load( int pid )
{
  // Llegó una tarea nueva, la pongo en la cola
  cerr << "load(); cargando nueva tarea en la cola: " << pid << endl;
  this->process_queue.push( pid );
}

/**
 * Este método es llamado cuando un proceso se desbloquea.
 *
 * @param pid "process ID"
 */
void SchedRR::unblock( int pid )
{
  // El proceso se liberó, así que lo encolo nuevamente
  this->process_queue.push( pid );
}

/**
 * run_next recibe una CPU,
 * encola el proceso actual (excepto que sea idle),
 * setea la cantidad de ticks de la CPU en 0,
 * y retorna el pid correspondiente al siguiente proceso
 *
 * @param cpu "ID de la CPU"
 */
int SchedRR::run_next( int cpu )
{
  // Si hay procesos en la cola, retorno el front()
  if ( !this->process_queue.empty() )
  {
    int sig = this->process_queue.front();
    this->process_queue.pop();
    this->cores_ticks[cpu] = 0;
    cerr << "Cambiando de tarea en la CPU " << cpu << "... current_pid:" << current_pid( cpu ) << ", siguiente tarea:" << sig << endl;
    return sig;
  }

  // Por defecto, retorno IDLE
  return IDLE_TASK;
}

/**
 * Este método se llama cada vez que transcurre un tick de reloj.
 *
 * @param  cpu "ID de la CPU"
 * @param  m "motivo"
 * @return el id del siguiente proceso
 */
int SchedRR::tick( int cpu, const enum Motivo m )
{
  // Sumo un tick al contador de ticks de esa CPU
  this->cores_ticks[cpu]++;
  cerr << endl << "TICK. CPU:" << cpu << ", motivo:" << m << ", ticks:" << this->cores_ticks[cpu] << "/" << this->cores_quantums[cpu] << endl;

  // Si el motivo es EXIT, corro el siguiente proceso
  if ( m == EXIT )
  {
    cerr << "Motivo: Exit" << endl;
    return this->run_next( cpu );
  }

  // Si el motivo es BLOCK, ....
  if ( m == BLOCK )
  {
    cerr << "Motivo: Block" << endl;
    // Asumo que el proceso IDLE jamás hace BLOCK...
    // No voy a encolar el proceso, lo encolo recién cuando me avisa que está unblock
    return this->run_next( cpu );
  }

  // Si el motivo es TICK, ...
  if ( m == TICK )
  {
    cerr << "Motivo: Tick" << endl;

    // Si el proceso actual es IDLE, o alcancé el QUANTUM, llamo a run_next
    if ( current_pid( cpu ) == IDLE_TASK || this->cores_ticks[cpu] >= this->cores_quantums[cpu] )
    {
      // Si el proceso actual no es IDLE, lo cargo en la cola
      if ( current_pid( cpu ) != IDLE_TASK )
      {
        cerr << "Alcancé el quantum" << endl;
        this->process_queue.push( current_pid( cpu ) );
      }

      return this->run_next( cpu );
    }
  }

  // Por defecto, devuelvo el pid del proceso actual
  cerr << "Retornando current_pid por defecto" << endl;
  return current_pid( cpu );
}
