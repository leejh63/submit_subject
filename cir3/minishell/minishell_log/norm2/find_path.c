/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find_path.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 16:29:49 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/11 16:29:50 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#define NO_PATH "Command not found: No such file or directory\n"

void	free_split(char **split)
{
	int	i;

	i = 0;
	while (split[i])
		free(split[i++]);
	free(split);
}

int	get_spilt_colon(t_env *env, char ***colon)
{
	char	*path;

	path = get_env(env, "PATH");
	if (!path)
		return (ft_putstr_fd("get_env error\n", 2), 1);
	*colon = ft_split(path, ':');
	if (!colon)
		return (perror("ft_split"), 1);
	return (0);
}

int	join_path(char *colon, char **access_path, char *argv)
{
	char	*path;

	path = ft_strjoin(colon, "/");
	if (!path)
		return (perror("ft_strjoin"), 1);
	*access_path = ft_strjoin(path, argv);
	free(path);
	if (!access_path)
		return (perror("ft_strjoin"), 1);
	return (0);
}

int	find_path_in_env(t_env *env, char **argv)
{
	char	**colon;
	char	*access_path;
	int		i;

	i = 0;
	if (get_spilt_colon(env, &colon))
		return (1);
	while (colon[i])
	{
		if (join_path(colon[i], &access_path, argv[0]))
			return (free_split(colon), 1);
		if (!access(access_path, F_OK))
		{
			free(argv[0]);
			argv[0] = access_path;
			return (free_split(colon), 0);
		}
		free(access_path);
		i++;
	}
	return (free_split(colon), ft_putstr_fd(NO_PATH, 2), 1);
}

int	check_find_path(char **argv, t_env *env)
{
	char	*pwd;
	char	*make_path;

	if (ft_strchr(argv[0], '/') == 0)
		if (find_path_in_env(env, argv))
			return (1);
	if (argv[0][0] != '/')
	{
		pwd = getcwd(NULL, 0);
		if (!pwd)
			return (perror("check_find_path, pwd"), 1);
		make_path = ft_strjoin(pwd, "/");
		if (!make_path)
			return (free(pwd), perror("check_find_path, make_path join"), 1);
		free(pwd);
		pwd = ft_strjoin(make_path, argv[0]);
		if (!pwd)
			return (free(make_path), perror("check_find_path, pwd"), 1);
		free(make_path);
		free(argv[0]);
		argv[0] = pwd;
	}
	if (access(argv[0], F_OK))
		return (perror("Command not found"), 1);
	return (0);
}
