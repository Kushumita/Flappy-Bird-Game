#include <SFML/Graphics.hpp>
#include <vector>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>

using namespace sf;
using namespace std;

struct Pipe {
    Sprite top, bottom;
    bool scored = false;
};

int loadHighScore() {
    ifstream file("highscore.txt");
    int hs = 0;
    if (file.is_open()) file >> hs;
    return hs;
}

void saveHighScore(int score) {
    ofstream file("highscore.txt");
    if (file.is_open()) file << score;
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    RenderWindow window(VideoMode(800, 600), "Flappy Bird");
    window.setFramerateLimit(60);

    // ---------------- FONT ----------------
    Font font;
    font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");

    Text scoreText, highText, gameOverText, pauseText;
    scoreText.setFont(font);
    highText.setFont(font);
    gameOverText.setFont(font);
    pauseText.setFont(font);

    scoreText.setCharacterSize(28);
    highText.setCharacterSize(28);
    gameOverText.setCharacterSize(30);
    pauseText.setCharacterSize(32);

    scoreText.setFillColor(Color::Black);
    highText.setFillColor(Color::Black);
    gameOverText.setFillColor(Color::Red);
    pauseText.setFillColor(Color::Blue);

    scoreText.setPosition(20, 15);
    highText.setPosition(520, 15);

    gameOverText.setString("GAME OVER\nPress R to Restart");
    gameOverText.setPosition(250, 250);

    pauseText.setString("PAUSED");
    pauseText.setPosition(330, 260);

    // ---------------- TEXTURES ----------------
    Texture birdTex, pipeTex;
    birdTex.loadFromFile("Assets_EndTermProj/Assets/Bird.png");
    pipeTex.loadFromFile("Assets_EndTermProj/Assets/Pipes.png");

    // ---------------- BIRD ----------------
    Sprite bird;
    bird.setTexture(birdTex);
    bird.setScale(0.15f, 0.15f);
    bird.setPosition(120.f, 250.f);

    float velocity = 0.f;
    float gravity = 900.f;
    float flapStrength = -320.f;

    // ---------------- PIPES ----------------
    vector<Pipe> pipes;
    Clock spawnClock;
    Clock gameClock;

    float pipeSpeed = 220.f;
    float gap = 170.f;

    // ---------------- SCORE ----------------
    int score = 0;
    int highScore = loadHighScore();

    bool gameOver = false;
    bool paused = false;

    auto resetGame = [&]() {
        bird.setPosition(120.f, 250.f);
        velocity = 0.f;
        pipes.clear();
        score = 0;
        gameOver = false;
        paused = false;
        spawnClock.restart();
    };

    // ================= GAME LOOP =================
    while (window.isOpen()) {
        float dt = gameClock.restart().asSeconds();
        Event event;

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Space && !gameOver && !paused)
                    velocity = flapStrength;

                if (event.key.code == Keyboard::P && !gameOver)
                    paused = !paused;

                if (event.key.code == Keyboard::R && gameOver)
                    resetGame();
            }
        }

        if (!paused && !gameOver) {
            // ---------------- BIRD PHYSICS ----------------
            velocity += gravity * dt;
            bird.move(0, velocity * dt);

            // ---------------- PIPE SPAWN ----------------
            if (spawnClock.getElapsedTime().asSeconds() > 1.8f) {
                Pipe p;

                float centerY = 150 + rand() % 250;

                p.top.setTexture(pipeTex);
                p.bottom.setTexture(pipeTex);

                p.top.setScale(0.35f, -1.2f);
                p.bottom.setScale(0.35f, 1.2f);

                p.top.setPosition(800.f, centerY - gap / 2.f);
                p.bottom.setPosition(800.f, centerY + gap / 2.f);

                pipes.push_back(p);
                spawnClock.restart();
            }

            // ---------------- PIPE UPDATE ----------------
            for (auto& p : pipes) {
                p.top.move(-pipeSpeed * dt, 0);
                p.bottom.move(-pipeSpeed * dt, 0);

                if (!p.scored && p.top.getPosition().x + p.top.getGlobalBounds().width < bird.getPosition().x) {
                    score++;
                    p.scored = true;
                }

                if (bird.getGlobalBounds().intersects(p.top.getGlobalBounds()) ||
                    bird.getGlobalBounds().intersects(p.bottom.getGlobalBounds())) {
                    gameOver = true;
                }
            }

            // Remove offscreen pipes
            pipes.erase(remove_if(pipes.begin(), pipes.end(),
                [](Pipe& p) {
                    return p.top.getPosition().x < -120;
                }), pipes.end());

            // Top / bottom collision
            if (bird.getPosition().y < 0 || bird.getPosition().y + bird.getGlobalBounds().height > 600)
                gameOver = true;

            // Save high score
            if (score > highScore) {
                highScore = score;
                saveHighScore(highScore);
            }
        }

        // ---------------- HUD ----------------
        scoreText.setString("SCORE: " + to_string(score));
        highText.setString("HIGH SCORE: " + to_string(highScore));

        // ---------------- DRAW ----------------
        window.clear(Color(235, 235, 235));

        for (auto& p : pipes) {
            window.draw(p.top);
            window.draw(p.bottom);
        }

        window.draw(bird);
        window.draw(scoreText);
        window.draw(highText);

        if (paused) window.draw(pauseText);
        if (gameOver) window.draw(gameOverText);

        window.display();
    }

    return 0;
}
