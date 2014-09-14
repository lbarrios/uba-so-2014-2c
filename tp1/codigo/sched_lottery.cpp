#include "sched_lottery.h"

using namespace std;

#define DEBUG(var) cout << #var << ": " << var << endl;
#define FOR(var, range) for (int var = 0; var < range; var++) 


SchedLottery::SchedLottery(vector<int> argn) {
    quantum = argn[1];
    seed = argn[2];
    srand(seed);
    total_tickets = 0;

    total_ticks = 0;
    FOR(i, CANT_PROCESOS) ticks_por_proceso[i] = 0;
}

SchedLottery::~SchedLottery() {
}

void SchedLottery::load(int pid) {
    tickets.push_back(ticket(pid, 1));
    total_tickets++;
}

void SchedLottery::unblock(int pid) {
    ticket *pid_tickets = &tickets[tickets_index(pid)];
    int new_tickets = pid_tickets->compensation;
    pid_tickets->count = new_tickets;
    total_tickets += new_tickets;
}

int SchedLottery::tickets_index(int pid) {
    FOR(i, tickets.size()) {
        if (tickets[i].pid == pid) return i;
    }
    throw invalid_argument("el pid no esta en tickets");
}

void SchedLottery::compensa(int pid) {
    int ratio = int((float)quantum / float(tick_number+1));
    ticket *pid_tickets = &tickets[tickets_index(pid)];

    int previous_count = pid_tickets->count;
    pid_tickets->compensation = ratio*previous_count;
    pid_tickets->count = 0;
    total_tickets -= previous_count;
}

void SchedLottery::no_compensa(int pid) {
    ticket *pid_tickets = &tickets[tickets_index(pid)];

    pid_tickets->count = 0;
    pid_tickets->compensation = 1;
    total_tickets--;
}

void SchedLottery::desaloja(int pid) {
    if (pid != IDLE_TASK) {
        int i = tickets_index(pid);
        total_tickets -= tickets[i].count;
        tickets.erase(tickets.begin()+i);
    }
}

int SchedLottery::run_lottery() {
    imprimi_acumulado();
    tick_number = 0;
    if (total_tickets == 0) {
        return IDLE_TASK;
    }

    int winner = rand() % total_tickets;
    int i = 0;
    int accum = tickets[0].count;
    while (accum <= winner) {
        i++;
        accum += tickets[i].count;
    }
    while (tickets[i].count == 0) i++;  // esto es importante
    if (i >= tickets.size()) {
        throw invalid_argument("nadie es duenio de ese ticket");
    }

    // descompenso
    int *count = &tickets[i].count;
    total_tickets -= (*count - 1);
    *count = 1;

    return tickets[i].pid;
}

int SchedLottery::tick(int cpu, const enum Motivo m) {
    int pid = current_pid(cpu);
    if (m == TICK) {
        ticks_por_proceso[pid]++;
        tick_number++;
        total_ticks++;
        if (tick_number < quantum) {
            return pid;
        }
    } else if (m == BLOCK)
        compensa(pid);
    else if (m == EXIT)
        desaloja(pid);
    return run_lottery();
}

void SchedLottery::imprimi_tickets() {
    DEBUG(total_tickets)
    FOR(i, tickets.size()) {
        DEBUG(i)
        DEBUG(tickets[i].count)
    }
}

void SchedLottery::imprimi_acumulado() {
    cerr << total_ticks << " ";
    FOR(i, CANT_PROCESOS-1) {
        cerr << ticks_por_proceso[i] << " ";
    }
    cerr << ticks_por_proceso[CANT_PROCESOS-1] << endl;
}
