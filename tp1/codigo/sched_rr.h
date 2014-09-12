#pragma once

#include <vector>
#include <queue>
#include "basesched.h"

class SchedRR : public SchedBase {
	public:
		SchedRR(std::vector<int> argn);
        ~SchedRR();
		virtual void load(int pid);
		virtual void unblock(int pid);
		virtual int tick(int cpu, const enum Motivo m);

private:
		int run_next(int cpu);
		int cores_count;
		std::queue<int> process_queue;
		std::vector<int> cores_quantums;
		std::vector<int> cores_ticks;
};
