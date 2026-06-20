/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:21:11 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 14:48:56 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	parse_number(char *str, long *number, int allow_zero)
{
	int		i;
	long	value;

	i = 0;
	value = 0;
	if (str[i] == '\0')
		return (0);
	while (str[i] != '\0')
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		value = (value * 10) + (str[i] - '0');
		if (value > INT_MAX)
			return (0);
		i++;
	}
	if (value == 0 && allow_zero == 0)
		return (0);
	*number = value;
	return (1);
}

static int	parse_required(char **argv, t_table *table)
{
	long	value;

	if (!parse_number(argv[1], &value, 0))
		return (0);
	table->philo_count = (int)value;
	if (!parse_number(argv[2], &table->time_to_die, 0))
		return (0);
	if (!parse_number(argv[3], &table->time_to_eat, 0))
		return (0);
	if (!parse_number(argv[4], &table->time_to_sleep, 0))
		return (0);
	return (1);
}

int	parse_arguments(int argc, char **argv, t_table *table)
{
	long	value;

	if (argc != 5 && argc != 6)
		return (0);
	if (!parse_required(argv, table))
		return (0);
	table->meal_limit = -1;
	if (argc == 6)
	{
		if (!parse_number(argv[5], &value, 1))
			return (0);
		table->meal_limit = (int)value;
	}
	return (1);
}
