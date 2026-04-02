#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#define COLOR_DEFAULT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_BRIGHT_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define COLOR_BRIGHT_GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_BRIGHT_BLUE (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_BRIGHT_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_BRIGHT_CYAN (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_BRIGHT_MAGENTA (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_DIM_GREEN (FOREGROUND_GREEN)
#define COLOR_DIM_BLUE (FOREGROUND_BLUE)

static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
static COORD topLeft = {0, 0};

void setColor(WORD c) { SetConsoleTextAttribute(hConsole, c); }
void gotoXY(short x, short y) { COORD c = {x, y}; SetConsoleCursorPosition(hConsole, c); }
void hideCursor(bool hide) { CONSOLE_CURSOR_INFO info; info.dwSize = 100; info.bVisible = !hide; SetConsoleCursorInfo(hConsole, &info); }
void clearScreen() { system("cls"); }

void playSound(int frequency, int duration) {
    Beep(frequency, duration);
}

void drawBorderedTitle(const std::string &title, WORD color = COLOR_BRIGHT_BLUE) {
    setColor(color);
    std::cout << "\n   ===============================\n";
    std::cout << "          " << title << "\n";
    std::cout << "   ===============================\n";
    setColor(COLOR_DEFAULT);
}

void showBanner() {
    setColor(COLOR_BRIGHT_CYAN);
    std::cout << "\n";
    std::cout << "  __  __                 ____                       _                 \n";
    std::cout << " |  \\/  | __ _ _______  |  _ \\ _ __ ___  _ __   __| | ___ _ __ ___  \n";
    std::cout << " | |\\/| |/ _` |_  / _| | |_) | '__/ _ \\/ _ \\ / _` |/ _ \\ '__/ _ \\ \n";
    std::cout << " | |  | | (_| |/ / (_| |  _ <| | | (_) | (_) | (_| |  __/ | | (_) |\n";
    std::cout << " |_|  |_|\\__,_/___\\__| |_| \\_|  \\___/ \\___/ \\__,_|\\___|_|  \\___/ \n";
    setColor(COLOR_BRIGHT_YELLOW);
    std::cout << "\n                === MAZE RUNNER (C++) ===\n\n";
    setColor(COLOR_DEFAULT);
}

struct Cell {
    int x, y;
    Cell() : x(0), y(0) {}
    Cell(int xx, int yy) : x(xx), y(yy) {}
};

struct ScoreEntry {
    std::string playerName;
    int score;
    int difficulty;
    time_t date;
    
    ScoreEntry() : score(0), difficulty(1), date(0) {}
    ScoreEntry(const std::string& name, int s, int d) : playerName(name), score(s), difficulty(d), date(time(0)) {}
};

class ScoreManager {
private:
    std::vector<ScoreEntry> scores;
    std::string filename;
    
    // Parse "name|score|difficulty|epoch"
    bool parseLine(const std::string& line, ScoreEntry& out) {
        size_t p1 = line.find('|');
        if (p1 == std::string::npos) return false;
        size_t p2 = line.find('|', p1 + 1);
        if (p2 == std::string::npos) return false;
        size_t p3 = line.find('|', p2 + 1);
        if (p3 == std::string::npos) return false;

        out.playerName = line.substr(0, p1);
        out.score = atoi(line.substr(p1 + 1, p2 - p1 - 1).c_str());
        out.difficulty = atoi(line.substr(p2 + 1, p3 - p2 - 1).c_str());
        out.date = (time_t)atol(line.substr(p3 + 1).c_str());
        return true;
    }

public:
    ScoreManager() : filename("scores.txt") {
        loadScores();
    }
    
