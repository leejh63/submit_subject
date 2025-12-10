/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:47:06 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:47:17 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	print_dptr(char **str)
{
	if (str == NULL)
	{
		printf("NULL pointer\n");
		return ;
	}
	while (*str)
	{
		printf("%s", *str);
		str++;
		if (*str)
			printf(", ");
	}
	printf("\n");
}

void	print_redir(t_redir *tcur)
{
	char	c;

	while (tcur)
	{
		if (tcur->type == inrd)
			c = 'i';
		else if (tcur->type == outrd)
			c = 'o';
		else if (tcur->type == aprd)
			c = 'a';
		else if (tcur->type == heredoc)
			c = 'h';
		else
			c = 'X';
		printf("%c:", c);
		print_dptr(tcur->sstr);
		tcur = tcur->next;
	}
	printf("\n");
}

void	print_exps(t_exp *exp)
{
	while (exp)
	{
		printf("=================================\n");
		print_dptr(exp->args);
		print_redir(exp->redir);
		printf("=================================\n");
		exp = exp->next;
	}
}
