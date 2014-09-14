#pragma once

#include <vector>
#include <queue>
#include "basesched.h"

class SchedRR : public SchedBase
{
public:
  SchedRR( std::vector<int> );
  ~SchedRR();
  virtual void load( int );
  virtual void unblock( int );
  virtual int tick( int, const enum Motivo );

private:
  int run_next( int );
  int cores_count;
  std::queue<int> process_queue;
  std::vector<int> cores_quantums;
  std::vector<int> cores_ticks;
};
