//
// Created by Per-Arne on 24.02.2017.
//
/*
 *
2
down vote
accepted
I've run into a similar issue myself and only after starting the application outside of the CLion IDE you'll actually get the informative error, which is that you're missing a DLL.

If you use the dynamic libraries, you'll have to copy the DLLs into the working directory or next to your application. Also don't forget to copy over the OpenAL DLL that ships with SFML if you ever want to use the audio module.

I really hope JetBrains can report the missing DLL error better in their IDE.
 */

#include "GUI.h"
#include "../Game.h"
#include <SFML/Graphics.hpp>


GUI::GUI(Game &game) :
        game(game),
        window(sf::VideoMode(800, 800), "X1337", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize){

    const Tilemap &tilemap = game.map;

    this->fullView = sf::View(sf::FloatRect(0, 0,
                                            tilemap.MAP_WIDTH * tilemap.TILE_WIDTH,
                                            tilemap.MAP_HEIGHT * tilemap.TILE_HEIGHT));
    this->fullView.setViewport(sf::FloatRect(0, 0, 1, 1));
    this->fullView.zoom(1);


    this->minimapView = sf::View(sf::FloatRect(0, 0,
                                               tilemap.MAP_WIDTH * tilemap.TILE_WIDTH,
                                               tilemap.MAP_HEIGHT * tilemap.TILE_HEIGHT));
    this->minimapView.setViewport(sf::FloatRect(.01, .25, .20, .22));

    this->povView = sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
    this->povView.setViewport(sf::FloatRect(0, 0, 1, 1));


    this->currentView = &this->povView;
    this->createFrame();


    this->font.loadFromFile("./data/fonts/arial.ttf");


}

void GUI::createFrame(){

    this->gameFrameTexture.loadFromFile("data/textures/game_frame.png");

    this->gameFrame.setTexture(this->gameFrameTexture);
    /*std::cout << this->gameFrameTexture.getSize().x << std::endl;
    this->gameFrame.setScale(sf::Vector2f((float)this->gameFrameTexture.getSize().x / window.getSize().x,
                                          (float)this->gameFrameTexture.getSize().y / window.getSize().y));
                                          */

}

void GUI::caption() {

}

int GUI::mouseClick(int mX, int mY)
{
    sf::Vector2f viewCenter = this->currentView->getCenter();
    sf::Vector2f halfExtents = this->currentView->getSize() / 2.0f;
    sf::Vector2f translation = viewCenter - halfExtents;

    mX += static_cast<int>(translation.x);
    mY += static_cast<int>(translation.y);

    int x = mX / this->game.map.TILE_WIDTH;
    int y = mY / this->game.map.TILE_HEIGHT;
    int cols = this->game.map.MAP_WIDTH;

    int idx = cols*y + x;
    //sf::Vector2f tiles = sf::Vector2f(mX / TILE_SIZE, mY / TILE_SIZE);

    return idx;

}

void GUI::render() {
    // Process events
    sf::Event event;
    while (window.pollEvent(event))
    {
        // Close window: exit
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                // get the current mouse position in the window
                sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                int idx = this->mouseClick(pixelPos.x, pixelPos.y);
                Tile &t = this->game.map.tiles[idx];
                this->leftClick(t);

            } else  if (event.mouseButton.button == sf::Mouse::Right)
            {
                // get the current mouse position in the window
                sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                int idx = this->mouseClick(pixelPos.x, pixelPos.y);
                Tile &t = this->game.map.tiles[idx];
                this->rightClick(t);

            }
        }


        if(event.type == sf::Event::KeyPressed)
        {
            if(event.key.code == sf::Keyboard::Left)
                this->panLeft = true;
            else if(event.key.code == sf::Keyboard::Right)
                this->panRight = true;
            else if(event.key.code == sf::Keyboard::Up)
                this->panUp = true;
            else if(event.key.code == sf::Keyboard::Down)
                this->panDown = true;


                // Change View
            else if(event.key.code == sf::Keyboard::Q) {
                this->currentView = &povView;
            }
            else if(event.key.code == sf::Keyboard::W) {
                this->currentView = &fullView;
            }
            else if(event.key.code == sf::Keyboard::R) {
                this->toggleFrame = !this->toggleFrame;
            }
        }
        else if(event.type == sf::Event::KeyReleased)
        {
            if(event.key.code == sf::Keyboard::Left)
                this->panLeft = false;
            else if(event.key.code == sf::Keyboard::Right)
                this->panRight = false;
            else if(event.key.code == sf::Keyboard::Up)
                this->panUp = false;
            else if(event.key.code == sf::Keyboard::Down)
                this->panDown = false;
        }
    }



    // Draw things here
    window.setView(*this->currentView);
    this->drawTiles();
    this->drawUnits();

    window.setView(this->fullView);
    this->drawSelected();  // Text


    /*if(this->toggleFrame){
        window.setView(this->fullView);
        window.draw(this->gameFrame);

        // TODO heavy
        window.setView(this->minimapView);
        this->drawTiles();
        this->drawUnits();
    }*/




    // Update the window
    window.display();

}

