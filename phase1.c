#include <phase1.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Process {
    int pid;
    int priority;
    int status;
    char name[MAXNAME];
    struct Process* child;
    USLOSS_Context context;
} Process;

Process process_table[MAXPROC];
Process* current_process;
int next_pid = 1;

// Process functions
void do_testcase_main()
{
    testcase_main();

    USLOSS_Console("Phase 1A TEMPORARY HACK: testcase_main() returned, simulation will now halt.\n");
    USLOSS_Halt(0);
}

void do_init()
{
    int pid = spork("testcase_main", do_testcase_main, NULL, USLOSS_MIN_STACK, 3);

    TEMP_switchTo(pid);
}

// Main phase 1 functions
void phase1_init(void)
{
    // Disable interrupts
    int old_psr = USLOSS_PsrGet();
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);

    // Checking kernel mode
    if((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call phase1_init while in user mode!\n");
        USLOSS_Halt(1);
    }

    memset(process_table, 0, sizeof(process_table));

    current_process = &process_table[next_pid];
    current_process->pid = next_pid++;
    current_process->priority = 6;
    strcpy(current_process->name, "init");

    char* stack = malloc(USLOSS_MIN_STACK);
    USLOSS_ContextInit(&current_process->context, stack, USLOSS_MIN_STACK, NULL, do_init);

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}

int spork(char *name, int(*func)(void *), void *arg, int stacksize, int priority)
{
    // Disable interrupts
    int old_psr = USLOSS_PsrGet();
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);

    // Checking kernel mode
    if((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call spork while in user mode!\n");
        USLOSS_Halt(1);
    }

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}

int join(int *status)
{
    // Disable interrupts
    int old_psr = USLOSS_PsrGet();
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);

    // Checking kernel mode
    if((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call join while in user mode!\n");
        USLOSS_Halt(1);
    }

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}

void quit_phase_1a(int status, int switchToPid)
{
    // Disable interrupts
    int old_psr = USLOSS_PsrGet();
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);

    // Checking kernel mode
    if((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call quit_phase_1a while in user mode!\n");
        USLOSS_Halt(1);
    }

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}

void quit(int status)
{
    // Disable interrupts
    int old_psr = USLOSS_PsrGet();
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);

    // Checking kernel mode
    if((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call quit while in user mode!\n");
        USLOSS_Halt(1);
    }

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}

int getpid(void)
{
    return current_process->pid;
}

void dumpProcesses(void)
{
    // Disable interrupts
    int old_psr = USLOSS_PsrGet();
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);

    // Checking kernel mode
    if((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call dumpProcesses while in user mode!\n");
        USLOSS_Halt(1);
    }

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}

void TEMP_switchTo(int pid)
{
    // Disable interrupts
    int old_psr = USLOSS_PsrGet();
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);

    // Checking kernel mode
    if((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call TEMP_switchTo while in user mode!\n");
        USLOSS_Halt(1);
    }
    
    Process* old = current_process;
    current_process = &process_table[pid];
    USLOSS_ContextSwitch(&old->context, &current_process->context);

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}