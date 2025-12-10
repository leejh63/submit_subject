#include "minishell.h"

int	is_all_ws(char *str)
{
	if (str == NULL)
		return -1;
	while(*str)
	{
		if (is_whitespace(*str))
			str++;
		else
			return (0);
	}
	return (1);
}

int	too_many_heredoc(t_tok	*toks)
{
	int	cnt;

	cnt = 0;
	while(toks)
	{
		if (toks->type == heredoc)
			cnt++;
		toks = toks->next;
	}
	if (cnt > 16)
	{
		ft_perror("maximum here-document count exceeded", NULL, NULL, 0);
		return (2);
	}
	else
		return (0);
}

void	hd_free_and_exit(t_tok *toks, t_env *envs, int status)
{
	ftok_lstclear(&toks);
	clear_history();
	free_env(envs);
	exit(status);
}

void	scan_and_parse(char *line, t_exp **chunks, t_env *envs) //syntax_error, heredoc
{
	t_tok	*toks;
	int status;

	toks = NULL;
	*chunks = NULL;
	toks = lex(line);
	free(line);
	if (toks == NULL)
		return (EXIT_FAILURE);
	if (validate_syntax(toks))
	{
		ftok_lstclear(&toks);
		set_q(envs, 2);
		return (2);
	}
	if (too_many_heredoc(toks))
		hd_free_and_exit(toks, envs, 2);
	status = handle_heredocs(toks);
	if (status)
	{
		set_q(envs, status);
		ftok_lstclear(&toks);
		return ;
	}
	*chunks = construct(toks, envs);
	if (chunks == NULL)
		set_q(envs, 1);
}
//===============lee======
void	free_env(t_env *env)
{
	t_env	*head;

	while (env)
	{
		head = env;
		env = env->next;
		if (head->key)
			free(head->key);
		if (head->val)
			free(head->val);
		free(head);
	}
}

int	reset_fd(int *save_fd)
{
	int	i;
	int	j;

	//ft_putendl_fd("rest_fd_called", 2);
	i = dup2(save_fd[0], STDIN_FILENO);
	if (i < 0)
		perror("failed to reset stdin");
	close(save_fd[0]);
	j = dup2(save_fd[1], STDOUT_FILENO);
	if (j < 0)
		perror("failed to reset stdout");
	close(save_fd[1]);
	return (i < 0 || j < 0);
}

int	redir_exe_builtin(t_exp *exp, t_env *envs)
{
	// 리다이렉션
	int	save_fd[2];
	save_fd[0] = dup(STDIN_FILENO);
	save_fd[1] = dup(STDOUT_FILENO);
	// pipe create 
	if (check_fd_redirect(exp, STDIN_FILENO, STDOUT_FILENO))
	{
		free_exp_envs(exp, envs);
		ft_putstr_fd("\n\n0error ocurr!\n\n", 2);
		return (1);
	}
	// 빌트인
	if (check_built_in(exp->args[0]) == 0) // 빌트인 이면 0 아니면 1
	{
		do_built_in(exp, &envs, STDOUT_FILENO);// 실패시 1 반환//현재 tmp
		if (ft_strcmp((exp->args)[0], "exit") == 0)
			exit (in_func_exit(exp, envs, save_fd[0], save_fd[1]));
		reset_fd(save_fd);
		return (free_exp_envs(exp, envs), 0);
	}
	else
	{
		if (check_find_path(exp->args, envs))// 실패시 1반환
			return (reset_fd(save_fd), free_exp_envs(exp, envs), 1);
		exec(exp, envs);
		ft_putstr_fd("2exe!  \n", 2);
	}
	reset_fd(save_fd);
	free_exp(exp);
	return (0);
}

int	envs_len(t_env *lst)
{
	int	i;

	i = 0;
	while (lst)
	{
		i++;
		lst = lst->next;
	}
	return (i);
}

char **env_to_array(t_env *lst)
{
	char	**arr;
	char	**cur;

	arr = calloc(sizeof(char *), envs_len(lst) + 1);
	cur = arr;
	while (lst)
	{
		int key_len = ft_strlen(lst->key);
		int val_len = ft_strlen(lst->val);
		*cur = calloc(sizeof(char), key_len + val_len + 2);
		ft_memcpy(*cur, lst->key, key_len);
		ft_memcpy(*cur + key_len, "=", 1);
		ft_memcpy(*cur + key_len + 1, lst->val, val_len);
		cur++;
		lst = lst-> next;
		
	}
	
	return (arr);
}

