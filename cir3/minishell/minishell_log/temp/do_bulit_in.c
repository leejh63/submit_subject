/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_bulit_in.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 19:09:09 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/10 19:09:10 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

//----------------------print----

void print_env_list(t_env *head)
{
    t_env *cur = head;
    while (cur)
    {
        printf("\nkey:%s \nval :%s\n",
               (cur->key && *cur->key) ? cur->key : "(null)",
               (cur->val && *cur->val) ? cur->val : "(null)");
        cur = cur->next;
    }
}

void print_argv(char **argv)
{
    for (int i = 0; argv[i] != NULL; ++i)
        printf("argv[%d] = %s\n", i, argv[i]);
}

//---------------------------------------------
t_env	*make_env(char *key, char *val)
{
	t_env	*var;

	var = malloc(sizeof(t_env));
	if (var == NULL)
		return (NULL);
	var->key = key;
	var->val = val;
	var->next = NULL;
	return (var);
}

void	ft_env_add_back(t_env **lst, t_env *new_node)
{
	t_env	*current;

	if (!*lst)
	{
		*lst = new_node;
		return ;
	}
	current = *lst;
	while (current->next)
		current = current->next;
	current->next = new_node;
	
}

int	set_env(t_env *env, char *key, char *val)
{
	t_env	*cur;
	(void) val;
	if (env == 0)
		return (1);
	cur = env;
	while (cur)
	{
		if (ft_strcmp(cur->key, key) == 0)
		{
			if (cur->val)
				free(cur->val);
			cur->val = ft_strdup(val);
			if (cur->val)
				return (1);
			return (0);
		}
		cur = cur->next;
	}
	return (1);
}

int	find_equal(char *env_str, int	*equal)
{
	int	i;

	i = 0;
	while (env_str[i])
	{
		if (env_str[i] == '=')
		{
			*equal = i;
			return (1);
		}
		i++;
	}
	*equal = 0;
	return (0);
}

char	*take_env_key(int equal, char *envp)
{
	if (equal)
		return (ft_strndup(envp, equal));
	else
		return (ft_strndup(envp, ft_strlen(envp)));
}


char	*get_env(t_env *envs, char *key)
{
	while (envs)
	{
		if (ft_strcmp(envs->key, key) == 0)
			return (envs->val);
		envs = envs->next;
	}
	return (NULL);
}

void	set_q(t_env *lst, int i)
{
	char *val;
	int	divisor;

	val = get_env(lst, "?");
	if (!val)
		return ;
	i = i & 0xff;
	/*if (i == 0) //없어도 될 듯?
	{
		ft_memcpy(val, "0", 2);
		return ;
	}*/
	divisor = 1;
	while (divisor < i)
		divisor *= 10;
	while(divisor)
	{
		*val = i / divisor + '0';
		i = i % divisor;
		divisor = divisor / 10;
		val++;
	}
	*val = '\0';
}

int	get_q(t_env *lst)
{
	return (ft_atoi(get_env(lst, "?")));
}

int	make_first_env(t_env **lst)
{
	t_env	*var;
	char	*key;
	char	*val;

	key = ft_strdup("?");
	if (key == NULL)
		return (1);
	val = ft_calloc(1, 4);
	if (val == NULL)
		return (1);
	val[0] = '0';
	var = make_env(key, val);
	if (var == NULL)
		return (1);
	ft_env_add_back(lst, var);
	return (0);
}

//실패시 1이상 반환
int init_bash(char **envp, t_env **lst)
{
	t_env	*var;
	int	equal;
	int	i;
	int shlvl;
	char	*key;
	char	*val;

	set_signal_handler();
	i = 0;
	if (make_first_env(lst))
		return (1);
	if (envp)
	{
		while (envp[i])
		{
			if (find_equal(envp[i], &equal))
			{
				val = ft_strndup(&envp[i][equal + 1], ft_strlen(&envp[i][equal + 1]));
				if (val == NULL)
					return (1);
			}
			else
				val = NULL;
			key = take_env_key(equal, envp[i]);
			if (key == NULL)
				return (1);
			var = make_env(key, val);
			if (!var)
				return (1);
			ft_env_add_back(lst, var);
			i++;
		}
	}
	
	val = get_env(*lst, "PWD");
	if (!val)
	{
		key = getcwd(NULL, 0);
		var = make_env("PWD", key);
		if (var)
			ft_env_add_back(lst, var);
		free(key);
	}
	val = get_env(*lst, "SHLVL");
	if (!val)
	{
		val = ft_itoa(1);
		if (!val)
			return (free(val), 1);
		var = make_env("SHLVL", val);
		if (var)
			ft_env_add_back(lst, var);
	}
	else
	{
		shlvl = ft_atoi(val) + 1;
		val = ft_itoa(shlvl * (!(shlvl < 0)));
		if (!val)
			return (1);
		set_env(*lst, "SHLVL", val);
		free(val);
	}
	return (0);
}

