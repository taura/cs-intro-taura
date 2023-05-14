/**
   @file cycle.h
   @brief a small procedure to get CPU/reference cycle
 */

/* these two are Linux-specific.
   make them zero on other OSes */
#if __linux__
#define HAVE_PERF_EVENT 1
#define HAVE_CLOCK_GETTIME 1
#else
#define HAVE_PERF_EVENT 0
#define HAVE_CLOCK_GETTIME 0
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>
#include <x86intrin.h>

#if HAVE_PERF_EVENT
#include <linux/perf_event.h>
#include <asm/unistd.h>

/**
   this is a wrapper to Linux system call perf_event_open
 */
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags) {
  int ret;
  ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                group_fd, flags);
  return ret;
}
#endif

/**
   @brief a structure encapsulating a performance counter
 */
typedef struct {
  pthread_t tid;                /**< thread ID this is valid for */
  int fd;                       /**< what perf_event_open returned  */
  const char * name;                  /**< name */
} perf_counter_t;

/**
   @brief make a perf_counter
   @details 
   perf_counter_t t = mk_perf_counter();
   long c0 = perf_counter_get(t);
      ... do something ...
   long c1 = perf_counter_get(t);
   long dc = c1 - c0; <- the number of CPU clocks between c0 and c1
  */
static perf_counter_t mk_perf_counter(unsigned long event_code, const char * name) {
  perf_counter_t cc = { 0, -1, name };
  cc.tid = pthread_self();
#if HAVE_PERF_EVENT
  struct perf_event_attr pe;
  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(struct perf_event_attr);
  pe.config = event_code;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;
  
  int fd = perf_event_open(&pe, 0, -1, -1, 0);
  if (fd == -1) {
    perror("perf_event_open");
  } else if (ioctl(fd, PERF_EVENT_IOC_RESET, 0) == -1) {
    perror("ioctl");
  } else if (ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) == -1) {
    perror("ioctl");
  } else {
    cc.fd = fd;
  }
#endif
  if (cc.fd == -1) {
    fprintf(stderr, "%s:%d:warning: OS does not support perf_event. event counts are always zero\n",
            __FILE__, __LINE__);
  }
  return cc;
}

/**
   @brief destroy a cpu clock counter
  */
static void perf_counter_destroy(perf_counter_t cc) {
  if (cc.fd != -1) {
    close(cc.fd);
  }
}

/**
   @brief get CPU clock
  */
static long long perf_counter_get(perf_counter_t cc) {
  pthread_t tid = pthread_self();
  if (tid != cc.tid) {
    fprintf(stderr,
            "%s:%d:perf_counter_get: the caller thread (%ld)"
            " is invalid (!= %ld)\n", 
            __FILE__, __LINE__, (long)tid, (long)cc.tid);
    return -1;
  } else {
    long long c = 0;
    if (cc.fd != -1) {
      ssize_t rd = read(cc.fd, &c, sizeof(long long));
      if (rd == -1) {
        perror("read"); 
        exit(EXIT_FAILURE);
      }
      assert(rd == sizeof(long long));
    }
    return c;
  }
}

/**
   @brief get ns
  */
static inline long long cur_time_ns() {
#if HAVE_CLOCK_GETTIME
  struct timespec ts[1];
  if (clock_gettime(CLOCK_REALTIME, ts) == -1) {
    perror("clock_gettime"); exit(1);
  }
  return ts->tv_sec * 1000000000L + ts->tv_nsec;
#else
  /* resort to us-level timer */
  struct timeval tv[1];
  if (gettimeofday(tv, 0) == -1) {
    perror("gettimeofday"); exit(1);
  }
  return tv->tv_sec * 1000000000L + tv->tv_usec * 1000L;
#endif
}

/**
   @brief read reference clock
  */
static inline long long rdtsc() {
  long long u;
  asm volatile ("rdtsc;shlq $32,%%rdx;orq %%rdx,%%rax":"=a"(u)::"%rdx");
  return u;
}

enum { max_n_counters = 3 };
typedef struct {
  long n;
  long ns;
  long tsc;
  const char * names[max_n_counters];
  long values[max_n_counters];
} counters_t;

typedef struct {
  long n;
  perf_counter_t pcs[max_n_counters]; /* PERF_COUNT_HW_CPU_CYCLES, PERF_COUNT_HW_INSTRUCTIONS */
  counters_t start;
} profiler_t;

static void profiler_reset(profiler_t * p) {
  /* start ticking */
  p->start.n = p->n;
  p->start.ns = cur_time_ns();
  for (int i = 0; i < p->n; i++) {
    p->start.names[i] = p->pcs[i].name;
    p->start.values[i] = perf_counter_get(p->pcs[i]);
  }
  p->start.tsc = _rdtsc();
}

static profiler_t mk_profiler() {
  profiler_t p;
  struct {
    unsigned long code;
    const char * name;
  } events[] = {
    { PERF_COUNT_HW_CPU_CYCLES, "cpu cycles" },
    { PERF_COUNT_HW_INSTRUCTIONS, "instructions" },
    { PERF_COUNT_HW_BRANCH_INSTRUCTIONS, "branch instructions" }
  };
  p.n = sizeof(events) / sizeof(events[0]);
  assert(p.n <= max_n_counters);
  for (int i = 0; i < p.n; i++) {
    p.pcs[i] = mk_perf_counter(events[i].code, events[i].name);
  }
  profiler_reset(&p);
  return p;
}

static counters_t profiler_get(profiler_t p) {
  counters_t c;
  c.n = p.n;
  c.ns = cur_time_ns() - p.start.ns;
  for (int i = 0; i < p.n; i++) {
    c.names[i] = p.start.names[i];
    c.values[i] = perf_counter_get(p.pcs[i]) - p.start.values[i];
  }
  c.tsc = _rdtsc() - p.start.tsc;
  return c;                     /* OK */
}

static void profiler_destroy(profiler_t p) {
  for (int i = 0; i < p.n; i++) {
    perf_counter_destroy(p.pcs[i]);
  }
}
