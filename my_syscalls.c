#include <fcntl.h>
#include <glob.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

int my_close(int file_descriptor) {
    int result;
    asm("syscall"
            :"=a" (result)
            :"a"(3), "D" (file_descriptor));

    return result;
}

/*

*/
void *my_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset) {
    void *result;

    register int r8 asm("r8") = fd;
    register off_t r9 asm("r9") = offset;
    register int r10 asm("r10") = flags;

    asm("syscall"
        :"=a" (result)
        :"a"(9), "D"(start), "S" (length), "d" (prot),
         "r" (r10), "r" (r8), "r" (r9));

    return result;
}

int my_execve(const char *pathname, char *const argv[], char *const envp[]) {
    int result;

    asm("syscall"
        :"=a" (result)
        :"a" (59), "D" (pathname), "S" (argv), "d" (envp));

    return result;
}

int main() {
    int fd = open("text.txt", O_CREAT | O_RDWR, S_IRWXU);
    printf("open status: %d\n", fd);

    char buf[] = "this is test message";
    size_t n = strlen(buf);
    write(fd, buf, n);

    char *string_from_file = (char*) my_mmap(NULL, n, PROT_READ, MAP_PRIVATE, fd, 0);

    printf("string from file:%s\n", string_from_file);

    printf("close status: %d\n", my_close(fd));

    printf("Start other program\n");
    int status = my_execve("/home/vovan/CLionProjects/OS1/cmake-build-debug/a.out", NULL, NULL);
    printf("If other program starts correctly you never will see this message\n");
    printf("Status: %d\n", status);
}