/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 17:47:17 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/01 21:34:13 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#define BUFFER_SIZE 1024

int	ft_strspn(char *s, char c)
{
	int	i;

	if (!s)
		return (-1);
	i = 0;
	while (s[i])
	{
		if (s[i] == c)
			return (i);
		i++;
	}
	if (c == 0)
		return (i);
	return (-1);
}

char	*ft_strmdup(char *s, int n)
{
	int		len;
	char	*ptr;

	ptr = (char *)malloc(n + 1);
	if (!ptr)
		return (NULL);
	len = ft_strspn(s, '\0');
	if (n < len)
		len = n;
	ft_memcpy(ptr, s, len);
	while (len < n + 1)
		ptr[len++] = '\0';
	return (ptr);
}

void	free_set_null(char **buf, int check_empty)
{
	if (check_empty)
	{
		if (*buf && **buf == '\0')
		{
			free(*buf);
			*buf = 0;
		}
	}
	else
	{
		if (*buf)
		{
			free(*buf);
			*buf = 0;
		}
	}
}

int	split(char **buf, char **ret)
{
	char	*nret;
	int		size;
	int		ret_size;

	if (!*buf)
		*buf = ft_strmdup("", BUFFER_SIZE);
	if (!*buf)
		return (0);
	if (!*ret)
		*ret = ft_strmdup("", 0);
	if (!*ret)
		return (0);
	ret_size = ft_strspn(*ret, '\0');
	size = ft_strspn(*buf, '\n') + 1;
	if (size == 0)
		size = ft_strspn(*buf, '\0');
	nret = ft_strmdup(*ret, ret_size + size);
	if (!nret)
		return (free_set_null(buf, 0), free_set_null(ret, 0), 0);
	if (free(*ret), ft_memcpy(nret + ret_size, *buf, size), size > 0)
		ft_memcpy(*buf, *buf + size, BUFFER_SIZE - size + 1);
	else
		**buf = '\0';
	*ret = nret;
	return (0);
}

char	*get_next_line(int fd)
{
	ssize_t		size;
	static char	*buf = 0;
	char		*ret;

	if (fd < 0 || BUFFER_SIZE < 1)
		return (free_set_null(&buf, 0), NULL);
	ret = 0;
	split(&buf, &ret);
	if (!ret || !buf)
		return (free_set_null(&buf, 0), NULL);
	while (ft_strspn(ret, '\n') < 0)
	{
		size = read(fd, buf, BUFFER_SIZE);
		if (size < 0)
			return (free(ret), free_set_null(&buf, 0), NULL);
		if (size == 0 && *ret)
			break ;
		buf[size] = 0;
		split(&buf, &ret);
		if (!ret)
			return (free_set_null(&buf, 0), NULL);
		if (*ret == 0 && size < 1)
			return (free(ret), free_set_null(&buf, 0), NULL);
	}
	return (free_set_null(&buf, 1), ret);
}

/*
#include <fcntl.h>
#include <stdio.h>

int	main(void)
{
	int		fd;
	char	*ptr;

	fd = open("giant_line.txt", O_RDONLY);
	while ((ptr = get_next_line(fd)))
	{
		write(1, ptr, ft_strchr(ptr, '\0'));
		write(1, "$", 1);
		free(ptr);
	}
	close(fd);
	return (0);
}*/
