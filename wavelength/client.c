#include "game.h"

void clear() { printf("\e[1;1H\e[2J"); }

// FIX #3/#5: Same reliable flush helper as server.
void flush_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_scale(int target, int guess) {
    for (int i = 1; i <= 10; i++) {
        if (i == target && i == guess) printf("\033[92m[X]\033[0m");
        else if (i == target)          printf("\033[94m[T]\033[0m");
        else if (i == guess)           printf("\033[91m[G]\033[0m");
        else                           printf("[%d]", i);
    }
    printf("\n");
}

int score_round(int target, int guess) {
    int diff = abs(target - guess);
    if (diff == 0) return 4;
    if (diff == 1) return 3;
    if (diff == 2) return 2;
    return 0;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    GamePacket packet;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        return -1;
    }

    printf("Connecting to Match My Freq Server...\n");
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed. Is the Server running?\n");
        return -1;
    }

    while (1) {
        if (recv(sock, &packet, sizeof(packet), 0) <= 0) break;
        if (packet.round > 4) break;

        int p2_is_psychic = !packet.p1_is_psychic;

        if (p2_is_psychic && packet.spectrum_idx == -1) {
            clear();
            printf("ROUND %d: YOU ARE THE PSYCHIC\n", packet.round);
            printf("Pick a Category (0-%d):\n", MAX_SPECTRUMS - 1);
            printf("----------------------------------------------------------\n");
            for (int i = 0; i < MAX_SPECTRUMS; i++) {
                printf("[%2d] %-22s - %-22s ", i, list[i].left, list[i].right);
                if ((i + 1) % 2 == 0) printf("\n");
            }
            printf("\n\nSelection: ");
            scanf("%d", &packet.spectrum_idx);
            flush_stdin(); // FIX #5
        }

        clear();
        printf("ROUND %d/4 | P1: %d | P2: %d\n", packet.round, packet.p1_score, packet.p2_score);
        printf("SPECTRUM: %s <---> %s\n", list[packet.spectrum_idx].left, list[packet.spectrum_idx].right);

        if (p2_is_psychic) {
            printf("\033[94mSYSTEM TARGET: %d\033[0m\n", packet.target);
            print_scale(packet.target, 0);
            printf("\nEnter your Clue: ");
            flush_stdin(); // FIX #5: replaces bare getchar() before fgets
            fgets(packet.clue, 100, stdin);
            packet.clue[strcspn(packet.clue, "\n")] = 0;

            send(sock, &packet, sizeof(packet), 0);

            printf("\nWaiting for Player 1's hypothesis...\n");
            recv(sock, &packet.guess, sizeof(int), 0);

            // FIX #2: Show result on client side too
            int pts = score_round(packet.target, packet.guess);
            printf("P1 guessed %d | Target was %d | P1 +%d pts\n", packet.guess, packet.target, pts);

        } else {
            printf("\nPSYCHIC'S CLUE: %s\n", packet.clue);
            print_scale(0, 0);
            printf("\nYour Hypothesis (1-10): ");

            // FIX #4: These three lines were entirely missing — code was cut off here.
            //         Without them, the client never sends its guess and the server
            //         blocks forever on recv(), making it look like they can't connect.
            scanf("%d", &packet.guess);
            flush_stdin(); // FIX #5
            send(sock, &packet.guess, sizeof(int), 0);

            // FIX #2: Show result
            int pts = score_round(packet.target, packet.guess);
            printf("You guessed %d | Target was %d | P2 +%d pts\n", packet.guess, packet.target, pts);
        }

        printf("\nPress Enter for next round...");
        flush_stdin(); // FIX #5: replaces double getchar()

    } // FIX #4: This closing brace for while(1) was also missing.

    printf("=== GAME OVER ===\n");
    close(sock);
    return 0;
}