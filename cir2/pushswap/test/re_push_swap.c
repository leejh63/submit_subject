/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   re_push_swap.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 06:18:08 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 12:37:31 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "ft_printf.h"

typedef struct s_stack
{
	struct s_stack	*next;
	long long		num;
}	t_stack;

t_stack	*new_num(long long num);
void	add_f(t_stack **head, t_stack *new);
void	add_b(t_stack **head, t_stack *new);
t_stack *det_f(t_stack **head);
t_stack *det_b(t_stack **head);

t_stack	*new_num(long long num)
{
	t_stack	*new;

	new = malloc(sizeof(t_stack));
	if (!new)
		return (NULL);
	new -> next = NULL;
	new -> num = num;
	return (new);
}

void	add_f(t_stack **head, t_stack *new)
{
	if (!*head)
	{
		new -> next = NULL;
		*head = new;
		return ;
	}
	new -> next = *head;
	*head = new;
}

void	add_b(t_stack **head, t_stack *new)
{
	t_stack *back;
	if (!*head)
	{
		add_f(head, new);
		return ;
	}
	back = (*head);
	while (back -> next)
		back = back -> next;
	back -> next = new;
	new -> next = NULL;
	return ;
}

t_stack *det_f(t_stack **head)
{
	t_stack *def;

	def = (*head);
	if (def -> next)
		(*head) = (*head)-> next;
	else
		(*head)=NULL;
	return (def); 
}

t_stack *det_b(t_stack **head)
{
	t_stack *deb;
	t_stack *tmp;

	if (!*head)
		return (NULL);
	deb = (*head);
	if (!(deb -> next))
	{
		*head = NULL;
		return (deb);
	}
	while (deb -> next)
	{
		if (!(deb -> next -> next))
			tmp = deb;
		deb = deb -> next;
	}
	tmp -> next = NULL;
	return (deb);
}
//---------
long long   stack_size(t_stack *head)
{
	long long   i;
	t_stack *stack;

	i = 0;
	stack = head;
	while (stack)
	{
		stack = stack -> next;
		i++;
	}
	return (i);
}

static void	free_all(char **words)
{
	int	i;

	i = 0;
	while (words[i])
	{
		free(words[i]);
		i++;
	}
	free(words);
}

void	free_stack(t_stack **head)
{
	t_stack	*tmp;

	if (*head)
	{
		while (*head)
		{
			tmp = (*head) -> next;
			free(*head);
			*head = tmp;
		}
		*head = NULL;
	}
}

int	free_all_stack(int argc, char **numbox, t_stack **ahead, t_stack **bhead)
{
	
	if (argc == 2)
		free_all(numbox);
	free_stack(ahead);
	free_stack(bhead);
	return (0);
}

//-----------------------------------------
void    swap(t_stack **head, char ab)
{
	long long   num;

	num = (*head)->num;
	(*head)->num = (*head)->next->num;
	(*head)->next ->num = num;
	ft_printf("s%c\n", ab);
}

void	sa(t_stack **head)
{
	swap(head, 'a');
}

void	sb(t_stack **head)
{
	swap(head, 'b');
}

void	ss(t_stack **ahead, t_stack **bhead)
{
	swap(ahead, 'a');
	swap(bhead, 'b');
}

void    push(t_stack **ahead, t_stack **bhead, char ab)
{
	t_stack *def;

	if (ab == 'b')
	{
		if (!ahead)
			return ;
		def = det_f(ahead);
		add_f(bhead, def);
	}
	else
	{
		if (!bhead)
			return ;
		def = det_f(bhead);
		add_f(ahead, def);
	}
	ft_printf("p%c\n", ab);
}

void	pa(t_stack **ahead, t_stack **bhead)
{
	push(ahead, bhead, 'a');
}

void	pb(t_stack **ahead, t_stack **bhead)
{
	push(ahead, bhead, 'b');
}

void	rrotate(t_stack **head, char ab)
{
	t_stack	*det;

	if (!*head)
		return ;
	det = det_b(head);
	add_f(head, det);
	ft_printf("rr%c\n", ab);
}

