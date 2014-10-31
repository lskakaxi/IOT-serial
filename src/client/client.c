#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

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
#define RESET_TERMINAL  0xffff

struct hdr {
    unsigned short type;
    unsigned int length;
    unsigned char byte[0];
} __attribute__((__packed__));

struct ack {
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

void Die(char *mess) { perror(mess); exit(1); }

void print_hex(unsigned char *hex, int size)
{
    unsigned int i;
    if (size <= 0) return;
    while (size--) {
        if (i == 16) {
            printf("\n");
            i = 0;
        }
        printf("%02x ", *hex++);
        i++;
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in net_serial_server;
    char buffer[BUFFSIZE];
    unsigned int echolen;
    int received = 0, sent = 0;
    unsigned int offload_size;
    char *serial_path;
    struct hdr *hdr;
    struct ack_status serial_status;

    printf("hdr size: %d\n", sizeof(struct hdr));
    if (argc != 4) {
        fprintf(stderr, "usage: TCPserial <server_ip> <port> <serial_path>\n");
        exit(1);
    }
    serial_path = argv[3];

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        Die("failed to create socket!\n");

    memset(&net_serial_server, 0, sizeof(net_serial_server));
    net_serial_server.sin_family = AF_INET;
    net_serial_server.sin_addr.s_addr = inet_addr(argv[1]);
    net_serial_server.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *) &net_serial_server, sizeof(net_serial_server)) < 0)
        Die("failed to connect with server!\n");

    memset(buffer, 0, sizeof(buffer));
    received = recv(sock, buffer, HEADER_SIZE, 0);
    while (received > 0) {
        hdr = (struct hdr *)buffer;
        if (hdr->length)
            received += recv(sock, hdr->byte, hdr->length, 0);
        printf("package len: %d received[%d]\n", hdr->length, received);
        print_hex(buffer, received);
        switch (hdr->type) {
            case GET_STATUS:
                printf("GET_STATUS\n");
                strncmp(serial_status.serial_path, "/dev/ttyUSB0", sizeof("/dev/ttyUSB0") + 1);
                serial_status.baud = 115200;
                serial_status.csize = 8;
                serial_status.parity = 0;
                serial_status.stopbits = 1;
                serial_status.is_open = 1;
                memcpy(hdr->byte, &serial_status, sizeof(serial_status));
                hdr->length = sizeof(serial_status);
                sent = send(sock, buffer, hdr->length + HEADER_SIZE, 0);
                print_hex(buffer, sent);
                if (sent != hdr->length + HEADER_SIZE)
                    Die("Ack GET_STATUS cmd error!\n");
                break;
            case OPEN_SERIAL:
            case SETUP_SERIAL:
            case READ_SERIAL:
            case WRITE_SERIAL:
            case CLOSE_SERIAL:
            case RESET_TERMINAL:
                close(sock);
                exit(0);
            default:
                Die("Invild protocol type!\n");
        }
        memset(buffer, 0, sizeof(buffer));
        received = recv(sock, buffer, HEADER_SIZE, 0);
    }
    close(sock);
    exit(0);
}
