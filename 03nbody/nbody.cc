/* 
 * nbody.cc
 */
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>
#include "counter.h"
#include "nbody.h"

void die(const char * s) {
  perror(s);
  exit(1);
}

#if __AVX512F__
enum { VS=64 };
#elif __AVX2__
enum { VS=32 };
#else
#error "neither __AVX512F__ nor __AVX2__ defined"
#endif

typedef int32_t particle_idx;
typedef real         realv         __attribute__((vector_size(VS),aligned(sizeof(real))));
typedef particle_idx particle_idxv __attribute__((vector_size(VS),aligned(sizeof(real))));
enum {
  n_lanes = sizeof(realv) / sizeof(real),
};

#if __AVX512F__
realv set1(real c) { return _mm512_set1_ps(c); }
#elif __AVX2__
realv set1(real c) { return _mm256_set1_ps(c); }
#else
#error "neither __AVX512F__ nor __AVX2__ defined"
#endif


struct vecv {
  realv x[3];
  vecv(realv a, realv b, realv c) {
    x[0] = a; x[1] = b; x[2] = c;
  }
  vecv(realv a) {
    x[0] = a; x[1] = a; x[2] = a;
  }
  vecv() { }
};

vec operator+(vec a, vec b) {
  return vec(a.x[0] + b.x[0], a.x[1] + b.x[1], a.x[2] + b.x[2]);
}

vecv operator+(vecv a, vecv b) {
  return vecv(a.x[0] + b.x[0], a.x[1] + b.x[1], a.x[2] + b.x[2]);
}

vec operator-(vec a, vec b) {
  return vec(a.x[0] - b.x[0], a.x[1] - b.x[1], a.x[2] - b.x[2]);
}

vecv operator-(vecv a, vecv b) {
  return vecv(a.x[0] - b.x[0], a.x[1] - b.x[1], a.x[2] - b.x[2]);
}

vecv operator-(vec a, vecv b) {
  return vecv(set1(a.x[0]) - b.x[0], set1(a.x[1]) - b.x[1], set1(a.x[2]) - b.x[2]);
}

vec operator*(vec a, real k) {
  return vec(k * a.x[0], k * a.x[1], k * a.x[2]);
}

vecv operator*(vecv a, realv k) {
  return vecv(k * a.x[0], k * a.x[1], k * a.x[2]);
}

vec operator/(vec a, real k) {
  return a * (1.0 / k);
}

real dot(vec u, vec v) {
  return u.x[0] * v.x[0] + u.x[1] * v.x[1] + u.x[2] * v.x[2];
}

realv dot(vecv u, vecv v) {
  return u.x[0] * v.x[0] + u.x[1] * v.x[1] + u.x[2] * v.x[2];
}

real norm2(vec u) {
  return dot(u, u);
}

realv norm2v(vecv u) {
  return dot(u, u);
}

typedef struct {
  particle_idx idx;
  real m;
  vec pos;
  vec vel;
  vec acc;
} particle;

typedef struct {
  particle_idxv idx;
  realv m;
  vecv pos;
  vecv vel;
  vecv acc;
} particlev;

typedef struct {
  long n_particles;
  long n_steps;
  real dt;
  real T;
  real rv;
  real eps;
  char * dump_file;
  long dump_n_particles;
  long dump_start;
  long dump_stop;
  long dump_interval;
  long seed;
} options_t;

#if __AVX512F__
realv rsqrtv(realv x) {
  __m256 x0 = {x[0], x[1], x[2], x[3]};
  __m256 x1 = {x[4], x[5], x[6], x[7]};
  __m256 y0 = _mm256_rsqrt_ps(x0);
  __m256 y1 = _mm256_rsqrt_ps(x1);
  realv y = {y0[0], y0[1], y0[2], y0[3], 
             y1[0], y1[1], y1[2], y1[3]};
  return y;
}
#elif __AVX2__
realv rsqrtv(realv x) {
  return _mm256_rsqrt_ps(x);
}
#endif

