/* Author: John Lorenz Salva
 *
 * observer.c reports the linux machine's current
 * kernel information.
 * Date: Spring 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <time.h>
#define CPU_FILE "/proc/cpuinfo"
#define DISK_FILE "/proc/diskstats"
#define HOST_FILE "/proc/sys/kernel/hostname"
#define KERNEL_VER_FILE "/proc/version"
#define LDAVG_FILE "/proc/loadavg"
#define MEM_FILE "/proc/meminfo"
#define STAT_FILE "/proc/stat"
#define UPTIME_FILE "/proc/uptime"
#define BUFFER_LENGTH 200

FILE *open_file(char* file);
void get_host_info(void);
void get_CPU_info(void);
void get_kernel_ver_info(void);
void get_uptime_info(void);
void get_stat_info(void);
void get_disk_info(void);
void get_mem_info(void);
void get_ldavg_info(int interval, int duration);

int main(int argc, char *argv[]) {
   char c1;
   char c2;
   char rep_type_name[16];
   int duration;
   int interval;
   struct timeval now;
   
   strcpy(rep_type_name, "Standard");
   if (argc > 1) {
      sscanf(argv[1], "%c%c", &c1, &c2);
      if (c1 != '-') {
         fprintf(stderr, "usage: observer [-s] [-l int dur]\n");
         exit(EXIT_FAILURE);
      }
      if (c2 == 's') {
         strcpy(rep_type_name, "Short");
      }
      if (c2 == 'l') {
		 if(argc != 4) {
		    fprintf(stderr, "usage: observer [-s] [-l int dur]\n");
		    exit(EXIT_FAILURE);
		 }
         strcpy(rep_type_name, "Long");
         interval = atoi(argv[2]);
         duration = atoi(argv[3]);
      }
   }
   
   gettimeofday(&now, NULL);
   printf("\nStatus report type %s at %s\n", rep_type_name, ctime(&(now.tv_sec)));
   get_host_info();
   get_CPU_info();
   get_kernel_ver_info();
   get_uptime_info();
   if(strcmp(rep_type_name, "Short") == 0) {
	   get_stat_info();
	   get_disk_info();
   } else if (strcmp(rep_type_name, "Long") == 0) {
	   get_stat_info();
	   get_disk_info();
	   get_mem_info();
	   get_ldavg_info(interval, duration);
   }
   return EXIT_SUCCESS;      
}

/* Opens the passed file name and error checks */
FILE *open_file(char* file) {
   FILE *ptr_file = fopen(file, "r");
   if (ptr_file == NULL) {
      printf("Error on fopen %s \n", file);
      exit(EXIT_FAILURE);
   }
   return ptr_file;
}

/* Gets the kernel host name from /proc/svs/kernel/hostname */
void get_host_info(void) {
   char data[BUFFER_LENGTH];
   FILE *ptr_file = open_file(HOST_FILE);
   fgets(data, sizeof data, ptr_file);
   printf("Machine hostname: %s", data);
   fflush(stdout);
   fclose(ptr_file);
}

/* Gets the name of the CPU on the machine from /proc/cpuinfo */
void get_CPU_info(void) {
   char data[BUFFER_LENGTH];
   char *token_ptr;
   FILE *ptr_file = open_file(CPU_FILE);
   int count = 0;
   while(fgets(data, sizeof data, ptr_file)) {
      if (strncmp(data, "model name", 10) == 0) {
         break;
      }
   }
   
   //tokenize the string and break when we get the name
   token_ptr = strtok(data, ":");
   while (token_ptr != NULL) {
	   if (count == 1) {
		   strncpy(data, token_ptr, BUFFER_LENGTH - 1);
		   break;
	   }
	   token_ptr = strtok(NULL, ":");
	   count++;
   }
   printf("CPU Model: %s", data);
   fflush(stdout);
   fclose(ptr_file);
}

/* Gets the kernal version info from /proc/version */
void get_kernel_ver_info(void) {
   char data[BUFFER_LENGTH];
   FILE *ptr_file = open_file(KERNEL_VER_FILE);
   fgets(data, sizeof data, ptr_file);
   printf("%s", data);
   fflush(stdout);
   fclose(ptr_file); 
}

/* Gets the amount time the system has been up
 * since the last system boot. Info is retreived
 * from /proc/uptime */
void get_uptime_info(void) {
   char data[BUFFER_LENGTH];
   double uptime;
   FILE *ptr_file = open_file(UPTIME_FILE);
   int days;
   int hours;
   int minutes;
   int seconds;
   int remainder;
   fscanf(ptr_file, "%s", data);
   fclose(ptr_file);
   uptime = atof(data);
   days = (uptime / (24 * 3600));
   remainder = ((int) uptime % (24 * 3600));
   hours = remainder / 3600;
   remainder %= 3600;
   minutes = remainder / 60;
   remainder %= 60;
   seconds = remainder;

   printf("Time since the system was last booted: %d:%d:%d:%d\n\n", days, hours, minutes, seconds);
}

/* Gets the status info containing the amount of time
 * the CPU has spent in user mode, system mode, and idle,
 * the amount of context switches that the CPU has made since
 * the system was last booted, the time when the system was
 * last booted, and the amount of processes created since the
 * system was last booted. Info retrieved from /proc/stat.
 */
