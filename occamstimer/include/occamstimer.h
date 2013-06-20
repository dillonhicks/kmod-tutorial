#ifndef LIBOCCAMSTIMER_H
#define LIBOCCAMSTIMER_H

#include <linux/occamstimer.h>

extern int occamstimer_open(void);
extern int occamstimer_close(int fd);

extern int occamstimer_add_work(int fd, char *data, struct timespec *exec_int);
extern int occamstimer_get_work(int fd, char *data);

extern int occamstimer_get_status(int fd, enum occamstimer_status *status);
extern int occamstimer_set_status(int fd, enum occamstimer_status status);

extern int occamstimer_start_device(int fd);
extern int occamstimer_pause_device(int fd);



#endif /* LIBOCCAMSTIMER_H */
