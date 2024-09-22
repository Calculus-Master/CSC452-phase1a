#include <phase1.h>
#include <stdio.h>
#include <string.h>

typedef struct Process {
    int pid;
    int priority;
    int status;
    char name[MAXNAME];
    USLOSS_Context context;
} Process;

Process process_table[MAXPROC];

void phase1_init(void)
{
    memset(process_table, 0, sizeof(process_table));
}

int spork(char *name, int(*func)(void *), void *arg, int stacksize, int priority)
{

}

int join(int *status)
{

}

void quit_phase_1a(int status, int switchToPid)
{

}

void quit(int status)
{

}

int getpid(void)
{

}

void dumpProcesses(void)
{

}

void TEMP_switchTo(int pid)
{

}