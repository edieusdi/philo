/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:20:38 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 13:32:48 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static long	elapsed_time(t_table *table)
{
	long	now;

	now = get_time_ms();
	if (now < 0)
		return (0);
	return (now - table->start_time);
}

void	print_status(t_philo *philo, const char *status)
{
	t_table	*table;

	table = philo->table;
	pthread_mutex_lock(&table->print_lock);
	pthread_mutex_lock(&table->stop_lock);
	if (table->stop == 0)
		printf("%ld %d %s\n", elapsed_time(table), philo->id, status);
	pthread_mutex_unlock(&table->stop_lock);
	pthread_mutex_unlock(&table->print_lock);
}

void	print_death(t_philo *philo)
{
	t_table	*table;

	table = philo->table;
	pthread_mutex_lock(&table->print_lock);
	pthread_mutex_lock(&table->stop_lock);
	if (table->stop == 0)
	{
		table->stop = 1;
		printf("%ld %d %s\n", elapsed_time(table),
			philo->id, DEATH_MSG);
	}
	pthread_mutex_unlock(&table->stop_lock);
	pthread_mutex_unlock(&table->print_lock);
}