void GUI::update(){

    sf::Vector2f center = this->povView.getCenter();
    sf::Vector2u windowSize = window.getSize();

    if(this->panLeft && center.x > (windowSize.x / 2))
        this->povView.move(sf::Vector2f(this->velocity * -1, 0));
    if(this->panRight && center.x < (game.map.MAP_WIDTH * game.map.TILE_WIDTH) -  (windowSize.x / 2))
        this->povView.move(sf::Vector2f(this->velocity, 0));
    if(this->panUp && center.y > (windowSize.y / 2))
        this->povView.move(sf::Vector2f(0, this->velocity * -1));
    if(this->panDown && center.y < (game.map.MAP_HEIGHT * game.map.TILE_HEIGHT) -  (windowSize.y / 2))
        this->povView.move(sf::Vector2f(0, this->velocity));

}

void GUI::drawSelected(){
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(16);
    text.setFillColor(sf::Color::Yellow);

    /*Player &p = game.players.

    text.setString("Lumber: " + std::to_string(p.getLumber()));
    text.setPosition(10,10);
    window.draw(text);

    text.setString("Gold: " + std::to_string(p.getGold()));
    text.setPosition(125,10);
    window.draw(text);

    text.setString("Oil: " + std::to_string(p.getOil()));
    text.setPosition(225,10);
    window.draw(text);

    text.setString("Food: " + std::to_string(p.getFoodConsumption()) + "/" + std::to_string(p.getFood()));
    text.setPosition(300,10);
    window.draw(text);

    text.setString("Units: " + std::to_string(p.getUnitCount()));
    text.setPosition(380,10);
    window.draw(text);

    text.setString("Seconds: " + std::to_string(game.getSeconds()));
    text.setPosition(450,10);
    window.draw(text);

    text.setString("Frames: " + std::to_string(game.getFrames()));
    text.setPosition(550,10);
    window.draw(text);

    text.setString("Score: " + std::to_string(0));
    text.setPosition(650,10);
    window.draw(text);
     */

    if(this->selectedTile)
    {
        text.setString("Type: " + selectedTile->name + " - (" + std::to_string(selectedTile->x) + "," + std::to_string(selectedTile->y) + ")");
        text.setPosition(10,860);
        window.draw(text);


        text.setString((selectedTile->harvestable) ? "Harvestable: Yes" : "Harvestable: No");
        text.setPosition(10,880);
        window.draw(text);

        text.setString((selectedTile->walkable) ? "Walkable: Yes" : "Walkable: No");
        text.setPosition(10,900);
        window.draw(text);

        text.setString("Resources: " + std::to_string(selectedTile->resources));
        text.setPosition(10,920);
        window.draw(text);
    }



    if (selectedUnit) {
        text.setCharacterSize(32);
        Unit* unit = selectedUnit;

        text.setString(unit->name + " (" +
                               std::to_string(unit->id ) +
                               ")" + " - " +
                               unit->state->name + " - (" +
                               std::to_string(unit->tile->x) + "," +
                               std::to_string(unit->tile->y) + ") - (" +
                               std::to_string(unit->player_.getId()) + ")"
        );
        text.setPosition(320,830);
        window.draw(text);

        text.setCharacterSize(20);

        text.setString("Lumber: " + std::to_string(unit->lumberCarry));
        text.setPosition(270,870);
        window.draw(text);

        text.setString("Gold: " + std::to_string(unit->goldCarry));
        text.setPosition(270,900);
        window.draw(text);

        text.setString("Oil: " + std::to_string(unit->oilCarry));
        text.setPosition(270,930);
        window.draw(text);

        text.setString("Health: " + std::to_string(unit->health) + "/" + std::to_string(unit->health_max));
        text.setPosition(385,870);
        window.draw(text);

        text.setString("Damage: " + std::to_string(unit->damage_min) + " - " + std::to_string(unit->damage_max));
        text.setPosition(385, 900);
        window.draw(text);

        text.setString("Armor: " + std::to_string(unit->armor));
        text.setPosition(385,930);
        window.draw(text);

        text.setString((unit->groundUnit) ? "Ground: Yes" : "Ground: No");
        text.setPosition(550,870);
        window.draw(text);

        text.setString((unit->waterUnit) ? "Water: Yes" : "Water: No");
        text.setPosition(550,900);
        window.draw(text);

        text.setString("Speed: " + std::to_string(unit->speed));
        text.setPosition(550,930);
        window.draw(text);

    }

}

void GUI::drawTiles(){
    window.draw(game.map);
}

void GUI::drawUnits() {

for(auto&p : game.players)
{
    for(auto&u: p->units){
        if(u.tile) {
            std::cout << u.id << std::endl;
            window.draw(*u.testSprite);
        }
    }
}

}

void GUI::leftClick(Tile &tile) {
    this->selectedTile = &tile;

    if(tile.occupant) {
        this->selectedUnit = tile.occupant;
    }else {
        this->selectedUnit = NULL;
    }
}

void GUI::rightClick(Tile &tile) {
    if(this->selectedUnit) {
        std::cout << tile.getPixelPosition().x << "---" << tile.getPixelPosition().y << std::endl;
        this->selectedUnit->rightClick(tile);
        this->selectedTile = &tile;
    }
}