#include "scrollingBanner.h"
#include "gameGlobalInfo.h"

GuiScrollingBanner::GuiScrollingBanner(GuiContainer* owner)
: GuiElement(owner, "")
{
    draw_offset = 0;
}

void GuiScrollingBanner::onDraw(sp::RenderTarget& renderer)
{
    draw_offset += update_clock.restart() * scroll_speed_per_second;

    if (!gameGlobalInfo || gameGlobalInfo->banner_string == "")
    {
        draw_offset = 0;
        return;
    }
/* TODO_GFX
    {
        sf::Texture* texture_ptr = textureManager.getTexture("gui/widget/ButtonBackground.png");
        sf::Vector2f texture_size = sf::Vector2f(texture_ptr->getSize());
        sf::VertexArray a(sf::TrianglesStrip, 4);

        a[0].position = sf::Vector2f(rect.position.x, rect.position.y);
        a[1].position = sf::Vector2f(rect.position.x, rect.position.y + rect.size.y);
        a[2].position = sf::Vector2f(rect.position.x + rect.size.x, rect.position.y);
        a[3].position = sf::Vector2f(rect.position.x + rect.size.x, rect.position.y + rect.size.y);

        a[0].texCoords = sf::Vector2f(texture_size.x / 2, 0);
        a[1].texCoords = sf::Vector2f(texture_size.x / 2, texture_size.y);
        a[2].texCoords = sf::Vector2f(texture_size.x / 2, 0);
        a[3].texCoords = sf::Vector2f(texture_size.x / 2, texture_size.y);

        for(int n=0; n<4; n++)
            a[n].color = glm::u8vec4{255,255,255,255};

        window.draw(a, texture_ptr);
    }
    {
        float font_size = rect.size.y;
        sf::Text text(gameGlobalInfo->banner_string, *bold_font, font_size);
        if (draw_offset > text.getLocalBounds().width + black_area)
            draw_offset -= text.getLocalBounds().width + black_area;
        float x = -draw_offset;
        float y = rect.position.y + rect.size.y / 2 - font_size + font_size * 0.35;

        while(x < window.getView().getSize().x)
        {
            text.setPosition(x, y);
            window.draw(text);
            x += text.getLocalBounds().width;
            x += black_area;
        }
    }
*/
}
