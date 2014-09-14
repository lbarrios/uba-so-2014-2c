#include "sched_rr2.h"
#include <iostream>
#include <algorithm>

/**
 * Round robin 2 recibe la cantidad de cores y sus respectivos quantum por parámetro
 */
SchedRR2::SchedRR2( vector<int> argn )
{
  // Asigno el tamaño de la cola de cores
  this->cores.resize( argn[0] );

  // Asigno los quantums de cada core
  for ( int i = 0; i < argn[0]; i++ )
  {
    this->cores[i].quantum = argn[i + 1];
  }
}

SchedRR2::~SchedRR2()
{
}

/**
 * Este método es llamado cuando se desea cargar un proceso a la cola de ejecución.
 * Voy a querer cargar la tarea en el core con menos procesos activos.
 *
 * @param pid "process ID"
 */
void SchedRR2::load( int pid )
{
  // Obtengo el core con menos procesos activos
  Core& core = *min_element( begin( this->cores ), end( this->cores ), core_process_comparator );
  // Agrego el proceso a la cola del core
  core.process_queue.push( pid );
  core.active_process.push_back( pid );
}

/**
 * Recibe el ID de un proceso y devuelve un iterador al core que le corresponde.
 * (está implementado de forma "ineficiente").
 *
 * @param  pid "ID del proceso"
 */
Core* SchedRR2::getProcessCore( int pid )
{
  // Recorro la lista de cores
  for ( auto& core : this->cores )
  {
    // Recorro la lista de procesos activos
    for ( int cpid : core.active_process )
    {
      // Si el core contiene al proceso "pid", lo retorno
      if ( cpid == pid )
      {
        return &( core );
      }
    }
  }

  // Jamás debería llegar a este punto.
  return NULL;
}

/**
 * Este método es llamado cuando un proceso se desbloquea.
 *
 * @param pid "process ID"
 */
void SchedRR2::unblock( int pid )
{
  // El proceso se liberó, así que lo encolo nuevamente
  Core* core = this->getProcessCore( pid );

  if ( !core )
  {
    throw invalid_argument( "No se puede obtener el procesador correspondiente al pid indicado." );
  }

  core->process_queue.push( pid );
}

/**
 * run_next recibe una CPU,
 * encola el proceso actual (excepto que sea idle),
 * setea la cantidad de ticks de la CPU en 0,
 * y retorna el pid correspondiente al siguiente proceso
 *
 * @param cpu "ID de la CPU"
 */
int SchedRR2::run_next( int cpu )
{
  // Si hay procesos en la cola, retorno el siguiente
  if ( !this->cores[cpu].process_queue.empty() )
  {
    return this->cores[cpu].runNextProcess();
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
int SchedRR2::tick( int cpu, const enum Motivo m )
{
  int pid = current_pid( cpu );
  // Sumo un tick al contador de ticks de esa CPU
  this->cores[cpu].ticks_count++;

  // Si el motivo es EXIT, o BLOCK, corro el siguiente proceso
  if ( m == EXIT || m == BLOCK )
  {
    // Si el motivo es EXIT, lo quito de la cola de activos
    if ( m == EXIT )
    {
      auto p = this->cores[cpu].active_process;
      this->cores[cpu].active_process.erase( find( begin( p ), end( p ), pid ) );
    }

    return this->run_next( cpu );
  }

  // Si el motivo es TICK, ...
  if ( m == TICK )
  {
    // Si el proceso actual es IDLE, o alcancé el QUANTUM, llamo a run_next
    if ( pid == IDLE_TASK || this->cores[cpu].isQuantumExpired() )
    {
      // Si el proceso actual no es IDLE, lo cargo en la cola
      if ( pid != IDLE_TASK )
      {
        this->cores[cpu].process_queue.push( pid );
      }

      return this->run_next( cpu );
    }
  }

  // Por defecto, devuelvo el pid del proceso actual
  return pid;
}
