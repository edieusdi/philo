/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   state.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:20:18 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 13:40:44 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	simulation_stopped(t_table *table)
{
	int	stopped;

	pthread_mutex_lock(&table->stop_lock);
	stopped = table->stop;
	pthread_mutex_unlock(&table->stop_lock);
	return (stopped);
}

void	stop_simulation(t_table *table)
{
	pthread_mutex_lock(&table->stop_lock);
	table->stop = 1;
	pthread_mutex_unlock(&table->stop_lock);
}

void	get_meal_state(t_philo *philo, long *last_meal, int *meal_count)
{
	pthread_mutex_lock(&philo->meal_lock);
	*last_meal = philo->last_meal;
	*meal_count = philo->meals_eaten;
	pthread_mutex_unlock(&philo->meal_lock);
}

void	set_last_meal(t_philo *philo, long timestamp)
{
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal = timestamp;
	pthread_mutex_unlock(&philo->meal_lock);
}

void	complete_meal(t_philo *philo)
{
	pthread_mutex_lock(&philo->meal_lock);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->meal_lock);
}