    void addScore(const std::string& name, int score, int difficulty) {
        scores.push_back(ScoreEntry(name, score, difficulty));
        std::sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
            return a.score > b.score;
        });
        if ((int)scores.size() > 10) scores.resize(10);
        saveScores();
    }
    
    void loadScores() {
        scores.clear();
        std::ifstream file(filename.c_str());
        if (!file) return;
        std::string line;
        while (std::getline(file, line)) {
            ScoreEntry e;
            if (parseLine(line, e)) scores.push_back(e);
        }
        std::sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
            return a.score > b.score;
        });
        if ((int)scores.size() > 10) scores.resize(10);
    }
    
    void saveScores() {
        std::ofstream file(filename.c_str());
        for (size_t i = 0; i < scores.size(); ++i) {
            const ScoreEntry& e = scores[i];
            file << e.playerName << "|" << e.score << "|" << e.difficulty << "|" << (long)e.date << "\n";
        }
    }
    
    void displayHighScores() {
        clearScreen();
        drawBorderedTitle("HIGH SCORES", COLOR_BRIGHT_YELLOW);
        
        if (scores.empty()) {
            setColor(COLOR_BRIGHT_BLUE);
            std::cout << "\n   No high scores yet! Be the first to set a record!\n";
            setColor(COLOR_DEFAULT);
        } else {
            setColor(COLOR_BRIGHT_CYAN);
            std::cout << std::setw(4) << "Rank" << std::setw(20) << "Name" << std::setw(10) << "Score" 
                     << std::setw(12) << "Difficulty" << std::setw(20) << "Date\n";
            std::cout << "   " << std::string(66, '=') << "\n";
            
            for (size_t i = 0; i < scores.size(); ++i) {
                const ScoreEntry& entry = scores[i];
                std::string diffStr = (entry.difficulty == 1) ? "Easy" : (entry.difficulty == 2) ? "Medium" : "Hard";
                char dateStr[32];
                tm* lt = localtime(&entry.date);
                if (lt) strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", lt);
                else strcpy(dateStr, "-");
                
                setColor(i == 0 ? COLOR_BRIGHT_YELLOW : COLOR_BRIGHT_GREEN);
                std::cout << std::setw(4) << (i + 1)
                          << std::setw(20) << entry.playerName 
                          << std::setw(10) << entry.score
                          << std::setw(12) << diffStr
                          << std::setw(20) << dateStr << "\n";
            }
        }
        
        setColor(COLOR_BRIGHT_MAGENTA);
        std::cout << "\n   Press any key to continue...";
        getch();
        setColor(COLOR_DEFAULT);
    }
};

class Maze {
public:
    std::vector<std::string> grid;
    int width, height;
    Cell start, exit;

    void load(const std::vector<std::string>& g) {
        grid = g;
        height = (int)grid.size();
        width = (height ? (int)grid[0].size() : 0);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (grid[y][x] == 'S') {
                    start.x = x;
                    start.y = y;
                    grid[y][x] = ' ';
                }
                if (grid[y][x] == 'E') {
                    exit.x = x;
                    exit.y = y;
                }
            }
        }
    }

    bool isWall(int x, int y) const {
        return (x < 0 || x >= width || y < 0 || y >= height) || grid[y][x] == '#';
    }
};

class Game {
private:
    ScoreManager scoreManager;
    
    // --------- Procedural maze generation (DFS backtracker) ----------
    void shuffleDirs(int dirs[4]) {
        for (int i = 0; i < 4; ++i) {
            int j = rand() % 4;
            int t = dirs[i]; dirs[i] = dirs[j]; dirs[j] = t;
        }
    }

    void carve(std::vector<std::string>& m, int w, int h, int x, int y) {
        m[y][x] = ' ';
        int dirs[4] = {0,1,2,3}; // 0=up,1=down,2=left,3=right
        shuffleDirs(dirs);
        for (int k = 0; k < 4; ++k) {
            int d = dirs[k];
            int dx = 0, dy = 0;
            if (d == 0) dy = -1;
            else if (d == 1) dy = 1;
            else if (d == 2) dx = -1;
            else dx = 1;
            int nx = x + dx * 2;
            int ny = y + dy * 2;
            if (nx > 0 && nx < w - 1 && ny > 0 && ny < h - 1 && m[ny][nx] == '#') {
                m[y + dy][x + dx] = ' ';
                carve(m, w, h, nx, ny);
            }
        }
    }

    void generateMaze(std::vector<std::string>& m, int w, int h) {
        // Ensure odd dimensions inside walls
        if (w < 7) w = 7;
        if (h < 7) h = 7;
        if (w % 2 == 0) w++;
        if (h % 2 == 0) h++;

        m.clear();
        // Build solid walls
        for (int y = 0; y < h; ++y) {
            std::string row(w, '#');
            m.push_back(row);
        }
        // Carve from (1,1)
        carve(m, w, h, 1, 1);

        // Place Start and Exit
        m[1][1] = 'S';
        m[h - 2][w - 2] = 'E';
    }

public:
    enum Difficulty { EASY = 1, MEDIUM = 2, HARD = 3 };