real rsqrt(real x) {
  return rsqrtv(set1(x))[0];
}

#include NBODYx_H

void ball(real x[3], unsigned short rg[3]) {
  for (int i = 0; i < 1000; i++) {
    real l = 0.0;
    for (long d = 0; d < 3; d++) {
      x[d] = 2.0 * erand48(rg) - 1.0;
      l += x[d] * x[d];
    }
    if (l < 1.0) return;
  }
  // if this happens, it's 99.999% likely to be a bug
  assert(0);
}

real gaussian(unsigned short rg[3]) {
  while (1) {
    real x = 2.0 * erand48(rg) - 1.0;
    real y = 2.0 * erand48(rg) - 1.0;
    real r2 = x * x + y * y;
    if (0.0 < r2 && r2 < 1.0) {
      return sqrt(-2.0 * log(r2) / r2) * x;
    }
  }
  assert(0);
}

void init(long n, particle * p, particlev * pv, options_t * o) {
  unsigned short rg[3] = { (unsigned short)(o->seed >> 32),
			   (unsigned short)(o->seed >> 16),
			   (unsigned short)(o->seed >> 0) };
  real M = 0.0;
  for (long i = 0; i < n; i++) {
    p[i].idx = i;
    p[i].m = 1.0 / n;
    M += p[i].m;
    ball(p[i].pos.x, rg);
  }
  real U = interact_all(n, p, pv, o);
  real sigma = sqrt(2 * o->rv * fabs(U) / (3 * M));
  for (long i = 0; i < n; i++) {
    for (long d = 0; d < 3; d++) {
      p[i].vel.x[d] = sigma * gaussian(rg);
      p[i].acc.x[d] = 0.0;
    }
  }
  // dummy particles
  for (long i = n; i % n_lanes != 0; i++) {
    p[i].idx = i;
    p[i].m = 0.0;
    for (long d = 0; d < 3; d++) {
      p[i].pos.x[d] = 0.0;
      p[i].vel.x[d] = 0.0;
      p[i].acc.x[d] = 0.0;
    }
  }
}

void dump_config_txt(long step, real t, real T, long n, particle * p,
                     real U, real K, FILE * wp) {
  fprintf(wp, "step: %ld t: %.9f T: %.9f n: %ld U: %f K: %f U+K: %f\n",
	  step, t, T, n, U, K, U + K);
  for (long i = 0; i < n; i++) {
    real m = p[i].m;
    vec pos = p[i].pos;
    vec vel = p[i].vel;
    vec acc = p[i].acc;
    fprintf(wp, "%ld m: %f pos: %f %f %f vel: %f %f %f acc: %f %f %f\n",
	    i, m,
	    pos.x[0], pos.x[1], pos.x[2],
	    vel.x[0], vel.x[1], vel.x[2],
	    acc.x[0], acc.x[1], acc.x[2]);
  }
  fflush(wp);
}

void dump_config(long step, real t, real T, long n, particle * p,
		 real U, real K, FILE * wp) {
  cfg_record c;
  c.step = step;
  c.n = n;
  c.t = t;
  c.T = T;
  c.U = U;
  c.K = K;
  for (long i = 0; i < n; i++) {
    c.idx = i;
    c.m = p[i].m;
    c.pos = p[i].pos;
    c.vel = p[i].vel;
    c.acc = p[i].acc;
    size_t w = fwrite(&c, sizeof(c), 1, wp);
    if (w != 1) die("fwrite");
  }
}

