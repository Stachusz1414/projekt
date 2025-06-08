#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <list>
#include "v3.h"


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
    sf::Clock phaseTransitionClock;
    bool gameOver = false;
    bool showDeathScreen = false;
    bool triangleActive = false;
    bool secondPhaseFlagActive = false;
    bool thirdPhaseFlagActive = false;
    sf::ConvexShape flag;
    sf::ConvexShape secondPhaseFlag;
    sf::ConvexShape thirdPhaseFlag;
    float highestReached = 0.0f;
    float phaseStartY = 0.0f; 
    Lava lava;
    std::vector<Spike> spikes;
    std::list<DisappearingPlatform> disappearingPlatforms;

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Infinite Platformer");
    window.setFramerateLimit(60);

    sf::View view(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
    window.setView(view);

    Player player(20.0f);
    Character enemy;
    bool enemyActive = false;

    sf::RectangleShape ground(sf::Vector2f(WINDOW_WIDTH, 20));
    ground.setFillColor(sf::Color(139, 69, 19));

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
    const int PLATFORM_COUNT = 30;
    const int PLATFORM_WIDTH = 100;
    const int PLATFORM_HEIGHT = 20;

    
    flag.setPointCount(3);
    flag.setPoint(0, sf::Vector2f(0, 0));
    flag.setPoint(1, sf::Vector2f(30, 50));
    flag.setPoint(2, sf::Vector2f(60, 0));
    flag.setFillColor(sf::Color::Green);
    flag.setOrigin(30, 25);

    secondPhaseFlag.setPointCount(3);
    secondPhaseFlag.setPoint(0, sf::Vector2f(0, 0));
    secondPhaseFlag.setPoint(1, sf::Vector2f(30, 50));
    secondPhaseFlag.setPoint(2, sf::Vector2f(60, 0));
    secondPhaseFlag.setFillColor(sf::Color::Yellow);
    secondPhaseFlag.setOrigin(30, 25);

    thirdPhaseFlag.setPointCount(3);
    thirdPhaseFlag.setPoint(0, sf::Vector2f(0, 0));
    thirdPhaseFlag.setPoint(1, sf::Vector2f(30, 50));
    thirdPhaseFlag.setPoint(2, sf::Vector2f(60, 0));
    thirdPhaseFlag.setFillColor(sf::Color::Magenta);
    thirdPhaseFlag.setOrigin(30, 25);

    
    float initialGroundY = WINDOW_HEIGHT - 20;
    float verticalSpacing = (initialGroundY - 100) / PLATFORM_COUNT;

    for (int i = 0; i < PLATFORM_COUNT; ++i) {
        sf::RectangleShape platform(sf::Vector2f(PLATFORM_WIDTH, PLATFORM_HEIGHT));
        float baseY = initialGroundY - i * verticalSpacing;
        float offsetY = static_cast<float>(rand() % 1);
        float y = baseY + offsetY;
        float x = static_cast<float>(rand() % (WINDOW_WIDTH - PLATFORM_WIDTH));
        platform.setPosition(x, y);
        platform.setFillColor(sf::Color::Blue);
        platforms.push_back(platform);
    }

    float lowestPlatformY = findLowestPlatformY(platforms);
    ground.setPosition(0, lowestPlatformY + PLATFORM_HEIGHT + GROUND_PLATFORM_GAP);
    lava.shape.setPosition(0, ground.getPosition().y + 50);

    while (window.isOpen()) {
        if (player.shape.getPosition().x > WINDOW_WIDTH) {
            player.shape.setPosition(0, player.shape.getPosition().y);
        }
        if (player.shape.getPosition().x < 0) {
            player.shape.setPosition(WINDOW_WIDTH, player.shape.getPosition().y);
        }
        sf::Event event;
        float deltaTime = gameClock.restart().asSeconds();

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space && !gameOver)
                player.jump();
        }

        if (gameOver) {
            deathSprite.setPosition(view.getCenter());
            if (deathClock.getElapsedTime().asSeconds() >= 5.0f) {
                window.close();
            }

            window.clear(sf::Color::Black);
            window.draw(deathSprite);
            window.display();
            continue;
        }

        
        player.velocity.x = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) player.velocity.x = -5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) player.velocity.x = 5.0f;

        
        if (firstphase) {
            
            player.update(platforms, ground, view);

            
            float currentHeight = player.shape.getPosition().y - phaseStartY;
            if (currentHeight < highestReached) {
                highestReached = currentHeight;

                
                if (!triangleActive && highestReached < -2000.0f) {
                    triangleActive = true;
                    flag.setPosition(
                        player.shape.getPosition().x + 300,
                        player.shape.getPosition().y - 300
                    );
                }
            }

            
            if (triangleActive) {
                sf::FloatRect flagBounds = flag.getGlobalBounds();
                sf::FloatRect playerBounds(
                    player.shape.getPosition().x - player.radius,
                    player.shape.getPosition().y - player.radius,
                    player.radius * 2,
                    player.radius * 2
                );

                if (flagBounds.intersects(playerBounds)) {
                    player.startLevitation();
                    enemyActive = false;
                    platforms.clear();
                    firstphase = false;
                    secondphase = true;
                    triangleActive = false;

                    
                    highestReached = 0.0f;
                    phaseStartY = player.shape.getPosition().y;

                    
                    view.setCenter(WINDOW_WIDTH / 2, phaseStartY + WINDOW_HEIGHT / 3);
                    window.setView(view);
                    phaseTransitionClock.restart();
                }
            }

            
            if (!enemyActive && spawnClock.getElapsedTime().asSeconds() >= SPAWN_INTERVAL) {
                enemy.radius = 30.0f;
                if (!enemy.texture.loadFromFile("circle_texture.png")) {
                    std::cerr << "Failed to load enemy texture!" << std::endl;
                }
                enemy.sprite.setTexture(enemy.texture);
                enemy.sprite.setOrigin(enemy.texture.getSize().x / 2.0f,
                    enemy.texture.getSize().y / 2.0f);
                enemy.sprite.setScale((2 * enemy.radius) / enemy.texture.getSize().x,
                    (2 * enemy.radius) / enemy.texture.getSize().y);
                enemy.shape.setRadius(enemy.radius);
                enemy.shape.setOrigin(enemy.radius, enemy.radius);
                enemy.shape.setPosition(player.shape.getPosition().x - 200,
                    player.shape.getPosition().y - 200);
                enemyActive = true;
                spawnClock.restart();
            }
        }
        else if (secondphase) {
            
            player.update(platforms, ground, view);

            
            float currentHeight = player.shape.getPosition().y - phaseStartY;

            
            if (!secondPhaseFlagActive && phaseTransitionClock.getElapsedTime().asSeconds() >= 5.0f &&
                currentHeight < -2000.0f) {
                secondPhaseFlagActive = true;
                secondPhaseFlag.setPosition(
                    player.shape.getPosition().x + 300,
                    player.shape.getPosition().y - 300
                );
            }

            
            if (secondPhaseFlagActive) {
                sf::FloatRect flagBounds = secondPhaseFlag.getGlobalBounds();
                sf::FloatRect playerBounds(
                    player.shape.getPosition().x - player.radius,
                    player.shape.getPosition().y - player.radius,
                    player.radius * 2,
                    player.radius * 2
                );

                if (flagBounds.intersects(playerBounds)) {
                    player.startLevitation();
                    enemyActive = false;
                    platforms.clear();
                    disappearingPlatforms.clear();
                    spikes.clear();
                    secondphase = false;
                    thirdphase = true;
                    secondPhaseFlagActive = false;

           
                    highestReached = 0.0f;
                    phaseStartY = player.shape.getPosition().y;

      
                    view.setCenter(WINDOW_WIDTH / 2, phaseStartY + WINDOW_HEIGHT / 3);
                    window.setView(view);
                    lava.shape.setPosition(0, ground.getPosition().y + 50);
                }
            }

          
            for (auto it = disappearingPlatforms.begin(); it != disappearingPlatforms.end(); ) {
                if (!it->active && it->timer.getElapsedTime().asSeconds() >= 3.0f) {
                    it = disappearingPlatforms.erase(it);
                }
                else {
                    ++it;
                }
            }

         
            for (auto& dp : disappearingPlatforms) {
                if (dp.active &&
                    player.shape.getPosition().y + player.radius > dp.shape.getPosition().y &&
                    player.shape.getPosition().y + player.radius < dp.shape.getPosition().y + dp.shape.getSize().y &&
                    player.shape.getPosition().x > dp.shape.getPosition().x - player.radius &&
                    player.shape.getPosition().x < dp.shape.getPosition().x + dp.shape.getSize().x + player.radius &&
                    player.velocity.y > 0) {

                    player.shape.setPosition(player.shape.getPosition().x, dp.shape.getPosition().y - player.radius);
                    player.velocity.y = 0;
                    player.canJump = true;
                    dp.timer.restart();
                    dp.active = false;
                    dp.shape.setFillColor(sf::Color(255, 0, 0, 100));
                }
            }


            for (auto& spike : spikes) {
                if (player.shape.getGlobalBounds().intersects(spike.bounds)) {
                    gameOver = true;
                    showDeathScreen = true;
                    deathClock.restart();
                    break;
                }
            }
        }
        else if (thirdphase) {
         
            lava.update(deltaTime);

        
            if (player.shape.getPosition().y + player.radius > lava.shape.getPosition().y) {
                gameOver = true;
                showDeathScreen = true;
                deathClock.restart();
            }

            player.update(platforms, ground, view);

            float currentHeight = player.shape.getPosition().y - phaseStartY;

          
            if (!thirdPhaseFlagActive && phaseTransitionClock.getElapsedTime().asSeconds() >= 5.0f &&
                currentHeight < -2000.0f) {
                thirdPhaseFlagActive = true;
                thirdPhaseFlag.setPosition(
                    player.shape.getPosition().x + 300,
                    player.shape.getPosition().y - 300
                );
            }

       
            for (auto& spike : spikes) {
                if (player.shape.getGlobalBounds().intersects(spike.bounds)) {
                    gameOver = true;
                    showDeathScreen = true;
                    deathClock.restart();
                    break;
                }
            }

     
            if (!enemyActive && spawnClock.getElapsedTime().asSeconds() >= SPAWN_INTERVAL) {
                enemy.radius = 30.0f;
                if (!enemy.texture.loadFromFile("circle_texture.png")) {
                    std::cerr << "Failed to load enemy texture!" << std::endl;
                }
                enemy.sprite.setTexture(enemy.texture);
                enemy.sprite.setOrigin(enemy.texture.getSize().x / 2.0f,
                    enemy.texture.getSize().y / 2.0f);
                enemy.sprite.setScale((2 * enemy.radius) / enemy.texture.getSize().x,
                    (2 * enemy.radius) / enemy.texture.getSize().y);
                enemy.shape.setRadius(enemy.radius);
                enemy.shape.setOrigin(enemy.radius, enemy.radius);
                enemy.shape.setPosition(player.shape.getPosition().x - 200,
                    player.shape.getPosition().y - 200);
                enemyActive = true;
                spawnClock.restart();
            }
        }

    
        if (enemyActive && (firstphase || thirdphase)) {
            sf::Vector2f direction = player.shape.getPosition() - enemy.shape.getPosition();
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance <= 10) {
                gameOver = true;
                showDeathScreen = true;
                deathClock.restart();
            }

            if (distance != 0) {
                direction /= distance;
            }

            float chaseSpeed = 3.0f;
            enemy.velocity = direction * chaseSpeed;
            enemy.shape.move(enemy.velocity);
            enemy.sprite.setPosition(enemy.shape.getPosition());
        }

      
        float viewMoveY = 0;
        if (player.shape.getPosition().y < view.getCenter().y - WINDOW_HEIGHT / 3)
            viewMoveY = player.shape.getPosition().y - (view.getCenter().y - WINDOW_HEIGHT / 3);
        else if (player.shape.getPosition().y > view.getCenter().y + WINDOW_HEIGHT / 3)
            viewMoveY = player.shape.getPosition().y - (view.getCenter().y + WINDOW_HEIGHT / 3);

        if (viewMoveY != 0) {
            view.move(0, viewMoveY);
            window.setView(view);
        }

 
        float highestY = findHighestPlatformY(platforms);
        platforms.erase(std::remove_if(platforms.begin(), platforms.end(),
            [highestY](const sf::RectangleShape& p) {
                return p.getPosition().y > highestY + PLATFORM_REMOVAL_THRESHOLD;
            }),
            platforms.end());

        float currentLowestPlatformY = findLowestPlatformY(platforms);
        float desiredGroundY = currentLowestPlatformY + PLATFORM_HEIGHT + GROUND_PLATFORM_GAP;
        if (desiredGroundY < ground.getPosition().y) {
            ground.setPosition(0, desiredGroundY);
            if (thirdphase) {
                lava.shape.setPosition(0, ground.getPosition().y + 50);
            }
        }

      
        while (countVisiblePlatforms(platforms, view) < PLATFORM_COUNT + 15) {
            float top = view.getCenter().y - WINDOW_HEIGHT / 2;
            float bottom = view.getCenter().y + WINDOW_HEIGHT / 2;

            float currentHighestY = findHighestPlatformY(platforms);
            if (currentHighestY == 0) currentHighestY = bottom;

            float y = currentHighestY - (MIN_VERTICAL_DISTANCE + rand() % (MAX_VERTICAL_DISTANCE - MIN_VERTICAL_DISTANCE + 1));
            if (y < top) break;

            float x = static_cast<float>(rand() % (WINDOW_WIDTH - PLATFORM_WIDTH));

            if (secondphase) {
            
                int platformType = rand() % 3;
                sf::RectangleShape newPlatform(sf::Vector2f(PLATFORM_WIDTH, PLATFORM_HEIGHT));
                newPlatform.setPosition(x, y);
                newPlatform.setFillColor(sf::Color(0, 255, 0));
                platforms.push_back(newPlatform);
                if (platformType == 0) {
                 
                    disappearingPlatforms.emplace_back(x, y);
                }
                else if (platformType == 1) {
              
                    sf::RectangleShape newPlatform(sf::Vector2f(PLATFORM_WIDTH, PLATFORM_HEIGHT));
                    newPlatform.setPosition(x, y);
                    newPlatform.setFillColor(sf::Color::Blue);
                    platforms.push_back(newPlatform);

          
                    int spikeCount = PLATFORM_WIDTH / 60;
                    for (int i = 0; i < spikeCount; i++) {
                        spikes.emplace_back(x + i * 30, y - 20);
                    }
                }
                else {
         
                    sf::RectangleShape newPlatform(sf::Vector2f(PLATFORM_WIDTH, PLATFORM_HEIGHT));
                    newPlatform.setPosition(x, y);
                    newPlatform.setFillColor(sf::Color::Blue);
                    platforms.push_back(newPlatform);
                }
            }
            else if (thirdphase) {
                
                sf::RectangleShape newPlatform(sf::Vector2f(PLATFORM_WIDTH, PLATFORM_HEIGHT));
                newPlatform.setPosition(x, y);
                newPlatform.setFillColor(sf::Color(255, 165, 0));
                platforms.push_back(newPlatform);

                
                if (rand() % 5 == 0) {
                    int spikeCount = PLATFORM_WIDTH / 30;
                    for (int i = 0; i < spikeCount; i++) {
                        spikes.emplace_back(x + i * 30, y - 20);
                    }
                }
            }
            else {
                
                sf::RectangleShape newPlatform(sf::Vector2f(PLATFORM_WIDTH, PLATFORM_HEIGHT));
                newPlatform.setPosition(x, y);
                newPlatform.setFillColor(sf::Color::Blue);
                platforms.push_back(newPlatform);
            }
        }

       
        window.clear(sf::Color::Black);


        if (!gameOver) {
     
            window.draw(ground);

      
            if (thirdphase) {
                window.draw(lava.shape);
            }

 
            for (const auto& platform : platforms)
                window.draw(platform);

 
            for (const auto& dp : disappearingPlatforms)
                window.draw(dp.shape);

         
            for (const auto& spike : spikes)
                window.draw(spike.shape);

        
            window.draw(player.sprite);

            if (enemyActive && (firstphase || thirdphase)) {
                window.draw(enemy.sprite);
            }

            if (triangleActive) window.draw(flag);
            if (secondPhaseFlagActive) window.draw(secondPhaseFlag);
            if (thirdPhaseFlagActive) window.draw(thirdPhaseFlag);
        }

    
        if (showDeathScreen) {
            window.draw(deathSprite);
        }

        window.display();
    }

    return 0;
}