void	rra(t_stack **head)
{
	rrotate(head, 'a');
}

void	rrb(t_stack **head)
{
	rrotate(head, 'b');
}

void	rrr(t_stack **ahead, t_stack **bhead)
{
	rrotate(ahead, 'a');
	rrotate(bhead, 'b');
}

void	rotate(t_stack **head, char ab)
{
	t_stack	*det;

	if (!*head)
		return ;
	det = det_f(head);
	add_b(head, det);
	ft_printf("r%c\n", ab);
}

void	ra(t_stack **head)
{
	rotate(head, 'a');
}

void	rb(t_stack **head)
{
	rotate(head, 'b');
}

void	rr(t_stack **ahead, t_stack **bhead)
{
	rotate(ahead, 'a');
	rotate(bhead, 'b');
}

//-------------------------------------------

// int	ft_isdigit(int str)
// {
// 	unsigned char	unstr;

// 	if (str < 0 || str > 255)
// 		return (0);
// 	unstr = (unsigned char)str;
// 	return (unstr >= '0' && unstr <= '9');
// }

const char	*ft_skipspace(const char *nptr)
{
	while (*nptr == ' ' || (*nptr >= '\t' && *nptr <= '\r'))
		nptr++;
	return (nptr);
}

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
//---------------------------------------------------
int	tmp_error(int error_num)
{
	if (error_num  < -1)
		ft_printf("\nsorted!\n");
	else if (error_num == -1)
		ft_printf("already sort!\n");
	else
		ft_printf("Error: %d\n", error_num);
	return (error_num);
}
//-----------------------------------------------------

int	is_not_num(char **numbox)
{
	int	i;
	int	j;

	i = -1;
	while (numbox[++i])
	{
		j = 0;
		while (numbox[i][j])
		{
			if (numbox[i][j] == '-')
				j++;
			if (!ft_isdigit(numbox[i][j]))
				return (1);
			while (ft_isdigit(numbox[i][j]))
				j++;
			if (numbox[i][j])
				return (1);
		}
	}
	return (0);
}

int	fill_check_box(int argc, char ***numbox, char **argv)
{
	if (argc == 1)
		return (tmp_error(1));
	(*numbox) = &(argv[1]);
	if (argc == 2)
	{
		(*numbox) = ft_split(argv[1], ' ');
		if (!*numbox)
			return (tmp_error(2));
	}
	if (is_not_num(*numbox))
		return (tmp_error(3));
	return (0);
}

int	box_size(char **numbox)
{
	int i;

	i = 0;
	while (numbox[i])
		i++;
	return (i);
}

