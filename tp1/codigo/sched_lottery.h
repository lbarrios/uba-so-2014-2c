#ifndef __SCHED_LOT__
#define __SCHED_LOT__

#include "basesched.h"
#include <vector>
#include <stdexcept>
#include <stdlib.h>
#include <iostream>

using namespace std;

struct ticket {
    int pid;
    int count;
    int to_be_compensated;
    ticket(int pid, int count): pid(pid), count(count) {};
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
    void desaloja(int);
    int tickets_index(int);
    int total_tickets;
    void imprimi_tickets();
};

#endif
