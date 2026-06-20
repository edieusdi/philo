/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 10:28:13 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 14:24:48 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

# define FORK_MSG "has taken a fork"
# define EAT_MSG "is eating"
# define SLEEP_MSG "is sleeping"
# define THINK_MSG "is thinking"
# define DEATH_MSG "died"

typedef struct s_table	t_table;

typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long			last_meal;
	pthread_t		thread;
	pthread_mutex_t	meal_lock;
	pthread_mutex_t	*first_fork;
	pthread_mutex_t	*second_fork;
	t_table			*table;
}	t_philo;

struct s_table
{
	int				philo_count;
	int				meal_limit;
	int				stop;
	int				threads_created;
	long			time_to_die;
	long			time_to_eat;
	long			time_to_sleep;
	long			start_time;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	start_lock;
	pthread_mutex_t	*forks;
	t_philo			*philos;
};

/* parse.c */
int		parse_arguments(int argc, char **argv, t_table *table);

/* init.c */
int		init_simulation(t_table *table);

/* thread.c */
int		start_threads(t_table *table);

/* routine.c */
void	*philo_routine(void *argument);

/* action.c */
void	lone_philosopher(t_philo *philo);
void	think_state(t_philo *philo);

/* monitor.c */
void	monitor_simulation(t_table *table);

/* state.c */
int		simulation_stopped(t_table *table);
void	stop_simulation(t_table *table);
void	get_meal_state(t_philo *philo, long *last_meal, int *meal_count);
void	set_last_meal(t_philo *philo, long timestamp);
void	complete_meal(t_philo *philo);

/* time.c */
long	get_time_ms(void);
void	precise_sleep(long duration, t_table *table);

/* print.c */
void	print_status(t_philo *philo, const char *status);
void	print_death(t_philo *philo);

/* cleanup.c */
void	destroy_fork_mutexes(t_table *table, int count);
void	destroy_meal_mutexes(t_table *table, int count);
int		cleanup_init(t_table *table, int forks_ready);
void	join_threads(t_table *table);
void	cleanup_simulation(t_table *table);

#endif