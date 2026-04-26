import socket
import json
import os
import random

SPECTRUMS = [ # Same list as server for indexing
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

def main():
    c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    c.connect(('127.0.0.1', 5555))
    p1_total, p2_total = 0, 0
    curr_spec = None

    for r in range(1, 5):
        p1_is_psychic = (r % 2 != 0)
        role_msg = "PLAYER 1 is the PSYCHIC" if p1_is_psychic else "PLAYER 2 (YOU) is the PSYCHIC"
        
        if curr_spec is None:
            clear()
            print(f"ROLES FOR ROUND {r}!\n{role_msg}\n")
            if not p1_is_psychic:
                print("PICK A CATEGORY!")
                for i in range(0, len(SPECTRUMS), 2):
                    l = f"[{i}] {SPECTRUMS[i][0]}-{SPECTRUMS[i][1]}"
                    ri = f"[{i+1}] {SPECTRUMS[i+1][0]}-{SPECTRUMS[i+1][1]}" if i+1 < len(SPECTRUMS) else ""
                    print(f"{l:40} {ri}")
                choice = int(input("\nCategory Number: "))
                curr_spec = SPECTRUMS[choice]
                c.send(json.dumps({"type":"setup", "spec":curr_spec}).encode())
            else:
                print("Waiting for Player 1 to pick a category...")
                data = json.loads(c.recv(1024).decode())
                curr_spec = data["spec"]

        left, right = curr_spec
        clear()
        print(f"ROUND {r} | P1: {p1_total} pts | P2: {p2_total} pts")
        print(f"SPECTRUM: {left} <---> {right}")

        if p1_is_psychic:
            print("\nWaiting for Player 1's clue...")
            data = json.loads(c.recv(1024).decode())
            clue, target = data["clue"], data["target"]
            print(f"Psychic's Clue: {clue}")
            print(get_scale())
            guess = int(input("Surmiser's Hypothesis: "))
            c.send(str(guess).encode())
        else:
            target = random.randint(1, 10) # Client generates local target if they are psychic
            print(f"SYSTEM RANDOM TARGET: {target}")
            print(get_scale(target))
            clue = input("Psychic's Clue: ")
            c.send(json.dumps({"type":"clue", "clue":clue, "target":target}).encode())
            print("\nWaiting for Player 1's Hypothesis...")
            guess = int(c.recv(1024).decode())

        diff = abs(target - guess)
        pts = 3 if diff == 0 else (2 if diff == 1 else (1 if diff == 2 else 0))
        if p1_is_psychic: p2_total += pts
        else: p1_total += pts

        clear()
        print("--- REVEAL ---")
        print(get_scale(target, guess))
        print(f"Target: {target} | Guess: {guess} | Points: {pts}")

        if r < 4:
            if not p1_is_psychic:
                choice = input("\nContinue with this category? (y/n): ").lower()
                if choice != 'y': curr_spec = None
                c.send(choice.encode())
            else:
                print("\nWaiting for Player 1 to decide on category...")
                choice = c.recv(1024).decode()
                if choice != 'y': curr_spec = None
            input("Press Enter for next round...")

    c.close()

if __name__ == "__main__": main()
