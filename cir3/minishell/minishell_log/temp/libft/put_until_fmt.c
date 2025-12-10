/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   put_until_fmt.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 16:36:20 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:47 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	put_until_fmt(char const **str)
{
	int	byte_size;

	byte_size = 0;
	while (**str != '\0' && **str != '%')
	{
		write(1, *(char **)str, 1);
		*str += 1;
		byte_size += 1;
	}
	return (byte_size);
}
