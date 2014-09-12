#include "sched_lottery.h"

using namespace std;

#define DEBUG(var) cout << #var << ": " << var << endl;

SchedLottery::SchedLottery(vector<int> argn) {
    quantum = argn[1];
    seed = argn[2];
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
    int new_tickets = tickets[tickets_index(pid)].to_be_compensated;
    tickets[tickets_index(pid)].count = new_tickets;
    total_tickets += new_tickets;
}

int SchedLottery::tickets_index(int pid) {
    for(int i = 0; i < tickets.size(); i++) {
        if (tickets[i].pid == pid) return i;
    }
    throw invalid_argument("el pid no esta en tickets");
}

void SchedLottery::compensa(int pid) {
    int compensation = int((float)quantum / float(tick_number+1));
    int previous_count = tickets[tickets_index(pid)].count;
    tickets[tickets_index(pid)].to_be_compensated = compensation*previous_count;
    tickets[tickets_index(pid)].count = 0;
    total_tickets -= previous_count;
}

void SchedLottery::desaloja(int pid) {
    if (pid != IDLE_TASK) {
        total_tickets -= tickets[tickets_index(pid)].count;
        tickets.erase(tickets.begin() + tickets_index(pid));
    }
}

void SchedLottery::imprimi_tickets() {
    cout << "imprimiendo estado de los tickets" << endl;
    for(int i = 0; i < tickets.size(); i++) {
        DEBUG(i)
        DEBUG(tickets[i].pid)
        DEBUG(tickets[i].count)
    }
    cout << "impreso el estado de los tickets" << endl;
}

int SchedLottery::run_lottery() {
    //imprimi_tickets();

    tick_number = 0;
    if (total_tickets == 0) {
        return IDLE_TASK;
    }

    int winner = rand() % total_tickets;
    int i = 0;
    int accum = tickets[0].count;
    while (accum < winner) {
        i++;
        accum += tickets[i].count;
    }
    if (i >= tickets.size()) {
        DEBUG(i)
        DEBUG(accum)
        DEBUG(winner)
        DEBUG(total_tickets)
        DEBUG(tickets.size())
        throw invalid_argument("nadie es duenio de ese ticket");
    }

    if (tickets[i].count != 0) {
        total_tickets -= tickets[i].count - 1;
        tickets[i].count = 1;
    }
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
