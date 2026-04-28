#include "game.h"

void clear() { printf("\e[1;1H\e[2J"); }

int score_round(int target, int guess) {
    int diff = abs(target - guess);
    if (diff == 0) return 4;
    if (diff == 1) return 3;
    if (diff == 2) return 2;
    return 0;
}

// FIX #3: Replaces unreliable double-getchar() with a proper buffer flush.
void flush_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    GamePacket packet;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
   setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -1;
    }
    listen(server_fd, 3);

    clear();
    printf("MATCH MY FREQ - P1 SERVER\nWaiting for Player 2...\n");
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    srand(time(0));
    packet.p1_is_psychic = rand() % 2;
    packet.p1_score = 0;
    packet.p2_score = 0;
    packet.spectrum_idx = -1;

    for (int r = 1; r <= 4; r++) {
        packet.round = r;

        if (packet.spectrum_idx == -1) {
            clear();
            if (packet.p1_is_psychic) {
                printf("ROUND %d: YOU ARE PSYCHIC. Pick Category (0-40):\n", r);
                for (int i = 0; i < MAX_SPECTRUMS; i++) {
                    printf("[%2d] %-22s - %-22s ", i, list[i].left, list[i].right);
                    if ((i + 1) % 2 == 0) printf("\n");
                }
                printf("\nSelection: ");
                scanf("%d", &packet.spectrum_idx);
                flush_stdin(); // FIX #3: flush after scanf instead of bare getchar()
            } else {
                printf("ROUND %d: Player 2 is picking a category...\n", r);
            }
        }

        packet.target = (rand() % 10) + 1;

        if (packet.p1_is_psychic) {
            clear();
            printf("ROUND %d/4 | P1: %d | P2: %d\n", r, packet.p1_score, packet.p2_score);
            printf("SPECTRUM: %s <---> %s\n", list[packet.spectrum_idx].left, list[packet.spectrum_idx].right);
            printf("TARGET: %d\n", packet.target);
            printf("Enter Clue: ");
            flush_stdin(); // FIX #3: flush before fgets to clear leftover newline
            fgets(packet.clue, 100, stdin);
            packet.clue[strcspn(packet.clue, "\n")] = 0;

            send(new_socket, &packet, sizeof(packet), 0);
            recv(new_socket, &packet.guess, sizeof(int), 0);

            // FIX #2: Actually compute and award points
            int pts = score_round(packet.target, packet.guess);
            packet.p2_score += pts;
            printf("\nP2 guessed %d | Target was %d | +%d pts\n", packet.guess, packet.target, pts);

        } else {
            send(new_socket, &packet, sizeof(packet), 0);
            recv(new_socket, &packet, sizeof(packet), 0);

            printf("\nClue Received: \"%s\"\nYour Hypothesis (1-10): ", packet.clue);
            scanf("%d", &packet.guess);
            flush_stdin(); // FIX #3

            send(new_socket, &packet.guess, sizeof(int), 0);

            // FIX #2: Actually compute and award points
            int pts = score_round(packet.target, packet.guess);
            packet.p1_score += pts;
            printf("\nYou guessed %d | Target was %d | +%d pts\n", packet.guess, packet.target, pts);
        }

        printf("\nScore -> P1: %d | P2: %d\n", packet.p1_score, packet.p2_score);
        packet.p1_is_psychic = !packet.p1_is_psychic;
        packet.spectrum_idx = -1;

        printf("Press Enter for next round...");
        flush_stdin(); // FIX #3: single reliable flush instead of getchar(); getchar();
    }

    clear();
    printf("=== GAME OVER ===\nP1: %d | P2: %d\n", packet.p1_score, packet.p2_score);
    if (packet.p1_score > packet.p2_score)      printf("P1 WINS!\n");
    else if (packet.p2_score > packet.p1_score) printf("P2 WINS!\n");
    else                                         printf("IT'S A TIE!\n");

    close(new_socket);
    close(server_fd);
    return 0;
}