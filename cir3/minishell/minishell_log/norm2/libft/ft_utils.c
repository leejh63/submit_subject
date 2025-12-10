/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:43:23 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:43 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

char	*tenary(int condition, char *s1, char *s2)
{
	if (condition)
		return (s1);
	else
		return (s2);
}

unsigned long long	ft_abs(int n)
{
	long long	llong;

	llong = n;
	if (llong < 0)
		llong *= -1;
	return ((unsigned long long) llong);
}

int	max(int n1, int n2)
{
	if (n1 >= n2)
		return (n1);
	return (n2);
}

int	min(int n1, int n2)
{
	if (n1 <= n2)
		return (n1);
	return (n2);
}
