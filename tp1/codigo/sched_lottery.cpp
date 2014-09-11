#include "sched_lottery.h"

using namespace std;

#define DEBUG(var) cout << #var << ": " << var << endl;

SchedLottery::SchedLottery(vector<int> argn) {
    quantum = argn[0];
    seed = argn[1];
    srand(seed);
    total_tickets = 0;
}

SchedLottery::~SchedLottery() {
}

void SchedLottery::load(int pid) {
    tickets.push_back(ticket(pid, 1));
    total_tickets++;
}

void SchedLottery::unblock(int pid) {
    // no hacemos nada
}

int SchedLottery::tickets_index(int pid) {
    for(int i = 0; i < tickets.size(); i++) {
        if (tickets[i].pid == pid) return i;
    }
    throw invalid_argument("el pid no esta en tickets");
}

void SchedLottery::compensa(int pid) {
    int compensation = int(float(tick_number+1) / (float)quantum);
    tickets[tickets_index(pid)].count = compensation;
    total_tickets += compensation-1;
}

void SchedLottery::desaloja(int pid) {
    if (pid != IDLE_TASK) {
        tickets.erase(tickets.begin() + tickets_index(pid));
        total_tickets--;
    }
}

int SchedLottery::run_lottery() {
    tick_number = 0;
    if (total_tickets == 0) {
        return IDLE_TASK;
    }

    int winner = rand() % total_tickets;
    int i = 0;
    int accum = 0;
    while (accum < winner) {
        accum += tickets[i].count;
        i++;
    }
    if (i >= tickets.size())
        throw invalid_argument("nadie es duenio de ese ticket");

    total_tickets -= tickets[i].count - 1;
    tickets[i].count = 1;
    return tickets[i].pid;
}

int SchedLottery::tick(int cpu, const enum Motivo m) {
    if (m == TICK) {
        tick_number++;
        if (tick_number >= quantum) {
            // desalojo a la tarea
        } else 
            return current_pid(cpu);

    } else if (m == BLOCK) {
        compensa(current_pid(cpu));
    } else if (m == EXIT) {
        desaloja(current_pid(cpu));

    }
    return run_lottery();
}