void get_stat_info(void) {
   char data[BUFFER_LENGTH];
   FILE *ptr_file = open_file(STAT_FILE);
   int cpu_user_time = 0;
   int cpu_sys_time = 0;
   int cpu_idle_time = 0;
   int ctxt = 0;
   int processes = 0;
   time_t btime;
   while(fgets(data, sizeof data, ptr_file) != NULL) {
      if(strncmp(data, "cpu ", 4) == 0) {
         char cpu_name_data[BUFFER_LENGTH];	//CPU name
         char ut1_data[BUFFER_LENGTH];		//User-time spent
         char ut2_data[BUFFER_LENGTH];		//User-time spent with low priority processes
         char st_data[BUFFER_LENGTH];		//System-time spent
         char it_data[BUFFER_LENGTH];		//Idle-time spent
         int ut1;
         int ut2;
         int st;
         int it;
         sscanf(data, "%s%s%s%s%s", cpu_name_data, ut1_data, ut2_data, st_data, it_data);
         ut1 = atoi(ut1_data);
         ut2 = atoi(ut2_data);
         st = atoi(st_data);
         it = atoi(it_data);
         cpu_user_time = (ut1 + ut2);
         cpu_sys_time = st;
         cpu_idle_time = it;
      } else if (strncmp(data, "ctxt", 4) == 0) {
         char ctxt_name_data[BUFFER_LENGTH];
         char ctxt_data[BUFFER_LENGTH];
         sscanf(data, "%s%s", ctxt_name_data, ctxt_data);
         ctxt = atoi(ctxt_data);
      } else if(strncmp(data, "btime", 5) == 0) {
         char btime_name_data[BUFFER_LENGTH];
         char btime_data[BUFFER_LENGTH];
         sscanf(data, "%s%s", btime_name_data, btime_data);
         btime = atoi(btime_data);
      } else if(strncmp(data, "process", 7) == 0) {
         char p_name_data[BUFFER_LENGTH];
         char p_data[BUFFER_LENGTH];
         sscanf(data, "%s%s", p_name_data, p_data);
         processes = atoi(p_data);
      }
   }
   fclose(ptr_file);
   
   printf("Time that all CPUs spent in user mode is: %d USER_HZ\n", cpu_user_time);
   printf("Time that all CPUs spent in system mode is: %d USER_HZ\n", cpu_sys_time);
   printf("Time that all CPUs spent in idle mode is: %d USER_HZ\n", cpu_idle_time);
   printf("Number of context switches the kernel has performed: %d\n", ctxt);
   printf("Time when the system last booted: %s", ctime(&btime));
   printf("Number of processes created since the system was booted: %d\n", processes);
}

/* Gets the amount of reads and writes made to the disk
 * from /proc/diskstats
 */
void get_disk_info(void) {
   char data[BUFFER_LENGTH];
   FILE *ptr_file = open_file(DISK_FILE);
   int read = 0;
   int write = 0;
   while(fgets(data, sizeof data, ptr_file) != NULL) {
      char skip[BUFFER_LENGTH];
      char name[BUFFER_LENGTH];
      char rd[BUFFER_LENGTH];
      char wr[BUFFER_LENGTH];
      sscanf(data, "%s%s%s%s%s%s%s%s", skip, skip, name, rd, skip, skip, skip, wr);
      if ((strcmp(name, "sda") == 0) || (strcmp(name, "sdb") == 0) || (strcmp(name, "sdc") == 0)) {
         read += atoi(rd);
         write += atoi(wr);
      }
   }
   fclose(ptr_file);
   
   printf("Number of disk reads: %d\n", read);
   printf("Number of disk writes: %d\n\n", write);
}

/* Gets the amount of memory the system has configured (in kB) and
 * the amount of memory that is free to use (in kB). Info retrieved
 * from /proc/meminfo
 */
void get_mem_info(void) {
   char data[BUFFER_LENGTH];
   FILE *ptr_file = open_file(MEM_FILE);
   int kb_ram_total;
   int kb_ram_free;
   while(fgets(data, sizeof data, ptr_file) != NULL) {
      char name[BUFFER_LENGTH];
      char mem_data[BUFFER_LENGTH];
      sscanf(data, "%s%s", name, mem_data);
      if(strncmp(name, "MemTotal", 8) == 0) {
         kb_ram_total = atoi(mem_data);
      } else if (strncmp(name, "MemFree", 7) == 0) {
         kb_ram_free = atoi(mem_data);
      }
   }
   fclose(ptr_file);
   
   printf("Amount of Memory on the machine: %d kB\n", kb_ram_total);
   printf("Amount of Free Memory on the machine: %d kB\n", kb_ram_free); 
}

/* Creates a list of averaged load averages that is sampled 
 * over a minute. Averages are computed every user-given
 * interval over a user-given duration. Info retrieved from
 * /proc/loadavg
 */
void get_ldavg_info(int interval, int duration) {
   char data[BUFFER_LENGTH];
   double total = 0;
   FILE *ptr_file = open_file(LDAVG_FILE);
   int i = 1;
   int iteration = 0;
   printf("Listing Averages of Load Averages every %ds interval up to %ds\n", interval, duration);
   printf("--------------------------------------------------\n\n");
   
   while (iteration < duration) {
      sleep(interval);
      char ld1_data[BUFFER_LENGTH];
	   double ld1;
      fgets(data, sizeof data, ptr_file);
      sscanf(data, "%s", ld1_data);
	   ld1 = atof(ld1_data);
	   total = (total + ld1) / i;
      printf("Avg. of %d load avg(s): %.2lf\n", i,  total);
	   i++;
      iteration += interval;
   }
}
