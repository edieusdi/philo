/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:20:12 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 13:17:18 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

long	get_time_ms(void)
{
	struct timeval	time;

	if (gettimeofday(&time, NULL) != 0)
		return (-1);
	return ((time.tv_sec * 1000L) + (time.tv_usec / 1000L));
}

void	precise_sleep(long duration, t_table *table)
{
	long	start;
	long	now;

	start = get_time_ms();
	if (start < 0)
		return ;
	while (!simulation_stopped(table))
	{
		now = get_time_ms();
		if (now < 0 || now - start >= duration)
			return ;
		if (duration - (now - start) > 10)
			usleep(1000);
		else
			usleep(100);
	}
}
