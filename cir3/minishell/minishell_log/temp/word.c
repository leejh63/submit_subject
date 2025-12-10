#include "minishell.h"

char **word_to_str(t_word *words)
{
	int i = word_lstsize(words);
	char **ret = calloc(sizeof(char *), i);
	t_word	*next;
	i = 0;
	while (words)
	{
		next = words->next;
		ret[i] = words->str;
		i++;
		free(words);
		words = next;
	}
	return (ret);
}

int	word_lstsize(t_word *words)
{
	int i = 0;
	if (words == NULL)
		return (0);
	while (words)
	{
		i++;
		words = words->next;
	}
	return (i);
}

int	word_append(t_word *cur, char *str)
{
	char *joined;

	joined = ft_strjoin(cur->str, str);
	if (joined == NULL)
		return (1);
	free(cur->str);
	cur->str = joined;
	return (0);
}

void	word_next(t_word **words, t_word **cur)
{
	if (*cur == NULL)
	{
		*cur = calloc(sizeof(t_word), 1);
		*words = *cur;
	}
	else if ((*cur)->str == NULL)
		return ;
	else
	{
		(*cur)->next = calloc(sizeof(t_word), 1);
		*cur = (*cur)->next;
	}
}

