/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   push_swap.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 18:39:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:21:16 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PUSH_SWAP_H
# define PUSH_SWAP_H
# include "./ft_printf.h"
# include "./libft.h"

typedef struct s_stack
{
	struct s_stack	*next;
	long long		num;
}	t_stack;

t_stack		*new_num(long long num);
t_stack		*det_f(t_stack **head);
t_stack		*det_b(t_stack **head);
void		add_f(t_stack **head, t_stack *new);
void		add_b(t_stack **head, t_stack *new);

void		free_stack(t_stack **head);
int			free_all_stack(int argc, char **numbox, t_stack **a, t_stack **b);

void		swap(t_stack **head, char ab);
void		sa(t_stack **head);
void		sb(t_stack **head);
void		ss(t_stack **ahead, t_stack **bhead);

void		push(t_stack **ahead, t_stack **bhead, char ab);
void		pa(t_stack **ahead, t_stack **bhead);
void		pb(t_stack **ahead, t_stack **bhead);

void		rotate(t_stack **head, char ab);
void		ra(t_stack **head);
void		rb(t_stack **head);
void		rr(t_stack **ahead, t_stack **bhead);

void		rrotate(t_stack **head, char ab);
void		rra(t_stack **head);
void		rrb(t_stack **head);
void		rrr(t_stack **ahead, t_stack **bhead);

int			tmp_error(int error_num);

int			is_not_num(char **numbox);
int			fill_check_box(int argc, char ***numbox, char **argv);

int			box_size(char **numbox);
int			init_table_stack(char **numbox, t_stack **ahead);

const char	*ft_skipspace(const char *nptr);
long long	l_ft_atoi(const char *nptr);
void		table_sort(long long *table, int table_size);	
int			table_check(long long *sort_table, int table_size);
int			fill_sort_table(char **numbox, long long *i_table, int t_size);

int			fill_stack_a(t_stack **ahead, long long *sort_table, char **numbox);
long long	find_ind(long long *sort_table, long long box_num);

long long	stack_size(t_stack *head);
int			make_init_table(long long size_stack, long long **tab);
int			fill_table(t_stack *head, long long *table);
int			check_atable(long long *a_table);
int			table_set(t_stack **a, long long **a_tab, long long **b_tab);

int			check_table_a(t_stack **a, t_stack **b, long long s, long long *t);
int			sort_two_a(t_stack **head);
int			sort_three_a(t_stack **head, long long *table);

void		update_table(t_stack *head, long long *table);
long long	find_table_ind(long long *table, long long num);
void		move_to_sort_a(t_stack **ahead, t_stack **bhead, long long *table);
int			sort_up_three_a(t_stack **ahead, t_stack **bhead, long long *table);

long long	find_inserta_ind(long long *table, long long num);
long long	find_insertb_ind(long long *table, long long num);
void		find_set_stack(t_stack **a, t_stack **b, long long *t, char ab);
void		move_to_short_a(t_stack **ahead, t_stack **bhead, long long ind);
void		move_to_short_b(t_stack **a, t_stack **b, long long ind, int pb_i);

long long	table_size(long long *table);
int			find_maxlen_init(long long *c_tab, long long *i_tab, long long t);

void		set_table(long long *max_table, long long num);
long long	find_max_last(long long *max_table, long long size);
long long	insert_max_table(long long *max_table, long long num);
long long	change_max_table(long long *max_table, long long num);
void		find_max_len(long long	*c, long long	*m, long long *i);

int			set_init_table(long long size_stack, long long **tab);
void		move_to_a(t_stack **ahead, t_stack **bhead, long long *a_table);
void		move_to_sort_a(t_stack **ahead, t_stack **bhead, long long *table);
int			sort_logic(t_stack **ahead, t_stack **bhead, long long a_size);

void		sorting_func(t_stack **a, t_stack **b, long long *i, long long *bt);
int			div_func(int div, int size_stack);
long long	init_size(long long *init_table, long long init_size);
int			find_atop(long long *i_table, long long atop, long long s_table);

int			move_b_init(t_stack **ahead, t_stack **bhead, long long *b_table);
int			sort_three_b(t_stack **ahead, t_stack **bhead, long long *b_table);
int			sort_up_three_b(t_stack **a, t_stack **b, long long *b_tab);
#endif
