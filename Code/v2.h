#pragma once
#ifndef v2_h
#define v2_h


const int MAX_HORIZONTAL_DISTANCE = 200;
const int MAX_VERTICAL_DISTANCE = 200;
const int GROUND_PLATFORM_GAP = 100;
const int MIN_VERTICAL_DISTANCE = 50;
const int PLATFORM_REMOVAL_THRESHOLD = 5000;
const float SPAWN_INTERVAL = 15.0f;
const float RESET_HEIGHT = 2000.0f;
const float LEVITATION_TIME = 5.0f;
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const float GRAVITY = 0.5f;
const float JUMP_STRENGTH = -15.0f;
const float SCROLL_SPEED = 2.0f;
int c = 0;
bool firstphase=true;
bool secondphase=false;
bool thirdphase=false;
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

        if (shape.getPosition().y + shape.getRadius() > ground.getPosition().y) {
            shape.setPosition(shape.getPosition().x, ground.getPosition().y - shape.getRadius());
            velocity.y = 0;
            canJump = true;
        }

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

    DisappearingPlatform(float x, float y) : active(true) {
        shape.setSize(sf::Vector2f(100, 20));
        shape.setFillColor(sf::Color::Red);
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
