/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 10:28:37 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 13:47:36 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	print_error(const char *message)
{
	int	length;

	length = 0;
	while (message[length] != '\0')
		length++;
	write(2, message, length);
	return (1);
}

int	main(int argc, char **argv)
{
	t_table	table;

	if (!parse_arguments(argc, argv, &table))
		return (print_error("Error: invalid arguments\n"));
	if (table.meal_limit == 0)
		return (0);
	if (!init_simulation(&table))
		return (print_error("Error: initialization failed\n"));
	if (!start_threads(&table))
	{
		join_threads(&table);
		cleanup_simulation(&table);
		return (print_error("Error: thread creation failed\n"));
	}
	monitor_simulation(&table);
	join_threads(&table);
	cleanup_simulation(&table);
	return (0);
}
