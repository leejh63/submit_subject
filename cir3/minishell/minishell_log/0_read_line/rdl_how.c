#include <readline/readline.h>	// readline 관련 헤더 // -lreadline 컴파일 플래그 필요
#include <readline/history.h>	// history 관련 헤더
#include <unistd.h>		// write 관련 헤더
#include <stdio.h>		// printf, perror 관련 헤더
#include <signal.h>		// signal 관련 헤더
#include <errno.h>		// err_num 관련 헤더 
					//	perror의 경우 <errno.h> 헤더 포함이 표준
#include <stdlib.h>		// free, malloc 관련 헤더
#include <sys/ioctl.h>		// ioctl() prototypes, TIOCSTI 정의 관련 헤더
#include <fcntl.h>		// open
#include <termios.h>		// 터미널 제어 관련 헤더 


/*/-------시그널 체크 후 메인루프에서 처리 -----------------
static volatile sig_atomic_t sig_int;

void	sig_int_func(int sig_no, siginfo_t *info, void *ctx)
{
	(void) sig_no;
	(void) info;
	(void) ctx;
	
	sig_int = 1;
}

void	ctrl_c(void)
{
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
	sig_int = 0;
}

int	signal_set(void)
{
	struct sigaction sig;

	sig.sa_sigaction = sig_int_func;
	sig.sa_flags = SA_SIGINFO;
	if (sigemptyset(&sig.sa_mask) == -1)
		return (perror("sig_empty_set"), 1);
	if (sigaddset(&sig.sa_mask, SIGTERM) == -1)
		return (perror("sig_add_set"), 1);
	if (sigaction(SIGINT, &sig, NULL) == -1)
		return (perror("sig_action"), 1);
	return (0);
}

int	main(void)
{
	
	char	*read_line;
	
	if (signal_set())
		return (1);
	while (1)
	{
		read_line = readline("test_readline>");
		if (sig_int)
		{
			ctrl+c()
			free(read_line)
			continue
		}
		if (!read_line)
		{
			printf("errno %d\n", errno);
			if (errno == EINTR)
				continue ;
			if (errno == 0)
				return (write(1, "exit\n", 5), 0);
			perror("read_line");
			return (1);
		}
		else
		{
			if (*read_line)
			{
				printf("%s\n", read_line);
				add_history(read_line);
			}
		}
		free(read_line);
	}
}
//-----------------------------------------------------------------*/



/*/-------터미널 제어 이스케이프 시퀀스-----------------
static volatile sig_atomic_t got_sigint = 0;

static void sigint_handler(int signo)
{
	(void)signo;
	
	const char seq[] =
		"\n"            // 개행
		"\r"            // 커서 맨 앞으로
		"\033[2K"       // 커서 라인 지우기
		"\033[?2004h"   // (옵션) bracketed-paste 모드 켜기
		"minishell> ";  // 새 프롬프트
	write(STDOUT_FILENO, seq, sizeof(seq) - 1);
}

int main(void)
{
	char *line;

	signal(SIGINT, sigint_handler);

	while ((line = readline("minishell> ")) != NULL) {
		//rl_on_new_line();
		if (*line)
			add_history(line);
		printf(">> %s\n", line);
		free(line);
	}
	return 0;
}
//---------------------------------------------------------------------------*/

//-------rl_signal_event_hook / rl_event_hook -----------------

static volatile sig_atomic_t got_sigint = 0;

static void handle_sigint(int signo)
{
	(void)signo;
	got_sigint = 1;
}

static int sigint_event_hook(void)
{
	if (got_sigint)
	{
		write (1, "\n", 1);
		rl_replace_line("", 0);
		rl_on_new_line();
		rl_redisplay();
		//got_sigint = 0;  // rl_signal_event_hook, rl_event_hook 차이를 보기위해선 이걸 주석처리후 확인해볼것
	}
	return 0;
}

int main(void)
{
	char *line;

	rl_catch_signals = 0;				// 시그널 핸들러 적용 여부
	signal(SIGINT, handle_sigint);			// Ctrl-C → handle_sigint 
	rl_signal_event_hook = sigint_event_hook;	// 이건 시그널 받을 때만 시도
	//rl_event_hook = sigint_event_hook;		//이건 readline 내부적으로 일정 시간마다 계속 시도함  
	while ((line = readline("mini-shell> ")) != NULL)
	{
		if (*line)
			add_history(line);
		printf("명령: %s\n", line);
		free(line);
	}
	puts("exit");
	return 0;
}
//-----------------------------------------------------------------*/


/*/-------------------------close / open-------------------------
static volatile sig_atomic_t	g_signum = 0;

void	sig_int_func(int sig)
{
	g_signum = sig;
	close(0); //stdin(파일 디스크립터 0)을 /dev/tty로 다시 열기
}

void	ctrl_c(void)
{
	//write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();

	// /dev/tty를 다시 열고
	int fd = open("/dev/tty", O_RDONLY);
	if (fd != STDIN_FILENO) {
		if (dup2(fd, STDIN_FILENO) < 0)
			perror("dup2");
		close(fd);
	}
	g_signum = 0;
}

int main(void)
{
	char *cmdline;
	signal(SIGINT, sig_int_func);

	while(1)
	{
		cmdline = readline(">");
		if (g_signum == SIGINT)
		{
			ctrl_c();
		}
		else if ((cmdline && *cmdline))
		{
			add_history(cmdline);
			printf("%s\n", cmdline);
		}
		else if (cmdline == NULL)
		{
			break ;
		}
		free(cmdline);
	}
}
//---------------------------------------------------------------------------*/



