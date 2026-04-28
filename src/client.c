#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888 // NEW PORT TO ESCAPE ZOMBIES

typedef struct {
    int cmd; 
    char text[2048];
} NetPacket;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    printf("\n*** MATCH MY FREQ (PORT 8888) ***\n");

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) exit(EXIT_FAILURE);
    serv_addr.sin_family = AF_INET; serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed. Start the Host first!\n"); return -1;
    }

    NetPacket pkt;
    while (recv(sock, &pkt, sizeof(NetPacket), MSG_WAITALL) > 0) {
        if (pkt.cmd == 0) {
            printf("%s", pkt.text); fflush(stdout);
        } 
        else if (pkt.cmd == 1) {
            system("clear");
        } 
        else if (pkt.cmd == 2) {
            int choice; char input[100];
            while (1) {
                printf("%s", pkt.text);
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    if (sscanf(input, "%d", &choice) == 1 && choice >= 1 && choice <= 10) {
                        NetPacket resp; sprintf(resp.text, "%d", choice);
                        send(sock, &resp, sizeof(NetPacket), 0); break;
                    }
                }
                printf("Invalid input. Enter a number between 1 and 10.\n");
            }
        } 
        else if (pkt.cmd == 3) {
            char input[1024];
            while (1) {
                printf("%s", pkt.text);
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    input[strcspn(input, "\n")] = 0;
                    if (strlen(input) > 0) {
                        NetPacket resp; strcpy(resp.text, input);
                        send(sock, &resp, sizeof(NetPacket), 0); break;
                    }
                }
                printf("Input cannot be empty.\n");
            }
        } 
        else if (pkt.cmd == 4) break;
    }

    close(sock);
    return 0;
}