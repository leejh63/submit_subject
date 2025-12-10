/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_map.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/06 16:41:55 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/25 15:45:00 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

static void	read_all(int fd, t_param *p, char *line)
{
	char	**temp;
	int		i;

	i = 0;
	temp = NULL;
	while (line)
	{
		temp = calloc(sizeof(char *), i + 2);
		if (temp == NULL)
			ft_exit(p, "malloc fail", (free(line), 1));
		ft_memcpy(temp, p->map.map, i * sizeof(char *));
		temp[i++] = line;
		free(p->map.map);
		p->map.map = temp;
		line = get_next_line(fd);
	}
	p->map.h = i;
	adjust_size(p);
}

static void	*ft_xfti(void *mlx, char *filename)
{
	int	w;
	int	h;
	int	i;

	while (*filename == ' ')
		filename++;
	i = 0;
	while (filename[i])
	{
		if (filename[i] == '\n')
			filename[i] = 0;
		i++;
	}
	return (mlx_xpm_file_to_image(mlx, filename, &w, &h));
}

static int	parse_one_line(char *line, t_param *p)
{
	if (!p->rsrc.n && !ft_strncmp(line, "NO ", 3))
		p->rsrc.n = ft_xfti(p->mlx, line + 3);
	else if (!p->rsrc.s && !ft_strncmp(line, "SO ", 3))
		p->rsrc.s = ft_xfti(p->mlx, line + 3);
	else if (!p->rsrc.e && !ft_strncmp(line, "EA ", 3))
		p->rsrc.e = ft_xfti(p->mlx, line + 3);
	else if (!p->rsrc.w && !ft_strncmp(line, "WE ", 3))
		p->rsrc.w = ft_xfti(p->mlx, line + 3);
	else if (p->map.f.i == -1 && !ft_strncmp(line, "F ", 2))
		p->map.f = parse_color(line + 2, p);
	else if (p->map.c.i == -1 && !ft_strncmp(line, "C ", 2))
		p->map.c = parse_color(line + 2, p);
	else
		return (1);
	return (0);
}

static void	parse_lines(int fd, t_param *p)
{
	char	*line;

	line = 0;
	while (1)
	{
		free(line);
		line = get_next_line(fd);
		if (line == NULL)
			break ;
		if (is_all_white(line))
			continue ;
		if (parse_one_line(line, p))
			break ;
	}
	read_all(fd, p, line);
}

void	parse_map(char *file_name, t_param *p)
{
	int		fd;

	p->map.f.i = -1;
	p->map.c.i = -1;
	fd = open(file_name, O_RDONLY);
	if (fd < 0)
		ft_exit(p, "", 1);
	parse_lines(fd, p);
	close(fd);
	if (p->map.map == NULL)
		ft_exit(p, "Error\nMap parsing failed", 1);
	validate_map(p);
}
