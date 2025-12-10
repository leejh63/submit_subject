#include "minishell.h"

t_tok	*prev_tok(t_tok *head, t_tok *this)
{
	t_tok	*cur;

	if (head == NULL || this == NULL || head == this)
		return (NULL);
	cur = head;
	while (cur->next)
	{
		if (cur->next == this)
			return (cur);
		cur = cur->next;
	}
	return (NULL);
}

t_tok	*get_wstok(void)
{
	t_tok	*tok;

	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->hd_fd = -1;
	tok->type = ws;
	return (tok);
}

void	get_one_chunk(t_tok **tok, t_tok **chunk)
{
	t_tok	*head;
	t_tok	*cur;
	t_tok	*next;
	t_tok	*prev;

	cur = *tok;
	while (cur->type == ws)
	{
		next = cur->next;
		cur->next = NULL;
		del_tok(cur);
		cur = next;
	}
	head = cur;
	while (cur)
	{
		if ((cur->type >= pip && cur->type <=heredoc) || cur->type == ws || cur->type == end)
		{
			if (head == cur)
				*chunk = NULL;
			else
				*chunk = head;
			*tok = cur;
			prev = prev_tok(head, cur);
			if (prev)
				prev->next = NULL;
			return ;
		}
		if (cur->type == word || cur->type == env)
		{
			cur = cur->next;
			continue ;
		}
		else if (cur->type == sq)
		{
			cur = cur->next;
			while(cur->type != sq)
				cur = cur->next;
			cur = cur->next;
		}
		else if (cur->type == dq)
		{
			cur = cur->next;
			while(cur->type != dq)
				cur = cur->next;
			cur = cur->next;
		}
	}
}

char **unquote_and_expand_param(t_tok *toks, t_env *envs)
{
	t_word	*words = NULL;
	t_word	*cur = NULL;
	char	**ret;

	word_next(&words, &cur);
	while(toks)
	{
		if (toks->type == sq)
		{
			toks = toks->next;
			while(toks->type != sq)
			{
				word_append(cur, toks->str);
				toks = toks->next;
			}
		}
		else if(toks->type == dq)
		{
			toks = toks->next;
			while(toks->type != dq)
			{
				if (toks->type == env)
				{
					word_append(cur, get_env(envs, toks->str + 1));//TODO
				}
				else
					word_append(cur, toks->str);
				toks = toks->next;
			}
		}
		else if(toks->type == env)
		{
			char *str = get_env(envs, toks->str + 1);
			while (*str)
			{
				if (is_whitespace(*str))
				{
					while(is_whitespace(*str) && *str)
						str++;
					word_next(&words, &cur);
				}
				else
				{
					int i;
					i = 0;
					while(!is_whitespace(str[i]) && str[i])
						i++;
					char c = str[i];
					str[i] = '\0';
					word_append(cur, str);
					str[i] = c;
					str = str + i;
				}
			}
		}
		else if (toks->type == ws)
			word_next(&words, &cur);
		else
			word_append(cur, toks->str);
		toks = toks->next;
	}
	if (cur->str)
		word_next(&words, &cur);
	ret = word_to_str(words);
	return (ret);
}

t_exp	*construct(t_tok *tokens, t_env *envs)
{
	t_tok	*cur;
	t_tok	*next;
	t_exp	*head;
	t_exp	*node;
	t_redir	*redir;

	cur = tokens;
	head = calloc(sizeof(t_exp), 1);
	node = head;
	while (cur && cur->type < end)
	{
		if (cur->type == ws)
		{
			next = cur->next;
			cur->next = NULL;
			if (node->cmd && ftok_lstlast(node->cmd)->type != ws)
				ftok_lstadd_back(&node->cmd, cur);
			else
				del_tok(cur);
			cur = next;
		}
		else if (cur->type == pip)
		{
			node->args = unquote_and_expand_param(node->cmd, envs);
			ftok_lstclear(&node->cmd);
			node = calloc(sizeof(t_exp), 1);
			fexp_lstadd_back(&head, node);
			next = cur->next;
			cur->next = NULL;
			del_tok(cur);
			cur = next;
		}
		else if (cur->type == inrd)
		{
			next = cur->next;
			cur->next = NULL;
			del_tok(cur);
			cur = next;
			redir = calloc(sizeof(t_redir), 1);
			redir->type = inrd;
			ft_redir_lstadd_back(&node->redir, redir);
			get_one_chunk(&cur, &redir->str);
			redir->sstr = unquote_and_expand_param(redir->str, envs);
			ftok_lstclear(&redir->str);
			if (node->cmd == NULL || ftok_lstlast(node->cmd)->type != ws)
				ftok_lstadd_back(&node->cmd, get_wstok());
		}
		else if (cur->type == outrd)
		{
			next = cur->next;
			cur->next = NULL;
			del_tok(cur);
			cur = next;
			redir = calloc(sizeof(t_redir), 1);
			redir->type = outrd;
			ft_redir_lstadd_back(&node->redir, redir);
			get_one_chunk(&cur, &redir->str);
			redir->sstr = unquote_and_expand_param(redir->str, envs);
			ftok_lstclear(&redir->str);
			if (node->cmd == NULL || ftok_lstlast(node->cmd)->type != ws)
				ftok_lstadd_back(&node->cmd, get_wstok());
		}
		else if (cur->type == aprd)
		{
			next = cur->next;
			cur->next = NULL;
			del_tok(cur);
			cur = next;
			redir = calloc(sizeof(t_redir), 1);
			redir->type = aprd;
			ft_redir_lstadd_back(&node->redir, redir);
			get_one_chunk(&cur, &redir->str);
			redir->sstr = unquote_and_expand_param(redir->str, envs);
			ftok_lstclear(&redir->str);
			if (node->cmd == NULL || ftok_lstlast(node->cmd)->type != ws)
				ftok_lstadd_back(&node->cmd, get_wstok());
		}
		else if (cur->type == heredoc)
		{
			int fd = cur->hd_fd;
			next = cur->next;
			cur->next = NULL;
			del_tok(cur);
			cur = next;
			redir = calloc(sizeof(t_redir), 1);
			redir->type = heredoc;
			redir->hd_fd = fd;
			ft_redir_lstadd_back(&node->redir, redir);
			//get_one_chunk(&cur, &redir->str); //아마 없어야 할건데 도대체 그 동안 어케 작동한거임?....
			if (node->cmd == NULL || ftok_lstlast(node->cmd)->type != ws)
				ftok_lstadd_back(&node->cmd, get_wstok());
		}
		else if (cur->type == word | cur->type == sq | cur->type == dq | cur->type == env)
		{
			get_one_chunk(&cur, &next);
			ftok_lstadd_back(&node->cmd, next);
		}
	}
	if (cur->type == end)
		del_tok(cur);
	node->args = unquote_and_expand_param(node->cmd, envs);
	ftok_lstclear(&node->cmd);
	return (head);
}
