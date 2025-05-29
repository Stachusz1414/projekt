#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const float GRAVITY = 0.5f;
const float JUMP_STRENGTH = -15.0f;
const float SCROLL_SPEED = 2.0f;
int c = 0;
const int MAX_HORIZONTAL_DISTANCE = 200;
const int MAX_VERTICAL_DISTANCE = 200;
const int GROUND_PLATFORM_GAP = 100;
const int MIN_VERTICAL_DISTANCE = 50;
const int PLATFORM_REMOVAL_THRESHOLD = 5000;
const float SPAWN_INTERVAL = 15.0f; 

class Character {
public:
    sf::Sprite sprite;
    sf::Texture texture;
    float radius;
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

class Player : public Character {
public:
    bool canJump;

    Player(float radius) {
        this->radius = radius;
        if (!texture.loadFromFile("grafika.png")) {
            
        }

        sprite.setTexture(texture);

        
        float xScale = (2 * radius) / texture.getSize().x;
        float yScale = (2 * radius) / texture.getSize().y;
        sprite.setScale(xScale, yScale);

        
        sprite.setOrigin(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f);
        sprite.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

        shape.setRadius(radius);
        shape.setOrigin(radius, radius);
        shape.setPosition(sprite.getPosition());

        velocity = sf::Vector2f(0, 0);
        canJump = true;
    }

    void update(std::vector<sf::RectangleShape>& platforms, sf::RectangleShape& ground, sf::View& view) {
        velocity.y += GRAVITY;
        shape.move(velocity);

        
        sprite.setPosition(shape.getPosition());

        // Kolizja z ziemią
        if (shape.getPosition().y + shape.getRadius() > ground.getPosition().y) {
            shape.setPosition(shape.getPosition().x, ground.getPosition().y - shape.getRadius());
            velocity.y = 0;
            canJump = true;
        }
        if (velocity.y < 0) {
            std::cout << "i chuj" << std::endl;
        }

        // Kolizje z platformami
        bool onPlatform = false;
        for (auto& platform : platforms) {
            if (shape.getPosition().y + shape.getRadius() > platform.getPosition().y &&
                shape.getPosition().y + shape.getRadius() < platform.getPosition().y + platform.getSize().y &&
                shape.getPosition().x > platform.getPosition().x - shape.getRadius() &&
                shape.getPosition().x < platform.getPosition().x + platform.getSize().x + shape.getRadius() &&
                velocity.y > 0) {

                shape.setPosition(shape.getPosition().x, platform.getPosition().y - shape.getRadius());
                velocity.y = 0;
                canJump = true;
                onPlatform = true;
            }
        }

        if (!onPlatform && velocity.y > 0) {
            canJump = false;
            c = 0;
        }

        // Górna granica
        if (shape.getPosition().y - shape.getRadius() < view.getCenter().y - WINDOW_HEIGHT / 2) {
            shape.setPosition(shape.getPosition().x, view.getCenter().y - WINDOW_HEIGHT / 2 + shape.getRadius());
            velocity.y = 0;
        }

        // Aktualizacja pozycji sprite'a po kolizjach
        sprite.setPosition(shape.getPosition());
    }

