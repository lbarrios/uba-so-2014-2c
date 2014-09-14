#ifndef __SCHED_LOT__
#define __SCHED_LOT__

#include "basesched.h"
#include <vector>
#include <stdexcept>
#include <stdlib.h>
#include <iostream>

using namespace std;

#define CANT_PROCESOS 10

struct ticket {
    int pid;
    int count;
    int compensation;
    ticket(int pid, int count): pid(pid), count(count) , compensation(1) {};
};

class SchedLottery : public SchedBase {
public:
    SchedLottery(std::vector<int> argn);
        ~SchedLottery();
    virtual void load(int pid);
    virtual void unblock(int pid);
    virtual int tick(int cpu, const enum Motivo m);
private:
    int quantum;
    int seed;
    vector<ticket> tickets;
    int tick_number;
    int run_lottery();
    void compensa(int);
    void no_compensa(int);
    void desaloja(int);
    int tickets_index(int);
    int total_tickets;
    void imprimi_tickets();
    void imprimi_acumulado();

    int total_ticks;
    int ticks_por_proceso[CANT_PROCESOS];
};

#endif
