#ifndef LIBHELLOWORLD_H
#define LIBHELLOWORLD_H

#define LIBHELLOWORLD_MAGIC 0xd34db33f

extern int helloworld_open(void);

extern int helloworld_close(int fd);

extern int helloworld_inc(int fd);


#endif /* LIBHELLOWORLD_H */
