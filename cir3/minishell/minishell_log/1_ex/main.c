/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_built_in.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 18:34:59 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/10 18:35:01 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

//아직 하지 않은 것: token to string, unquoting, var extending.

//execve 할 때는 char *envp[]형태로 만들어야 하는데 이렇게 저장하는 게 맞을까?
//일단 임시로 이렇게
typedef	struct s_env
{
	struct s_env	*next;
	char	*key;
	char	*val;
}	t_env;

int main(int argc, char *argv[], char **envp)
{
	char *line;
	int status;
	t_env	*env;
	t_tok	*toks;
	t_exp	*chunks;

	(void)argc;
	(void)argv;
	if(init_bash(envp, &env)) //env, signal, tcset
		exit(2);
	while(1)
	{
		line = readline(">");
		if (line && *line)
		{
			add_history(line);
			continue ;
		}
		line_to_argv(line);

	}
}

t_env	*make_env(char *key, char *val)
{
	t_env	*var;

	var = malloc(sizeof(t_env));
	if (var == NULL)
		return (NULL);
	var->key = key;
	var->val = val;
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

	if (env == 0)
		return (1);
	cur = env;
	while (cur)
	{
		if (ft_strcmp(cur->key, key) == 0)
		{
			free(cur->val);
			cur->val = ft_strdup(val);
			return (0);
		}
	}
	return (1);
}

char	*get_env(t_env_list *env_list, char *key)
{
	t_env	*env;
	t_env	*cur;

	env = env_list->lst;
	if (env == 0)
		return (NULL);
	cur = env;
	while (cur)
	{
		if (ft_strcmp(cur->key, key) == 0)
			return (cur->val);
	}
	if (ft_strcmp(cur->key, "?") == 0)
		return (env_list->last_status);
	return (NULL);
}

//실패시 1이상 반환
int init_bash(char **envp, t_env **lst)
{
	t_env	*var;
	char	*cur;

	if (envp)
	{
		cur = *envp;
		while (cur)
		{
			ft_strchr(cur, '=');
			var = make_env(ft_strndup(cur, ft_strchr(cur, '=') - cur - 1), ft_strdup(ft_strchr(cur, '=') + 1));
			if (var)
				ft_env_add_back(lst, var);
			envp++;
			cur = *envp;
		}
	}
	if (set_env(*lst, ft_strdup("PWD"), getcwd(NULL, 0)))
	{
		var = make_env(ft_strdup("PWD"), getcwd(NULL, 0));
		if (var)
			ft_env_add_back(lst, var);
	}
	int shlvl = ft_atoi(getenv("SHLVL"));
	if (set_env(*lst, "SHLVL", ft_itoa(shlvl + 1)))
	{
		var = make_env(ft_strdup("SHLVL"), ft_itoa(shlvl + 1));
		ft_env_add_back(lst, var);
	}
	return (0);
}

void	pipe_and_fork(t_exp *toks)
{
	t_exp *cur;
	int pid;
	int status;
	int	pipes[2][2] = {{-1, -1}, {-1, -1}};

	pid = 0;
	cur = toks;
	if (do_bulit_in())
		do_builtin(toks);
}

int get_status(int status)
{
	if(status && 0xff)
		return (status && 0xff + 128);
	else
		return (status >>8);
}


//const char *no_fork[] = {"cd", "unset", "exit", "echo"NULL};

void	do_built_in()
{
	save_original_stdfd();
	do_cmd();
	restore_original_stdfd();
}


void	do_cd(t_env *env, char *dst)
{
	char	*pwd;

	if (chdir(dst) == 0)
	{
		pwd = get_env(env, "PWD");
		if (pwd)
			set_env(env, "OLDPWD", pwd);
		set_env(env, "PWD", pwd);
	}
}

int exec(t_exp *tok, t_env_list *env)
{
	char	**arg;
	if (is_builtin(arg))
		do_built_in(arg, env);


}