int	argv_len(char **argv)
{
	int	i;

	i = 0;
	while (argv[i])
		i++;
	return (i);
}

int	in_func_env(char ** argv, t_env *envs, int out_fd)
{
	int	argc;

	argc = argv_len(argv);
	if (argc != 1)
		return (printf("\nenv wrong args!\n"), 1);
	while (envs)
	{
		if (envs->val)
		{
			write(out_fd, envs->key, ft_strlen(envs->key));
			write(out_fd, "=", 1);
			write(out_fd, envs->val, ft_strlen(envs->val));
			write(out_fd, "\n", 1);
		}
		envs = envs->next;
	}
	return (0);
}

int	in_func_pwd(char **argv, t_env *envs, int out_fd)
{
	(void) envs;
	int	argc;
	char	*pwd;

	argc = argv_len(argv);
	if (argc != 1)
		return (printf("\npwd wrong args!\n"), 1);
	pwd = getcwd(NULL, 0);
	ft_putendl_fd(pwd, out_fd);
	free(pwd);
	return (0);
}

int	in_func_echo(char **argv, t_env *envs, int out_fd)
{
	(void) envs;
	int	argc;
	int	i;
	
	i = 1;
	argc = argv_len(argv) - 1;
	if (argc == 0)
		return (write(out_fd, "\n", 1), 0);
	if (ft_strcmp(argv[1], "-n") == 0)
	{
		i = 2;
		while (argv[i])
		{
			write(out_fd, argv[i], ft_strlen(argv[i]));
			if (argc != i)
				write(out_fd, " ", 1);
			i++;
		}
	}
	else
	{
		while (argv[i])
		{
			write(out_fd, argv[i], ft_strlen(argv[i]));
			if (argc != i)
				write(out_fd, " ", 1);
			i++;
		}
		write(out_fd, "\n", 1);
	}
	return (0);
}

int	in_func_unset(char **argv, t_env **envs, int out_fd)
{
	int	argc;
	t_env	*head;
	t_env	*pre;

	print_env_list(*envs);
	argc = argv_len(argv);
	if (argc == 1)
		return (write(out_fd, "", 0), 1);
	while (--argc)
	{
		head = *envs;
		pre = *envs;
		while (head)
		{
			if (ft_strcmp(head->key, argv[argc]) == 0)
			{
				if (head == pre)
					(*envs) = head->next;
				else
					pre->next = head->next;
				if (head->val)
					free(head->val);
				if (head->key)
					free(head->key);
				free(head);
				break ;
					
			}
			pre = head;
			head = head->next;
		}
	}
	print_env_list(*envs);
	return (0);
}

void	sorting_env(t_env *envs)
{
	char	*key;
	char	*val;
	t_env	*i;
	t_env	*j;

	while (envs)
	{
		i = envs;
		j = envs->next;
		while (j)
		{
			if (ft_strcmp(i->key, j->key) > 0)
			{
				key = i->key;
				val = i->val;
				i->key = j->key;
				i->val = j->val;
				j->key = key;
				j->val = val;
			}
			j = j->next;
		}
		envs = envs->next;
	}
}

void	write_export(t_env *envs, int out_fd)
{
	print_env_list(envs);
	t_env	*sort_envs;

	sort_envs = envs;
	sorting_env(sort_envs);
	while (sort_envs)
	{
		write(out_fd, "declare -x ", 11);
		write(out_fd, sort_envs->key, ft_strlen(sort_envs->key));
		if (sort_envs->val)
		{
			write(out_fd, "=", 1);
			write(out_fd, sort_envs->val, ft_strlen(sort_envs->val));
		}
		write(out_fd, "\n", 1);
		sort_envs = sort_envs->next;
	}
}

int	in_func_export(char **argv, t_env **envs, int out_fd)
{
	int	i;
	int	equal;
	t_env *var;
	char	*val;
	char	*key;

	if (argv[1] == NULL)
		return (write_export(*envs, out_fd), 0);
	print_env_list(*envs);
	i = 1;
	while (argv[i])
	{
		if (find_equal(argv[i], &equal))
		{
			val = ft_strndup(&argv[i][equal + 1], ft_strlen(&argv[i][equal + 1]));
			if (val == NULL)
				return (1);
		}
		else
			val = NULL;
		key = take_env_key(equal, argv[i]);
		if (key == NULL)
			return (1);
		var = make_env(key, val);
		if (!var)
			return (1);
		ft_env_add_back(envs, var);
		i++;
	}
	print_env_list(*envs);
	return (0);
}

