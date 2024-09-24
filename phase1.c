#include <phase1.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define READY_STATE 0
#define RUNNING_STATE 1
#define JOIN_BLOCKED_STATE 2
#define ZAP_BLOCKED_STATE 3
#define TERMINATED_STATE 4
#define JOIN_CLEANED_STATE 5

typedef struct Process
{
    int pid;
    int priority;
    int status;
    int state;
    char name[MAXNAME];
    char *stack;

    int (*func)(void *);
    void *arg;

    struct Process *parent;
    struct Process *next_sibling;
    struct Process *children;
    USLOSS_Context context;
} Process;

Process process_table[MAXPROC];
Process *current_process;
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

    USLOSS_Console("Phase 1A TEMPORARY HACK: init() manually switching to testcase_main() after using spork() to create it.\n");    

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
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
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

int spork(char *name, int (*func)(void *), void *arg, int stacksize, int priority)
{
    // Disable interrupts
    int old_psr = USLOSS_PsrGet();
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);

    // Checking kernel mode
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call spork while in user mode!\n");
        USLOSS_Halt(1);
    }

    // Check parameters
    if (stacksize < USLOSS_MIN_STACK)
        return -2; // Stack size too small
    else if (name == NULL || func == NULL)
        return -1; // Name or function is NULL
    else if (strlen(name) > MAXNAME)
        return -1; // Name too long
    else if (priority < 1 || priority > 6)
        return -1; // Priority out of range

    int slot = next_pid % MAXPROC; // TODO: Cycle through until empty slot found, return -1 if not
    Process *new_process = &process_table[slot];
    memset(new_process, 0, sizeof(Process));

    new_process->pid = next_pid++;
    new_process->priority = priority;
    strcpy(new_process->name, name);

    new_process->stack = malloc(stacksize);
    new_process->func = func;
    new_process->arg = arg;

    USLOSS_ContextInit(&(new_process->context), new_process->stack, stacksize, NULL, process_wrapper); // TODO: check weird function pointer stuff

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
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call join while in user mode!\n");
        USLOSS_Halt(1);
    }

    // if status pointer is null, return -3
    if (status == NULL)
        return -3;

    // if the process does not have any children return -2
    if (current_process->children == NULL)
        return -2;

    // Check if all children have already been joined
    Process *child = current_process->children;
    int all_joined = 1;
    while (child != NULL)
    {
        if (child->state != JOIN_CLEANED_STATE)
        {
            all_joined = 0;
            break;
        }
        child = child->next_sibling;
    }
    if (all_joined)
    {
        return -2;
    }

    // Iterate through children to find any dead processes
    child = current_process->children;
    while (child != NULL)
    {
        if (child->state == TERMINATED_STATE)
        {
            // Found a dead child
            int child_pid = child->pid;

            *status = process_table[child_pid].status; // Store the status through the out-pointer
            child->state = JOIN_CLEANED_STATE;

            // Remove the child from the parent's list
            if (child == current_process->children)
            {
                current_process->children = child->next_sibling;
            }
            else
            {
                Process *prev = current_process->children;
                while (prev->next_sibling != child)
                {
                    prev = prev->next_sibling;
                }
                prev->next_sibling = child->next_sibling;
            }

            // Free the child's stack memory
            if (child->stack != NULL)
            {
                free(child->stack);
                child->stack = NULL;
            }

            // Mark the child as joined
            child->state = JOIN_BLOCKED_STATE;

            // Return the child's PID
            return child_pid;
        }
        child = child->next_sibling;
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
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call quit_phase_1a while in user mode!\n");
        USLOSS_Halt(1);
    }

    // Check if the current process has any children that haven't been joined
    Process *child = current_process->children;
    while (child != NULL)
    {
        if (child->state != JOIN_CLEANED_STATE)
        {
            USLOSS_Console("ERROR: Process pid %d called quit() while it still had children.\n", current_process->pid);
            USLOSS_Halt(1);
        }
        child = child->next_sibling;
    }

    // status for this process stored in the process table entry
    Process *current = current_process;
    process_table[current->pid].status = status;

    current->state = TERMINATED_STATE;

    TEMP_switchTo(switchToPid);

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}

void quit(int status)
{
    // Disable interrupts
    int old_psr = USLOSS_PsrGet();
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);

    // Checking kernel mode
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
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
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call dumpProcesses while in user mode!\n");
        USLOSS_Halt(1);
    }
    
    USLOSS_Console(" PID  PPID  NAME              PRIORITY  STATE\n");

    for (int i = 0; i < MAXPROC; i++) {
        Process *proc = &process_table[i];
        if (proc->pid != 0) {
            char state[20];
            switch (proc->state) {
                case READY_STATE:
                    strcpy(state, "Runnable");
                    break;
                case RUNNING_STATE:
                    strcpy(state, "Running");
                    break;
                case JOIN_BLOCKED_STATE:
                    strcpy(state, "Join Blocked");
                    break;
                case ZAP_BLOCKED_STATE:
                    strcpy(state, "Zap Blocked");
                    break;
                case TERMINATED_STATE:
                    sprintf(state, "Terminated(%d)", proc->status);
                    break;
                case JOIN_CLEANED_STATE:
                    strcpy(state, "Join Cleaned");
                    break;
                default:
                    strcpy(state, "Unknown");
            }

            USLOSS_Console("%3d %5d  %-16s  %d         %s\n",
                           proc->pid,
                           proc->parent ? proc->parent->pid : 0,
                           proc->name,
                           proc->priority,
                           state);
        }
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
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        USLOSS_Console("ERROR: Someone attempted to call TEMP_switchTo while in user mode!\n");
        USLOSS_Halt(1);
    }

    Process *old = current_process;
    current_process = &process_table[pid % MAXPROC];
    USLOSS_ContextSwitch(pid == 1 ? NULL : &(old->context), &(current_process->context));

    // Restore interrupts
    USLOSS_PsrSet(old_psr);
}