void help(options_t * o, char * argv0) {
  fprintf(stderr,
	  "usage:\n"
	  "  %s [options]:\n"
	  "options:\n"
	  " -n N : simulate N particles (%ld)\n"
	  " -t dt : time step (%f)\n"
	  " -T T : simulation end time (%f)\n"
	  " -s N : simulate N steps (%ld)\n"
	  " -r rv : virial ratio (%f)\n"
	  " -e eps : softening parameter (%f)\n"
	  " -o FILE : dump output to FILE (%s)\n"
	  " -N N : dump N particles (%ld)\n"
	  " -d A:B:C : dump between step A up to step B-1, with interval C (i.e., dump at step A, A+C, A+2C, ...) (%ld:%ld:%ld)\n"
	  " -x X : set random seed to X, to change initial configuration (%ld)\n"
          ,
	  argv0,
	  o->n_particles,
          o->dt,
          o->T,
	  o->n_steps,
          o->rv,
          o->eps,
	  o->dump_file,
	  o->dump_n_particles,
	  o->dump_start,
	  (o->dump_stop == -1 ? o->n_steps : o->dump_stop),
	  o->dump_interval,
	  o->seed);
}

/*
 *  a:b:c   a to b, every c iterations
 */

int parse_dump(char * s, options_t * o) {
  char * s0 = s;
  char * col0 = strchr(s0, ':');
  char * s1 = (col0 ? col0 + 1 : 0); // char after the first :, or NULL
  char * col1 = (s1 ? strchr(s1, ':') : 0);
  char * s2 = (col1 ? col1 + 1 : 0); // char after the second :, or NULL
  /* parse a of a:b:c 
     parse it if we miss the first colon (i.e., this is the last part)
     or we have the first colon and it is at least a char ahead of s0 */
  if (!s1 || col0 - s0 > 0) {
    if (isdigit(s0[0])) {	// lightly check if s0 seems to start a number
      o->dump_start = atol(s0);
    } else {
      fprintf(stderr,
	      "error: malformed dump interval %s (valid form: a, a:b, a:b:c)\n",
	      s);
      return 0;
    }
  }
  // b part of a:b:c
  if (s1 && (!s2 || col1 - s1 > 0)) {
    if (isdigit(s1[0])) {
      o->dump_stop = atol(s1);
    } else {
      fprintf(stderr,
	      "error: malformed dump interval %s (valid form: a, a:b, a:b:c)\n",
	      s);
      return 0;
    }
  }
  if (s2 && strlen(s2) > 0) {
    if (isdigit(s2[0])) {
      o->dump_interval = atol(s2);
    } else {
      fprintf(stderr,
	      "error: malformed dump interval %s (valid form: a, a:b, a:b:c)\n",
	      s);
      return 0;
    }
  }
  return 1;			// OK
}

void set_defaults(options_t * o) {
  if (o->n_steps == 0) {
    o->n_steps = o->T / o->dt;
  }
  if (o->dump_n_particles == -1) {
    o->dump_n_particles = o->n_particles;
  }
  if (o->dump_stop == -1) {
    o->dump_stop = o->n_steps;
  }
}

int parse_args(int argc, char ** argv, options_t * o) {
  o->n_particles = 1024;
  o->n_steps = 0;
  o->dt = 0.001;
  o->T = 0.1;
  o->rv = 0.5;
  o->eps = 1.0e-3;

  o->dump_file = 0;
  o->dump_n_particles = -1;
  o->dump_start = 0;
  o->dump_stop = -1;
  o->dump_interval = 1;
  o->seed = 9876543210987654L;
  while (1) {
    int f = getopt(argc, argv, "n:s:t:T:r:e:o:N:d:x:vp");
    switch (f) {
    case -1:
      set_defaults(o);
      return 1;			// OK
    case 'n':
      o->n_particles = atol(optarg);
      break;
    case 's':
      o->n_steps = atol(optarg);
      break;
    case 't':
      o->dt = atof(optarg);
      break;
    case 'T':
      o->T = atof(optarg);
      break;
    case 'r':
      o->rv = atof(optarg);
      break;
    case 'e':
      o->eps = atof(optarg);
      break;
    case 'o':
      o->dump_file = strdup(optarg);
      break;
    case 'N':
      o->dump_n_particles = atol(optarg);
      break;
    case 'd':
      if (!parse_dump(optarg, o)) return 0; // NG
      break;
    case 'x':
      o->seed = atol(optarg);
      break;
    default:
      help(o, argv[0]); exit(1);
    }
  }
  assert(0);			// should not reach here
}

