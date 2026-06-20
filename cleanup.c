/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:20:32 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 14:31:45 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	destroy_fork_mutexes(t_table *table, int count)
{
	while (count > 0)
	{
		count--;
		pthread_mutex_destroy(&table->forks[count]);
	}
}

void	destroy_meal_mutexes(t_table *table, int count)
{
	while (count > 0)
	{
		count--;
		pthread_mutex_destroy(&table->philos[count].meal_lock);
	}
}

int	cleanup_init(t_table *table, int forks_ready)
{
	if (forks_ready)
		destroy_fork_mutexes(table, table->philo_count);
	free(table->philos);
	free(table->forks);
	table->philos = NULL;
	table->forks = NULL;
	pthread_mutex_destroy(&table->start_lock);
	pthread_mutex_destroy(&table->print_lock);
	pthread_mutex_destroy(&table->stop_lock);
	return (0);
}

void	join_threads(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->threads_created)
	{
		pthread_join(table->philos[i].thread, NULL);
		i++;
	}
}

void	cleanup_simulation(t_table *table)
{
	destroy_meal_mutexes(table, table->philo_count);
	destroy_fork_mutexes(table, table->philo_count);
	pthread_mutex_destroy(&table->start_lock);
	pthread_mutex_destroy(&table->print_lock);
	pthread_mutex_destroy(&table->stop_lock);
	free(table->philos);
	free(table->forks);
	table->philos = NULL;
	table->forks = NULL;
}
