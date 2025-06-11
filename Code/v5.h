#pragma once
#ifndef v3_h
#define v3_h

#include <SFML/Graphics.hpp>

const int MAX_HORIZONTAL_DISTANCE = 200;
const int MAX_VERTICAL_DISTANCE = 200;
const int GROUND_PLATFORM_GAP = 100;
float MIN_HORIZONTAL_DISTANCE = 100.0f;
float MIN_VERTICAL_DISTANCE = 100.0f;
const int FLAG = 1500;
const float SPAWN_INTERVAL = 5.0f;
const float RESET_HEIGHT = 2000.0f;
const float LEVITATION_TIME = 1.0f;
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const float GRAVITY = 0.5f;
const float JUMP_STRENGTH = -15.0f;
const float SCROLL_SPEED = 2.0f;
int MIN_VISIBLE_PLATFORMS = 22;
const int MAX_PLATFORMS = 150;
int c = 0;
int pc = 0;
int Phase = 1;
sf::Clock gameTimeClock;
bool showScore = false;
sf::Font font;
sf::Text scoreText;
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
    bool isLevitating;
    sf::Clock levitationClock;
    bool hasShield = false;

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
        isLevitating = false;
    }

    void update(std::vector<sf::RectangleShape>& platforms, sf::RectangleShape& ground, sf::View& view) {
        if (!isLevitating) {
            velocity.y += GRAVITY;
        }
        else {
            velocity.y = 0.1f;

            if (levitationClock.getElapsedTime().asSeconds() >= LEVITATION_TIME) {
                isLevitating = false;
            }
        }

        shape.move(velocity);
        sprite.setPosition(shape.getPosition());

        bool onPlatform = false;


        if (checkGrformCollision(ground)) {
            onPlatform = true;
        }


        for (auto& platform : platforms) {
            if (checkPlatformCollision(platform)) {
                onPlatform = true;
            }
        }


        if (!onPlatform && velocity.y > 0) {
            canJump = false;
            c = 0;
        }


        if (shape.getPosition().y - shape.getRadius() < view.getCenter().y - WINDOW_HEIGHT / 2) {
            shape.setPosition(shape.getPosition().x, view.getCenter().y - WINDOW_HEIGHT / 2 + shape.getRadius());
            velocity.y = 0;
        }

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

    void startLevitation() {
        isLevitating = true;
        levitationClock.restart();
    }

private:
    bool checkPlatformCollision(const sf::RectangleShape& platform) {
        sf::Vector2f playerPos = shape.getPosition();
        float radius = shape.getRadius();

        sf::Vector2f platformPos = platform.getPosition();
        sf::Vector2f platformSize = platform.getSize();

        if (playerPos.y + radius > platformPos.y &&
            playerPos.y + radius < platformPos.y + platformSize.y &&
            playerPos.x > platformPos.x - radius &&
            playerPos.x < platformPos.x + platformSize.x + radius &&
            velocity.y > 0) {

            shape.setPosition(playerPos.x, platformPos.y - radius);
            velocity.y = 0;
            canJump = true;

            return true;
        }


        return false;
    }

    bool checkGrformCollision(const sf::RectangleShape& ground) {
        sf::Vector2f playerPos = shape.getPosition();
        float radius = shape.getRadius();

        sf::Vector2f platformPos = ground.getPosition();
        sf::Vector2f platformSize = ground.getSize();

        if (playerPos.y + radius > platformPos.y &&
            playerPos.y + radius < platformPos.y + platformSize.y &&
            playerPos.x > platformPos.x - radius &&
            playerPos.x < platformPos.x + platformSize.x + radius &&
            velocity.y > 0) {

            shape.setPosition(playerPos.x, platformPos.y - radius);
            velocity.y = 0;
            canJump = true;

            return true;
        }


        return false;
    }
};

class Item {
public:
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2f position;
    bool active = true;

    virtual void applyEffect(Player& player) = 0;
    virtual ~Item() = default;
};

class Feather : public Item {
public:
    Feather(const sf::Vector2f& pos) {
        texture.loadFromFile("feather.png");
        sprite.setTexture(texture);
        sprite.setScale(0.2f, 0.2f); // ⬅️ skalowanie do 20%
        sprite.setPosition(pos);
        position = pos;
    }

    void applyEffect(Player& player) override {
        player.canJump = true;  // przywraca double-jump
        c = 0;
        active = false;
    }
};

class Clock : public Item {
    sf::Vector2f savedPosition;
public:
    Clock(const sf::Vector2f& pos, const sf::Vector2f& playerPosition) {
        texture.loadFromFile("clock.png");
        sprite.setTexture(texture);
        sprite.setScale(0.2f, 0.2f); // ⬅️ zmniejszenie
        sprite.setPosition(pos);
        position = pos;
        savedPosition = playerPosition;
    }

    void applyEffect(Player& player) override {
        player.shape.setPosition(savedPosition);
        player.sprite.setPosition(savedPosition);
        active = false;
    }
};

class Shield : public Item {
public:
    Shield(const sf::Vector2f& pos) {
        texture.loadFromFile("shield.png");
        sprite.setTexture(texture);
        sprite.setScale(0.2f, 0.2f); // ⬅️ zmniejszenie
        sprite.setPosition(pos);
        position = pos;
    }

    void applyEffect(Player& player) override {
        player.hasShield = true;
        active = false;
    }
};

class Spike {
public:
    sf::ConvexShape shape;
    sf::FloatRect bounds;

    Spike(float x, float y) {
        shape.setPointCount(3);
        shape.setPoint(0, sf::Vector2f(0, 20));
        shape.setPoint(1, sf::Vector2f(10, 0));
        shape.setPoint(2, sf::Vector2f(20, 20));
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(x, y);
        bounds = shape.getGlobalBounds();
    }
};

class DisappearingPlatform {
public:
    sf::RectangleShape shape;
    sf::Clock timer;
    bool active;
    float disappearDelay;
    bool touched;

    DisappearingPlatform(float x, float y, float delay = 4.0f) :
        active(true), disappearDelay(delay), touched(false) {
        shape.setSize(sf::Vector2f(100, 20));
        shape.setFillColor(sf::Color(0, 255, 255));
        shape.setPosition(x, y);
    }
};

class Lava {
public:
    sf::RectangleShape shape;
    float speed;

    Lava() : speed(1.0f) {
        shape.setSize(sf::Vector2f(WINDOW_WIDTH, 50));
        shape.setFillColor(sf::Color(255, 100, 0, 200));
    }

    void update(float deltaTime) {
        shape.move(0, -speed * deltaTime * 60);
        speed += 0.001f;
    }
};


#endif
