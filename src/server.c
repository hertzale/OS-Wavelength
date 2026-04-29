#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 8888

typedef struct {
    int cmd; 
    char text[2048];
} NetPacket;

typedef struct {
    char left[50];
    char right[50];
} Spectrum;

Spectrum SPECTRUMS[] = {
    {"Underrated Skill", "Overrated Skill"}, {"Terrible First Date Spot", "Perfect First Date Spot"},
    {"Awkward Conversation Topic", "Engaging Conversation Topic"}, {"Worst Reason to Break Up", "Valid Reason to Break Up"},
    {"Green Flag", "Red Flag"}, {"Conyo Energy", "Kanal Humor"},
    {"Princess Treatment", "Bare Minimum"}, {"Micro-cheating", "Totally Harmless"},
    {"Tell Your Strict Mom", "Tell Your Chill Dad"}, {"Worst Chore", "Most Satisfying Chore"},
    {"Weird Thing to Own", "Normal Thing to Own"}, {"Confrontational", "Avoidant"},
    {"Cancellable", "Unproblematic"}, {"Side Eye", "Full Confrontation"},
    {"Bed Rotting", "Main Character Grind"}, {"Dramatic", "Nonchalant"},
    {"Strict Parent", "Lenient Parent"}, {"Galing Mansyon", "Galing Kalye"},
    {"Main Character", "NPC Energy"}, {"Chronically Online", "Touch Grass"},
    {"Soft Launch", "Hard Launch"}, {"Overthink Malala", "Walang Pake"},
    {"Clingy", "Emotionally Unavailable"}, {"Hopeless Romantic", "Hopeful Romantic"},
    {"Minimal Effort", "Overachiever Tryhard"}
};
int NUM_SPECTRUMS = sizeof(SPECTRUMS) / sizeof(Spectrum);

void net_print(int sock, const char* msg) {
    NetPacket p; p.cmd = 0; strcpy(p.text, msg); send(sock, &p, sizeof(NetPacket), 0);
}
void net_clear(int sock) {
    NetPacket p; p.cmd = 1; send(sock, &p, sizeof(NetPacket), 0);
}
int net_ask_int(int sock, const char* prompt, int min, int max) {
    NetPacket p; 
    char input_prompt[2048];
    strcpy(input_prompt, prompt);
    
    while(1) {
        p.cmd = 2; strcpy(p.text, input_prompt); send(sock, &p, sizeof(NetPacket), 0);
        recv(sock, &p, sizeof(NetPacket), MSG_WAITALL); 
        int val = atoi(p.text);
        if (val >= min && val <= max) return val;
        
        sprintf(input_prompt, "Invalid! Enter a number between %d and %d.\n%s", min, max, prompt);
    }
}
void net_ask_str(int sock, const char* prompt, char* out) {
    NetPacket p; p.cmd = 3; strcpy(p.text, prompt); send(sock, &p, sizeof(NetPacket), 0);
    recv(sock, &p, sizeof(NetPacket), MSG_WAITALL); strcpy(out, p.text);
}
void broadcast(int sock, const char* msg) {
    printf("%s", msg); net_print(sock, msg);
}

void draw_banners(int client_sock, int r, int host_is_psychic) {
    system("clear"); net_clear(client_sock);
    
    char host_buf[512], client_buf[512];
    sprintf(host_buf, 
        "======================================================\n"
        "       MATCH MY FREQ  |  ROUND %d  |  YOU ARE: %s     \n"
        "======================================================\n\n", 
        r, host_is_psychic ? "PSYCHIC" : "SURMISER");
    printf("%s", host_buf);

    sprintf(client_buf, 
        "======================================================\n"
        "       MATCH MY FREQ  |  ROUND %d  |  YOU ARE: %s     \n"
        "======================================================\n\n", 
        r, !host_is_psychic ? "PSYCHIC" : "SURMISER");
    net_print(client_sock, client_buf);
}

void format_scale(char* buffer, int target, int guess) {
    strcpy(buffer, "");
    for (int i = 1; i <= 10; i++) {
        char temp[32];
        if (target > 0 && guess > 0 && i == target && i == guess) sprintf(temp, "[X] ");
        else if (target > 0 && i == target) sprintf(temp, "[T] ");
        else if (guess > 0 && i == guess) sprintf(temp, "[G] ");
        else sprintf(temp, "[%d] ", i);
        strcat(buffer, temp);
    }
    strcat(buffer, "\n");
}

