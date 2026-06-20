/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:20:57 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 14:48:54 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	check_death(t_philo *philo)
{
	long	now;
	long	elapsed;

	pthread_mutex_lock(&philo->meal_lock);
	now = get_time_ms();
	if (now < 0)
	{
		pthread_mutex_unlock(&philo->meal_lock);
		stop_simulation(philo->table);
		return (1);
	}
	elapsed = now - philo->last_meal;
	pthread_mutex_unlock(&philo->meal_lock);
	if (elapsed >= philo->table->time_to_die)
	{
		print_death(philo);
		return (1);
	}
	return (0);
}

static int	all_meals_completed(t_table *table)
{
	int		i;
	int		meals_eaten;
	long	last_meal;

	if (table->meal_limit < 0)
		return (0);
	i = 0;
	while (i < table->philo_count)
	{
		get_meal_state(&table->philos[i],
			&last_meal, &meals_eaten);
		if (meals_eaten < table->meal_limit)
			return (0);
		i++;
	}
	return (1);
}

void	monitor_simulation(t_table *table)
{
	int	i;

	while (!simulation_stopped(table))
	{
		i = 0;
		while (i < table->philo_count)
		{
			if (check_death(&table->philos[i]))
				return ;
			i++;
		}
		if (all_meals_completed(table))
		{
			stop_simulation(table);
			return ;
		}
		usleep(500);
	}
}
