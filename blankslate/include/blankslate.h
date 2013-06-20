#ifndef LIBBLANKSLATE_H
#define LIBBLANKSLATE_H

extern int blankslate_open(void);

extern int blankslate_close(int fd);

extern int blankslate_set_name(int fd, char *name);

extern int blankslate_reset_name(int fd);

extern int blankslate_set_rutabaga_count(int fd, int count);

#endif /* LIBBLANKSLATE_H */
