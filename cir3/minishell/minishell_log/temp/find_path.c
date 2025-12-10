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

void	free_split(char **split)
{
	int	i;

	i = 0;
	while (split[i])
		free(split[i++]);
	free(split);
}

int	find_path_in_env(t_env *env, char **argv)
{
	char	**colon;
	char	*path;
	char	*access_path;
	int	i;

	i = 0;
	path = get_env(env, "PATH");//여기 path는 환경변수 리스트에 있다 함부로 동적할당 하면 안됨
	if (!path)
		return (perror("find_path_in_env, no PATH in env!"), 1);
	colon = ft_split(path, ':');
	if (!colon)
		return (perror("find_path_in_env, ft_split wrong!"), 1);
	while (colon[i])
	{
		//printf("colon[%d]: %s\n", i, colon[i]);
		path = ft_strjoin(colon[i], "/");
		if (!path)
			return (free_split(colon), perror("find_path_in_env, path join"), 1);
		access_path = ft_strjoin(path, argv[0]);
		if (!access_path)
			return (free_split(colon), free(path), perror("find_path_in_env, access_path join"), 1);
		free(path);
		if (!access(access_path, F_OK))
		{
			//printf("find %s path: %s\n", argv[0], colon[i]);
			free(argv[0]);
			argv[0] = access_path;
			return (free_split(colon), 0);// access_path는 이제 인자 해제 하면안됨
		}
		free(access_path);
		i++;
	}
	return (free_split(colon), perror("1command not found"), 1);
}

int	check_find_path(char **argv, t_env *env)
{
	(void) env;
	char	*pwd;
	char	*make_path;

	if (ft_strchr(argv[0], '/') == 0)
	{
		if (!find_path_in_env(env, argv))
		{
			//printf("find path in env: %s\nyou must check the access permit\n", argv[0]);
			return (0);
		}
		else
			return (1);
			
	}
	else
	{
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
			
			//printf("find relativity ");//
		}
		//else
			//printf("find absolute ");//
		if (!access(argv[0], F_OK))
		{
			//printf("path: %s!\nyou must check the access permit\n", argv[0]);
		}
		else
			return (perror("2command not found"), 1);
	}
	return (0);
}

































