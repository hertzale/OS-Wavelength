#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 5555
#define MAX_SPECTRUMS 41

typedef struct {
    char left[50];
    char right[50];
} Spectrum;

typedef struct {
    int round;
    int target;
    int guess;
    int spectrum_idx;
    char clue[100];
    int p1_score;
    int p2_score;
    int p1_is_psychic;
} GamePacket;

// FIX #1: Added 'static' — each .c file gets its own private copy,
//         preventing linker conflicts.
static Spectrum list[MAX_SPECTRUMS] = {
    {"Underrated Skill", "Overrated Skill"}, {"Terrible First Date Spot", "Perfect First Date Spot"},
    {"Awkward Conversation Topic", "Engaging Conversation Topic"}, {"Asian Parent", "Western Parent"},
    {"Worst Reason to Break Up", "Valid Reason to Break Up"}, {"Green Flag", "Red Flag"},
    {"Asian Dominated Job Field", "White Dominated Job Field"}, {"Private School", "Public School"},
    {"Girl's Girl", "Mean Girl"}, {"Princess Treatment", "Bare Minimum"},
    {"Fruity", "Straight"}, {"Cheating", "Not Cheating"},
    {"Forgivable", "Unforgivable"}, {"Tell Mom", "Tell Dad"},
    {"Flirty", "Friendly"}, {"Worst Chore", "Most Satisfying Chore"},
    {"Weird Thing to Own", "Normal Thing to Own"}, {"Man Behavior", "Woman Behavior"},
    {"Feminist", "Misandrist"}, {"Confrontational", "Avoidant"},
    {"Cancellable", "Unproblematic"}, {"Side Eye", "Full Confrontation"},
    {"Bed Rotting", "Productive"}, {"Ghosting", "Communicating"},
    {"Logical", "Brainrot"}, {"Dramatic", "Nonchalant"},
    {"Casual", "Dating"}, {"Strict Parent", "Lenient Parent"},
    {"Galing Mansyon", "Galing Kalye"}, {"Aircon", "Kanal"},
    {"Conyo Energy", "Kanal Humor"}, {"Micro-cheating", "Totally Harmless"},
    {"Tell Your Strict Mom", "Tell Your Chill Dad"}, {"Bed Rotting", "Main Character Grind"},
    {"Main Character", "NPC Energy"}, {"Chronically Online", "Touch Grass"},
    {"Soft Launch", "Hard Launch"}, {"Overthink Malala", "Walang Pake"},
    {"Clingy", "Emotionally Unavailable"}, {"Hopeless Romantic", "Hopeful Romantic"},
    {"Minimal Effort", "Overachiever Tryhard"}
};

#endif