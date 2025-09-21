#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <locale.h>
#include <sys/select.h>
#include "proto.h"


#define BUF_SIZE 1024


static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ip-servidor> <porta>\nEx.: %s 127.0.0.1 5000\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Porta inválida.\n");
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) die("socket");
    

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "IP inválido: %s\n", server_ip);
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) die("connect");


    printf("Conectado a %s:%d\n", server_ip, port);
    printf("Digite expressões (no formato prefixo ou infixo) e pressione ENTER. Digite QUIT para sair.\n");

    fd_set rset;
    char envio[BUF_SIZE];
    char resp[BUF_SIZE];


    int stdin_closed = 0;
    for (;;) {
        FD_ZERO(&rset);
        if (!stdin_closed)
            FD_SET(STDIN_FILENO, &rset);
        FD_SET(sockfd, &rset);

        int maxfd = (sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO);

        if (select(maxfd + 1, &rset, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            die("select");
        }

        if (!stdin_closed && FD_ISSET(STDIN_FILENO, &rset)) {
            if (fgets(envio, sizeof(envio), stdin) == NULL) {
                // EOF: shutdown write side, but keep reading from server
                shutdown(sockfd, SHUT_WR);
                stdin_closed = 1;
            } else {
                size_t len = strlen(envio);
                if (len > 0) {
                    ssize_t n = send(sockfd, envio, len, 0);
                    if (n < 0) {
                        perror("send");
                        break;
                    }
                }
            }
        }

        if (FD_ISSET(sockfd, &rset)) {
            ssize_t n = recv(sockfd, resp, sizeof(resp) - 1, 0);
            if (n <= 0) {
                if (n < 0) perror("recv");
                //printf("Servidor encerrou a conexão.\n");
                break;
            }
            resp[n] = '\0';
            printf("%s", resp);
            fflush(stdout);
        }
    }

    close(sockfd);
    return 0;
}