int main(int argc, char ** argv) {
  options_t o[1];
  if (!parse_args(argc, argv, o)) return 1; // NG
  long n = o->n_particles;
  long nv = (n + n_lanes - 1) / n_lanes;
  long n_steps = o->n_steps;
  real dt = o->dt;
  real T = o->T;
  real rv = o->rv;
  real eps = o->eps;
  particle  * p  = (particle *) malloc(sizeof(particle)  * nv * n_lanes);
  particlev * pv = (particlev *)malloc(sizeof(particlev) * nv);
  FILE * wp = 0;

  printf("n_particles: %ld,"
         " n_steps: %ld,"
         " dt: %f,"
         " T: %f,"
         " rv: %f,"
         " eps: %f,"
         " dump_file: %s,"
         " dump_n_particles: %ld,"
         " dump: %ld:%ld:%ld,"
         " seed: %ld"
         "\n",
         n,
         n_steps,
         dt,
         T,
         rv,
         eps,
         o->dump_file ? o->dump_file : "None",
         o->dump_n_particles,
         o->dump_start, o->dump_stop, o->dump_interval,
         o->seed);
  if (o->dump_file && (o->dump_start < n_steps)) {
    if (strcmp(o->dump_file, "-") == 0) {
      wp = stdout;
    } else {
      wp = fopen(o->dump_file, "w");
      if (!wp) { perror("fopen"); }
    }
    if (!wp) {
      return 1; // NG
    }
  }
  
  init(n, p, pv, o);
  real t = 0.0;
  profiler_t pr = mk_profiler();
  for (long i = 0; i < n_steps; i++) {
    counters_t t0 = profiler_get(pr);
    real U = interact_all(n, p, pv, o);
    counters_t t1 = profiler_get(pr);
    real K = kinetic(n, p);
    counters_t t2 = profiler_get(pr);
    if (wp && o->dump_start <= i && i < o->dump_stop &&
	(i - o->dump_start) % o->dump_interval == 0) {
      dump_config_txt(i, t, T, o->dump_n_particles, p, U, K, wp);
    }
    t += update(n, p, dt);
    counters_t t3 = profiler_get(pr);
    if (o->dump_start <= i && i < o->dump_stop &&
	(i - o->dump_start) % o->dump_interval == 0) {
      printf("iteration: %ld\n"
	     "interact_all: %ld ref clocks, %ld cpu clocks, %.3f sec\n"
	     "kinetic:      %ld ref clocks, %ld cpu clocks, %.3f sec\n"
	     "update:       %ld ref clocks, %ld cpu clocks, %.3f sec\n",
	     i,
	     t1.tsc - t0.tsc,
             t1.values[0] - t0.values[0],
             (t1.ns - t0.ns) * 1.0e-9,
	     t2.tsc - t1.tsc,
	     t2.values[0] - t1.values[0],
             (t2.ns - t1.ns) * 1.0e-9,
	     t3.tsc - t2.tsc,
	     t3.values[0] - t2.values[0],
             (t3.ns - t2.ns) * 1.0e-9);
      printf("interact_all: %f ppi/cpu clocks\n", 
	     (n * (n - 1)) / (double)(t1.values[0] - t0.values[0]));
      printf("iteration: %f ppi/cpu clocks\n", 
	     (n * (n - 1)) / (double)(t3.values[0] - t0.values[0]));
    }
  }
  counters_t end_t = profiler_get(pr);
  printf("all iterations: %f ppi/cpu clocks\n", 
         (n * (n - 1) * n_steps) / (double)end_t.values[0]);
  profiler_destroy(pr);
  if (wp && wp != stdout) {
    fclose(wp);
  }
  free(pv);
  free(p);
  return 0;
}

