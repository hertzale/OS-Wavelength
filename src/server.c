#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

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
int net_ask_int(int sock, const char* prompt) {
    NetPacket p; p.cmd = 2; strcpy(p.text, prompt); send(sock, &p, sizeof(NetPacket), 0);
    recv(sock, &p, sizeof(NetPacket), MSG_WAITALL); return atoi(p.text);
}
void net_ask_str(int sock, const char* prompt, char* out) {
    NetPacket p; p.cmd = 3; strcpy(p.text, prompt); send(sock, &p, sizeof(NetPacket), 0);
    recv(sock, &p, sizeof(NetPacket), MSG_WAITALL); strcpy(out, p.text);
}
void broadcast(int sock, const char* msg) {
    printf("%s", msg); net_print(sock, msg);
}
void broadcast_clear(int sock) {
    system("clear"); net_clear(sock);
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
    client_sock = accept(server_fd, (struct sockaddr )&address, (socklen_t)&addrlen);

    char host_name[50], client_name[50], p1_name[50], p2_name[50], buf[1024];

    broadcast_clear(client_sock);
    
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

    int p1_total = 0, p2_total = 0;
    int curr_category = -1;

    for (int r = 1; r <= 5; r++) {
        int p1_is_psychic = (r % 2 != 0); 
        int host_is_psychic = (p1_is_psychic && host_is_p1) || (!p1_is_psychic && !host_is_p1);
        
        char* psychic_name = p1_is_psychic ? p1_name : p2_name;
        char* surmiser_name = p1_is_psychic ? p2_name : p1_name;

        broadcast_clear(client_sock);
        broadcast(client_sock, "ROLES FOR THIS ROUND!\n\n");
        sprintf(buf, "PLAYER %d (%s) is the PSYCHIC\n", p1_is_psychic ? 1 : 2, psychic_name); broadcast(client_sock, buf);
        sprintf(buf, "PLAYER %d (%s) is the SURMISER\n\n", p1_is_psychic ? 2 : 1, surmiser_name); broadcast(client_sock, buf);
        broadcast(client_sock, "The PSYCHIC will pick the category by entering the number.\n\n");
        sleep(2);

        if (curr_category == -1) {
            broadcast_clear(client_sock);
            broadcast(client_sock, "PICK A CATEGORY!\nCategory List\n\n");
            
            for (int i = 0; i < NUM_SPECTRUMS; i++) {
                sprintf(buf, "[%d] %s - %s\n", i, SPECTRUMS[i].left, SPECTRUMS[i].right);
                broadcast(client_sock, buf);
            }
            
            if (host_is_psychic) {
                net_print(client_sock, "\nWaiting for PSYCHIC to pick...\n"); 
                curr_category = get_local_int("\nCategory Number: ", 0, NUM_SPECTRUMS - 1);
            } else {
                printf("\nWaiting for PSYCHIC to pick...\n"); 
                curr_category = net_ask_int(client_sock, "\nCategory Number: ");
            }
        }

        Spectrum spec = SPECTRUMS[curr_category];
        int target = (rand() % 10) + 1;
        char clue[256];
        char scale_buf[256];

        broadcast_clear(client_sock);
        sprintf(buf, "[ %s <---> %s ]\n\n", spec.left, spec.right); broadcast(client_sock, buf);

        if (host_is_psychic) {
            sprintf(buf, "Target Number: %d\n", target); printf("%s", buf);
            format_scale(scale_buf, target, 0); printf("%s\n", scale_buf); 
            printf("The PSYCHIC will give the clue to help SURMISER give a better guess.\n\n");
            
            net_print(client_sock, "Waiting for Psychic to see target and formulate clue...\n"); 
            
            get_local_str("Psychic's Clue: ", clue, 256);
        } else {
            sprintf(buf, "Target Number: %d\n", target); net_print(client_sock, buf);
            format_scale(scale_buf, target, 0); net_print(client_sock, scale_buf); 
            net_print(client_sock, "The PSYCHIC will give the clue to help SURMISER give a better guess.\n\n");
            
            printf("Waiting for Psychic to see target and formulate clue...\n"); 
            
            net_ask_str(client_sock, "Psychic's Clue: ", clue);
        }

        broadcast_clear(client_sock);
        sprintf(buf, "[ %s <---> %s ]\n\n", spec.left, spec.right); broadcast(client_sock, buf);
        sprintf(buf, "Psychic's Clue: %s\n\n", clue); broadcast(client_sock, buf);

        int guess = 0;
        if (!host_is_psychic) { 
            format_scale(scale_buf, 0, 0); printf("%s\n", scale_buf); 
            printf("The SURMISER will now guess the target number.\n\n");
            net_print(client_sock, "Waiting for Surmiser's Hypothesis...\n"); 
            
            guess = get_local_int("Surmiser's Hypothesis (1-10): ", 1, 10);
        } else { 
            format_scale(scale_buf, 0, 0); net_print(client_sock, scale_buf); 
            net_print(client_sock, "The SURMISER will now guess the target number.\n\n");
            printf("Waiting for Surmiser's Hypothesis...\n"); 
            
            guess = net_ask_int(client_sock, "Surmiser's Hypothesis (1-10): ");
        }

        int diff = abs(target - guess);
        int pts = (diff == 0) ? 3 : ((diff == 1) ? 2 : ((diff == 2) ? 1 : 0));
        
        if (p1_is_psychic) p2_total += pts; else p1_total += pts;

        broadcast_clear(client_sock);
        sprintf(buf, "[ %s <---> %s ]\n", spec.left, spec.right); broadcast(client_sock, buf);
        sprintf(buf, "Psychic's Clue: %s\n\n", clue); broadcast(client_sock, buf);
        
        format_scale(scale_buf, target, guess); broadcast(client_sock, scale_buf); 
        
        sprintf(buf, "\nTarget Number was: %d\n", target); broadcast(client_sock, buf);
        sprintf(buf, "Surmiser Guessed: %d\n\n", guess); broadcast(client_sock, buf);
        
        sprintf(buf, "Surmiser got %d points!\n\n", pts); broadcast(client_sock, buf);
        
        sprintf(buf, "PLAYER 1’s Total Points: %d\n", p1_total); broadcast(client_sock, buf);
        sprintf(buf, "PLAYER 2’s Total Points: %d\n", p2_total); broadcast(client_sock, buf);

        if (r < 5) {
            char choice[10];
            if (host_is_psychic) {
                net_print(client_sock, "\nWaiting for PSYCHIC to decide...\n");
                get_local_str("\nContinue [y], Change Category [n], or Quit [q]? ", choice, 10);
            } else {
                printf("\nWaiting for PSYCHIC to decide...\n");
                net_ask_str(client_sock, "\nContinue [y], Change Category [n], or Quit [q]? ", choice);
            }
            
            if (choice[0] == 'q' || choice[0] == 'Q') {
                break; 
            } else if (choice[0] == 'n' || choice[0] == 'N') {
                curr_category = -1; 
            }
        }
    }

    broadcast_clear(client_sock);
    int final_avg = (p1_total + p2_total); 
    broadcast(client_sock, "=== FINAL RESULTS ===\n");
    if (final_avg >= 10) broadcast(client_sock, "- You matched each other’s FREQ!\n");
    else if (final_avg >= 5) broadcast(client_sock, "- Not quite there yet…\n");
    else broadcast(client_sock, "- Oops–you are polar opposites.\n");

    NetPacket q; q.cmd = 4; send(client_sock, &q, sizeof(NetPacket), 0);
    close(client_sock); close(server_fd);
    return 0;
}