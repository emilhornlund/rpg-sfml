#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({800u, 600u}), "RPG SFML");
    window.setVerticalSyncEnabled(true);

    sf::Clock clock;
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scan::Escape) window.close();
            }
        }

        const float dt = clock.restart().asSeconds();
        // update(dt);

        window.clear(sf::Color::Black);
        // draw();
        window.display();
    }
}
