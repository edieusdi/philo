/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebin-ahm <ebin-ahm@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 10:28:13 by ebin-ahm          #+#    #+#             */
/*   Updated: 2026/06/20 12:51:04 by ebin-ahm         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define FORK_MSG "has taken a fork"
#define EAT_MSG "is eating"
#define SLEEP_MSG "is sleeping"
#define THINK_MSG "is thinking"
#define DEATH_MSG "died"

typedef struct s_table t_table;

#endif