    Difficulty difficulty;
    Maze maze;
    Cell player;
    int lives;
    int steps;
    bool quit;
    bool win;
    clock_t startClock;
    int finalScore;

    int base; int stepPenalty; int timePenalty; int winBonus;

    Game() : lives(5), steps(0), quit(false), win(false), finalScore(0) {}
    
    void configureByDifficulty(Difficulty d) {
        difficulty = d;
        if (d == EASY) { stepPenalty = 4; timePenalty = 1; }
        else if (d == MEDIUM) { stepPenalty = 5; timePenalty = 2; }
        else { stepPenalty = 6; timePenalty = 3; }
        base = 1000; winBonus = 150;
        lives = 5; // Always 5 lives
    }

    void loadRandomMaze() {
        std::vector<std::string> mazeData;

        // Scale maze size per difficulty (odd sizes for generator)
        int w, h;
        if (difficulty == EASY) { w = 31; h = 19; }     // small
        else if (difficulty == MEDIUM) { w = 35; h = 23; } // medium
        else { w = 41; h = 27; }                        // large

        generateMaze(mazeData, w, h);

        maze.load(mazeData);
        player = maze.start;
        lives = 5;
        steps = 0;
        quit = false;
        win = false;
    }

    void drawHUD(int curScore, int elapsed) {
        setColor(COLOR_BRIGHT_BLUE);
        std::cout << "   ";
        for (int i = 0; i < maze.width + 2; i++) std::cout << "#"; 
        std::cout << "\n";

        for (int y = 0; y < maze.height; y++) {
            std::cout << "   ";
            for (int x = -1; x <= maze.width; x++) {
                if (x == -1 || x == maze.width) { 
                    setColor(COLOR_BRIGHT_BLUE); 
                    std::cout << "#"; 
                }
                else if (x == player.x && y == player.y) { 
                    setColor(COLOR_BRIGHT_GREEN); 
                    std::cout << "P"; 
                }
                else {
                    char ch = maze.grid[y][x];
                    if (x == maze.exit.x && y == maze.exit.y) { 
                        setColor(COLOR_BRIGHT_YELLOW); 
                        std::cout << "E"; 
                    }
                    else if (ch == '#') { 
                        setColor(COLOR_DIM_BLUE); 
                        std::cout << "#"; 
                    }
                    else { 
                        setColor(COLOR_DEFAULT); 
                        std::cout << " "; 
                    }
                }
            }
            std::cout << "\n";
        }
        
        std::cout << "   ";
        setColor(COLOR_BRIGHT_BLUE);
        for (int i = 0; i < maze.width + 2; i++) std::cout << "#"; 
        std::cout << "\n";

        setColor(COLOR_BRIGHT_CYAN);
        std::cout << "\n   Lives: ";
        setColor(COLOR_BRIGHT_RED);
        for (int i = 0; i < lives; i++) {
            std::cout << "? ";
        }
        setColor(COLOR_BRIGHT_CYAN);
        std::cout << "(" << lives << " remaining)";
        
        setColor(COLOR_BRIGHT_CYAN);
        std::cout << "\n   Steps: " << steps;
        std::cout << "   Time: " << elapsed << "s";
        std::cout << "   Score: " << curScore << "\n";
        
        setColor(COLOR_DEFAULT);
    }

    int computeScore(int elapsed) {
        int s = base - steps * stepPenalty - elapsed * timePenalty + (win ? winBonus : 0);
        return s > 0 ? s : 0;
    }

    void play() {
        startClock = clock();
        hideCursor(true);
        playSound(800, 100);
        
        while (!quit && !win && lives > 0) {
            int elapsed = (int)((clock() - startClock) / CLOCKS_PER_SEC);
            int curScore = computeScore(elapsed);
            gotoXY(0, 0);
            drawHUD(curScore, elapsed);

            if (kbhit()) {
                char c = tolower(getch());
                int nx = player.x, ny = player.y;
                if (c == 'a') nx--; 
                else if (c == 'd') nx++; 
                else if (c == 'w') ny--; 
                else if (c == 's') ny++; 
                else if (c == 'x') { quit = true; break; }
                
                if (nx != player.x || ny != player.y) {
                    if (maze.isWall(nx, ny)) {
                        playSound(300, 100);
                        lives--;
                        elapsed = (int)((clock() - startClock) / CLOCKS_PER_SEC);
                        curScore = computeScore(elapsed);
                        gotoXY(0, 0);
                        drawHUD(curScore, elapsed);
                    } else {
                        player.x = nx; player.y = ny; steps++;
                        playSound(600, 50);
                    }
                }
            }
            if (player.x == maze.exit.x && player.y == maze.exit.y) { 
                win = true; 
                playSound(1200, 200);
                playSound(1400, 200);
                playSound(1600, 300);
            }
            Sleep(30);
        }
        
        finalScore = computeScore((int)((clock() - startClock) / CLOCKS_PER_SEC));
        hideCursor(false);
    }
    