void	table_sort(long long *table, int table_size)
{
	int	i;
	int	j;
	long long	tmp;

	i = 0;
	while (i < table_size)
	{
		j = i + 1;
		while (j < table_size)
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

int	table_check(long long *sort_table, int table_size)
{
	int	i;

	i = 0;
	while (++i < table_size)
		if (sort_table[i - 1] == sort_table[i])
			return (1);
	return (0);
}

int	fill_sort_table(char **numbox, long long *init_table, int table_size)
{
	int	i;
	long long	box_num;

	i = -1;
	while (++i < table_size)
	{
		box_num = l_ft_atoi(numbox[i]);
		if (box_num < -2147483648 || box_num > 2147483647)
			return (free(init_table), tmp_error(5));
		init_table[i] = box_num;
	}
	table_sort(init_table, table_size);
	if (table_check(init_table, table_size))
		return (free(init_table), tmp_error(6));
	return (0);
}

long long 	find_ind(long long *sort_table,  long long box_num)
{
	long long	i;

	i = 0;
	while (box_num != sort_table[i])
		i++;
	return (i);
}

int	fill_stack_a(t_stack **ahead, long long *sort_table, char **numbox)
{
	int			i;
	t_stack		*new;

	i = -1;
	while (numbox[++i])
	{
		new = new_num(find_ind(sort_table, l_ft_atoi(numbox[i])));
		if (!new)
			return (free(sort_table), 1);
		add_b(ahead, new);
	}
	return (0);
}

int	init_table_stack(char **numbox, t_stack **ahead)
{
	int			table_size;
	long long	*init_table;

	table_size = box_size(numbox);
	init_table = malloc(sizeof(long long) * table_size);
	if (!init_table)
		return (tmp_error(4));
	if (fill_sort_table(numbox, init_table, table_size))
		return (56);
	if (fill_stack_a(ahead, init_table, numbox))
		return (tmp_error(7));
	free(init_table);
	return (0);
}

int	fill_table(t_stack *head, long long *table)
{
	t_stack	*stack;
	int		i;

	i = 0;
	stack = head;
	while (stack)
	{
		table[i] = stack -> num;
		stack = stack -> next;
		i++;
	}
	table[i] = -1;
	return (0);
}

int	make_init_table(long long size_stack, long long **tab)
{
	int			i;

	i = -1;
	*tab = malloc(sizeof(long long) * (size_stack + 1));
	if (!*tab)
		return (tmp_error(8));
	while (++i <= size_stack)
		(*tab)[i] = -1;
	return (0);
}

int	check_atable(long long *a_table)
{
	int	i;

	i = 1;
	while (a_table[i] != -1)
	{
		if (a_table[i - 1] > a_table[i])
			return (0);
		i++;
	}
	return (1);
}

int	table_set(t_stack **ahead, long long **a_table, long long **b_table)
{
	long long	size_stack;

	size_stack = stack_size(*ahead);
	if (make_init_table(size_stack, a_table))
		return (tmp_error(9));
	if (make_init_table(size_stack, b_table))
	{
		free(*a_table);
		*a_table = NULL;
		return (tmp_error(10));
	}
	fill_table(*ahead, *a_table);
	if (check_atable(*a_table))
		return (tmp_error(-1));
	return (0);
}

int	sort_two_a(t_stack **head)
{
	sa(head);
	return (0);
}

int	sort_three_a(t_stack **head, long long *table)
{
	if (table[0] < table[1])
	{
		if(table[0] > table[2])
			return (rra(head), -2);
		else if (table[1] > table[2])
			return (sa(head), ra(head), -2);
	}
	else
	{
		if (table[0] < table[2])
			return (sa(head), -2);
		else
		{
			if (table[1] < table[2])
				return (ra(head), -2);
			else
				return (sa(head), rra(head), -2);
		}
	}
	return (0);
}

void update_table(t_stack *head, long long *table)
{
	long long i;

	i = 0;
	while (head)
	{
		table[i] = head -> num;
		head = head -> next;
		i++;
	}
	table[i] = -1;
}

long long	find_table_ind(long long *table, long long num)
{
	long long	i;

	i = 0;
	while (table[i] != -1)
	{
		if (num == table[i])
			return (i);
		i++;
	}
	return (0);
}

long long	find_inserta_ind(long long *table, long long num)
{
	long long	i;

	i = 1;
	while (table[i] != -1)
	{
		if (num > table[i - 1] && num < table[i])
			return (i);
		if(table[i - 1] > table[i])
			if (num > table[i - 1] || num < table[i])
				return (i);
		i++;
	}
	return (0);
}

long long	find_insertb_ind(long long *table, long long num)
{
	long long	i;

	i = 1;
	while (table[i] != -1)
	{
		if (table[i - 1] > table[i])
		{
			if (num < table[i - 1] && num > table[i])
				return (i);
		}
		else
		{
			if (num > table[i] || num < table[i - 1])
				return (i);
		}
		i++;
	}
	return (0);
}

void	move_to_short_a(t_stack **ahead, t_stack **bhead, long long ind)
{
	long long	size_stack;
	long long	count;


	size_stack = stack_size(*ahead);
	if (ind < (size_stack - ind))
	{
		count = ind;
		while (count--)
			rotate(ahead, 'a');
	}
	else
	{
		count = size_stack - ind;
		while (count--)
			rrotate(ahead, 'a');
	}
	if (stack_size(*bhead))
		pa(ahead, bhead);
}

void	move_to_short_b(t_stack **ahead, t_stack **bhead, long long ind, int pb_i)
{
	long long	size_stack;
	long long	count;


	size_stack = stack_size(*bhead);
	if (ind < (size_stack - ind))
	{
		count = ind;
		while (count--)
			rotate(bhead, 'b');
	}
	else
	{
		count = size_stack - ind;
		while (count--)
			rrotate(bhead, 'b');
	}
	if (pb_i)
		pb(ahead, bhead);
}

void	find_set_stack(t_stack **ahead, t_stack **bhead, long long *table, char ab)
{
	long long	i;
	long long	num;

	if (ab == 'a')
	{
		num = (*bhead)->num;
		update_table(*ahead, table);
		i = find_inserta_ind(table, num);
		move_to_short_a(ahead, bhead, i);
		update_table(*ahead, table);
	}
	else
	{
		num = (*ahead)->num;
		update_table(*bhead, table);
		i = find_insertb_ind(table, num);
		move_to_short_b(ahead, bhead, i, 1);
		update_table(*bhead, table);
	}
}

void	move_to_sort_a(t_stack **ahead, t_stack **bhead, long long *table)
{
	long long i;

	update_table(*ahead, table);
	i = find_table_ind(table, 0);
	move_to_short_a(ahead, bhead, i);
}

// void	move_to_sort_b(t_stack **ahead, t_stack **bhead, long long *table)
// {
// 	long long i;

// 	update_table(*bhead, table);
// 	i = find_table_ind(table, stack_size(*ahead) + stack_size(*bhead) - 1);
// 	move_to_short_b(ahead, bhead, i, 0);
// }

int	sort_up_three_a(t_stack **ahead, t_stack **bhead, long long *table)
{
	long long size_stack;

	size_stack = stack_size(*ahead);
	while (size_stack != 3)
	{
		pb(ahead, bhead);
		size_stack = stack_size(*ahead);
	}
	update_table(*ahead, table);
	sort_three_a(ahead, table);
	size_stack = stack_size(*bhead);
	while (size_stack > 0)
	{
		find_set_stack(ahead, bhead, table, 'a');
		size_stack--;
	}
	move_to_sort_a(ahead, bhead, table);
	return (0);
}

//--------------100 미만-------------------
int	check_table_a(t_stack **ahead, t_stack **bhead, long long a_size, long long *table)
{
	if (a_size == 2)
		return (sort_two_a(ahead));
	else if (a_size == 3)
		return (sort_three_a(ahead, table));
	else if (a_size <= 50)
		return (sort_up_three_a(ahead, bhead, table));
	return (1);
}
//--------------100 미만 end----------------

//--------------100 이상 정렬 --------------

//--------------

// long long	find_next_ind(long long *table, long long now_ind)
// {
// 	long long	next_ind;

// 	next_ind = now_ind + 1;
// 	while (table[next_ind] != table[now_ind])
// 	{
// 		if (table[next_ind] > table[now_ind])
// 			return (next_ind);
// 		next_ind++;
// 		if (table[next_ind] == -1)
// 			next_ind = 0;
// 	}	
// 	return (-1);
// }

int	set_init_table(long long size_stack, long long **tab)
{
	int			i;

	i = -1;
	*tab = malloc(sizeof(long long) * (size_stack + 1));
	if (!*tab)
		return (tmp_error(8));
	while (++i <= size_stack)
		(*tab)[i] = -1;
	return (0);
}

// void	fill_now_table(long long *table, long long *init_table, long long start_ind)
// {
// 	int	i;

// 	i = start_ind;
// 	while (table[i] != -1)
// 	{
// 		if (table[start_ind] <= table[i])
// 			init_table[i] = 1;
// 		i++;
// 	}
// }

// long long	sum_max_table(long long *init_table)
// {
// 	long long	sum;
// 	long long	i;
	
// 	i = 0;
// 	sum = 0;
// 	while (init_table[i] != -1)
// 	{
// 		sum += init_table[i];
// 		i++;
// 	}
// 	return (sum);
// }

long long	table_size(long long *table)
{
	long long	i;
	
	i = 0;
	while (table[i] != -1)
		i++;
	return (i);
}

void	set_table(long long *max_table, long long num)
{
	long long i;

	i = 0;
	while (max_table[i] != -1)
	{
		max_table[i] = num;
		i++;
	}
}

long long	find_max_last(long long *max_table, long long size)
{
	long long	i;
	long long	max;

	i = 0;
	max = -1;
	while (i < size)
	{
		if (max < max_table[i])
			max = max_table[i];
		i++;
	}
	return (max);
}

// long long	find_num_ind(long long *max_table, long long num, long long size)
// {
// 	long long	i;

// 	i = 0;
// 	while (i < size)
// 	{
// 		if (max_table[i] == num)
// 			return (i);
// 		i++;
// 	}
// 	return (-1);
// }

long long	insert_max_table(long long *max_table, long long num)
{
	long long	i;

	i = 0;
	while (max_table[i] != -1)
		i++;
	max_table[i] = num;
	return (i);
}

long long	change_max_table(long long *max_table, long long num)
{
	long long	i;

	i = 0;
	while (max_table[i] < num)
		i++;
	max_table[i] = num;
	return (i);
}

// long long	max_table_len(long long *max_table)
// {
// 	long long	i;

// 	i = 0;
// 	while (max_table[i] != -1)
// 		i++;
// 	return (i);
// }

void	find_max_len(long long	*check_table, long long	*max_table, long long *index_table)
{
	long long	i;
	long long	total_size;

	i = 0;
	total_size = table_size(check_table);
	set_table(max_table, -1);
	set_table(index_table, -1);
	max_table[i] = check_table[i];
	while (check_table[i] != -1)
	{
		if (check_table[i] > find_max_last(max_table, total_size))
			index_table[i] = insert_max_table(max_table, check_table[i]);
		else
			index_table[i] = change_max_table(max_table, check_table[i]);
		i++;
	}
}

// long long	find_check_table_ind(long long *check_table, long long num)
// {
// 	long long i;

// 	i = 0;
// 	while (check_table[i] != -1)
// 	{
// 		if (check_table[i] == num)
// 			return (i);
// 		i++;
// 	}
// 	return (-1);
// }

// long long	find_num_check_table(long long *check_table, long long num)
// {
// 	long long	i;

// 	i = 0;
// 	while (check_table[i] != -1)
// 	{
// 		if (check_table[i] == num)
// 			return (i);
// 		i++;
// 	}
// 	return (-1);
// }

int	find_maxlen_init(long long *check_table, long long *init_table, long long total_size)
{
	long long	*max_table;
	long long	*index_table;
	long long	max_len;
	long long	i;

	if (set_init_table(total_size, &max_table))
		return (tmp_error(100));
	if (set_init_table(total_size, &index_table))
		return (free(max_table), tmp_error(100));
	find_max_len(check_table, max_table, index_table);
	max_len = table_size(max_table) - 1;
	i = table_size(index_table);
	while (--i)
	{
		if (index_table[i] == max_len)
		{
			init_table[i] = check_table[i];
			max_len--;
		}
	}
	return (free(max_table), free(index_table), 0);
}

int	sort_three_b(t_stack **ahead, t_stack **bhead, long long *b_table)
{
	long long a_top;
	
	a_top = (*ahead)->num;
	update_table(*bhead, b_table);
	pb(ahead, bhead);
	if (b_table[0] > b_table[1])
	{
		if (a_top < b_table[0] && a_top > b_table[1])
			sb(bhead);
	}
	else
	{
		if (a_top < b_table[0] || a_top > b_table[1])
			sb(bhead);
	}
	return (2);
}

int	sort_up_three_b(t_stack **ahead, t_stack **bhead, long long *b_table)
{
	update_table(*bhead, b_table);
	find_set_stack(ahead, bhead, b_table, 'b');
	return (0);
}

int	move_b_init(t_stack **ahead, t_stack **bhead, long long *b_table)
{
	long long	b_size;

	b_size = stack_size(*bhead);
	if (b_size <= 1)
		return (pb(ahead, bhead), 1);
	if (b_size == 2)
		return (sort_three_b(ahead, bhead, b_table), 3);
	if (b_size >= 3)
		return (sort_up_three_b(ahead, bhead, b_table), 4);
	return (0);
}

int	find_atop(long long *init_table, long long atop, long long size_table)
{
	long long i;

	i = 0;
	while (i < size_table)
	{
		if (init_table[i] == atop)
			return (0);
		i++;
	}
	return (1);
}

long long	init_size(long long *init_table, long long init_size)
{
	long long	i;
	long long	count;

	i = 0;
	count = 0;
	while (i < init_size)
	{
		if (init_table[i] != -1)
			count++;
		i++;
	}
	return (count);
}

int	div_func(int div, int size_stack)
{
	int	div_num;

	div_num = (25 * size_stack + 1500)/1000;
	return (div - size_stack / div_num);
}

void	sorting_func(t_stack **ahead, t_stack **bhead, long long *init_table, long long *b_table)
{
	long long	k;
	long long	div;
	long long	total_size;

	total_size = stack_size(*ahead) + stack_size(*bhead);
	div = total_size;
	while (stack_size(*ahead) != init_size(init_table, total_size))
	{
		k = -1;
		div = div_func(div,  total_size);
		while (++k < stack_size(*ahead) && stack_size(*ahead) != init_size(init_table, total_size))
		{
			update_table(*bhead, b_table);
			if (find_atop(init_table, (*ahead)->num, total_size))
			{
				if (div <= (*ahead)->num)
					move_b_init(ahead, bhead, b_table);
				else
					ra(ahead);
			}
			else
				ra(ahead);
		}
	}
}

void	move_to_a(t_stack **ahead, t_stack **bhead, long long *a_table)
{
	long long size_stack;

	size_stack = stack_size(*bhead);
	update_table(*ahead, a_table);
	while (size_stack > 0)
	{
		find_set_stack(ahead, bhead, a_table, 'a');
		size_stack--;
	}
}

int	sort_logic(t_stack **ahead, t_stack **bhead, long long a_size, long long b_size)
{
	(void) b_size;
	int			i;
	long long	*a_table;
	long long	*b_table;
	long long	*init_table;

	i = table_set(ahead, &a_table, &b_table);
	if (i)
		return (free(a_table), free(b_table), 1);
	if (!check_table_a(ahead, bhead, a_size, a_table))
		return (free(a_table), free(b_table), 1);
	if (set_init_table(a_size, &init_table))
		return (free(a_table), free(b_table), 1);
	if (find_maxlen_init(a_table, init_table, a_size))
		return (free(a_table), free(b_table), free(init_table), 1);
	sorting_func(ahead, bhead, init_table, b_table);
	move_to_a(ahead, bhead, a_table);
	move_to_sort_a(ahead, bhead, a_table);
	return (free(a_table), free(b_table), free(init_table), 0);
}

int	main(int argc, char **argv)
{
	char		**numbox;
	t_stack		*ahead;
	t_stack		*bhead;

	ahead = NULL;
	bhead = NULL;
	if (fill_check_box(argc, &numbox, argv))
		return (free_all_stack(argc, numbox, &ahead, &bhead));
	if (init_table_stack(numbox, &ahead))
		return (free_all_stack(argc, numbox, &ahead, &bhead));
	sort_logic(&ahead, &bhead, stack_size(ahead), stack_size(bhead));
	free_all_stack(argc, numbox, &ahead, &bhead);
	ahead = NULL;
	bhead = NULL;
	return (0);
}