/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:20:46 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 14:24:26 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	release_forks(t_philo *philo)
{
	pthread_mutex_unlock(philo->second_fork);
	pthread_mutex_unlock(philo->first_fork);
}

static int	take_forks(t_philo *philo)
{
	pthread_mutex_lock(philo->first_fork);
	if (simulation_stopped(philo->table))
	{
		pthread_mutex_unlock(philo->first_fork);
		return (0);
	}
	print_status(philo, FORK_MSG);
	pthread_mutex_lock(philo->second_fork);
	if (simulation_stopped(philo->table))
	{
		release_forks(philo);
		return (0);
	}
	print_status(philo, FORK_MSG);
	return (1);
}

static int	eat_meal(t_philo *philo)
{
	long	now;

	now = get_time_ms();
	if (now < 0)
	{
		release_forks(philo);
		stop_simulation(philo->table);
		return (0);
	}
	set_last_meal(philo, now);
	print_status(philo, EAT_MSG);
	precise_sleep(philo->table->time_to_eat, philo->table);
	if (simulation_stopped(philo->table))
	{
		release_forks(philo);
		return (0);
	}
	complete_meal(philo);
	release_forks(philo);
	return (1);
}

static int	start_routine(t_philo *philo)
{
	pthread_mutex_lock(&philo->table->start_lock);
	pthread_mutex_unlock(&philo->table->start_lock);
	if (simulation_stopped(philo->table))
		return (0);
	if (philo->table->philo_count == 1)
	{
		lone_philosopher(philo);
		return (0);
	}
	if (philo->id % 2 == 0)
		precise_sleep(philo->table->time_to_eat / 2, philo->table);
	return (!simulation_stopped(philo->table));
}

void	*philo_routine(void *argument)
{
	t_philo	*philo;

	philo = (t_philo *)argument;
	if (!start_routine(philo))
		return (NULL);
	while (!simulation_stopped(philo->table))
	{
		if (!take_forks(philo) || !eat_meal(philo))
			break ;
		print_status(philo, SLEEP_MSG);
		precise_sleep(philo->table->time_to_sleep, philo->table);
		if (simulation_stopped(philo->table))
			break ;
		think_state(philo);
	}
	return (NULL);
}
