#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

static char *serial_path;
static int sock;

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

void send_ack(char *recv_buf, int is_good, int error)
{
    int sent;
    struct hdr *hdr = (struct hdr *)recv_buf;
    struct ack *ack = (struct ack *)hdr->byte;

    if (!sock) return;
    ack->type = hdr->type;
    hdr->type = ACK;
    hdr->length = sizeof(struct ack);
    if (is_good) {
        ack->ack = ACK_OK;
        ack->error = 0;
    } else {
        ack->ack = ACK_NACK;
        ack->error = errno;
    }
    sent = send(sock, recv_buf, hdr->length + HEADER_SIZE, 0);
    print_hex(buffer, sent);
    if (sent != hdr->length + HEADER_SIZE)
        Die("Ack OPEN_SERIAL cmd error!\n");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in net_serial_server;
    char buffer[BUFFSIZE];
    unsigned int echolen;
    int received = 0, sent = 0;
    unsigned int offload_size;
    struct hdr *hdr;
    struct ack_status serial_status;
    struct ack *ack;
    int ret;

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
                strncmp(serial_status.serial_path, serial_path, SERIAL_PATH_SIZE);
                serial_get_status(&serial_status);
                memcpy(hdr->byte, &serial_status, sizeof(serial_status));
                hdr->length = sizeof(serial_status);
                sent = send(sock, buffer, hdr->length + HEADER_SIZE, 0);
                print_hex(buffer, sent);
                if (sent != hdr->length + HEADER_SIZE)
                    Die("Ack GET_STATUS cmd error!\n");
                break;
            case OPEN_SERIAL:
                printf("OPEN_SERIAL\n");
                if (serial_path) {
                    ret = serial_open(serial_path);
                    send_ack(buffer, ret < 0 ? 0 : 1, errno);
                }
                break;
            case SETUP_SERIAL:
                printf("SETUP_SERIAL\n");
                memcpy(&serial_status, hdr->byte, sizeof(serial_status));
                serial_setup(&serial_status);
                send_ack(buffer, 1, 0);
                break;
            case READ_SERIAL:
            case WRITE_SERIAL:
            case CLOSE_SERIAL:
                printf("CLOSE_SERIAL\n");
                serial_close();
                send_ack(buffer, 1, 0);
                break;
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
