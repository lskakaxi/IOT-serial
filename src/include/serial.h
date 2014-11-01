#ifndef _SERIAL_H
#define _SERIAL_H

void serial_get_status(struct ack_status *status);
void serial_setup(struct ack_status *status);
int serial_open(const char *path);
int serial_read(char *buf, int bufsize);
int serial_write(char *buf, int size);
void serial_close(void);
#endif
