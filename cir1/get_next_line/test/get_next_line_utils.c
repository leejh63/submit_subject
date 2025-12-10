/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 12:56:12 by marvin            #+#    #+#             */
/*   Updated: 2025/04/25 12:56:12 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

int	get_next_nl(const char *save, char end)
{
	int	i;

	i = -1;
	if (end == '\n')
	{
		if (!save)
			return (-1);
		while (save[++i])
			if (save[i] == '\n')
				return (i);
		return (-1);
	}
	i = 0;
	while (save && save[i])
		i++;
	return (i);
}

char	*get_dup(char *save, int sind)
{
	int		i;
	int		slen;
	char	*dupsave;

	if (!save)
		return (NULL);
	slen = sind + 1;
	if (sind < 0)
		slen = get_next_nl(save, '\0');
	dupsave = (char *)malloc(sizeof(char) * (slen + 1));
	if (!dupsave)
		return (NULL);
	dupsave[slen] = '\0';
	i = -1;
	while (++i < slen)
		dupsave[i] = save[i];
	return (dupsave);
}

char	*get_join(char *save, char *buf)
{
	int		slen;
	int		blen;
	int		i;
	char	*newsave;

	slen = get_next_nl(save, '\0');
	blen = get_next_nl(buf, '\0');
	newsave = (char *)malloc(sizeof(char) * (slen + blen + 1));
	if (!newsave)
		return (NULL);
	newsave[slen + blen] = '\0';
	i = -1;
	while (++i < slen)
		newsave[i] = save[i];
	i = -1;
	while (++i < blen)
		newsave[slen + i] = buf[i];
	free(save);
	return (newsave);
}

char	*get_line_one(char **save, char **buf, int sind)
{
	char	*newsave;
	char	*getline;

	if (!*save || !**save)
		return (get_cln(save, buf));
	getline = get_dup(*save, sind);
	if (!getline)
		return (get_cln(save, buf));
	if (sind < 0)
	{
		get_cln(save, buf);
		return (getline);
	}
	newsave = get_dup(&(*save)[sind + 1], -1);
	if (!newsave)
	{
		free(getline);
		return (get_cln(save, buf));
	}
	get_cln(save, buf);
	*save = newsave;
	return (getline);
}

void	*get_cln(char **cln1, char **cln2)
{
	free(*cln2);
	free(*cln1);
	*cln1 = NULL;
	*cln2 = NULL;
	return (NULL);
}