    void showGameOver() {
        clearScreen();
        if (win) {
            setColor(COLOR_BRIGHT_GREEN);
            std::cout << "\n\n";
            std::cout << "   ================================\n";
            std::cout << "           VICTORY ACHIEVED!       \n";
            std::cout << "   ================================\n";
            std::cout << "        You escaped the maze!      \n";
            std::cout << "                                   \n";
            std::cout << "          Final Score: " << std::setw(4) << finalScore << "    \n";
            std::cout << "   ================================\n";
        } else {
            setColor(COLOR_BRIGHT_RED);
            std::cout << "\n\n";
            std::cout << "   ================================\n";
            std::cout << "             GAME OVER!            \n";
            std::cout << "   ================================\n";
            std::cout << "        Better luck next time!     \n";
            std::cout << "                                   \n";
            std::cout << "          Final Score: " << std::setw(4) << finalScore << "    \n";
            std::cout << "   ================================\n";
        }
        
        if (win) {
            std::cout << "\n   Enter your name for high scores: ";
            std::string playerName;
            std::cin >> playerName;
            scoreManager.addScore(playerName, finalScore, difficulty);
        }
        
        setColor(COLOR_DEFAULT);
    }
    
    bool askPlayAgain() {
        std::cout << "\n\n";
        setColor(COLOR_BRIGHT_YELLOW);
        std::cout << "   Play again? (Y/N): ";
        char choice = tolower(getch());
        std::cout << choice << "\n";
        return choice == 'y';
    }
};

void showMainMenu() {
    clearScreen();
    showBanner();
    
    setColor(COLOR_BRIGHT_CYAN);
    std::cout << "\n   Controls: WASD to move, X to quit game\n";
    
    drawBorderedTitle("MAIN MENU", COLOR_BRIGHT_MAGENTA);
    
    setColor(COLOR_BRIGHT_YELLOW);
    std::cout << "   1. Start Game\n";
    std::cout << "   2. High Scores\n";
    std::cout << "   3. Exit\n";
    std::cout << "\n   Select an option (1-3): ";
    setColor(COLOR_DEFAULT);
}

void showDifficultyMenu() {
    clearScreen();
    drawBorderedTitle("SELECT DIFFICULTY", COLOR_BRIGHT_GREEN);
    
    setColor(COLOR_BRIGHT_YELLOW);
    std::cout << "   1. Easy (small maze)\n";
    std::cout << "   2. Medium (bigger maze)\n";
    std::cout << "   3. Hard (largest maze)\n";
    std::cout << "\n   Select difficulty (1-3): ";
    setColor(COLOR_DEFAULT);
}

int main() {
    srand((unsigned)time(NULL));
    Game game;
    ScoreManager scoreManager;
    
    while (true) {
        showMainMenu();
        char choice = getch();
        
        if (choice == '1') {
            showDifficultyMenu();
            char diffChoice = getch();
            if (diffChoice >= '1' && diffChoice <= '3') {
                game.configureByDifficulty(static_cast<Game::Difficulty>(diffChoice - '0'));
                
                bool playAgain = true;
                while (playAgain) {
                    game.loadRandomMaze();
                    game.play();
                    game.showGameOver();
                    playAgain = game.askPlayAgain();
                }
            }
        } 
        else if (choice == '2') {
            scoreManager.displayHighScores();
        }
        else if (choice == '3') {
            clearScreen();
            setColor(COLOR_BRIGHT_CYAN);
            std::cout << "\n\n   Thanks for playing Maze Runner!\n";
            std::cout << "   Goodbye!\n\n";
            setColor(COLOR_DEFAULT);
            break;
        }
    }
    
    return 0;
}

