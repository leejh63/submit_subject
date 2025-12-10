/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   push_swap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/22 15:17:55 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/05/30 12:07:30 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include "ft_printf.h"
#include "libft.h"

typedef struct s_stack
{
	struct s_stack	*next;
	struct s_stack	*last;
	long long			num;
}	t_stack;


void	tmp_sort_logic(t_stack **ahead, t_stack **bhead, int asize);
void	tmp_fill_btable(long long *btable, t_stack *bhead);
void	tmp_init_bstack(t_stack **ahead, t_stack **bhead, int pivot);
void	laststack(char ab, t_stack *head);
void	pfstack(char ab, t_stack *head);
void	rotate(t_stack **head);
void	rrotate(t_stack **head);
void	ss(t_stack *ahead, t_stack *bhead);
void	push(t_stack **start, t_stack **dest);
void	swap(t_stack *head);
t_stack	*detachb(t_stack **head);
t_stack	*detachf(t_stack **head);
void	addb(t_stack **head, t_stack *new);
void	addf(t_stack **head, t_stack *new);
void	freestack(t_stack **ahead, t_stack **bhead);
t_stack	*new_num(long long num);

//-----------------스택 리스트----------------

t_stack	*new_num(long long num)
{
	t_stack	*new;

	new = malloc(sizeof(t_stack));
	if (!new)
		return (NULL);
	new -> next = NULL;
	new -> last = new;
	new -> num = num;
	return (new);
}

void	freestack(t_stack **ahead, t_stack **bhead)
{
	//널처리
	t_stack	*tmp;

	if (*ahead)
	{
		while (*ahead)
		{
			tmp = (*ahead) -> next;
			free(*ahead);
			*ahead = tmp;
		}
	}
	if (*bhead)
	{
		while (*bhead)
		{
			tmp = (*bhead) -> next;
			free(*bhead);
			*bhead = tmp;
		}
	}
	*ahead = NULL;
	*ahead = NULL;
}

static void	free_all(char **words, unsigned int ind)
{
	while (ind--)
		free(words[ind]);
	free(words);
}

void	addf(t_stack **head, t_stack *new)
{
	//널처리
	if (!*head)
	{
		new -> next = NULL;
		new -> last = new;
		*head = new;
		return ;
	}
	new -> next = *head;
	new -> last = (*head) -> last;
	*head = new;
	return ;
}

void	addb(t_stack **head, t_stack *new)
{
	//널처리
	t_stack	*tmp;

	if (!*head)
	{
		new -> next = NULL;
		new -> last = new;
		*head = new;
		return ;
	}
	tmp = (*head) -> last;
	(*head) -> last = new;
	tmp -> next = new;
	new -> next = NULL;
	return ;
}

t_stack	*detachf(t_stack **head)
{
	//널처리
	t_stack	*detach;

	detach = *head;
	if (detach -> next)
	{
		detach -> next -> last = detach -> last;
		*head = detach -> next;
	}
	else
	{
		*head = NULL;
		detach -> last = detach;
	}
	return (detach);
}

t_stack	*detachb(t_stack **head)
{
	//널처리
	t_stack	*detach;
	t_stack	*last;
	int		i;

	i = 0;
	detach = *head;
	last = (*head) -> last;
	if (detach != last)
	{
		while (detach -> next != (*head) -> last)
		{
			detach = detach -> next;
			i++;
		}
		detach -> next = NULL;
		(*head) -> last = detach;
	}
	else
	{
		*head = NULL;
		detach -> last = detach;
	}
	return (last);
}

//---------------스택 함수-----------------

void	swap(t_stack *head)
{
	// 널처리 필요
	long long	tmp;

	tmp = head -> num;
	head -> num = head -> next -> num;
	head -> next -> num = tmp;
	return ;
}

void	push(t_stack **start, t_stack **dest)
{
	// 널처리 필요
	t_stack	*attach;

	if (!*start)
		return ;
	attach = detachf(start);
	addf(dest, attach);
	return ;
}

