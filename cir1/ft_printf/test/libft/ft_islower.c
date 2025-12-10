/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_islower.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 13:03:11 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/01 13:03:18 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

int	ft_islower(int str)
{
	unsigned char	unstr;

	unstr = (unsigned char)str;
	return (unstr <= 'z' && unstr >= 'a');
}