int	is_numeric(const char *nptr)
{
	while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r')
		nptr++;
	if (*nptr == '-' || *nptr == '+')
		nptr++;
	if (*nptr < '0'|| *nptr > '9')
		return (0);
	while (*nptr >= '0'&& *nptr <= '9')
		nptr++;
	while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r')
		nptr++;
	if (*nptr == '\0')
		return (1);
	else
		return (0);
}

int	in_range(const char *nptr)
{
	static const char	*limit[] = {"9223372036854775807", "9223372036854775808"};
	int					len;
	int					sign;

	while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r')
		nptr++;
	sign = 0;
	if (*nptr == '-')
		sign = 1;
	if (*nptr == '-' || *nptr == '+')
		nptr++;
	len = 0;
	while (nptr[len] >= '0'&& nptr[len] <= '9')
		len++;
	if (len < 20)
		return (1);
	else if (len > 20)
		return (0);
	if (ft_strncmp(nptr, limit[sign], 20) <= 0)
		return (0);
	else
		return (1);
}

int	tmp_atoi(const char *nptr)
{
	long long	ret;
	int		sign;

	ret = 0;
	sign = 1;
	if (*nptr == '-')
	{
		sign = -1;
		nptr++;
		if (!*nptr)
			return (-1);
	}
	while (*nptr >= '0' && *nptr <= '9')
	{
		ret *= 10;
		ret += *nptr - '0';
		nptr++;
	}
	ret = ret * sign;
	return (ret & 0xff);
}

int	in_func_exit(t_exp *exp, t_env *envs, int in_fd, int out_fd)
{
	int	argc;
	int	exit_num;

	argc = argv_len(exp->args);
	if (argc > 2)
	{
		ft_putstr_fd("bash: exit: ", 2);
		ft_putstr_fd(exp->args[1], 2);
		ft_putstr_fd(": Too many args!\n", 2);
		free_exp_envs(exp, envs);
		close(in_fd);
		close(out_fd);
		return (1);
	}
	if (argc == 1)
		exit_num = 0;
	else if (!is_numeric(exp->args[1]) || !in_range(exp->args[1]))
	{
		ft_putstr_fd("bash: exit: ", 2);
		ft_putstr_fd(exp->args[1], 2);
		ft_putstr_fd(": numeric argument required\n", 2);
		exit_num = 2;
	}
	else
	{
		exit_num = tmp_atoi(exp->args[1]);
	}
	//free_all();	//TODO
	free_exp_envs(exp, envs);
	close(in_fd);
	close(out_fd);
	return (exit_num);
}

int	in_func_cd(char **args, t_env *envs, int out_fd)
{
	(void) out_fd;
	char	*now_path;
	char	*pwd;
	int	argc;

	argc = argv_len(args);
	if (argc > 2)
	{
		printf("\n\ntoo many argvs\n\n");
		return (1);
	}
	if (argc == 1)
		return (0);
	else
		now_path = args[1];
	if (chdir(now_path) == 0)
	{
		pwd = get_env(envs, "PWD");
		if (pwd)
			set_env(envs, "OLDPWD", pwd);
		printf("pre path!: =%s=\n\n", pwd);
		now_path = getcwd(NULL, 0);
		if (!now_path)
		{
			perror("getcwd, in_func_cd");
			return (1);
		}
		printf("now path!: =%s=\n\n", now_path);
		free(now_path);
		return (0);
	}
	else
	{
		perror("chdir, in_func_cd");
		return (1);
	}
}


int	check_built_in(char *args)
{
	if (ft_strcmp(args, "env") == 0)
		return (0);
	else if (ft_strcmp(args, "pwd") == 0)
		return (0);
	else if (ft_strcmp(args, "echo") == 0)
		return (0);
	else if (ft_strcmp(args, "unset") == 0)
		return (0);
	else if (ft_strcmp(args, "export") == 0)
		return (0);
	else if (ft_strcmp(args, "exit") == 0)
		return (0);
	else if (ft_strcmp(args, "cd") == 0)
		return (0);
	else
		return (1);
}


int	do_built_in(t_exp *exp, t_env **envs, int out_fd)
{
	//print_argv(argv);
	//print_env_list(envs);
	if (ft_strcmp((exp->args)[0], "env") == 0)
		return (in_func_env(exp->args, *envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "pwd") == 0)
		return (in_func_pwd(exp->args, *envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "echo") == 0)
		return (in_func_echo(exp->args, *envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "unset") == 0)
		return (in_func_unset(exp->args, envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "export") == 0)
		return (in_func_export(exp->args, envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "cd") == 0)
		return (in_func_cd(exp->args, *envs, out_fd));
	else
		return (1);
}





























