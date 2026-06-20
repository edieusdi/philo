/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:20:25 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 14:48:51 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	init_table_locks(t_table *table)
{
	if (pthread_mutex_init(&table->stop_lock, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&table->print_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&table->stop_lock);
		return (0);
	}
	if (pthread_mutex_init(&table->start_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&table->print_lock);
		pthread_mutex_destroy(&table->stop_lock);
		return (0);
	}
	return (1);
}

static int	init_fork_locks(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->philo_count)
	{
		if (pthread_mutex_init(&table->forks[i], NULL) != 0)
		{
			destroy_fork_mutexes(table, i);
			return (0);
		}
		i++;
	}
	return (1);
}

static void	assign_forks(t_philo *philo, int index, t_table *table)
{
	int	next;

	next = (index + 1) % table->philo_count;
	if (index < next)
	{
		philo->first_fork = &table->forks[index];
		philo->second_fork = &table->forks[next];
	}
	else
	{
		philo->first_fork = &table->forks[next];
		philo->second_fork = &table->forks[index];
	}
}

static int	init_philosophers(t_table *table)
{
	int		i;
	t_philo	*philo;

	i = 0;
	while (i < table->philo_count)
	{
		philo = &table->philos[i];
		if (pthread_mutex_init(&philo->meal_lock, NULL) != 0)
		{
			destroy_meal_mutexes(table, i);
			return (0);
		}
		philo->id = i + 1;
		philo->meals_eaten = 0;
		philo->last_meal = 0;
		philo->table = table;
		assign_forks(philo, i, table);
		i++;
	}
	return (1);
}

int	init_simulation(t_table *table)
{
	table->forks = NULL;
	table->philos = NULL;
	table->stop = 0;
	table->threads_created = 0;
	table->start_time = 0;
	if (!init_table_locks(table))
		return (0);
	table->forks = malloc(sizeof(pthread_mutex_t) * table->philo_count);
	table->philos = malloc(sizeof(t_philo) * table->philo_count);
	if (table->forks == NULL || table->philos == NULL)
		return (cleanup_init(table, 0));
	if (!init_fork_locks(table))
		return (cleanup_init(table, 0));
	if (!init_philosophers(table))
		return (cleanup_init(table, 1));
	return (1);
}
