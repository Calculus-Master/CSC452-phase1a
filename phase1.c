#include <phase1.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Process {
    int pid;
    int priority;
    int status;
    char name[MAXNAME];
    char* stack;

    int(*func)(void *);
    void* arg;

    struct Process* parent;
    struct Process* next_sibling;
    struct Process* children;
    USLOSS_Context context;
} Process;

Process process_table[MAXPROC];
Process* current_process;
int next_pid = 1;
char init_stack[USLOSS_MIN_STACK];

// Process functions
int do_testcase_main()
{
    testcase_main();

    USLOSS_Console("Phase 1A TEMPORARY HACK: testcase_main() returned, simulation will now halt.\n");
    USLOSS_Halt(0);

    return 0;
}

int do_init()
{
    phase2_start_service_processes();
    phase3_start_service_processes();
    phase4_start_service_processes();
    phase5_start_service_processes();

    int pid = spork("testcase_main", do_testcase_main, NULL, USLOSS_MIN_STACK, 3);

    TEMP_switchTo(pid);

    return 0;
}

void process_wrapper()
{
    USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    int status = current_process->func(current_process->arg);
    quit_phase_1a(status, current_process->parent->pid);
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

    current_process = &process_table[next_pid % MAXPROC];
    current_process->pid = next_pid++;
    current_process->priority = 6;
    strcpy(current_process->name, "init");

    current_process->stack = init_stack;

    current_process->func = do_init;
    current_process->arg = NULL;

    USLOSS_ContextInit(&(current_process->context), init_stack, USLOSS_MIN_STACK, NULL, process_wrapper);

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

    // Check parameters
    if(stacksize < USLOSS_MIN_STACK) return -2; // Stack size too small
    else if(name == NULL || func == NULL) return -1; // Name or function is NULL
    else if(strlen(name) > MAXNAME) return -1; // Name too long
    else if(priority < 1 || priority > 6) return -1; // Priority out of range

    int slot = next_pid % MAXPROC; //TODO: Cycle through until empty slot found, return -1 if not
    Process* new_process = &process_table[slot];
    memset(new_process, 0, sizeof(Process));

    new_process->pid = next_pid++;
    new_process->priority = priority;
    strcpy(new_process->name, name);

    new_process->stack = malloc(stacksize);
    new_process->func = func;
    new_process->arg = arg;

    USLOSS_ContextInit(&(new_process->context), new_process->stack, stacksize, NULL, process_wrapper); //TODO: check weird function pointer stuff

    new_process->parent = current_process;
    new_process->next_sibling = current_process->children;
    current_process->children = new_process;

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
    return new_process->pid;
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
    current_process = &process_table[pid % MAXPROC];
    USLOSS_ContextSwitch(pid == 1 ? NULL : &(old->context), &(current_process->context));

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}