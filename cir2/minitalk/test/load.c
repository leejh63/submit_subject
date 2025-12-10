/*
 * load_test.c ― 여러 클라이언트를 동시에 실행해
 *               서버(test.c) 동시성 문제를 빠르게 재현한다.
 *
 * build: cc load_test.c -o load_test
 *
 * usage: ./load_test <server_pid> <msg1> <msg2> ...
 *        예) ./load_test 123456 "HELLO1" "HELLO2"
 */

#include <unistd.h>   /* fork, execl */
#include <sys/wait.h> /* wait */
#include <stdlib.h>
#include <stdio.h>

static int spawn_client(const char *pid, const char *msg)
{
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return -1;
    }
    if (child == 0) {                 /* child 프로세스 */
        execl("./cla", "./cla", pid, msg, (char *)0);
        perror("execl");              /* execl 실패 시만 출력 */
        _exit(1);
    }
    return 0;                         /* parent는 계속 진행 */
}

int main(int ac, char **av)
{
    if (ac < 4) {
        fprintf(stderr,
            "usage: %s <server_pid> <msg1> <msg2> [msg3 ...]\n", av[0]);
        return 1;
    }

    const char *srv_pid = av[1];

    /* 각 메시지마다 클라이언트 하나씩 동시 실행 */
    for (int i = 2; i < ac; ++i)
        spawn_client(srv_pid, av[i]);

    /* 모든 자식이 끝날 때까지 대기 */
    while (wait(NULL) > 0)
        ;

    puts("all clients finished");
    return 0;
}

