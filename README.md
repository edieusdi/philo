*This project has been created as part of the 42 curriculum by ebin-ahm.*

# Philosophers

## Table of Contents

- [Description](#description)
- [Project Structure](#project-structure)
- [Structures](#structures)
- [Synchronization Design](#synchronization-design)
- [Deadlock and Starvation Prevention](#deadlock-and-starvation-prevention)
- [Timing and Monitoring](#timing-and-monitoring)
- [Program Algorithm](#program-algorithm)
- [Instructions](#instructions)
- [Testing](#testing)
- [Valgrind: Memory Testing](#valgrind-memory-testing)
- [Helgrind: Data-Race Testing](#helgrind-data-race-testing)
- [DRD: Additional Thread Testing](#drd-additional-thread-testing)
- [Compiler Sanitizers](#compiler-sanitizers)
- [Quick Evaluation Checklist](#quick-evaluation-checklist)
- [Technical Choices](#technical-choices)
- [Resources](#resources)

---

## Description

---

### Purpose

To solve the Dining Philosophers problem in C using POSIX threads and mutexes, while following the 42 Norm and the rules defined by the Philosophers subject.

The purpose of this project is to understand how multiple threads execute concurrently, how shared resources must be protected, and how incorrect synchronization can result in data races, deadlocks, starvation, corrupted output, or a program that simply waits forever while everyone insists they are being productive.

### Brief Overview

One or more philosophers sit around a circular table. There is one fork between every pair of philosophers, meaning that the number of philosophers is also the number of forks.

Each philosopher repeatedly performs the following cycle:

```text
Take two forks -> Eat -> Release both forks -> Sleep -> Think -> Repeat
```

A philosopher requires two forks to eat. If a philosopher does not begin eating before `time_to_die` milliseconds have passed since the start of their last meal, they die and the simulation stops.

Each philosopher is represented by a separate thread. Each fork is represented by a mutex. Shared simulation data is also protected using dedicated mutexes to prevent data races.

The simulation stops when either:

1. A philosopher dies.
2. Every philosopher has eaten at least the optional meal limit.

### Program Arguments

The program receives four mandatory arguments and one optional argument:

```bash
./philo number_of_philosophers time_to_die time_to_eat time_to_sleep [meal_limit]
```

| Argument | Description |
|---|---|
| `number_of_philosophers` | Number of philosophers and forks |
| `time_to_die` | Maximum time in milliseconds since the start of the last meal |
| `time_to_eat` | Time in milliseconds required to eat while holding two forks |
| `time_to_sleep` | Time in milliseconds spent sleeping |
| `meal_limit` | Optional number of meals each philosopher must complete |

All arguments must contain digits only and must fit inside `INT_MAX`.

The four mandatory values must be greater than zero. A `meal_limit` of zero is accepted and causes the program to exit immediately because every philosopher has already eaten at least zero meals.

### Output Format

Every philosopher state is printed using one of the following formats:

```text
timestamp_in_ms philosopher_id has taken a fork
timestamp_in_ms philosopher_id is eating
timestamp_in_ms philosopher_id is sleeping
timestamp_in_ms philosopher_id is thinking
timestamp_in_ms philosopher_id died
```

Example:

```text
0 1 has taken a fork
0 1 has taken a fork
0 1 is eating
200 1 is sleeping
400 1 is thinking
```

The timestamp is measured from the common start of the simulation rather than from the Unix epoch.

Printing is protected by a mutex so that two messages cannot overlap. Once the simulation stops, ordinary status messages are no longer printed.

---

## Project Structure

---

```text
philo/
├── Makefile
├── README.md
├── philo.h
├── philo.c
├── parse.c
├── init.c
├── thread.c
├── routine.c
├── action.c
├── monitor.c
├── state.c
├── time.c
├── print.c
└── cleanup.c
```

### File Responsibilities

| File | Responsibility |
|---|---|
| `philo.c` | Main control flow, error handling, and program shutdown |
| `philo.h` | Structures, constants, includes, and function prototypes |
| `parse.c` | Argument validation and safe numeric conversion |
| `init.c` | Allocation, mutex initialization, philosopher setup, and fork assignment |
| `thread.c` | Philosopher thread creation and synchronized simulation start |
| `routine.c` | Fork acquisition, eating, sleeping, and the philosopher thread routine |
| `action.c` | Single-philosopher behaviour and controlled thinking delay |
| `monitor.c` | Death detection and meal-limit completion checks |
| `state.c` | Mutex-protected access to shared simulation and meal state |
| `time.c` | Millisecond timestamps and interruptible sleeping |
| `print.c` | Serialized status and death messages |
| `cleanup.c` | Thread joining, mutex destruction, allocation rollback, and memory cleanup |
| `Makefile` | Compilation, object management, and rebuild rules |

---

# Structures

## `t_table`

`t_table` stores all data shared by the simulation:

```c
typedef struct s_table
{
	int				philo_count;
	int				meal_limit;
	int				stop;
	int				threads_created;
	long				time_to_die;
	long				time_to_eat;
	long				time_to_sleep;
	long				start_time;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	start_lock;
	pthread_mutex_t	*forks;
	t_philo			*philos;
} t_table;
```

Important fields:

- `stop` indicates whether the simulation has ended.
- `threads_created` records how many threads were successfully created, allowing partial thread-creation failures to be cleaned safely.
- `start_time` is the common start timestamp used for every printed message.
- `forks` points to one mutex per fork.
- `philos` points to one `t_philo` structure per philosopher.

## `t_philo`

`t_philo` stores data belonging to one philosopher:

```c
typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long				last_meal;
	pthread_t		thread;
	pthread_mutex_t	meal_lock;
	pthread_mutex_t	*first_fork;
	pthread_mutex_t	*second_fork;
	t_table			*table;
} t_philo;
```

Important fields:

- `id` starts from 1, while the internal array index starts from 0.
- `last_meal` stores the absolute timestamp at which the philosopher most recently started eating.
- `meals_eaten` counts only completed meals.
- `meal_lock` protects both `last_meal` and `meals_eaten`.
- `first_fork` and `second_fork` store the fork acquisition order.
- `table` gives the philosopher access to shared simulation data.

---

# Synchronization Design

## Fork Mutexes

Each fork is represented by one `pthread_mutex_t`.

```text
Unlocked fork mutex -> fork is available
Locked fork mutex   -> fork is currently held
```

A philosopher must lock both assigned fork mutexes before entering the eating state. Both forks remain locked for the complete `time_to_eat` duration and are released afterwards.

## `meal_lock`

Each philosopher has one `meal_lock` protecting:

```text
last_meal
meals_eaten
```

The philosopher thread updates these values while the main monitoring thread reads them. Every access is synchronized to prevent data races and inconsistent state.

## `stop_lock`

`stop_lock` protects the shared `table->stop` flag.

The flag is read by:

- philosopher routines;
- precise sleeping;
- the single-philosopher loop;
- the main monitor.

It is written when:

- a philosopher dies;
- all philosophers reach the meal limit;
- thread creation or time initialization fails.

## `print_lock`

`print_lock` ensures that only one complete output message is printed at a time.

The print functions also check `table->stop` while holding `stop_lock`, preventing normal messages from appearing after the death message.

The lock order used by the printing functions is always:

```text
print_lock -> stop_lock
```

Using the same order consistently avoids creating another deadlock while trying to report that the program avoided deadlock.

## `start_lock`

`start_lock` is used as a starting gate.

The main thread locks it before creating the philosopher threads. Each philosopher thread immediately waits for the same mutex. Once every thread has been created, the main thread records one common `start_time`, initializes every `last_meal`, and releases the gate.

```text
Main locks start_lock
		|
		v
Main creates all philosopher threads
		|
		v
Threads wait behind start_lock
		|
		v
Main sets start_time and last_meal
		|
		v
Main unlocks start_lock
		|
		v
All philosopher threads may begin
```

This avoids early-created philosophers beginning significantly before later-created philosophers.

---

# Deadlock and Starvation Prevention

## Global Fork Ordering

Each philosopher sits between:

```text
fork[index]
fork[(index + 1) % philosopher_count]
```

The final philosopher connects back to fork 0, completing the circular table.

During initialization, the lower-indexed fork is assigned as `first_fork` and the higher-indexed fork is assigned as `second_fork`.

Every philosopher therefore follows the same ordering rule:

```text
Lock the lower fork index first
Lock the higher fork index second
```

This breaks circular waiting, which is one of the required conditions for deadlock.

Without ordered locking, every philosopher could hold one fork and wait forever for the next fork. With a consistent global order, the circular dependency cannot be formed.

## Initial Stagger

Even-numbered philosophers wait for half of `time_to_eat` before beginning their first fork acquisition:

```c
if (philo->id % 2 == 0)
	precise_sleep(time_to_eat / 2, table);
```

This reduces the initial collision between every philosopher attempting to obtain forks at the same time.

## Odd Philosopher Count

With an odd number of philosophers, the table cannot be divided into equal alternating pairs. A philosopher can repeatedly lose access to forks even when deadlock is prevented.

For odd philosopher counts, a controlled thinking delay is calculated as:

```text
think_time = (time_to_eat * 2) - time_to_sleep
```

If the result is greater than zero, the philosopher waits for that duration after printing the thinking state. This reduces immediate fork reacquisition and gives the philosopher who missed the previous eating group a better opportunity to eat.

## One Philosopher

A single philosopher has only one fork. Since two forks are required to eat, the philosopher:

1. Locks the only fork.
2. Prints one fork message.
3. Waits until the monitor detects starvation.
4. Releases the fork.
5. Exits.

The same mutex is never locked twice by the same thread.

Expected behaviour:

```bash
./philo 1 800 200 200
```

```text
0 1 has taken a fork
800 1 died
```

The exact timestamp may vary slightly because thread scheduling is controlled by the operating system.

---

# Timing and Monitoring

## Millisecond Time

`gettimeofday()` returns seconds and microseconds. These values are converted to milliseconds:

```text
milliseconds = (seconds * 1000) + (microseconds / 1000)
```

The displayed timestamp is:

```text
current_time - start_time
```

## Precise Sleeping

A single large `usleep()` call would prevent a philosopher from noticing that the simulation stopped until the whole sleep completed.

`precise_sleep()` instead:

1. Records the start time.
2. Repeatedly checks elapsed time.
3. Checks the simulation stop flag during the wait.
4. Sleeps in short intervals to avoid a full busy loop.

The function uses 1 ms intervals when more than 10 ms remain, and shorter intervals near the end of the requested duration.

This makes eating and sleeping interruptible while maintaining better timing accuracy than one large `usleep()` call.

## Death Detection

The main thread runs `monitor_simulation()` while philosopher threads execute their routines.

For each philosopher, the monitor checks:

```text
current_time - last_meal >= time_to_die
```

`last_meal` is updated immediately when eating begins. The comparison is performed while holding the philosopher's `meal_lock`, preventing a race between the monitor checking the deadline and the philosopher beginning a new meal.

When a death is detected, `print_death()`:

1. Locks output.
2. Locks the stop state.
3. Confirms that the simulation has not already stopped.
4. Sets `stop` to 1.
5. Prints exactly one death message.

## Meal Completion

The optional meal limit is satisfied only when every philosopher has completed at least the requested number of meals.

The meal counter is incremented after the full eating duration has completed. A meal interrupted by simulation shutdown is not counted as complete.

No additional message is printed when the meal limit is reached. The simulation simply stops.

---

# Program Algorithm

## Main Control Flow

1. Validate the number of arguments.
2. Parse every numeric argument safely.
3. Exit immediately when `meal_limit` is zero.
4. Initialize shared mutexes.
5. Allocate fork and philosopher arrays.
6. Initialize one mutex per fork.
7. Initialize every philosopher and their `meal_lock`.
8. Assign adjacent forks using a global lock order.
9. Lock the simulation starting gate.
10. Create one thread per philosopher.
11. Record one common start time.
12. Initialize every philosopher's `last_meal`.
13. Release the starting gate.
14. Monitor starvation and meal completion in the main thread.
15. Set the shared stop flag when the simulation ends.
16. Join every successfully created thread.
17. Destroy all initialized mutexes.
18. Free every allocated block.

## Philosopher Routine

```text
Wait for the common start gate
		|
		v
Handle the single-philosopher case
		|
		v
Apply initial stagger when ID is even
		|
		v
Take first fork
		|
		v
Take second fork
		|
		v
Update last_meal
		|
		v
Print eating state
		|
		v
Eat for time_to_eat
		|
		v
Count completed meal
		|
		v
Release both forks
		|
		v
Print sleeping state
		|
		v
Sleep for time_to_sleep
		|
		v
Print thinking state
		|
		v
Apply odd-count thinking delay when required
		|
		v
Repeat until simulation stops
```

## Error Cleanup

Initialization is performed in stages. If one stage fails, only resources that were successfully initialized are destroyed.

Examples:

- If allocation fails, allocated arrays are freed and shared mutexes are destroyed.
- If fork initialization fails halfway, only previously initialized fork mutexes are destroyed.
- If philosopher initialization fails, completed `meal_lock` mutexes and all fork mutexes are destroyed.
- If thread creation fails halfway, only successfully created threads are joined.

This prevents double destruction, invalid joins, and memory leaks during partial failure paths.

---

## Instructions

---

### Compilation

To compile the program:

```bash
make
```

To remove object files:

```bash
make clean
```

To remove object files and the executable:

```bash
make fclean
```

To rebuild everything:

```bash
make re
```

The executable produced is:

```text
philo
```

### Execution

Without a meal limit:

```bash
./philo 5 800 200 200
```

With a meal limit:

```bash
./philo 5 800 200 200 3
```

### More Examples

One philosopher:

```bash
./philo 1 800 200 200
```

Four philosophers expected to survive:

```bash
./philo 4 410 200 200
```

A timing set expected to cause a death:

```bash
./philo 4 310 200 100
```

Exit immediately because the meal limit is zero:

```bash
./philo 5 800 200 200 0
```

---

# Testing

Testing concurrent programs requires repeated runs because thread scheduling can change between executions. One successful run does not prove that the program is correct. The scheduler is under no obligation to recreate the same convenient timing twice.

## 1. Norminette

```bash
norminette *.c *.h
```

Expected result:

```text
All files: OK!
```

## 2. Makefile Rules

Clean and rebuild:

```bash
make fclean
make
```

Run `make` again:

```bash
make
```

Expected result:

```text
make: Nothing to be done for 'all'.
```

This confirms that the Makefile does not relink unnecessarily.

Test every required rule:

```bash
make clean
make
make fclean
make re
```

## 3. Invalid Arguments

```bash
./philo
./philo 5 800 200
./philo 0 800 200 200
./philo -5 800 200 200
./philo 5 abc 200 200
./philo 5 800 200 200 3 extra
./philo 2147483648 800 200 200
```

Each invalid command should print:

```text
Error: invalid arguments
```

Check the exit status immediately after one invalid command:

```bash
echo $?
```

Expected:

```text
1
```

## 4. Zero Meal Limit

```bash
./philo 5 800 200 200 0
```

Expected:

- immediate exit;
- no output;
- exit status 0.

```bash
echo $?
```

## 5. Single Philosopher

```bash
./philo 1 800 200 200 > single.out
cat single.out
```

Expected broad behaviour:

```text
0 1 has taken a fork
800 1 died
```

Validate the output:

```bash
grep -c "has taken a fork" single.out
grep -c "is eating" single.out
grep -c "died" single.out
```

Expected counts:

```text
1
0
1
```

The death message should be the final line:

```bash
tail -n 1 single.out
```

## 6. Expected Death

```bash
./philo 4 310 200 100 > death.out
```

Check that one death occurred:

```bash
grep -c "died" death.out
```

Expected:

```text
1
```

Check that death is the final output:

```bash
tail -n 1 death.out
```

## 7. Survival Test

Use `timeout` for a simulation that is expected to continue indefinitely:

```bash
timeout 5 ./philo 4 410 200 200 > survive4.out
```

Check for death:

```bash
grep "died" survive4.out
```

Expected: no output.

A timeout exit status of 124 means that the program was still running after five seconds:

```bash
timeout 5 ./philo 4 410 200 200 > /dev/null
echo $?
```

Expected:

```text
124
```

Test an odd philosopher count:

```bash
timeout 5 ./philo 5 800 200 200 > survive5.out
```

Expected: no death before timeout.

Confirm that every philosopher reached the eating state:

```bash
grep "is eating" survive5.out \
	| awk '{print $2}' \
	| sort -n \
	| uniq
```

Expected philosopher IDs:

```text
1
2
3
4
5
```

## 8. Meal-Limit Test

```bash
./philo 5 800 200 200 3 > meals.out
```

The program should exit by itself.

Check for death:

```bash
grep "died" meals.out
```

Expected: no output.

Count eating messages for each philosopher:

```bash
awk '$3 == "is" && $4 == "eating" \
{count[$2]++} \
END {for (i = 1; i <= 5; i++) print i, count[i]}' meals.out
```

Expected result:

```text
1 3
2 3
3 3
4 3
5 3
```

A philosopher may occasionally begin one additional meal before the monitor observes that every philosopher reached the limit. Every count must be at least 3.

## 9. Output Format Validation

Generate an output file:

```bash
./philo 5 800 200 200 3 > format.out
```

Search for any line that does not match an approved message:

```bash
grep -Ev \
'^[0-9]+ [1-9][0-9]* (has taken a fork|is eating|is sleeping|is thinking|died)$' \
format.out
```

Expected: no output.

## 10. Repeated Tests

Run the meal-limit test 20 times:

```bash
for i in $(seq 1 20); do
	./philo 5 800 200 200 3 > run.out
	if grep -q "died" run.out; then
		echo "Run $i: FAILED"
		break
	else
		echo "Run $i: passed"
	fi
done
```

Repeated odd-count survival test:

```bash
for i in $(seq 1 10); do
	timeout 3 ./philo 5 800 200 200 > run.out
	if grep -q "died" run.out; then
		echo "Run $i: FAILED"
		break
	else
		echo "Run $i: survived"
	fi
done
```

## 11. Large Philosopher Count

```bash
timeout 5 ./philo 200 800 200 200 > large.out
```

This checks thread creation, larger allocation sizes, mutex initialization, synchronized startup, and cleanup under a heavier load.

A heavily loaded machine may affect timing, so this test should be interpreted together with race and memory tools rather than used alone.

---

# Valgrind: Memory Testing

Valgrind significantly slows down threaded programs. Use a large `time_to_die` during memory and race testing so the instrumentation itself does not cause starvation.

Do not judge exact timing accuracy from a Valgrind run.

## Debug Build

```bash
make fclean
make CFLAGS="-Wall -Wextra -Werror -pthread -g3"
```

## Main Memcheck Test

```bash
valgrind \
	--tool=memcheck \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--errors-for-leak-kinds=all \
	--error-exitcode=42 \
	--log-file=memcheck.log \
	./philo 5 10000 200 200 3 > /dev/null
```

Check the exit status immediately:

```bash
echo $?
```

Expected:

```text
0
```

If Valgrind detects an error, the command returns 42 because of `--error-exitcode=42`.

Inspect the report:

```bash
tail -n 30 memcheck.log
```

Search for important results:

```bash
grep -E \
"Invalid|definitely lost|indirectly lost|possibly lost|ERROR SUMMARY" \
memcheck.log
```

A clean result should contain:

```text
definitely lost: 0 bytes in 0 blocks
indirectly lost: 0 bytes in 0 blocks
ERROR SUMMARY: 0 errors from 0 contexts
```

## Death-Path Memcheck

```bash
valgrind \
	--tool=memcheck \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--error-exitcode=42 \
	--log-file=memcheck-death.log \
	./philo 4 1000 600 300 > /dev/null
```

This checks cleanup when the simulation stops because a philosopher dies.

## Single-Philosopher Memcheck

```bash
valgrind \
	--tool=memcheck \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--error-exitcode=42 \
	--log-file=memcheck-single.log \
	./philo 1 300 200 200 > /dev/null
```

This checks that the single fork is released before the mutex is destroyed.

## Invalid-Argument Memcheck

```bash
valgrind \
	--tool=memcheck \
	--leak-check=full \
	--show-leak-kinds=all \
	./philo abc 800 200 200
```

The program should print the argument error and show no memory errors.

## Important Note About `timeout`

Avoid using `timeout` around a Memcheck command when checking final leak cleanup:

```bash
# Not suitable for a final leak result
timeout 5 valgrind ./philo 5 800 200 200
```

`timeout` terminates the program externally before the normal join and cleanup path can finish. This may leave allocated memory reported at exit even when the normal cleanup path is correct.

Use a meal-limited simulation that exits naturally instead.

---

# Helgrind: Data-Race Testing

Memcheck detects memory errors, but it does not detect data races. Helgrind checks conflicting memory access, mutex use, and lock-order problems.

## Main Helgrind Test

```bash
valgrind \
	--tool=helgrind \
	--history-level=approx \
	--fair-sched=yes \
	--error-exitcode=43 \
	--log-file=helgrind.log \
	./philo 5 10000 200 200 3 > /dev/null
```

Check the exit status immediately:

```bash
echo $?
```

Expected:

```text
0
```

Search the report:

```bash
grep -E \
"Possible data race|lock order|destroying a locked mutex|ERROR SUMMARY" \
helgrind.log
```

A clean result should end with:

```text
ERROR SUMMARY: 0 errors from 0 contexts
```

## Death-Path Helgrind

```bash
valgrind \
	--tool=helgrind \
	--history-level=approx \
	--fair-sched=yes \
	--error-exitcode=43 \
	--log-file=helgrind-death.log \
	./philo 4 3000 1800 900 > /dev/null
```

This exercises shared state and fork release during shutdown caused by starvation.

## Single-Philosopher Helgrind

```bash
valgrind \
	--tool=helgrind \
	--history-level=approx \
	--fair-sched=yes \
	--error-exitcode=43 \
	--log-file=helgrind-single.log \
	./philo 1 1000 200 200 > /dev/null
```

This checks synchronization between the monitor and the single philosopher waiting with one fork.

---

# DRD: Additional Thread Testing

DRD is another Valgrind tool for detecting data races and thread synchronization problems. It can be used as a second opinion after Helgrind.

```bash
valgrind \
	--tool=drd \
	--check-stack-var=yes \
	--error-exitcode=44 \
	--log-file=drd.log \
	./philo 5 10000 200 200 3 > /dev/null
```

Inspect the result:

```bash
grep -E "Conflicting|ERROR SUMMARY" drd.log
```

Expected:

```text
ERROR SUMMARY: 0 errors from 0 contexts
```

---

# Compiler Sanitizers

Sanitizers provide another method of detecting invalid memory access, undefined behaviour, and data races.

Do not combine ThreadSanitizer with AddressSanitizer in the same build.

## AddressSanitizer and UndefinedBehaviorSanitizer

```bash
make fclean
make CFLAGS="-Wall -Wextra -Werror -pthread -g3 \
-fsanitize=address,undefined -fno-omit-frame-pointer"
```

Run naturally terminating tests:

```bash
./philo 5 10000 200 200 3
./philo 4 1000 600 300
./philo 1 300 200 200
```

No sanitizer report should be printed.

## ThreadSanitizer

```bash
make fclean
make CFLAGS="-Wall -Wextra -Werror -pthread -g3 \
-fsanitize=thread -fno-omit-frame-pointer"
```

Run with instrumentation-friendly timing:

```bash
./philo 5 10000 200 200 3
```

Expected: no ThreadSanitizer warning.

ThreadSanitizer heavily changes thread timing. A philosopher death during a tight timing test does not by itself prove a race. The important result is whether ThreadSanitizer reports conflicting unsynchronized access.

## Restore the Normal Build

After Valgrind or sanitizer testing:

```bash
make fclean
make
```

---

# Quick Evaluation Checklist

A reviewer can use the following checklist for a quick first pass.

## Build

```bash
norminette *.c *.h
make fclean
make
make
```

Confirm:

- compilation uses `cc -Wall -Wextra -Werror`;
- no Norm error;
- no unnecessary relinking;
- no global variables.

## Basic Behaviour

```bash
./philo 1 800 200 200
./philo 4 310 200 100
./philo 5 800 200 200 3
```

Confirm:

- one philosopher takes one fork and dies;
- a death-producing test prints exactly one death as the final line;
- the meal-limited test exits naturally without death;
- every eating state is preceded by two fork messages for that philosopher;
- no ordinary message appears after death.

## Synchronization

Review that:

- each fork has one mutex;
- `last_meal` and `meals_eaten` use `meal_lock`;
- the stop flag uses `stop_lock`;
- printing uses `print_lock`;
- all threads begin behind `start_lock`;
- fork acquisition uses a consistent global order;
- threads are joined before mutexes are destroyed;
- all allocations are freed.

## Tool Checks

```bash
valgrind --tool=memcheck ...
valgrind --tool=helgrind ...
```

Confirm:

```text
All heap blocks were freed
ERROR SUMMARY: 0 errors from 0 contexts
```

---

# Technical Choices

## Why the Monitor Runs in Main

The main thread does not act as a philosopher. After starting every philosopher thread, it becomes the monitor.

This avoids creating another monitoring thread and keeps simulation ownership clear:

```text
Main thread        -> monitors and coordinates shutdown
Philosopher threads -> eat, sleep, think
```

## Why Meals Are Counted After Eating

A meal is counted only after `time_to_eat` completes. This prevents an interrupted eating state from being treated as a completed meal.

## Why Death Is Printed by a Dedicated Function

`print_death()` combines three operations while holding the required locks:

1. Check whether the simulation already stopped.
2. Stop the simulation.
3. Print the death message.

This ensures that only one death is printed and normal messages cannot follow it.

## Why `threads_created` Is Stored

If `pthread_create()` fails after some threads were created, only valid thread IDs can be joined. `threads_created` tracks that exact number.

## Why Libft Is Not Used

Libft is not authorized for this project. Numeric parsing, string length for errors, timing, cleanup, and all synchronization logic are implemented directly using the allowed functions.

---

## Resources

---

### Classic References

- 42 Philosophers subject, version 13.0
- 42 Norminette rules
- `man pthread_create`
- `man pthread_join`
- `man pthread_mutex_init`
- `man pthread_mutex_lock`
- `man pthread_mutex_unlock`
- `man pthread_mutex_destroy`
- `man gettimeofday`
- `man usleep`
- `man valgrind`
- `man helgrind`
- Linux man-pages documentation for POSIX threads and process timing
- Edsger W. Dijkstra's Dining Philosophers problem

### Topics Studied

- POSIX threads
- Mutex ownership and shared-resource protection
- Data races
- Deadlocks and circular waiting
- Starvation and fairness
- Lock ordering
- Thread lifecycle and joining
- Accurate timing in concurrent programs
- Partial initialization rollback
- Memory-leak testing
- Race detection using Helgrind and ThreadSanitizer

### AI-Assisted Learning

AI (ChatGPT) was used for:

1. Clarifying thread, mutex, deadlock, starvation, and timing concepts.
2. Breaking the project into smaller implementation stages.
3. Explaining the purpose of each structure, mutex, and function.
4. Reviewing edge cases such as one philosopher, odd philosopher counts, and partial thread creation.
5. Developing functional, Valgrind, Helgrind, DRD, and sanitizer testing commands.
6. Reviewing architecture and identifying synchronization risks to be manually verified.
7. Assisting with README structure and technical explanations.

All code was manually typed, reviewed, tested, and verified by ebin-ahm. AI-generated suggestions were only used after the logic was understood and checked against the project requirements.
