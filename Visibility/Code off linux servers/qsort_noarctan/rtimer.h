#ifndef RTIMER_H
#define RTIMER_H

#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

typedef struct {
  struct rusage rut1, rut2, ruttot;
  struct timeval tv1, tv2, tvtotal;
} Rtimer;

#define rt_init(rt)					\
  rt.ruttot.ru_utime.tv_usec = 0;			\
  rt.ruttot.ru_utime.tv_sec = 0;			\
  rt.ruttot.ru_stime.tv_usec = 0;			\
  rt.ruttot.ru_stime.tv_sec = 0;			\
  rt.tvtotal.tv_sec = 0;			       	\
  rt.tvtotal.tv_usec = 0;

#define rt_start(rt)	                                \
  if((getrusage(RUSAGE_SELF, &rt.rut1) < 0)		\
	 || (gettimeofday(&(rt.tv1), NULL) < 0)) {	\
	perror("rusage/gettimeofday");	                \
	exit(1);					\
  }


/* updates endtimes, then stores the difference in the totals */
#define rt_stop(rt)						\
  if((getrusage(RUSAGE_SELF, &rt.rut2) < 0)			\
	 || (gettimeofday(&(rt.tv2), NULL) < 0)) {		\
        perror("rusage/gettimeofday");				\
        exit(1);						\
  }								\
  rt.ruttot.ru_utime.tv_usec += rt.rut2.ru_utime.tv_usec - rt.rut1.ru_utime.tv_usec; \
  rt.ruttot.ru_utime.tv_sec += rt.rut2.ru_utime.tv_sec - rt.rut1.ru_utime.tv_sec; \
  rt.ruttot.ru_stime.tv_usec += rt.rut2.ru_stime.tv_usec - rt.rut1.ru_stime.tv_usec;	\
  rt.ruttot.ru_stime.tv_sec += rt.rut2.ru_stime.tv_sec - rt.rut1.ru_stime.tv_sec;	\
  rt.tvtotal.tv_sec += rt.tv2.tv_sec - rt.tv1.tv_sec;			\
  rt.tvtotal.tv_usec += rt.tv2.tv_usec - rt.tv1.tv_usec;    


/* not required to be called, but makes values print as 0. 
   obviously a hack */
#define rt_zero(rt) bzero(&(rt),sizeof(Rtimer));
	
/*all the funtions for calculating and outputting cumulative times*/
#define rt_u_useconds_total(rt)				        \
	((double)rt.ruttot.ru_utime.tv_usec +			\
	  (double)rt.ruttot.ru_utime.tv_sec*1000000)		
	
#define rt_s_useconds_total(rt)					\
	 ((double)rt.ruttot.ru_stime.tv_usec +	       	\
	   (double)rt.ruttot.ru_stime.tv_sec*1000000)		

#define rt_w_useconds_total(rt)				\
	 ((double)rt.tvtotal.tv_usec +			\
	   (double)rt.tvtotal.tv_sec*1000000)		

#define rt_seconds_total(rt) (rt_w_useconds_total(rt)/1000000)

#define rt_sprint_total(buf, rt) rt_sprint_safe_total(buf,rt)

char * rt_sprint_safe_total(char *buf, Rtimer rt);


/*all the functions for calculating and outputting one timer*/
#define rt_u_useconds(rt)				        \
	(((double)rt.rut2.ru_utime.tv_usec +			\
	  (double)rt.rut2.ru_utime.tv_sec*1000000)		\
	 - ((double)rt.rut1.ru_utime.tv_usec +			\
		(double)rt.rut1.ru_utime.tv_sec*1000000))

#define rt_s_useconds(rt)					\
	 (((double)rt.rut2.ru_stime.tv_usec +			\
	   (double)rt.rut2.ru_stime.tv_sec*1000000)		\
	  - ((double)rt.rut1.ru_stime.tv_usec +			\
		 (double)rt.rut1.ru_stime.tv_sec*1000000))

#define rt_w_useconds(rt)				\
	 (((double)rt.tv2.tv_usec +			\
	   (double)rt.tv2.tv_sec*1000000)		\
	  - ((double)rt.tv1.tv_usec +			\
		 (double)rt.tv1.tv_sec*1000000))

#define rt_seconds(rt) (rt_w_useconds(rt)/1000000)

#define rt_sprint(buf, rt) rt_sprint_safe(buf,rt)

char * rt_sprint_safe(char *buf, Rtimer rt);


#endif /* RTIMER_H */
