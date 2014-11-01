#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#define HEADER_SIZE 6
#define SERIAL_PATH_SIZE 64
#define GET_STATUS_SIZE 72
#define BUFFSIZE 512

#define ACK_OK 0
#define ACK_NACK 1

#define GET_STATUS      1
#define OPEN_SERIAL     2
#define SETUP_SERIAL    3
#define READ_SERIAL     4
#define WRITE_SERIAL    5
#define CLOSE_SERIAL    6
#define ACK		0xff
#define RESET_TERMINAL  0xffff

struct hdr {
    unsigned short type;
    unsigned int length;
    unsigned char byte[0];
} __attribute__((__packed__));

struct ack {
    unsigned char type;
    unsigned char error;
} __attribute__((__packed__));

struct ack_status {
    char serial_path[SERIAL_PATH_SIZE];
    unsigned int baud;
    unsigned char csize;
    unsigned char parity;
    unsigned char stopbits;
    unsigned char is_open;
} __attribute__((__packed__));

#endif