void	ss(t_stack *ahead, t_stack *bhead)
{
	// 널처리 필요
	swap(ahead);
	swap(bhead);
	return ;
}

void	rrotate(t_stack **head)
{
	t_stack	*detach;

	if (!*head)
		return ;
	detach = detachb(head);
	addf(head, detach);
	return ;
}

void	rotate(t_stack **head)
{
	t_stack	*detach;

	if (!*head)
		return ;
	detach = detachf(head);
	addb(head, detach);
	return ;
}
//--------------------테스트 출력용----------------------------

void	pfstack(char ab, t_stack *head)
{
	t_stack	*stack;
	int		i;

	i = 0;
	stack = head;
	printf("%c스택: \n", ab);
	while (stack)
	{
		printf("stack[%d]: %lld   현재 주소 %p    next 주소 %p    last 주소: %p\n", i++, stack -> num, stack, stack -> next, stack -> last);
		stack = stack -> next;
	}
	printf("\n");
	return ;
}

//-----------------------테스트 출력용---------------------------------


//-------------------- libft-----------------------
int	ft_isdigit(int str)
{
	unsigned char	unstr;

	if (str < 0 || str > 255)
		return (0);
	unstr = (unsigned char)str;
	return (unstr >= '0' && unstr <= '9');
}

const char	*ft_skipspace(const char *nptr)
{
	while (*nptr == ' ' || (*nptr >= '\t' && *nptr <= '\r'))
		nptr++;
	return (nptr);
}

//----수정
long long	l_ft_atoi(const char *nptr)
{
	int				minus;
	long long		num;

	if (!nptr)
		return (0);
	minus = 1;
	nptr = ft_skipspace(nptr);
	if (*nptr == '-' || *nptr == '+')
	{
		if (*nptr == '-')
			minus = -1;
		nptr++;
	}
	num = 0;
	while (ft_isdigit(*nptr))
	{
		num = num * 10 + (*nptr - '0');
		nptr++;
	}
	return (minus * num);
}

int	count_num(const char *s)
{
	int	count;
	int	i;

	count = 0;
	i = 0;
	while (s[i])
	{
		if (ft_isdigit(s[i]))
		{
			count++;
			while (ft_isdigit(s[i]) && s[i])
				i++;
		}
		else
			i++;
	}
	return (count);
}

//--------------------end libft-----------------------

//--------------------tmp main logic-----------------------

void	tmp_init_bstack(t_stack **ahead, t_stack **bhead, int numsize)
{
	//2, 3로직을 따로 빼는게 좋을듯
	// 라인수/ 분할 필요
	long long	check_atop;
	long long	check_btop;
	int	pivot;
	int	i;
	int	nb;

	i = 0;
	nb = 0;
	pivot = numsize / 2;
	while (nb <= 2) // 이건 임시
	{
		if (!*ahead || !((*ahead) -> next) )
			break ;
		check_atop = (*ahead) -> num;
		if(check_atop > pivot)
		{
			rotate(ahead);
			printf("ra\n");
			i++;
			continue ;
		}
		else
		{
			if (nb == 0)
			{
				push(ahead, bhead);
				printf("pb\n");
				i++;
				nb++;
				continue ;
			}
			check_btop = (*bhead) -> num;
			if (nb == 1)
			{
				if (check_btop > check_atop)
				{
					push(ahead, bhead);
					printf("pb\n");
					swap(*bhead);
					printf("sb\n");
					i++;
					nb++;
					continue ;
				}
				else
				{
					push(ahead, bhead);
					printf("pb\n");
					i++;
					nb++;
					continue ;
				}
			}
			if (check_atop > check_btop)
			{
				push(ahead, bhead);
				printf("pb\n");
				i++;
				nb++;
				continue ;
			}
			else
			{
				swap(*bhead);
				printf("sb\n");
				check_btop = (*bhead) -> num;
				if (check_atop > check_btop)
				{
					push(ahead, bhead);
					printf("pb\n");
					i++;
					nb++;
					continue ;
				}
				else
				{
					swap(*bhead);
					printf("sb\n");
					push(ahead, bhead);
					printf("pb\n");
					i++;
					nb++;
					continue ;
				}
			}
		}
	}
}