int exec(t_exp *toks, t_env *envs)
{
	char **envp;
	envp = env_to_array(envs);
	//print_argv(toks->args);
	//print_argv(envp);
	execve(toks->args[0], toks->args, envp);
	perror("execve fail");
	ft_putstr_fd("minishell: ", 2);
	perror(toks->args[0]);
	free_double_ptr(envp);
	return (1);
}


int	pipe_and_fork(t_exp *toks, t_env *envs)
{
	t_exp *cur;
	t_exp *next;
	int pid;
	int status;
	int	pipes[3][2] = {{0, 1}, {0, 1}, {0, 1}};

	pid = 0;
	cur = toks;
	if (cur->next == NULL && cur->args && check_built_in(cur->args[0]) == 0)
	{
		ft_putstr_fd("\n\nonly one cmd\n\n", 2);
		return (redir_exe_builtin(toks, envs));
	}
	while (cur)
	{
		ft_memcpy(pipes[0], pipes[1], sizeof(int) * 2);
		ft_memcpy(pipes[1], pipes[2], sizeof(int) * 2);
		if (pipes[0][1] > 2)
			close(pipes[0][1]);
		if (cur->next)
		{
			if (pipe(pipes[1]))
			{
				if (pipes[0][0] > 2)
					close(pipes[0][0]);
				free_exp_envs(cur, envs);
				ft_putstr_fd("\n\npipe erro\n\n", 2);
				return (1);
			}
		}
		
		pid = fork();
		if (pid == -1)
		{
			if (pipes[0][0] > 2)
				close(pipes[0][0]);
			if (pipes[1][0] > 2)
				close(pipes[1][0]);
			if (pipes[1][1] > 2)
				close(pipes[1][1]);
			free_exp_envs(cur, envs);
			ft_putstr_fd("fork error\n", 2);
			return (1);
		}
		else if (pid == 0)
		{
			if (pipes[1][0] > 2)
				close (pipes[1][0]);
			if (check_fd_redirect(cur, pipes[0][0], pipes[1][1]))
			{
				free_exp_envs(cur, envs);
				ft_putstr_fd("redir error\n", 2);
				exit (1);
			}
			if (check_built_in(cur->args[0]) == 0) // 빌트인 이면 0 아니면 1
			{
				do_built_in(cur, &envs, STDOUT_FILENO);// 실패시 1 반환//현재 tmp
				if (ft_strcmp((cur->args)[0], "exit") == 0)
				{
					close(pipes[0][1]);
					exit (in_func_exit(cur, envs, pipes[0][0], pipes[1][1]));
				}
			}
			else
			{
				if (check_find_path(&cur->args[0], envs))// 실패시 1반환
				{
					free_exp_envs(cur, envs);
					ft_putstr_fd("find_path error\n", 2);
					exit (127);
				}
				exec(cur, envs);
			}
			free_exp_envs(cur, envs);
			exit (0);
		}
		if (pipes[0][0] > 2)
			close(pipes[0][0]);
		next = cur->next;
		cur->next = NULL;
		free_exp(cur);
		cur = next;
	}
	//t_exp와 t_redir을 순회하면서 heredoc_fd들도 닫아야함
	//	free_exp(toks);
	waitpid(pid, &status, 0);
	while(wait(NULL) == 0)
		;
	if (WIFEXITED(status)) {
		return (WEXITSTATUS(status));
	} else if (WIFSIGNALED(status))
		return (WTERMSIG(status));
	return (0);
}

//===============lee======
//============================================================================

char *get_prompt(void)
{
	static const char *prompt = ">";
	if (isatty(0))
		return (prompt);
	else
		return (NULL);
}

int main(int argc, char **argv, char **envp)
{
	(void) argc, (void) argv;
	char	*line;
	t_env	*envs;
	t_exp	*exp;

	envs = NULL;
	if(init_bash(envp, &envs))
		return (1);
	while (1)
	{
		exp = NULL;

		line = readline(get_prompt());
		if (line == NULL)
			break ;
		if (is_all_ws(line))
		{
			free(line);
			continue;
		}
		add_history(line);
		scan_and_parse(line, &exp, envs);
		free(line);
		print_exps(exp);
		pipe_and_fork(exp, envs);
	}
	clear_history();
	free_env(envs);
	return (0);
}
