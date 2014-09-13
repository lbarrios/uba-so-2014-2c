#include "tasks.h"
#include "stdlib.h"
#include "assert.h"
#include <ctime>

#define SEED 0
#define FOR(var, range) for (int var = 0; var < range; var++) 

using namespace std;

void TaskCPU(int pid, vector<int> params) { // params: n
	uso_CPU(pid, params[0]); // Uso el CPU n milisegundos.
}

void TaskIO(int pid, vector<int> params) { // params: ms_pid, ms_io,
	uso_CPU(pid, params[0]); // Uso el CPU ms_pid milisegundos.
	uso_IO(pid, params[1]); // Uso IO ms_io milisegundos.
}

void TaskAlterno(int pid, vector<int> params) { // params: ms_pid, ms_io, ms_pid, ...
	for(int i = 0; i < (int)params.size(); i++) {
		if (i % 2 == 0) uso_CPU(pid, params[i]);
		else uso_IO(pid, params[i]);
	}
}

void TaskConsola(int pid, vector<int> params) { // params: n, bmin, bmax
    srand(time(NULL));
    int r;
    int n = params[0];
    int bmin = params[1];
    int bmax = params[2];
    for(int i = 0; i < n; i++) {
        r = bmin + rand()%(bmax-bmin);
        uso_IO(pid, r);
    }
}

void TaskBatch(int pid, vector<int> params) { // params: total_cpu, cant_bloqueos
    int total_cpu = params[0];
    int cant_bloqueos = params[1];
    assert(cant_bloqueos < total_cpu);
    vector<bool> tiempo_bloqueo(total_cpu, false);
    srand(time(NULL));
    int r;
    FOR(i, cant_bloqueos) {
        r = rand() % total_cpu;
        while(tiempo_bloqueo[r]) {
            r = rand() % total_cpu;
        }
        tiempo_bloqueo[r] = true;
    }
    FOR(i, total_cpu) {
        if (tiempo_bloqueo[i])
            uso_IO(pid, 1);
        else uso_CPU(pid, 1);
    }
}

void tasks_init(void) {
	/* Todos los tipos de tareas se deben registrar acá para poder ser usadas.
	 * El segundo parámetro indica la cantidad de parámetros que recibe la tarea
	 * como un vector de enteros, o -1 para una cantidad de parámetros variable. */
	register_task(TaskCPU, 1);
	register_task(TaskIO, 2);
	register_task(TaskAlterno, -1);
	register_task(TaskConsola, 3);
	register_task(TaskBatch, 2);
}
