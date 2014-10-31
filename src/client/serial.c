#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define SERIAL_PATH "/dev/ttyS0"
static int serial_fd;
static struct termios curr_options;

void serial_set_baud(int fd, speed_t speed)
{
    cfsetispeed(&curr_options, speed);
    cfsetospeed(&curr_options, speed);
    tcsetattr(fd, TCSANOW, &curr_options);
}

/* CSIZE  Character size mask.  Values are CS5, CS6, CS7, or CS8. */
void serial_set_csize(int fd, int data_bits)
{
    switch (data_bits) {
    case CS5:
    case CS6:
    case CS7:
    case CS8:
        curr_options.c_cflag &= ~CSIZE;
        curr_options.c_cflag |= data_bits;
        tcsetattr(fd, TCSANOW, &curr_options);
        break;
    default:
        printf("No such csize %d\n", data_bits);
    }
}

void serial_set_parity(int fd, int parity)
{
    switch (parity) {
    case 0: /* no parity */
        curr_options.c_cflag &= ~PARENB;
        break;
    case 1: /* even parity */
        curr_options.c_cflag |= PARENB;
        curr_options.c_cflag &= ~PARODD;
        break;
    case 2: /* odd parity */
        curr_options.c_cflag |= PARENB;
        curr_options.c_cflag |= PARODD;
        break;
    default:
        printf("Invalid parity %d!\n", parity);
    }
    tcsetattr(fd, TCSANOW, &curr_options);
}

void serial_set_stopbits(int fd, int stopbits)
{
    if (stopbits == 2)
        curr_options.c_cflag |= CSTOPB;
    else
        curr_options.c_cflag &= ~CSTOPB;
    tcsetattr(fd, TCSANOW, &curr_options);
}

/* 115200 8N1 */
void serial_setup_default(int fd)
{
    serial_set_baud(fd, B115200);
    serial_set_csize(fd, CS8);
    serial_set_stopbits(fd, 1);
    serial_set_parity(fd, 0);
}

int serial_open(const char *path)
{
    if (serial_fd) {
        printf("Serial port %s has been opened!\n", path);
        return 0;
    }
    /* O_NDELAY - return 0 when read if no data */
    serial_fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        perror("Unable to open port");
        return -1;
    } else
        fcntl(serial_fd, F_SETFL, 0);

    tcgetattr(serial_fd, &curr_options);
    /* make serial port in RAW mode */
    curr_options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
            | INLCR | IGNCR | ICRNL | IXON);
    curr_options.c_oflag &= ~OPOST;
    curr_options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    curr_options.c_cflag &= ~(CSIZE | PARENB);
    curr_options.c_cflag |= CS8;
    curr_options.c_cflag |= (CLOCAL | CREAD);
    /* disable hardware flow control */
    curr_options.c_cflag &= ~CRTSCTS;
    /* disable software flow control */
    curr_options.c_iflag &= ~(IXON | IXOFF | IXANY);

    serial_setup_default(serial_fd);
    tcsetattr(serial_fd, TCSANOW, &curr_options);

    return serial_fd;
}

void serial_close(int fd)
{
    close(fd);
    serial_fd = 0;
}

int serial_write(int fd, char *buf, int size)
{
    return write(fd, buf, size);
}

int serial_read(int fd, char *buf, int bufsize)
{
    return read(fd, buf, bufsize);
}

#ifdef TEST_STANDALONE
int main(void)
{
    int fd;
    char c;
    char buf[] = "Hello serial!\n";
    char inbuf[100];
    int n;
    fd = serial_open("/dev/ttyUSB0");
    if (fd == -1) {
        perror("cannot open port!");
        return -1;
    }
    while (c = getch()) {
        putchar(c);
        if (c == 'q') {
            serial_close(fd);
            return 0;
        }
        serial_write(fd, &c, 1);// sizeof(buf));
        n = serial_read(fd, inbuf, sizeof(inbuf));
        inbuf[n] = '\0';
        if (n != 0)
            printf("%s", inbuf);
    }
    return 0;
}
#endif
