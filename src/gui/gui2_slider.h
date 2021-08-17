#ifndef GUI2_SLIDER_H
#define GUI2_SLIDER_H

#include "gui2_element.h"
#include "gui2_label.h"

class GuiBasicSlider : public GuiElement
{
public:
    typedef std::function<void(float value)> func_t;
protected:
    float min_value;
    float max_value;
    float value;
    func_t func;
public:
    GuiBasicSlider(GuiContainer* owner, string id, float min_value, float max_value, float start_value, func_t func);

    virtual void onDraw(sp::RenderTarget& window);
    virtual bool onMouseDown(glm::vec2 position);
    virtual void onMouseDrag(glm::vec2 position);
    virtual void onMouseUp(glm::vec2 position);

    GuiBasicSlider* setValue(float value);
    GuiBasicSlider* setRange(float min, float max);
    float getValue() const;
};

class GuiSlider : public GuiBasicSlider
{
public:
    typedef std::function<void(float value)> func_t;
protected:
    struct TSnapPoint
    {
        float value;
        float range;
    };
    std::vector<TSnapPoint> snap_points;
    GuiLabel* overlay_label;
public:
    GuiSlider(GuiContainer* owner, string id, float min_value, float max_value, float start_value, func_t func);

    virtual void onDraw(sp::RenderTarget& window);
    virtual bool onMouseDown(glm::vec2 position);
    virtual void onMouseDrag(glm::vec2 position);
    virtual void onMouseUp(glm::vec2 position);

    GuiSlider* setValueSnapped(float value);
    GuiSlider* clearSnapValues();
    GuiSlider* addSnapValue(float value, float range);
    GuiSlider* addOverlay();
};

class GuiSlider2D : public GuiElement
{
public:
public:
    typedef std::function<void(glm::vec2 value)> func_t;
protected:
    struct TSnapPoint
    {
        glm::vec2 value;
        glm::vec2 range;
    };
    glm::vec2 min_value;
    glm::vec2 max_value;
    glm::vec2 value;
    std::vector<TSnapPoint> snap_points;
    func_t func;
public:
    GuiSlider2D(GuiContainer* owner, string id, glm::vec2 min_value, glm::vec2 max_value, glm::vec2 start_value, func_t func);

    virtual void onDraw(sp::RenderTarget& window);
    virtual bool onMouseDown(glm::vec2 position);
    virtual void onMouseDrag(glm::vec2 position);
    virtual void onMouseUp(glm::vec2 position);

    GuiSlider2D* clearSnapValues();
    GuiSlider2D* addSnapValue(glm::vec2 value, glm::vec2 range);
    GuiSlider2D* setValue(glm::vec2 value);
    glm::vec2 getValue();
};

#endif//GUI2_SLIDER_H