    void jump() {
        if (canJump) {
            velocity.y = JUMP_STRENGTH;
            c += 1;
            if (c == 2) {
                canJump = false;
                c = 0;
            }
        }
    }
};

int countVisiblePlatforms(const std::vector<sf::RectangleShape>& platforms, const sf::View& view) {
    int count = 0;
    float top = view.getCenter().y - WINDOW_HEIGHT / 2;
    float bottom = view.getCenter().y + WINDOW_HEIGHT / 2;

    for (const auto& platform : platforms) {
        float y = platform.getPosition().y;
        float h = platform.getSize().y;

        if (y + h >= top && y <= bottom) {
            count++;
        }
    }
    return count;
}

float findHighestPlatformY(const std::vector<sf::RectangleShape>& platforms) {
    if (platforms.empty()) return 0;
    float highest = platforms[0].getPosition().y;
    for (const auto& platform : platforms) {
        if (platform.getPosition().y < highest) {
            highest = platform.getPosition().y;
        }
    }
    return highest;
}

float findLowestPlatformY(const std::vector<sf::RectangleShape>& platforms) {
    if (platforms.empty()) return 0;
    float lowest = platforms[0].getPosition().y;
    for (const auto& platform : platforms) {
        if (platform.getPosition().y > lowest) {
            lowest = platform.getPosition().y;
        }
    }
    return lowest;
}

int main() {
    srand(time(0));
    sf::Clock gameClock;
    sf::Clock spawnClock;
    sf::Clock deathClock;
    bool gameOver = false;
    bool showDeathScreen = false;

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Infinite Platformer");
    window.setFramerateLimit(60);

    sf::View view(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
    window.setView(view);

    Player player(20.0f);
    Character Adas;
    bool AdasActive = false;

    sf::RectangleShape ground(sf::Vector2f(WINDOW_WIDTH, 20));
    ground.setFillColor(sf::Color(139, 69, 19));

    // Death screen
    sf::Texture deathTexture;
    sf::Sprite deathSprite;
    if (!deathTexture.loadFromFile("smierc.png")) {
        std::cerr << "Failed to load death texture!" << std::endl;
        return -1;
    }
    deathSprite.setTexture(deathTexture);
    deathSprite.setOrigin(deathTexture.getSize().x / 2.0f, deathTexture.getSize().y / 2.0f);
    deathSprite.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);

    std::vector<sf::RectangleShape> platforms;
    const int PLATFORM_COUNT = 55;
    const int PLATFORM_WIDTH = 100;
    const int PLATFORM_HEIGHT = 20;
    const int MAX_VERTICAL_OFFSET = 30;

    float initialGroundY = WINDOW_HEIGHT - 20;
    float verticalSpacing = (initialGroundY - 100) / PLATFORM_COUNT;

    for (int i = 0; i < PLATFORM_COUNT; ++i) {
        sf::RectangleShape platform(sf::Vector2f(PLATFORM_WIDTH, PLATFORM_HEIGHT));
        float baseY = initialGroundY - i * verticalSpacing;
        float offsetY = static_cast<float>(rand() % (2 * MAX_VERTICAL_OFFSET + 1) - MAX_VERTICAL_OFFSET);
        float y = baseY + offsetY;
        float x = static_cast<float>(rand() % (WINDOW_WIDTH - PLATFORM_WIDTH));
        platform.setPosition(x, y);
        platform.setFillColor(sf::Color::Blue);
        platforms.push_back(platform);
    }

    float lowestPlatformY = findLowestPlatformY(platforms);
    ground.setPosition(0, lowestPlatformY + PLATFORM_HEIGHT + GROUND_PLATFORM_GAP);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space && !gameOver)
                player.jump();
        }

        if (gameOver) {
            if (deathClock.getElapsedTime().asSeconds() >= 5.0f) {
                window.close();
            }
            continue;
        }

        if (!gameOver) {
            player.velocity.x = 0;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) player.velocity.x = -5.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) player.velocity.x = 5.0f;

            player.update(platforms, ground, view);

            float viewMoveY = 0;
            if (player.shape.getPosition().y < view.getCenter().y - WINDOW_HEIGHT / 3)
                viewMoveY = player.shape.getPosition().y - (view.getCenter().y - WINDOW_HEIGHT / 3);
            else if (player.shape.getPosition().y > view.getCenter().y + WINDOW_HEIGHT / 3)
                viewMoveY = player.shape.getPosition().y - (view.getCenter().y + WINDOW_HEIGHT / 3);

            if (viewMoveY != 0) {
                view.move(0, viewMoveY);
                window.setView(view);
            }

            // Spawn Adas every 15 seconds if not active
            if (spawnClock.getElapsedTime().asSeconds() >= SPAWN_INTERVAL) {
                Adas.radius = 30.0f;
                if (!Adas.texture.loadFromFile("circle_texture.png")) {
                    std::cerr << "Failed to load Adas texture!" << std::endl;
                }
                Adas.sprite.setTexture(Adas.texture);
                Adas.sprite.setOrigin(Adas.texture.getSize().x / 2.0f,
                    Adas.texture.getSize().y / 2.0f);
                Adas.sprite.setScale((2 * Adas.radius) / Adas.texture.getSize().x,
                    (2 * Adas.radius) / Adas.texture.getSize().y);
                Adas.shape.setRadius(Adas.radius);
                Adas.shape.setOrigin(Adas.radius, Adas.radius);
                Adas.shape.setPosition(player.shape.getPosition().x - 200,
                    player.shape.getPosition().y - 200);
                AdasActive = true;
                spawnClock.restart();
            }

            // Update Adas if active
            if (AdasActive) {
                // Calculate direction to player
                sf::Vector2f direction = player.shape.getPosition() - Adas.shape.getPosition();
                float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                
                if (distance <= 10) {
                    gameOver = true;
                    showDeathScreen = true;
                    deathClock.restart();
                }

                if (distance != 0) {
                    direction /= distance; // Normalize vector
                }

                // Chase speed
                float chaseSpeed = 3.0f;
                Adas.velocity = direction * chaseSpeed;

                // Update position
                Adas.shape.move(Adas.velocity);
                Adas.sprite.setPosition(Adas.shape.getPosition());
            }

            
            float highestY = findHighestPlatformY(platforms);

            // Usuń platformy, które są poniżej highestY 
            platforms.erase(std::remove_if(platforms.begin(), platforms.end(),
                [highestY](const sf::RectangleShape& p) {
                    return p.getPosition().y > highestY + PLATFORM_REMOVAL_THRESHOLD;
                }),
                platforms.end());

            // Podnieś ziemię do poziomu najniższej platformy 
            float currentLowestPlatformY = findLowestPlatformY(platforms);
            float desiredGroundY = currentLowestPlatformY + PLATFORM_HEIGHT + GROUND_PLATFORM_GAP;
            if (desiredGroundY < ground.getPosition().y) {
                ground.setPosition(0, desiredGroundY);
            }

            // Dodawanie nowych platform, jeśli jest ich za mało
            while (countVisiblePlatforms(platforms, view) < PLATFORM_COUNT+15) {
                float top = view.getCenter().y - WINDOW_HEIGHT / 2;
                float bottom = view.getCenter().y + WINDOW_HEIGHT / 2;

                float currentHighestY = findHighestPlatformY(platforms);
                if (currentHighestY == 0) currentHighestY = bottom;

                float y = currentHighestY - (MIN_VERTICAL_DISTANCE + rand() % (MAX_VERTICAL_DISTANCE - MIN_VERTICAL_DISTANCE + 1));
                if (y < top) break;

                float x = static_cast<float>(rand() % (WINDOW_WIDTH - PLATFORM_WIDTH));
                sf::RectangleShape newPlatform(sf::Vector2f(PLATFORM_WIDTH, PLATFORM_HEIGHT));
                newPlatform.setPosition(x, y);
                newPlatform.setFillColor(sf::Color::Blue);
                platforms.push_back(newPlatform);
            }
        }

        window.clear(sf::Color::Black);

        if (!gameOver) {
            window.draw(ground);
            for (const auto& platform : platforms)
                window.draw(platform);
            window.draw(player.sprite);
            if (AdasActive) {
                window.draw(Adas.sprite);
            }
        }

        if (showDeathScreen) {
            window.draw(deathSprite);
        }

        window.display();
    }

    return 0;
}
