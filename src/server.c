#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <locale.h>
#include <ctype.h>
#include <sys/select.h>
#include "proto.h"



static int listen_fd = -1;
static int clients[MAX_CLIENTS];
static char client_bufs[MAX_CLIENTS][BUF_SIZE];
static size_t client_usados[MAX_CLIENTS];

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}


static int conta(char *linha, double *out, int formato) {
    char *save = NULL;
    char *a = strtok_r(linha, " \t", &save);
    char *b = strtok_r(NULL, " \t", &save);
    char *c = strtok_r(NULL, " \t", &save);
    char *extra = strtok_r(NULL, " \t", &save);

    if (!a || !b || !c || extra) return CALC_ERROR;

    char *e1, *e2, *op;
    double A, B;

    if (formato == FORMATO_PREFIXO) { // prefixo: op A B
        op = a;
        A = strtod(b, &e1);
        B = strtod(c, &e2);
    }
    else { // infixo: A op B
        A = strtod(a, &e1);
        op = b;
        B = strtod(c, &e2);
    }

    if (*e1 || *e2) return CALC_ERROR;

    if ((formato == FORMATO_PREFIXO && strcmp(op, OP_ADD) == 0) || (formato == FORMATO_INFIXO && strcmp(op, OP_ADD_INFIX) == 0)) *out = A + B;
    else if ((formato == FORMATO_PREFIXO && strcmp(op, OP_SUB) == 0) || (formato == FORMATO_INFIXO && strcmp(op, OP_SUB_INFIX) == 0)) *out = A - B;
    else if ((formato == FORMATO_PREFIXO && strcmp(op, OP_MUL) == 0) || (formato == FORMATO_INFIXO && strcmp(op, OP_MUL_INFIX) == 0)) *out = A * B;
    else if ((formato == FORMATO_PREFIXO && strcmp(op, OP_DIV) == 0) || (formato == FORMATO_INFIXO && strcmp(op, OP_DIV_INFIX) == 0)) {
        if (B == 0.0) return CALC_DIVZERO;
        *out = A / B;
    }
    else return CALC_ERROR;

    return CALC_SUCCESS;
}

