export LC_ALL=C
ifeq ($(strip $(CXX)),)
	CXX=g++-4.6
endif
CXXFLAGS= -Wall -pedantic -g -ggdb -std=c++0x
LDFLAGS= -lpthread

OBJS=main.o simu.o basesched.o basetask.o tasks.o sched_rr.o sched_fcfs.o sched_rr2.o sched_lottery.o
MAIN=simusched

.PHONY: all clean new runRR
all: $(MAIN)

$(MAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

basesched.o: basesched.cpp basesched.h
basetask.o: basetask.cpp basetask.h
main.o: main.cpp simu.h basetask.h basesched.h tasks.h sched_fcfs.h \
 sched_rr.h sched_rr2.h
sched_fcfs.o: sched_fcfs.cpp sched_fcfs.h basesched.h
sched_rr.o: sched_rr.cpp sched_rr.h basesched.h
sched_rr2.o: sched_rr2.cpp sched_rr2.h basesched.h
sched_lottery.o: sched_lottery.cpp sched_lottery.h basesched.h
simu.o: simu.cpp simu.h basetask.h basesched.h
simusched.o: simusched.cpp basetask.h basesched.h tasks.h
tasks.o: tasks.cpp tasks.h basetask.h

clean: cleanRR
	rm -f $(OBJS) $(MAIN)

new: clean all

RR_TEMP_TXT=sched_rr_out_temp.txt
RR_TEMP_DBG=sched_rr_dbg_temp.txt
RR_TEMP_PNG=sched_rr_out_temp.png
ifeq ($(strip $(CORES)),)
	CORES=2
endif
ifeq ($(strip $(SCHED_PARAM)),)
	SCHED_PARAM=2 2
endif
ifeq ($(strip $(TASK_FILE)),)
	TASK_FILE=tasks/sched_rr/tareas.tsk
endif
runRR: all
	clear && \
	./simusched $(TASK_FILE) $(CORES) 1 1 SchedRR $(SCHED_PARAM) 2> $(RR_TEMP_DBG) > $(RR_TEMP_TXT) && \
	cat $(RR_TEMP_DBG) && \
	python graphsched.py < $(RR_TEMP_TXT) > $(RR_TEMP_PNG) && xdg-open $(RR_TEMP_PNG) &

cleanRR:
	rm -f $(RR_TEMP_TXT) $(RR_TEMP_PNG) $(RR_TEMP_DBG)
