/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/20 13:43:01 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 13:43:08 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	create_threads(t_table *table)
{
	t_philo	*philo;

	while (table->threads_created < table->philo_count)
	{
		philo = &table->philos[table->threads_created];
		if (pthread_create(&philo->thread, NULL,
				philo_routine, philo) != 0)
			return (0);
		table->threads_created++;
	}
	return (1);
}

static int	set_start_data(t_table *table)
{
	int		i;
	long	start;

	start = get_time_ms();
	if (start < 0)
		return (0);
	table->start_time = start;
	i = 0;
	while (i < table->philo_count)
	{
		table->philos[i].last_meal = start;
		i++;
	}
	return (1);
}

int	start_threads(t_table *table)
{
	pthread_mutex_lock(&table->start_lock);
	if (!create_threads(table) || !set_start_data(table))
	{
		stop_simulation(table);
		pthread_mutex_unlock(&table->start_lock);
		return (0);
	}
	pthread_mutex_unlock(&table->start_lock);
	return (1);
}
