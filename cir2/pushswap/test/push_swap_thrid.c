//--------------3구간 정렬

long long	div_parti(long long size_stack, long long det)
{
	return (size_stack/det);
}

int find_parti(t_stack **head, long long div)
{
	long long	s_top;

	s_top = (*head)-> num;
	if (s_top >= 0 && s_top < (1 * div))
		return (1);
	else if (s_top >= (1 * div)  && s_top < (2 * div))
		return (2);
	else if (s_top >= (2 * div) && s_top < (3 * div))
		return (3);
	else // ((3 * div) <= top <= numsize)
		return (4);
}

int	parti_check(long long *table, int div, int	*signal)
{
	int	i;
	int	sum;

	i = 0;
	while (table[i] != -1)
	{
		if (table[i] >= (0) * div  && table[i] < (1 * div))
			signal[0] = 1;
		if (table[i] >= (1) * div  && table[i] < (2 * div))
			signal[1] = 1;
		if (table[i] >= (2) * div  && table[i] < (3 * div))
			signal[2] = 1;
		i++;
	}
	i = -1;
	sum = 0;
	while (++i < 3)
		sum += signal[i];
	return (sum);
}

int change_range(int range)
{
	if (range == 3)
		return (2);
	else if (range == 2)
		return (1);
	else// (arange == 1)
		return (3);
}

int	find_hind(int range, long long *table, int div, int *signal)
{
	int i;
	int head_ind;

	i = 0;
	head_ind = -1;
	if (!signal[range - 1])
		range = change_range(range);
	while (table[i] != -1)
	{
		if (table[i] >= (range - 1) * div && table[i] < (range  * div))
		{
			head_ind = i;
			while (table[i] >= (range - 1) * div && table[i] < (range  * div))
				i++;
		}
		else
			i++;
	}
	return (head_ind);
}

int	find_tind(int range, long long *table, int div)
{
	int i;
	int tail_ind;
	int	signal;

	i = 0;
	tail_ind = -1;
	signal = 0;
	while (table[i] != -1)
	{
		if (table[i] >= (range - 1) * div && table[i] < (range  * div))
		{
			while (table[i] >= (range - 1) * div && table[i] < (range  * div))
				i++;
			if (!signal)
				tail_ind = i - 1;
			signal = 1;
		}
		else
			i++;
	}
	return (tail_ind);
}

void	init_signal(long long *b_table, int *signal, long long div)
{
	int	i;

	i = -1;
	while (++i < 3)
		signal[i] = 0;
	i = 0;
	while (b_table[i] != -1)
	{
		if (b_table[i] >= (0) * div  && b_table[i] < (1 * div))
			signal[0] = 1;
		if (b_table[i] >= (1) * div  && b_table[i] < (2 * div))
			signal[1] = 1;
		if (b_table[i] >= (2) * div  && b_table[i] < (3 * div))
			signal[2] = 1;
		i++;
	}
}

void	two_ro_pu(t_stack **ahead, t_stack **bhead, int two_head_ind)
{
	int b_size;
	int	count;

	b_size = stack_size(*bhead);
	count = two_head_ind;
	if (two_head_ind > (b_size - two_head_ind))
		count = b_size - two_head_ind;
	while (count)
	{
		if (two_head_ind <= (b_size - two_head_ind))
			rb(bhead);
		else
			rrb(bhead);
		count--;
	}
	pb(ahead, bhead);
}

void two_parti(t_stack **ahead, t_stack **bhead, long long *b_table, long long div)
{
	int	head_ind;
	int	signal[3];

	init_signal(b_table, signal, div);
	if (parti_check(b_table, div, signal) == find_parti(ahead, div))
		pb(ahead, bhead);
	else
	{
		head_ind = find_hind(find_parti(ahead, div), b_table, div, signal);
		two_ro_pu(ahead, bhead, head_ind);
	}
}

void	thr_ro_pu(t_stack **ahead, t_stack **bhead, int head_ind, int tail_ind)
{
	(void) ahead;
	int b_size;
	int count;

	b_size = stack_size(*bhead) - 1;
	if (head_ind > ((b_size) - tail_ind))
		count = (b_size) - tail_ind;
	else
		count = head_ind;
	while (count)
	{
		if (head_ind <= ((b_size) - tail_ind))
			rb(bhead);
		else
			rrb(bhead);
		count--;
	}
	pb(ahead, bhead);
}

void	three_parti(t_stack **ahead, t_stack **bhead, long long *b_table, long long div)
{
	int	head_ind;
	int	tail_ind;
	int	signal[3];

	init_signal(b_table, signal, div);
	head_ind = find_hind(find_parti(ahead, div), b_table, div, signal);
	tail_ind = find_tind(find_parti(ahead, div), b_table, div);
	if (find_parti(ahead, div) == find_parti(bhead, div))
		pb(ahead, bhead);
	else
		thr_ro_pu(ahead, bhead, head_ind, tail_ind);	
}

void parti_branch(t_stack **ahead, t_stack **bhead, long long *b_table, long long div)
{
	int			b_parti;
	int			signal[3];

	update_table(*bhead, b_table);
	init_signal(b_table, signal, div);
	b_parti = parti_check(b_table, div, signal);
	if (b_parti == 0 || b_parti == 1)
		pb(ahead, bhead);
	else if (b_parti == 2)
		two_parti(ahead, bhead, b_table, div);
	else if (b_parti == 3)
		three_parti(ahead, bhead, b_table, div);
}

int	third_partion(t_stack **ahead, t_stack **bhead, long long *a_table, long long *b_table)
{
	long long	div;
	int			a_parti;

	div = div_parti(stack_size(*ahead), 4);
	while ( 3 * div > stack_size(*bhead))
	{
		a_parti = find_parti(ahead, div);
		if (a_parti == 4)
			ra(ahead);
		else
			parti_branch(ahead, bhead, b_table, div);

	}
	sort_up_three(ahead, bhead, a_table, 'a');
	return (0);
}
//--------------