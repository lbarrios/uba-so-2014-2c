#pragma once

#include <vector>
#include <queue>
#include <stdexcept>
#include "basesched.h"

using namespace std;

/**
 * Este struct representa un core
 */
struct Core
{
  Core()
  	: quantum(0), ticks_count(0), active_process(vector<int>()), process_queue(queue<int>())
  { };
  int quantum;
  int ticks_count;
  vector<int> active_process;
  queue<int> process_queue;
  int runNextProcess( void )
  {
    int sig = this->process_queue.front();
    this->process_queue.pop();
    this->ticks_count = 0;
    return sig;
  }
  bool isQuantumExpired()
  {
    return this->ticks_count >= this->quantum;
  }
};

/**
 * Compara dos cores por cantidad de procesos
 */
struct core_process_comparator_t
{
  bool operator() ( Core a, Core b )
  {
    return a.active_process.size() < b.active_process.size();
  }
};

class SchedRR2 : public SchedBase
{
public:
  SchedRR2( vector<int> argn );
  ~SchedRR2();
  virtual void load( int pid );
  virtual void unblock( int pid );
  virtual int tick( int cpu, const enum Motivo m );

private:
  int run_next( int cpu );
  Core* getProcessCore( int );
  vector<Core> cores;
};