void	tmp_fill_btable(long long *btable, t_stack *bhead)
{
	int		i;
	t_stack	*bstack;

	i = 0;
	bstack = bhead;
	while (bstack)
	{
		btable[i] = bstack -> num;
		bstack = bstack -> next;
		i++;
	}
	btable[i] = -1;
}

void	tmp_table_sort(long long *table, int table_size)
{
	int	i;
	int	j;
	long long	tmp;

	i = 0;
	while (i < table_size)
	{
		j = i + 1;
		while (j <= table_size)
		{
			if (table[i] > table[j])
			{
				tmp = table[j];
				table[j] = table[i];
				table[i] = tmp;
			}
			j++;
		}
		i++;
	}
}

long long	*tmp_ind_table(int numsize, char **numbox)
{
	int			j;
	int			i;
	long long			*ind_table;
	long long	num;

	ind_table = malloc(sizeof(long long) * numsize);
	if (!ind_table)
		return (NULL);
	i = 0;
	if (numbox[i][0] == '.')
		i = 1;
	j = -1;
	while (++j < numsize)
	{
		num = l_ft_atoi(numbox[i]);
		if (num < -2147483648 || num > 2147483647)
			return (NULL);
		ind_table[j] = num;
		i++;
	}
	tmp_table_sort(ind_table, numsize - 1);
	return (ind_table);
}

int	tmp_find_ind(long long *ind_table, long long num, int table_size)
{
	int	ind;

	ind = 0;
	while (ind <= table_size)
	{
		if (ind_table[ind] == num)
			return (ind);
		ind++;
	}
	return (ind);
}

t_stack	*tmp_fill_astack(int numsize, char **numbox)
{
	int			i;
	int			j;
	long long			*ind_table;
	long long	num;
	t_stack		*ahead;

	ahead = NULL;
	ind_table = tmp_ind_table(numsize, numbox);
	if (!ind_table)
		return (NULL);
	i = 0;
	if (numbox[i][0] == '.')
		i = 1;
	j = -1;
	while (++j < numsize)
	{
		num = l_ft_atoi(numbox[i]);
		if (num < -2147483648 || num > 2147483647)
			return (NULL);
		num = tmp_find_ind(ind_table, num, numsize - 1);
		addb(&ahead, new_num(num));
		i++;
	}
	free(ind_table);
	return (ahead);
}

void	tmp_rotate(int right, int left, t_stack **bhead, t_stack **ahead)
{
	(void) ahead;//일단 임시
	int	tmp_count;

	if (right <= (left - right))
	{
		tmp_count = right;
		while (tmp_count)
		{
			rotate(bhead);
			printf("rb\n");
			tmp_count--;
		}
	}
	else
	{
		tmp_count = (left - right);
		while (tmp_count)
		{
			rrotate(bhead);
			printf("rrb\n");
			tmp_count--;
		}
	}					
}

void	tmp_insert(t_stack **ahead, t_stack **bhead, int *nb)
{
	push(ahead, bhead);
	printf("pb\n");
	*nb += 1;		
}

long long	*tmp_init_btable(int table_size)
{
	long long	*btable;
	int	i;

	btable = malloc(sizeof(long long) * (table_size + 1));
	if (!btable)
		return (NULL);
	i = 0;
	while (i <= table_size)
	{
		btable[i] = -1;
		i++;
	}
	return (btable);
}

int	tmp_pivot_func(int pivot, int numsize)
{
	int	div;
	// 500개 > 13 // 100개 > 9개 
	div = 9;
	if (!pivot)
		return (numsize / div);
	return (pivot + numsize / div);
}