static int processa(const char *linha, double *out) {
    char tmp[BUF_SIZE];     // Copia a linha porque conta() modifica a string
    strncpy(tmp, linha, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    if (strcmp(tmp, CMD_QUIT) == 0) return CMD_QUIT_CODE;
    if (strcmp(tmp, CMD_HELP) == 0) return CMD_HELP_CODE;
    if (strcmp(tmp, CMD_VERSION) == 0) return CMD_VERSION_CODE;

    char cpy[BUF_SIZE];
    strncpy(cpy, tmp, sizeof(cpy) - 1);
    cpy[sizeof(cpy) - 1] = 0;



    if (isupper(tmp[0])) return conta(cpy, out, FORMATO_PREFIXO); // Tenta formato prefixo
    else if (isdigit(tmp[0]) || tmp[0] == '-' || tmp[0] == '+') return conta(cpy, out, FORMATO_INFIXO); // Tenta formato infixo (inclui números negativos)
    return CALC_ERROR;
}

static void ok(int fd, double r) {
    setlocale(LC_NUMERIC, "C");
    char b[64];
    int n = snprintf(b, sizeof(b), "%s %.6f\n", RESP_OK, r);
    send(fd, b, n, 0);
}

static void erro(int fd, const char *cod, const char *msg) {
    char b[128];
    int n = snprintf(b, sizeof(b), "%s %s %s\n", RESP_ERR, cod, msg);
    send(fd, b, n, 0);
}

static void help(int fd) {
    const char *h = PROTO_HELP;
    char plano[BUF_SIZE];
    size_t j = 0;
    for (size_t i = 0; h[i] && j < sizeof(plano) - 1; i++) {
        char c = h[i];
        if (c == '\n') c = ' ';
        plano[j++] = c;
    }
    plano[j] = 0;
    char out[BUF_SIZE];
    int n = snprintf(out, sizeof(out), "%s HELP %s\n", RESP_OK, plano);
    send(fd, out, n, 0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Use %s <porta>\nEx: %s 5001\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Porta inválida!!\n");
        return EXIT_FAILURE;
    }

    // Criação do socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) die("socket");

    // Habilitar reuso da porta
    int yes = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        die("setsockopt(SO_REUSEADDR)");

    // Configurar endereço
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        die("bind");
    
    // listen
    if (listen(listen_fd, 8) < 0)
        die("listen");
    
    printf("\nServidor calculadora conectado e ouvindo em 0.0.0.0:%d ...\n", port);

    // Inicializar array de clientes
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1; // slot livre
        client_usados[i] = 0;
    }

    // Configurar select
    fd_set allset, rset;
    FD_ZERO(&allset);
    FD_SET(listen_fd, &allset);
    int maxfd = listen_fd;
    int max_i = -1;

    for (;;) {
        rset = allset; // cópia (select modifica o set)
        
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0) {
            if (errno == EINTR) continue; // interrompido por sinal
            die("select");
        }

        // Novo cliente chegando?
        if (FD_ISSET(listen_fd, &rset)) {
            struct sockaddr_in cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            int connfd = accept(listen_fd, (struct sockaddr*)&cliaddr, &clilen);
            
            if (connfd < 0) {
                perror("accept");
            } else {
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &cliaddr.sin_addr, ip, sizeof(ip));
                printf("Novo cliente conectado %s:%d (fd=%d)\n", 
                       ip, ntohs(cliaddr.sin_port), connfd);

                // Adiciona na lista de clientes
                int i;
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i] < 0) {
                        clients[i] = connfd;
                        client_usados[i] = 0;
                        break;
                    }
                }
                if (i == MAX_CLIENTS) {
                    fprintf(stderr, "Muitos clientes, recusando.\n");
                    send(connfd, "ERR " ERR_BUSY " busy\n", 18, 0);
                    close(connfd);
                } else {
                    FD_SET(connfd, &allset);
                    if (connfd > maxfd) maxfd = connfd;
                    if (i > max_i) max_i = i;
                }
            }
            if (--nready <= 0) continue; // nada mais pronto
        }

        // Verifica dados vindos dos clientes existentes
        for (int i = 0; i <= max_i; i++) {
            int fd = clients[i];
            if (fd < 0) continue;
            if (FD_ISSET(fd, &rset)) {
                ssize_t n = recv(fd, client_bufs[i] + client_usados[i],
                                 sizeof(client_bufs[i]) - 1 - client_usados[i], 0);
                
                if (n <= 0) {
                    if (n < 0) perror("recv");
                    printf("Cliente fd=%d desconectou.\n", fd);
                    close(fd);
                    FD_CLR(fd, &allset);
                    clients[i] = -1;
                    client_usados[i] = 0;
                } else {
                    client_usados[i] += n;
                    client_bufs[i][client_usados[i]] = 0;

                    // Processar mensagens completas (terminadas por \n)
                    char *ini = client_bufs[i];
                    for (;;) {
                        char *nl = strchr(ini, '\n');
                        if (!nl) break;
                        *nl = 0;

                        char linha[BUF_SIZE];
                        strncpy(linha, ini, sizeof(linha) - 1);
                        linha[sizeof(linha) - 1] = 0;

                        if (linha[0]) {
                            double r;
                            int rc = processa(linha, &r);
                            if (rc == CALC_SUCCESS) ok(fd, r);
                            else if (rc == CALC_DIVZERO) erro(fd, ERR_DIVZERO, "divisao_por_zero");
                            else if (rc == CMD_HELP_CODE) help(fd);
                            else if (rc == CMD_VERSION_CODE) {
                                char b[64];
                                int m = snprintf(b, sizeof(b), "%s VERSION %s\n", RESP_OK, APP_VERSION);
                                send(fd, b, m, 0);
                            }
                            else if (rc == CMD_QUIT_CODE) {
                                send(fd, "OK bye\n", 7, 0);
                                close(fd);
                                FD_CLR(fd, &allset);
                                clients[i] = -1;
                                client_usados[i] = 0;
                                break;
                            }
                            else erro(fd, ERR_INVALID, "entrada_invalida");
                        }

                        ini = nl + 1;
                    }

                    // Mover dados restantes para o início do buffer
                    size_t resto = client_bufs[i] + client_usados[i] - ini;
                    memmove(client_bufs[i], ini, resto);
                    client_usados[i] = resto;
                }
                if (--nready <= 0) break; // nenhum FD restante pronto
            }
        }
    }

    close(listen_fd);
    return 0;
}
