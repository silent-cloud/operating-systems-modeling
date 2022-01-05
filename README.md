# operating-systems-modeling
CSC 139, models of Linux OS functions and principles. 
### /proc - observer.c

```
Usage: observer.c {-s | -l <int> <dur>}
```

Retrieves the Linux kernel state by reporting:

- CPU model name
- Kernel version
- Time since system was last booted formatted in dd:hh:mm:ss
- Current date and time
- Machine host name

-s - append information about:

- the amount of memory configured
- memory currently available
- list of load averages (each averaged over the last minute)

-l - append memory information about:

- the amount of memory configured
- memory currently available
- a list of load averages over a custom time interval set by the user. 

<int> is the amount of seconds that elapsed before the program reads the next load average from kernel.
<dur> is the duration, in seconds, in which the program should list the load averages. 


