#include "Game.hpp"

#include <SFML/System/Exception.hpp>

#include <exception>
#include <iostream>

int main()
{
    try
    {
        bug_toss::Game game;
        game.run();
    }
    catch (const sf::Exception& exception)
    {
        std::cerr << "SFML error: " << exception.what() << '\n';
        return 1;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }
}
