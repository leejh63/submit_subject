#include "minishell.h"

volatile sig_atomic_t	g_signum = 0;

void	sig_int_func(int sig_no)
{
	g_signum = sig_no;
	if (sig_no == SIGINT) //isa
	{
		write(1, "\n", 1); 	// 개행으로 한줄 뒤로
		rl_replace_line("", 0);	// 현재 편집 라인을 "" 이걸로 대체
		rl_on_new_line();		// readline 내부 커서 동기화
		rl_redisplay();		// 프롬프트 입력줄을 터미널에 다시작성.
	}
}

void	set_signum(int sig_no)
{
	g_signum = sig_no;
}

void	set_signal_handler(void)
{
	signal(SIGQUIT, SIG_IGN);
	if (isatty(1))
		signal(SIGINT, sig_int_func);
	else
		signal(SIGINT, set_signum);
}