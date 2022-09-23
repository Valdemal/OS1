#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/personality.h>

#define INT3 0xcc
#define CODE_SEGMENT_ADDRESS ((void*)0x555555554000)


void start_child_process(const char *execute_filename);

void start_parent_process(pid_t pid, long breakpoint_offset);

void print_state(const struct user_regs_struct *regs_ptr);

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Invalid count of arguments!\n");
        exit(1);
    }

    char *filename = argv[1];

    if (access(filename, F_OK) == -1) {
        fprintf(stderr, "Error opening file %s!\n", filename);
        exit(-1);
    }

    printf("Enter breakpoint address:");
    long breakpoint_relative_address;
    scanf("%lx", &breakpoint_relative_address);
    printf("\n");

    pid_t pid = fork();
    if (pid == 0) { // Child process code
        start_child_process(filename);
    } else if (pid > 0) { // Parent process code
        start_parent_process(pid, breakpoint_relative_address);
    } else {
        fprintf(stderr, "Can't create child process!\n");
        exit(-1);
    }
}

void start_child_process(const char *const execute_filename) {
    // Start tracing of this process by parent
    ptrace(PTRACE_TRACEME, 0, 0, 0);

    // Address space randomization off
    personality(ADDR_NO_RANDOMIZE);

    execl(execute_filename, execute_filename, NULL);
}

void start_parent_process(pid_t pid, long breakpoint_offset) {
    int status;
    waitpid(pid, &status, 0);

    // Get the instruction code at its address
    const void *instruction_address = CODE_SEGMENT_ADDRESS + breakpoint_offset;
    long instruction_code = ptrace(PTRACE_PEEKTEXT, pid, instruction_address, NULL);

    // change instruction in the memory
    ptrace(PTRACE_POKETEXT, pid, instruction_address, INT3);

    // Continue child process
    ptrace(PTRACE_CONT, pid, 0, 0);
    waitpid(pid, &status, 0);

    // reading the values of registers
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);

    // returning to memory initial instruction
    ptrace(PTRACE_POKETEXT, pid, instruction_address, instruction_code);

    // Stop debugged process
    ptrace(PTRACE_KILL, pid, NULL, NULL);

    print_state(&regs);
}

void print_state(const struct user_regs_struct *const regs_ptr) {
    printf("RAX:%llx\t", regs_ptr->rax);
    printf("RBX:%llx\n", regs_ptr->rbx);
    printf("RCX:%llx\t", regs_ptr->rcx);
    printf("RDX:%llx\n", regs_ptr->rdx);
    printf("RSI:%llx\t", regs_ptr->rsi);
    printf("RDI:%llx\n", regs_ptr->rdi);
    printf("RBP:%llx\t", regs_ptr->rbp);
    printf("RSP:%llx\n", regs_ptr->rsp);
    printf("RCX:%llx\t", regs_ptr->rcx);
    printf("RIP:%llx\n", regs_ptr->rip);
    printf("orig_rax:%llx\n", regs_ptr->orig_rax);
    printf("\n");
}