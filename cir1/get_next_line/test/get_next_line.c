/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 11:26:24 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/21 11:26:25 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

char	*get_next_line(int fd)
{
	static char		*save;
	char			*buf;
	int				sind;
	ssize_t			rbyte;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	buf = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!buf)
		return (NULL);
	sind = get_next_nl(save, '\n');
	while (sind < 0)
	{
		rbyte = read(fd, buf, BUFFER_SIZE);
		if (rbyte == 0)
			return (get_line_one(&save, &buf, sind));
		if (rbyte < 0)
			return (get_cln(&save, &buf));
		buf[rbyte] = '\0';
		save = get_join(save, buf);
		if (!save)
			return (get_cln(&save, &buf));
		sind = get_next_nl(save, '\n');
	}
	return (get_line_one(&save, &buf, sind));
}