int get_local_int(const char* prompt, int min, int max) {
    int choice; char buffer[100];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &choice) == 1 && choice >= min && choice <= max) return choice;
        }
        printf("Invalid input. Enter a number between %d and %d.\n", min, max);
    }
}
void get_local_str(const char* prompt, char* out, int max_len) {
    while (1) {
        printf("%s", prompt);
        if (fgets(out, max_len, stdin) != NULL) {
            out[strcspn(out, "\n")] = 0;
            if (strlen(out) > 0) return;
        }
        printf("Input cannot be empty.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[1]);
    srand(time(NULL));
    int server_fd, client_sock;
    struct sockaddr_in address; int opt = 1; int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) exit(EXIT_FAILURE);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET; address.sin_addr.s_addr = INADDR_ANY; address.sin_port = htons(port);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 1);

    system("clear");
    printf("\n*** MATCH MY FREQ - SERVER ***\n");
    printf("Waiting for the Client terminal to connect on port %d...\n", port);
    client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    char host_name[50], client_name[50], p1_name[50], p2_name[50], buf[1024];

    system("clear"); net_clear(client_sock);
    
    net_print(client_sock, "Input names:\nWaiting for Host...\n");
    get_local_str("Input names:\nPLAYER 1: ", host_name, 50);
    
    printf("Waiting for connected player to enter name...\n");
    net_ask_str(client_sock, "PLAYER 2: ", client_name);

    int host_is_p1 = (rand() % 2 == 0);
    if (host_is_p1) {
        strcpy(p1_name, host_name); strcpy(p2_name, client_name);
    } else {
        strcpy(p1_name, client_name); strcpy(p2_name, host_name);
    }

    while (1) {
        int p1_total = 0, p2_total = 0;
        int curr_category = -1;

        for (int r = 1; r <= 5; r++) {
            int p1_is_psychic = (r % 2 != 0); 
            int host_is_psychic = (p1_is_psychic && host_is_p1) || (!p1_is_psychic && !host_is_p1);
            
            char* psychic_name = p1_is_psychic ? p1_name : p2_name;
            char* surmiser_name = p1_is_psychic ? p2_name : p1_name;

            draw_banners(client_sock, r, host_is_psychic);
            
            sprintf(buf, 
                "+----------------------------------------------------+\n"
                "| [ROLES FOR THIS ROUND]                             |\n"
                "| PSYCHIC : %-30s (P%d) |\n"
                "| SURMISER: %-30s (P%d) |\n"
                "+----------------------------------------------------+\n\n", 
                psychic_name, p1_is_psychic ? 1 : 2, surmiser_name, p1_is_psychic ? 2 : 1);
            broadcast(client_sock, buf);
            
            broadcast(client_sock, 
                "+----------------------------------------------------+\n"
                "| [INSTRUCTION]                                      |\n"
                "| The PSYCHIC will pick the category number.         |\n"
                "+----------------------------------------------------+\n\n");
            sleep(2);

            if (curr_category == -1) {
                for (int i = 0; i < NUM_SPECTRUMS; i++) {
                    sprintf(buf, "[%2d] %s - %s\n", i+1, SPECTRUMS[i].left, SPECTRUMS[i].right);
                    broadcast(client_sock, buf);
                }
                
                if (host_is_psychic) {
                    net_print(client_sock, "\nWaiting for PSYCHIC to pick...\n"); 
                    curr_category = get_local_int("\nCategory Number: ", 1, NUM_SPECTRUMS) - 1;
                } else {
                    printf("\nWaiting for PSYCHIC to pick...\n"); 
                    curr_category = net_ask_int(client_sock, "\nCategory Number: ", 1, NUM_SPECTRUMS) - 1;
                }
            }

            Spectrum spec = SPECTRUMS[curr_category];
            int target = (rand() % 10) + 1;
            char clue[256];
            char scale_buf[256];

            draw_banners(client_sock, r, host_is_psychic);
            sprintf(buf, "[ %s <---> %s ]\n\n", spec.left, spec.right); broadcast(client_sock, buf);

            if (host_is_psychic) {
                sprintf(buf, "Target Number: %d\n", target); printf("%s", buf);
                format_scale(scale_buf, target, 0); printf("%s\n", scale_buf); 
                
                printf("+----------------------------------------------------+\n"
                       "| [INSTRUCTION]                                      |\n"
                       "| Give a clue to help SURMISER guess the target.     |\n"
                       "+----------------------------------------------------+\n\n");
                net_print(client_sock, "Waiting for Psychic to see target and formulate clue...\n"); 
                
                get_local_str("Psychic's Clue: ", clue, 256);
            } else {
                sprintf(buf, "Target Number: %d\n", target); net_print(client_sock, buf);
                format_scale(scale_buf, target, 0); net_print(client_sock, scale_buf); 
                
                net_print(client_sock, 
                       "+----------------------------------------------------+\n"
                       "| [INSTRUCTION]                                      |\n"
                       "| Give a clue to help SURMISER guess the target.     |\n"
                       "+----------------------------------------------------+\n\n");
                printf("Waiting for Psychic to see target and formulate clue...\n"); 
                
                net_ask_str(client_sock, "Psychic's Clue: ", clue);
            }

            draw_banners(client_sock, r, host_is_psychic);
            sprintf(buf, "[ %s <---> %s ]\n\n", spec.left, spec.right); broadcast(client_sock, buf);
            sprintf(buf, "Psychic's Clue: %s\n\n", clue); broadcast(client_sock, buf);

            int guess = 0;
            if (!host_is_psychic) { 
                format_scale(scale_buf, 0, 0); printf("%s\n", scale_buf); 
                printf("+----------------------------------------------------+\n"
                       "| [INSTRUCTION]                                      |\n"
                       "| Guess the target number based on the clue!         |\n"
                       "+----------------------------------------------------+\n\n");
                net_print(client_sock, "Waiting for Surmiser's Hypothesis...\n"); 
                
                guess = get_local_int("Surmiser's Hypothesis (1-10): ", 1, 10);
            } else { 
                format_scale(scale_buf, 0, 0); net_print(client_sock, scale_buf); 
                net_print(client_sock, 
                       "+----------------------------------------------------+\n"
                       "| [INSTRUCTION]                                      |\n"
                       "| Guess the target number based on the clue!         |\n"
                       "+----------------------------------------------------+\n\n");
                printf("Waiting for Surmiser's Hypothesis...\n"); 
                
                guess = net_ask_int(client_sock, "Surmiser's Hypothesis (1-10): ", 1, 10);
            }

            int diff = abs(target - guess);
            int pts = (diff == 0) ? 3 : ((diff == 1) ? 2 : ((diff == 2) ? 1 : 0));
            
            if (p1_is_psychic) p2_total += pts; else p1_total += pts;

            draw_banners(client_sock, r, host_is_psychic);
            sprintf(buf, "[ %s <---> %s ]\n", spec.left, spec.right); broadcast(client_sock, buf);
            sprintf(buf, "Psychic's Clue: %s\n\n", clue); broadcast(client_sock, buf);
            
            format_scale(scale_buf, target, guess); broadcast(client_sock, scale_buf); 
            
            sprintf(buf, 
                "\n+----------------------------------------------------+\n"
                "| [SCORING]                                          |\n"
                "| Target was %-2d      Surmiser Guessed %-2d             |\n"
                "| Points earned this round: %-24d |\n"
                "+----------------------------------------------------+\n\n", target, guess, pts);
            broadcast(client_sock, buf);
            
            if (r < 5) {
                int p1_is_next_psychic = ((r + 1) % 2 != 0); 
                int host_is_next_psychic = (p1_is_next_psychic && host_is_p1) || (!p1_is_next_psychic && !host_is_p1);

                char choice[10];
                if (host_is_next_psychic) {
                    net_print(client_sock, "\nWaiting for NEXT ROUND'S PSYCHIC to decide...\n");
                    get_local_str("\n[NEXT PSYCHIC] Continue [y], Change Category [n], or Quit [q]? ", choice, 10);
                } else {
                    printf("\nWaiting for NEXT ROUND'S PSYCHIC to decide...\n");
                    net_ask_str(client_sock, "\n[NEXT PSYCHIC] Continue [y], Change Category [n], or Quit [q]? ", choice);
                }
                
                if (choice[0] == 'q' || choice[0] == 'Q') {
                    char confirm[10];
                    if (host_is_next_psychic) {
                        printf("Waiting for SURMISER to confirm quit...\n");
                        net_ask_str(client_sock, "\n[SURMISER] The PSYCHIC wants to quit. Do you agree to end the game? [y/n]: ", confirm);
                    } else {
                        net_print(client_sock, "Waiting for SURMISER to confirm quit...\n");
                        get_local_str("\n[SURMISER] The PSYCHIC wants to quit. Do you agree to end the game? [y/n]: ", confirm, 10);
                    }

                    if (confirm[0] == 'y' || confirm[0] == 'Y') {
                        broadcast(client_sock, "\nBoth players agreed to quit. Tallying final scores...\n");
                        sleep(2);
                        break; 
                    } else {
                        broadcast(client_sock, "\nSurmiser refused to quit! The game continues...\n");
                        sleep(2);
                    }
                } else if (choice[0] == 'n' || choice[0] == 'N') {
                    curr_category = -1; 
                }
            }
        }

        system("clear"); net_clear(client_sock);
        int final_avg = (p1_total + p2_total); 
        
        broadcast(client_sock, 
            "+====================================================+\n"
            "|               *** FINAL SCORE *** |\n"
            "+====================================================+\n");
            
        sprintf(buf, 
            "\n  PLAYER 1 (%s) Total: %d\n"
            "  PLAYER 2 (%s) Total: %d\n"
            "  --------------------------------\n"
            "  TEAM COMBINED SCORE: %d\n\n", p1_name, p1_total, p2_name, p2_total, final_avg);
        broadcast(client_sock, buf);

        if (final_avg >= 10) broadcast(client_sock, ">> VERDICT: You matched each other’s FREQ!\n");
        else if (final_avg >= 5) broadcast(client_sock, ">> VERDICT: Not quite there yet…\n");
        else broadcast(client_sock, ">> VERDICT: Oops–you are polar opposites.\n");

        char play_again[10];
        printf("\n");
        net_print(client_sock, "\nWaiting for Host to decide on a new game...\n");
        get_local_str("Do you want to play a new game? [y/n]: ", play_again, 10);
        
        if (play_again[0] == 'n' || play_again[0] == 'N') {
            break; 
        }
    }

    NetPacket q; q.cmd = 4; send(client_sock, &q, sizeof(NetPacket), 0);
    close(client_sock); close(server_fd);
    return 0;
}