#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <list>
#include "v5.h"
#include <sstream>
std::list<std::unique_ptr<Item>> items;
bool isValidPlatformPosition(float x, float y, const std::vector<sf::RectangleShape>& platforms,
    const std::list<DisappearingPlatform>& disappearingPlatforms,
    float minHorizontalDist, float minVerticalDist) {


    for (const auto& platform : platforms) {

        float px = platform.getPosition().x;
        float py = platform.getPosition().y;

        if (abs(x - px) < minHorizontalDist && abs(y - py) < minVerticalDist) {
            return false;
        }
    }


    for (const auto& dp : disappearingPlatforms) {
        float px = dp.shape.getPosition().x;
        float py = dp.shape.getPosition().y;

        if (abs(x - px) < minHorizontalDist && abs(y - py) < minVerticalDist) {
            return false;
        }
    }

    return true;
}

int countVisiblePlatforms(const std::vector<sf::RectangleShape>& platforms, const sf::View& view) {
    int count = 0;


    float top = view.getCenter().y - view.getSize().y / 2.f;
    float bottom = view.getCenter().y + view.getSize().y / 2.f;

    for (const auto& platform : platforms) {
        sf::FloatRect bounds = platform.getGlobalBounds();
        float platformTop = bounds.top;
        float platformBottom = bounds.top + bounds.height;


        if (platformBottom >= top && platformTop <= bottom) {
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

void generatePlatform(float& lastPlatformY,
    std::vector<sf::RectangleShape>& platforms,
    std::list<DisappearingPlatform>& disappearingPlatforms,
    std::vector<Spike>& spikes,
    int phase,
    sf::RectangleShape& ground,
    const Player& player,
    std::list<std::unique_ptr<Item>>& items){



    pc += 1;
    int platformWidth = 100;
    int platformHeight = 20;


    float y = lastPlatformY - (25.0f + static_cast<float>(rand() % 25));

    if (y > player.shape.getPosition().y + 1500) {
        return;
    }

    float x = static_cast<float>(rand() % (WINDOW_WIDTH - platformWidth));


    int attempts = 0;
    while (!isValidPlatformPosition(x, y, platforms, disappearingPlatforms,
        MIN_HORIZONTAL_DISTANCE, MIN_VERTICAL_DISTANCE) && attempts < 10) {
        x = static_cast<float>(rand() % (WINDOW_WIDTH - platformWidth));
        attempts++;
        if (attempts >= 5) {

            x = static_cast<float>(rand() % (WINDOW_WIDTH - platformWidth));
            y -= 100;
        }
    }

    if (attempts >= 5) {
        return;
    }

    switch (phase) {
    case 1: {
        sf::RectangleShape newPlatform(sf::Vector2f(platformWidth, platformHeight));
        newPlatform.setPosition(x, y);
        newPlatform.setFillColor(sf::Color::Blue);
        platforms.push_back(newPlatform);
        if (rand() % 20 == 0) { // 5% szansy na item
            int itemType = rand() % 3;
            sf::Vector2f itemPos(x + 30, y - 40); // nad platform¹

            switch (itemType) {
            case 0:
                items.push_back(std::make_unique<Feather>(itemPos));
                break;
            case 1:
                items.push_back(std::make_unique<Clock>(itemPos, player.shape.getPosition()));
                break;
            case 2:
                items.push_back(std::make_unique<Shield>(itemPos));
                break;
            }
        }
        break;
    }
    case 2: {
        int platformType = (rand() % 11) + 1;

        if (platformType % 3 == 0) {
            disappearingPlatforms.emplace_back(x, y, 20.0f);

        }
        else if (platformType % 10) {
            sf::RectangleShape newPlatform(sf::Vector2f(platformWidth, platformHeight));
            newPlatform.setPosition(x, y);

            if (platformType == 1) {
                newPlatform.setFillColor(sf::Color::Green);
                platforms.push_back(newPlatform);


                for (int i = 0; i < platformWidth / 100; i++) {
                    spikes.emplace_back(x + i * 30, y - 20);
                }
            }
            else {
                newPlatform.setFillColor(sf::Color(0, 255, 0));
                platforms.push_back(newPlatform);
            }
        }
        break;
    }
    case 3: {
        int platformType = (rand() % 11) + 1;

        if (platformType % 3 == 0) {
            disappearingPlatforms.emplace_back(x, y, 10.0f);


        }
        else if (platformType % 10) {
            sf::RectangleShape newPlatform(sf::Vector2f(platformWidth, platformHeight));
            newPlatform.setPosition(x, y);

            if (platformType == 1) {
                newPlatform.setFillColor(sf::Color(255, 125, 0));
                platforms.push_back(newPlatform);


                for (int i = 0; i < platformWidth / 50; i++) {
                    spikes.emplace_back(x + i * 30, y - 20);
                }
            }
            else {
                newPlatform.setFillColor(sf::Color(255, 125, 0));
                platforms.push_back(newPlatform);
            }
        }
        break;
    }
    }


    lastPlatformY = y;
}

int main() {
    bool waitingToStart = true;
    std::list<std::unique_ptr<Item>> items;
    srand(time(0));
    sf::Clock gameClock;
    sf::Clock spawnClock;
    sf::Clock deathClock;
    sf::Clock phaseTransitionClock;
    bool gameOver = false;
    bool showDeathScreen = false;
    bool triangleActive = false;


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
    std::vector<sf::RectangleShape> platforms;
    Player player(20.0f);


    Character enemy;
    bool enemyActive = false;

    sf::RectangleShape ground(sf::Vector2f(2000, 600));
    ground.setFillColor(sf::Color(139, 69, 19));

    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;


    }

    sf::Text startText;
    startText.setFont(font);
    startText.setCharacterSize(40);
    startText.setFillColor(sf::Color::White);
    startText.setString("Naciœnij Enter, aby rozpocz¹æ grê");

    sf::FloatRect startBounds = startText.getLocalBounds();
    startText.setOrigin(startBounds.left + startBounds.width / 2.0f, startBounds.top + startBounds.height / 2.0f);
    startText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);

    scoreText.setFont(font);
    scoreText.setCharacterSize(50);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setStyle(sf::Text::Bold);
    scoreText.setFont(font);
    scoreText.setCharacterSize(50);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50);
    sf::Texture deathTexture;
    sf::Sprite deathSprite;
    if (!deathTexture.loadFromFile("smierc.png")) {
        std::cerr << "Failed to load death texture!" << std::endl;
        return -1;
    }
    deathSprite.setTexture(deathTexture);
    deathSprite.setOrigin(deathTexture.getSize().x / 2.0f, deathTexture.getSize().y / 2.0f);
    deathSprite.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);


    const int PLATFORM_COUNT = 100;
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



    float lastPlatformY = 500;
    for (int i = 0; i < MAX_PLATFORMS; ++i) {
        generatePlatform(lastPlatformY, platforms, disappearingPlatforms, spikes, Phase, ground, player, items);
    }



    float lowestPlatformY = findLowestPlatformY(platforms);
    ground.setPosition(0, player.shape.getPosition().y + 30);


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (waitingToStart && event.key.code == sf::Keyboard::Enter) {
                    waitingToStart = false;
                }
                else if (!waitingToStart && !gameOver && event.key.code == sf::Keyboard::Space) {
                    player.jump();
                }
            }
        }
        if (waitingToStart) {
            window.clear(sf::Color::Black);
            window.draw(startText);
            window.display();
            continue;
        }

        for (auto it = disappearingPlatforms.begin(); it != disappearingPlatforms.end(); ) {
            if (!it->active) {
                it = disappearingPlatforms.erase(it);
            }
            else {
                ++it;
            }
        }


        if (player.shape.getPosition().x > WINDOW_WIDTH) {
            player.shape.setPosition(0, player.shape.getPosition().y);
        }
        if (player.shape.getPosition().x < 0) {
            player.shape.setPosition(WINDOW_WIDTH, player.shape.getPosition().y);
        }
        //sf::Event event;
        float deltaTime = gameClock.restart().asSeconds();

        while (window.pollEvent(event)) {

           

            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (waitingToStart && event.key.code == sf::Keyboard::Enter) {
                    waitingToStart = false;
                }
                if (!waitingToStart && !gameOver && event.key.code == sf::Keyboard::Space) {
                    player.jump();
                }
            }

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

        sf::FloatRect playerBounds = player.shape.getGlobalBounds();





        if (Phase == 1) {
            player.update(platforms, ground, view);
            for (auto it = items.begin(); it != items.end();) {
                if ((*it)->sprite.getGlobalBounds().intersects(player.shape.getGlobalBounds())) {
                    (*it)->applyEffect(player);
                    it = items.erase(it);
                }
                else {
                    ++it;
                }
            }
            float currentHeight = player.shape.getPosition().y - phaseStartY;
            if (currentHeight < highestReached) {
                highestReached = currentHeight;

                if (!triangleActive && highestReached < -1500) {
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
                    Phase = 2;
                    ground.setPosition(0, player.shape.getPosition().y + 1000);
                    triangleActive = false;

                    highestReached = 0.0f;
                    phaseStartY = player.shape.getPosition().y;
                    lastPlatformY = phaseStartY;
                    lastPlatformY += 700;
                    view.setCenter(WINDOW_WIDTH / 2, phaseStartY + WINDOW_HEIGHT / 3);
                    window.setView(view);
                    phaseTransitionClock.restart();
                    for (int i = 0; i < MAX_PLATFORMS; ++i) {
                        generatePlatform(lastPlatformY, platforms, disappearingPlatforms, spikes, Phase, ground, player, items);
                    }

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
        else if (Phase == 2) {
            player.update(platforms, ground, view);
            for (auto it = items.begin(); it != items.end();) {
                if ((*it)->sprite.getGlobalBounds().intersects(player.shape.getGlobalBounds())) {
                    (*it)->applyEffect(player);
                    it = items.erase(it);
                }
                else {
                    ++it;
                }
            }
            float currentHeight = player.shape.getPosition().y - phaseStartY;

            if (!triangleActive &&

                player.shape.getPosition().y < -5000.0f) {
                triangleActive = true;
                secondPhaseFlag.setPosition(
                    player.shape.getPosition().x + 300,
                    player.shape.getPosition().y - 300
                );
            }

            if (triangleActive) {

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
                    spikes.clear();
                    disappearingPlatforms.clear();
                    Phase = 3;
                    triangleActive = false;

                    highestReached = 0.0f;
                    phaseStartY = player.shape.getPosition().y;
                    lastPlatformY = phaseStartY;
                    lastPlatformY += 700;
                    view.setCenter(WINDOW_WIDTH / 2, phaseStartY + WINDOW_HEIGHT / 3);
                    window.setView(view);
                    phaseTransitionClock.restart();
                    for (int i = 0; i < MAX_PLATFORMS; ++i) {
                        generatePlatform(lastPlatformY, platforms, disappearingPlatforms, spikes, Phase, ground, player, items);

                    }
                    lava.shape.setPosition(0, player.shape.getPosition().y + 500);
                }
            }
            for (auto it = disappearingPlatforms.begin(); it != disappearingPlatforms.end(); ) {
                if (!it->active) {
                    it = disappearingPlatforms.erase(it);
                }
                else {

                    ++it;
                }
            }


            for (auto& dp : disappearingPlatforms) {
                sf::Vector2f platformPos = dp.shape.getPosition();
                sf::Vector2f playerPos = player.shape.getPosition();
                sf::Vector2f platformSize = dp.shape.getSize();


                if (playerPos.y + player.radius >= platformPos.y &&
                    playerPos.y + player.radius <= platformPos.y + 5.0f &&
                    playerPos.x + player.radius > platformPos.x &&
                    playerPos.x - player.radius < platformPos.x + platformSize.x &&
                    player.velocity.y > 0)
                {
                    player.velocity.y = 0;
                    player.canJump = true;
                    player.shape.setPosition(playerPos.x, platformPos.y - player.radius);

                    if (!dp.touched) {
                        dp.touched = true;
                        dp.timer.restart();
                        dp.shape.setFillColor(sf::Color(255, 100, 100));
                    }
                }

                if (dp.touched && dp.timer.getElapsedTime().asSeconds() > dp.disappearDelay) {
                    dp.active = false;
                }


                if (dp.touched) {
                    float timeLeft = dp.disappearDelay - dp.timer.getElapsedTime().asSeconds();
                    if (timeLeft < 1.0f) {
                        int alpha = (static_cast<int>(timeLeft * 1000) % 200) > 100 ? 255 : 100;
                        dp.shape.setFillColor(sf::Color(255, 0, 0, alpha));
                    }
                }
            }



            for (auto& spike : spikes) {
                if (player.shape.getGlobalBounds().intersects(spike.shape.getGlobalBounds())) {
                    if (player.hasShield) {
                        player.hasShield = false;
                    }
                    else {
                        gameOver = true;
                        showDeathScreen = true;
                        deathClock.restart();
                    }
                    showDeathScreen = true;
                    deathClock.restart();
                    break;
                }
            }
        }
        else if (Phase == 3) {
            lava.update(deltaTime);

            if (player.shape.getPosition().y + player.radius > lava.shape.getPosition().y) {
                if (player.hasShield) {
                    player.hasShield = false;
                }
                else {
                    gameOver = true;
                    showDeathScreen = true;
                    deathClock.restart();
                }
                showDeathScreen = true;
                deathClock.restart();
            }

            player.update(platforms, ground, view);
            for (auto it = items.begin(); it != items.end();) {
                if ((*it)->sprite.getGlobalBounds().intersects(player.shape.getGlobalBounds())) {
                    (*it)->applyEffect(player);
                    it = items.erase(it);
                }
                else {
                    ++it;
                }
            }


            if (!triangleActive &&
                player.shape.getPosition().y < -9000.0f) {

                triangleActive = true;

                thirdPhaseFlag.setPosition(
                    player.shape.getPosition().x + 300,
                    player.shape.getPosition().y - 300

                );

            }
            if (triangleActive) {
                sf::FloatRect flagBounds = thirdPhaseFlag.getGlobalBounds();
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
                    spikes.clear();
                    disappearingPlatforms.clear();

                    float elapsedTime = gameTimeClock.getElapsedTime().asSeconds();
                    int score = static_cast<int>(elapsedTime * 10);
                    showScore = true;


                    std::ostringstream ss;
                    ss << "Your Score: " << score;
                    scoreText.setString(ss.str());


                    sf::FloatRect textRect = scoreText.getLocalBounds();
                    scoreText.setOrigin(textRect.left + textRect.width / 2.0f,
                        textRect.top + textRect.height / 2.0f);
                    scoreText.setPosition(view.getCenter());


                }
            }
            for (auto& spike : spikes) {
                if (player.shape.getGlobalBounds().intersects(spike.bounds)) {
                    if (player.hasShield) {
                        player.hasShield = false;
                    }
                    else {
                        gameOver = true;
                        showDeathScreen = true;
                        deathClock.restart();
                    }
                    showDeathScreen = true;
                    deathClock.restart();
                    break;
                }
            }
            for (auto& dp : disappearingPlatforms) {
                sf::Vector2f platformPos = dp.shape.getPosition();
                sf::Vector2f playerPos = player.shape.getPosition();
                sf::Vector2f platformSize = dp.shape.getSize();


                if (playerPos.y + player.radius >= platformPos.y &&
                    playerPos.y + player.radius <= platformPos.y + 5.0f &&
                    playerPos.x + player.radius > platformPos.x &&
                    playerPos.x - player.radius < platformPos.x + platformSize.x &&
                    player.velocity.y > 0)
                {
                    player.velocity.y = 0;
                    player.canJump = true;
                    player.shape.setPosition(playerPos.x, platformPos.y - player.radius);

                    if (!dp.touched) {
                        dp.touched = true;
                        dp.timer.restart();
                        dp.shape.setFillColor(sf::Color(255, 100, 100));
                    }
                }


                if (dp.touched && dp.timer.getElapsedTime().asSeconds() > dp.disappearDelay) {
                    dp.active = false;
                }


                if (dp.touched) {
                    float timeLeft = dp.disappearDelay - dp.timer.getElapsedTime().asSeconds();
                    if (timeLeft < 1.0f) {
                        int alpha = (static_cast<int>(timeLeft * 1000) % 200) > 100 ? 255 : 100;
                        dp.shape.setFillColor(sf::Color(255, 100, 100, alpha));
                    }
                }
            }

            if (!enemyActive && spawnClock.getElapsedTime().asSeconds() >= SPAWN_INTERVAL) {
                enemy.radius = 30.0f;

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

        if (enemyActive && (Phase == 1 || Phase == 3)) {
            sf::Vector2f direction = player.shape.getPosition() - enemy.shape.getPosition();
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (distance <= 10) {
                if (player.hasShield) {
                    player.hasShield = false;
                }
                else {
                    gameOver = true;
                    showDeathScreen = true;
                    deathClock.restart();
                }
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
        float viewTop = view.getCenter().y - WINDOW_HEIGHT / 2;
        float viewBottom = view.getCenter().y + WINDOW_HEIGHT / 2;


        float currentLowestPlatformY = findLowestPlatformY(platforms);

        int visiblePlatforms = countVisiblePlatforms(platforms, view);
        float lastPlatformY = platforms.empty() ? viewTop : findHighestPlatformY(platforms);
        int spawnAttempts = 0;
        const int MAX_SPAWN_ATTEMPTS = 50;
        if (spawnAttempts >= MAX_SPAWN_ATTEMPTS) {
            std::cerr << "Warning: couldn't generate enough platforms after many attempts.\n";
        }

        while (visiblePlatforms < MIN_VISIBLE_PLATFORMS && spawnAttempts < MAX_SPAWN_ATTEMPTS && MAX_PLATFORMS>pc) {
            generatePlatform(lastPlatformY, platforms, disappearingPlatforms, spikes, Phase, ground, player, items);
            visiblePlatforms = countVisiblePlatforms(platforms, view);
            spawnAttempts++;
        }
        /*

        window.clear(sf::Color::Black);

        if (!gameOver) {
            window.draw(ground);

            if (Phase == 3) {
                window.draw(lava.shape);
            }

            for (const auto& platform : platforms)
                window.draw(platform);

            for (const auto& dp : disappearingPlatforms)
                window.draw(dp.shape);

            for (const auto& spike : spikes)
                window.draw(spike.shape);

            window.draw(player.sprite);

            if (enemyActive && (Phase == 1 || Phase == 3)) {
                window.draw(enemy.sprite);
            }

            if (triangleActive) {
                if (Phase == 1) {
                    window.draw(flag);
                }
                else if (Phase == 2) {
                    window.draw(secondPhaseFlag);
                }

                else if (Phase == 3) {
                    window.draw(thirdPhaseFlag);

                }
            }

            if (showDeathScreen) {
                window.draw(deathSprite);
            }
        }
        */
        window.clear(sf::Color::Black);

        if (!gameOver) {
            window.draw(ground);

            if (Phase == 3) {
                window.draw(lava.shape);
            }

            for (const auto& platform : platforms)
                window.draw(platform);

            for (const auto& item : items)
                window.draw(item->sprite);

            for (const auto& dp : disappearingPlatforms)
                window.draw(dp.shape);

            for (const auto& dp : disappearingPlatforms)
                window.draw(dp.shape);

            for (const auto& spike : spikes)
                window.draw(spike.shape);

            window.draw(player.sprite);

            if (enemyActive && (Phase == 1 || Phase == 3)) {
                window.draw(enemy.sprite);
            }

            if (triangleActive) {
                if (Phase == 1) {
                    window.draw(flag);
                }
                else if (Phase == 2) {
                    window.draw(secondPhaseFlag);
                }
                else if (Phase == 3) {
                    window.draw(thirdPhaseFlag);
                }
            }

            if (showDeathScreen) {
                window.draw(deathSprite);
            }
        }

        window.display();
    }

    return 0;
}
