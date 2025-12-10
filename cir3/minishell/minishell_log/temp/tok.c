#include "minishell.h"

/*t_tok	*ftok_lstnew(char *str, t_tt type)
{
	t_tok	*ptr;

	if (str == NULL)
		return (NULL);
	ptr = (t_tok *)malloc(sizeof(t_tok));
	if (!ptr)
		return (0);
	ptr->str = str;
	ptr->type = type;
	ptr->next = 0;
	return (ptr);
}*/

void	ftok_lstadd_back(t_tok **lst, t_tok *new)
{
	t_tok	*cursor;

	if (lst == 0 || new == 0)
		return ;
	if (*lst == 0)
		*lst = new;
	else
	{
		cursor = *lst;
		while (cursor->next)
			cursor = cursor->next;
		cursor->next = new;
	}
}

void	del_tok(t_tok *tok)
{
	free(tok->str);
	free(tok);
}

void	ftok_lstclear(t_tok **lst)
{
	t_tok	*n1;
	t_tok	*n2;

	if (lst == 0)
		return ;
	n1 = *lst;
	while (n1)
	{
		n2 = n1->next;
		del_tok(n1);
		n1 = n2;
	}
	*lst = 0;
}

t_tok	*ftok_lstlast(t_tok *lst)
{
	if (lst == NULL)
		return (NULL);
	while (lst->next)
	{
		lst = lst->next;
	}
	return (lst);
}