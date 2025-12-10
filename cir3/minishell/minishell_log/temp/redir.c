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

void	ft_redir_lstadd_back(t_redir **lst, t_redir *new)
{
	t_redir	*cursor;

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
/*
void	ftok_lstdelone(t_tok *lst)
{
	free(lst->str);
	free(lst);
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
		ftok_lstdelone(n1);
		n1 = n2;
	}
	*lst = 0;
}

*/
