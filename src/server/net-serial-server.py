#!/usr/bin/env python
#coding=utf-8

import socket
import threading
import SocketServer
import os,sys
import logging
import struct
import time

logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(levelname)s %(message)s',
                    filename='log.txt',
                    filemode='a+')

TIMEOUT = 30;
HEADER_SIZE = 6;
SERIAL_PATH_SIZE = 64;
GET_STATUS_SIZE = 72;

ACK_OK = 0;
ACK_NACK = 1;

''' Packet type
Packet struct:
struct hdr {
    unsigned short type;
    unsigned int length;
    unsigned char byte[]
};
struct.unpack fmt str: "=HI"

hdr.length = 1
struct ack_ {
    unsigned char error;
};
struct.unpack fmt str: "=B"

hdr.length = 72
strcut ack_status {
    char serial_path[SERIAL_PATH_SIZE];
    unsigned int baud;
    unsigned char csize;
    unsigned char parity;
    unsigned char stopbits;
    unsigned char is_open;
};
struct.unpack fmt str: "={}sIBBBB".format(SERIAL_PATH_SIZE)

'''
GET_STATUS =    1;
OPEN_SERIAL =   2;
SETUP_SERIAL =  3;
READ_SERIAL =   4;
WRITE_SERIAL =  5;
CLOSE_SERIAL =  6;
RESET_TERMINAL =0xffff;

def LogTemplate(s):
    return '[' + threading.current_thread().name + ']: ' + str(s);

def Log(s):
    ss = LogTemplate(s);
    print ss;
    logging.info(ss);

def LogE(s):
    ss = LogTemplate(s);
    print ss;
    logging.error(ss);


class SerialClientRequestHandler(SocketServer.BaseRequestHandler):

    def setup(self):
        # set timeout to 30s
        self.request.settimeout(TIMEOUT);
        msg = 'Begin to serve client ' + str(self.client_address);
        Log(msg);

    def handle(self):
        cur_thread = threading.current_thread();
        try:
            # main loop to process protocol
            hdr = struct.pack("=HI", GET_STATUS, 0);
            self.request.send(hdr);
            status_ack_hdr = self.request.recv(HEADER_SIZE);
            (type, length) = struct.unpack("=HI", status_ack_hdr);
            print repr(status_ack_hdr), type, length
            if type != GET_STATUS or length != GET_STATUS_SIZE:
                LogE('Unexcepted data received! Invalid client!');
                return;
            status_ack = self.request.recv(GET_STATUS_SIZE);
            (serial_path, baud, csize, parity, stopbits, is_open) = \
                    struct.unpack("={}sIBBBB".format(SERIAL_PATH_SIZE), status_ack);
            Log('client [' + str(self.client_address) +
                     '] status: baud[' + str(baud) + '] csize[' + str(csize) + '] parity[' +
                     str(parity) + '] stopbits[' + str(stopbits) + '] is_open[' + str(is_open));
            hdr = struct.pack("=HI", RESET_TERMINAL, 0);
            Log('Close client' + str(self.client_address));
            #client_db.insert(self.request.client_address, (serial_path, baud, csize, parity, stopbits, is_open));
        except socket.timeout:
            LogE('client ' + str(self.client_address) + ' timeout!!');

    def finish(self):
        self.request.close();
        Log('finished');

class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    pass

def StartServer(in_addr):
    server = ThreadedTCPServer(in_addr, SerialClientRequestHandler);
    server_thread = threading.Thread(target=server.serve_forever);
    server_thread.daemon = True;
    server_thread.start();
    Log('Server loop running in thread: ' + server_thread.name);
    time.sleep(10);
    Log('Server stop!');
    server.shutdown();

if __name__ == "__main__":
    HOST, PORT = "localhost", 2014
    StartServer((HOST, PORT));
