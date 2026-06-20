/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   action.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/20 14:24:03 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 18:13:16 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	lone_philosopher(t_philo *philo)
{
	pthread_mutex_lock(philo->first_fork);
	print_status(philo, FORK_MSG);
	while (!simulation_stopped(philo->table))
		usleep(100);
	pthread_mutex_unlock(philo->first_fork);
}

void	think_state(t_philo *philo)
{
	long	think_time;

	print_status(philo, THINK_MSG);
	if (philo->table->philo_count % 2 == 0)
		return ;
	think_time = (philo->table->time_to_eat * 2)
		- philo->table->time_to_sleep;
	if (think_time > 0)
		precise_sleep(think_time, philo->table);
}