void	tmp_sort_logic(t_stack **ahead, t_stack **bhead, int numsize)
{
	// 라인수/ 분할 필요
	int	i;
	int	nb;
	int	func_pivot;
	long long	*btable;
	long long	check_atop;

	tmp_init_bstack(ahead, bhead, numsize);
	nb = 3; //임시 값?
	btable = tmp_init_btable(numsize + 1);
	if (!btable)
		return ;
	tmp_fill_btable(btable, *bhead);
	func_pivot = tmp_pivot_func(0, numsize);
	while (nb < func_pivot && (numsize - nb) > 0)
	{
		check_atop = (*ahead) -> num;
		if (check_atop <= func_pivot)
		{
			i = 0;
			while (i < numsize + 1)
			{
				tmp_fill_btable(btable, *bhead);
				if (btable[i] > btable[i + 1] && btable[i + 1] != -1)
				{
					if (check_atop < btable[i] && check_atop > btable[i + 1])
					{
						tmp_rotate((i + 1), nb, bhead, ahead);
						break ;
					}
				}
				else
				{
					if (check_atop < btable[i] || check_atop > btable[i + 1])
					{
						tmp_rotate((i + 1), nb, bhead, ahead);
						break ;
					}
				}
				i++;
			}
			tmp_insert(ahead, bhead, &nb);
		}
		else
		{
			rotate(ahead);
			printf("ra\n");
		}
		if (nb == func_pivot)
			func_pivot = tmp_pivot_func(func_pivot, numsize);
	}
	tmp_fill_btable(btable, *bhead);
	tmp_rotate(btable[0], numsize - 1, bhead, ahead);
	while (numsize--)
	{
		push(bhead, ahead);
		printf("pa\n");
	}
	free(btable);
}

//----------------------------
int	tmp_error(int error_num)
{
	if (error_num == 0)
		ft_printf("Error: no input arg!\n");//
	else if (error_num == 1)
		ft_printf("Error: worng args!\n");//
	else if (error_num == 2)
		ft_printf("Error: it is not int!\n");//
	else if (error_num == 3)
		ft_printf("Error: malloc error!\n");//
	return (1);
}

void	tmp_ft_skipspace(const char *nptr, int *index)
{
	while (nptr[*index] == ' ' || (nptr[*index] >= '\t' && nptr[*index] <= '\r'))
		*index += 1;
	return ;
}

int	tmp_check_args(char **argv, int argc)
{
	// 라인수
	int		i;
	int		ii;
	int		cot;
	char	*strs;

	i = 0;
	cot = 0;
	while (++i < argc)
	{
		strs = argv[i];
		ii = 0;
		while (strs[ii])
		{
			tmp_ft_skipspace(strs, &ii);
			if (strs[ii] == '-')
				ii++;
			if (!ft_isdigit(strs[ii]))
				return (-1);
			cot += 1;
			while (ft_isdigit(strs[ii]))
				ii++;
		}
	}
	if (argc != 2)
		cot = argc - 1;
	return (cot);
}

//-------------------------tmp end----------------------
int	main(int argc, char **argv)
{
	char		**numbox;
	int			numsize;
	t_stack		*ahead;
	t_stack		*bhead;

	if (argc == 1)
		return (tmp_error(0));
	numsize = tmp_check_args(argv, argc);
	if (numsize == -1)
		return (tmp_error(1));
	if (argc == 2)
		numbox = ft_split(argv[1], ' ');
	else
		numbox = argv;
	ahead = tmp_fill_astack(numsize, numbox);
	if (!ahead)
		return (tmp_error(2));
	bhead = NULL;
	tmp_sort_logic(&ahead, &bhead, numsize);
	freestack(&ahead, &bhead);
	if (argc == 2)
		free_all(numbox, numsize);
	return (0);
}
