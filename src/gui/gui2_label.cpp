#include "gui2_label.h"

GuiLabel::GuiLabel(GuiContainer* owner, string id, string text, float text_size)
: GuiElement(owner, id), text(text), text_size(text_size), text_color(glm::u8vec4{255,255,255,255}), text_alignment(sp::Alignment::Center), background(false), bold(false), vertical(false)
{
}

void GuiLabel::onDraw(sp::RenderTarget& renderer)
{
    if (background)
        renderer.drawStretched(rect, "gui/widget/LabelBackground.png", selectColor(colorConfig.label.background));
    glm::u8vec4 color = selectColor(colorConfig.label.forground);
    sp::Font* font = main_font;
    if (bold)
        font = bold_font;
    if (vertical)
        renderer.drawText(rect, text, text_alignment, text_size, font, color, sp::Font::FlagVertical);
    else
        renderer.drawText(rect, text, text_alignment, text_size, font, color);
}

GuiLabel* GuiLabel::setText(string text)
{
    this->text = text;
    return this;
}

string GuiLabel::getText() const
{
    return text;
}

GuiLabel* GuiLabel::setAlignment(sp::Alignment alignment)
{
    text_alignment = alignment;
    return this;
}

GuiLabel* GuiLabel::addBackground()
{
    background = true;
    return this;
}

GuiLabel* GuiLabel::setVertical()
{
    vertical = true;
    return this;
}

GuiLabel* GuiLabel::setBold(bool bold)
{
    this->bold = bold;
    return this;
}
