// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>


SPECTRUMS = [
    ("Underrated Skill", "Overrated Skill"), ("Terrible First Date Spot", "Perfect First Date Spot"),
    ("Awkward Conversation Topic", "Engaging Conversation Topic"), ("Asian Parent", "Western Parent"),
    ("Worst Reason to Break Up", "Valid Reason to Break Up"), ("Green Flag", "Red Flag"),
    ("Asian Dominated Job Field", "White Dominated Job Field"), ("Private School", "Public School"),
    ("Girl’s Girl", "Mean Girl"), ("Princess Treatment", "Bare Minimum"),
    ("Fruity", "Straight"), ("Cheating", "Not Cheating"),
    ("Forgivable", "Unforgivable"), ("Tell Mom", "Tell Dad"),
    ("Flirty", "Friendly"), ("Worst Chore", "Most Satisfying Chore"),
    ("Weird Thing to Own", "Normal Thing to Own"), ("Man Behavior", "Woman Behavior"),
    ("Feminist", "Misandrist"), ("Confrontational", "Avoidant"),
    ("Cancellable", "Unproblematic"), ("Side Eye", "Full Confrontation"),
    ("Bed Rotting", "Productive"), ("Ghosting", "Communicating"),
    ("Logical", "Brainrot"), ("Dramatic", "Nonchalant"),
    ("Casual", "Dating"), ("Strict Parent", "Lenient Parent"),
    ("Galing Mansyon", "Galing Kalye"), ("Aircon", "Kanal"),
    ("Conyo Energy", "Kanal Humor"), ("Micro-cheating", "Totally Harmless"),
    ("Tell Your Strict Mom", "Tell Your Chill Dad"), ("Bed Rotting", "Main Character Grind"),
    ("Main Character", "NPC Energy"), ("Chronically Online", "Touch Grass"),
    ("Soft Launch", "Hard Launch"), ("Overthink Malala", "Walang Pake"),
    ("Clingy", "Emotionally Unavailable"), ("Hopeless Romantic", "Hopeful Romantic"),
    ("Minimal Effort", "Overachiever Tryhard")
]

def clear(): os.system('clear')

def get_scale(target=None, guess=None):
    scale = ""
    for i in range(1, 11):
        if i == target and i == guess: scale += "\033[92m[X]\033[0m"
        elif i == target: scale += "\033[94m[T]\033[0m"
        elif i == guess: scale += "\033[91m[G]\033[0m"
        else: scale += f"[{i}]"
    return scale

def calculate_points(target, guess):
    diff = abs(target - guess)
    if diff == 0: return 3
    if diff == 1: return 2
    if diff == 2: return 1
    return 0

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('127.0.0.1', 5555))
    s.listen(1)
    
    clear()
    print("MATCH MY FREQ - WAITING FOR PLAYER 2...")
    conn, addr = s.accept()
    
    p1_total, p2_total = 0, 0
    curr_spec = None

    for r in range(1, 5): # 4 Rounds
        # DETERMINE ROLES
        p1_is_psychic = (r % 2 != 0)
        role_msg = "PLAYER 1 (YOU) is the PSYCHIC" if p1_is_psychic else "PLAYER 2 is the PSYCHIC"
        
        # CATEGORY SELECTION (Psychic chooses)
        if curr_spec is None:
            clear()
            print(f"ROLES FOR ROUND {r}!\n{role_msg}\n")
            if p1_is_psychic:
                print("PICK A CATEGORY!")
                for i in range(0, len(SPECTRUMS), 2):
                    l = f"[{i}] {SPECTRUMS[i][0]}-{SPECTRUMS[i][1]}"
                    ri = f"[{i+1}] {SPECTRUMS[i+1][0]}-{SPECTRUMS[i+1][1]}" if i+1 < len(SPECTRUMS) else ""
                    print(f"{l:40} {ri}")
                choice = int(input("\nCategory Number: "))
                curr_spec = SPECTRUMS[choice]
                conn.send(json.dumps({"type":"setup", "spec":curr_spec}).encode())
            else:
                print("Waiting for Player 2 to pick a category...")
                data = json.loads(conn.recv(1024).decode())
                curr_spec = data["spec"]

        left, right = curr_spec
        target = random.randint(1, 10)
        
        clear()
        print(f"ROUND {r} | P1: {p1_total} pts | P2: {p2_total} pts")
        print(f"SPECTRUM: {left} <---> {right}")
        
        if p1_is_psychic:
            print(f"SYSTEM RANDOM TARGET: {target}")
            print(get_scale(target))
            clue = input("Psychic's Clue: ")
            conn.send(json.dumps({"type":"clue", "clue":clue, "target":target}).encode())
            print("\nWaiting for Player 2's Hypothesis...")
            guess = int(conn.recv(1024).decode())
        else:
            print("\nWaiting for Player 2's clue...")
            data = json.loads(conn.recv(1024).decode())
            clue, target = data["clue"], data["target"]
            print(f"Psychic's Clue: {clue}")
            print(get_scale())
            guess = int(input("Surmiser's Hypothesis: "))
            conn.send(str(guess).encode())

        # Scoring logic
        pts = calculate_points(target, guess)
        if p1_is_psychic: p2_total += pts
        else: p1_total += pts
        
        clear()
        print("--- REVEAL ---")
        print(get_scale(target, guess))
        print(f"Target: {target} | Guess: {guess} | Points: {pts}")
        print(f"P1 Total: {p1_total} | P2 Total: {p2_total}")

        if r < 4:
            if p1_is_psychic:
                choice = input("\nContinue with this category? (y/n): ").lower()
                if choice != 'y': curr_spec = None
                conn.send(choice.encode())
            else:
                print("\nWaiting for Player 2 to decide on category...")
                choice = conn.recv(1024).decode()
                if choice != 'y': curr_spec = None
            input("Press Enter for next round...")

    clear()
    print("=== FINAL EVALUATION ===")
    final_avg = (p1_total + p2_total)
    if final_avg >= 10: print("You matched each other’s FREQ! 📡")
    elif final_avg >= 5: print("Not quite there yet… 🧠")
    else: print("Oops–you are polar opposites. 🧊")
    conn.close()

if __name__ == "__main__": main()

