#include "playerInfo.h"
#include "gameGlobalInfo.h"
#include "windowScreen.h"
#include "epsilonServer.h"
#include "main.h"

#include "screenComponents/viewport3d.h"
#include "screenComponents/indicatorOverlays.h"
#include "screenComponents/shipDestroyedPopup.h"

WindowScreen::WindowScreen(float angle, uint8_t flags)
: angle(angle)
{
    viewport = new GuiViewport3D(this, "VIEWPORT");
    if (flags & flag_callsigns)
      viewport->showCallsigns();
    if (flags & flag_headings)
      viewport->showHeadings();
    if (flags & flag_spacedust)
      viewport->showSpacedust();
    viewport->setPosition(0, 0, ATopLeft)->setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);

    new GuiShipDestroyedPopup(this);

    new GuiIndicatorOverlays(this);
}

void WindowScreen::update(float delta)
{
    if (game_client && game_client->getStatus() == GameClient::Disconnected)
    {
        destroy();
        disconnectFromServer();
        returnToMainMenu();
        return;
    }

    if (my_spaceship)
    {
        camera_yaw = my_spaceship->getRotation() + angle;
        camera_pitch = 0.0f;

        sf::Vector2f position = my_spaceship->getPosition() + sf::rotateVector(sf::Vector2f(my_spaceship->getRadius(), 0), camera_yaw);

        camera_position.x = position.x;
        camera_position.y = position.y;
        camera_position.z = 0.0;
    }
}

void WindowScreen::onKey(const SDL_Keysym& key, int unicode)
{
    switch(key.scancode)
    {
    case SDL_SCANCODE_LEFT:
        angle -= 5.0f;
        break;
    case SDL_SCANCODE_RIGHT:
        angle += 5.0f;
        break;

    //TODO: This is more generic code and is duplicated.
    case SDL_SCANCODE_ESCAPE:
    case SDL_SCANCODE_HOME:
        destroy();
        returnToShipSelection();
        break;
    case SDL_SCANCODE_P:
        if (game_server)
            engine->setGameSpeed(0.0);
        break;
    default:
        break;
    }
}
