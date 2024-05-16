
struct Player{
    bool inPattern; // Showing pattern for player
    bool patternStart; // In start phase of pattern
    int patternIndex; // Current phase of pattern
    int patternMax; // Max phase of pattern
    unsigned long prevTime; // Time of previous pattern step
    int score; // Player's score
    int expected; // The button the player needs to press next
    bool failed; // Indicates if the player is not able to play anymore 
};