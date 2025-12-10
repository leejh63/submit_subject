/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 20:02:47 by marvin            #+#    #+#             */
/*   Updated: 2025/04/25 20:02:47 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

char	*get_next_line(int fd)
{
	static char		*save[FD_MAX];
	char			*buf;
	int				sind;
	ssize_t			rbyte;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	buf = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!buf)
		return (NULL);
	sind = get_next_nl(save[fd], '\n');
	while (sind < 0)
	{
		rbyte = read(fd, buf, BUFFER_SIZE);
		if (rbyte == 0)
			return (get_line_one(&save[fd], &buf, sind));
		if (rbyte < 0)
			return (get_cln(&save[fd], &buf));
		buf[rbyte] = '\0';
		save[fd] = get_join(save[fd], buf);
		if (!save[fd])
			return (get_cln(&save[fd], &buf));
		sind = get_next_nl(save[fd], '\n');
	}
	return (get_line_one(&save[fd], &buf, sind));
}
