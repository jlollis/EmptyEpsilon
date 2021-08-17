#include "gui2_arrowbutton.h"

GuiArrowButton::GuiArrowButton(GuiContainer* owner, string id, float angle, func_t func)
: GuiButton(owner, id, "", func), angle(angle)
{
}

void GuiArrowButton::onDraw(sp::RenderTarget& renderer)
{
    renderer.drawRotatedSprite("gui/widget/SelectorArrow.png", getCenterPoint(), std::min(rect.size.x, rect.size.y), angle, selectColor(colorConfig.button.forground));
}
