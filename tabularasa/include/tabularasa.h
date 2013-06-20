#ifndef LIBTABULARASA_H
#define LIBTABULARASA_H

extern int tabularasa_open(void);

extern int tabularasa_close(int fd);

extern int tabularasa_set_name(int fd, char *name);

extern int tabularasa_reset_name(int fd);

#endif /* LIBTABULARASA_H */
