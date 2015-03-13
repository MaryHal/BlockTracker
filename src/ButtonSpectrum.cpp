#include "ButtonSpectrum.hpp"

#include <cmath>

#include <fontgen/OpenGLFont.hpp>

ButtonSpectrum::ButtonSpectrum()
{
    buttons.emplace_back();
}

void ButtonSpectrum::clear()
{
    buttons.clear();
    buttons.emplace_back();
}

void ButtonSpectrum::addButton(const JoystickInput& joystick)
{
    // This is horrible.
    if (joystick.buttonChange(myButtons::D) &&
        joystick.getButton(myButtons::D) == GLFW_PRESS)
    {
        buttons.back().emplace_back(L'D');
    }
    if (joystick.buttonChange(myButtons::A) &&
        joystick.getButton(myButtons::A) == GLFW_PRESS)
    {
        buttons.back().emplace_back(L'A');
    }
    if (joystick.buttonChange(myButtons::B) &&
        joystick.getButton(myButtons::B) == GLFW_PRESS)
    {
        buttons.back().emplace_back(L'B');
    }
    if (joystick.buttonChange(myButtons::C) &&
        joystick.getButton(myButtons::C) == GLFW_PRESS)
    {
        buttons.back().emplace_back(L'C');
    }

    if (joystick.axisChange(myAxis::HORI) &&
        joystick.getAxis(myAxis::HORI) <= -1.0f)
    {
        buttons.back().emplace_back(L'⇦');
    }
    if (joystick.axisChange(myAxis::HORI) &&
        joystick.getAxis(myAxis::HORI) >= 1.0f)
    {
        buttons.back().emplace_back(L'⇨');
    }
    if (joystick.axisChange(myAxis::VERT) &&
        joystick.getAxis(myAxis::VERT) <= -1.0f)
    {
        buttons.back().emplace_back(L'⇧');
    }
    if (joystick.axisChange(myAxis::VERT) &&
        joystick.getAxis(myAxis::VERT) >= 1.0f)
    {
        buttons.back().emplace_back(L'⇩');
    }

    // This is HORRIBLE
    if (joystick.buttonChange(myButtons::D) &&
        joystick.getButton(myButtons::D) == GLFW_RELEASE)
    {
        unhold(L'D');
    }
    if (joystick.buttonChange(myButtons::A) &&
        joystick.getButton(myButtons::A) == GLFW_RELEASE)
    {
        unhold(L'A');
    }
    if (joystick.buttonChange(myButtons::B) &&
        joystick.getButton(myButtons::B) == GLFW_RELEASE)
    {
        unhold(L'B');
    }
    if (joystick.buttonChange(myButtons::C) &&
        joystick.getButton(myButtons::C) == GLFW_RELEASE)
    {
        unhold(L'C');
    }

    // Oh my god
    if (joystick.axisChange(myAxis::HORI) &&
        joystick.getAxis(myAxis::HORI) > -1.0f)
    {
        unhold(L'⇦');
    }
    if (joystick.axisChange(myAxis::HORI) &&
        joystick.getAxis(myAxis::HORI) < 1.0f)
    {
        unhold(L'⇨');
    }
    if (joystick.axisChange(myAxis::VERT) &&
        joystick.getAxis(myAxis::VERT) > -1.0f)
    {
        unhold(L'⇧');
    }
    if (joystick.axisChange(myAxis::VERT) &&
        joystick.getAxis(myAxis::VERT) < 1.0f)
    {
        unhold(L'⇩');
    }
}

// This is horrible.
void ButtonSpectrum::unhold(wchar_t button)
{
    for (auto iter = buttons.rbegin();
         iter != buttons.rend();
         ++iter)
    {
        for (auto str_iter = iter->rbegin();
             str_iter != iter->rend();
             ++str_iter)
        {
            if (str_iter->button == button)
            {
                str_iter->held = false;
                return;
            }
        }
    }
}

void ButtonSpectrum::newSection()
{
    buttons.emplace_back();
}

void ButtonSpectrum::draw(float x, float y, const fgen::OpenGLFont& font) const
{
    glPushMatrix();

    // Display the pressed buttons for the last 10 pieces.
    int i = 10;
    for (auto iter = buttons.rbegin();
         i > 0 && iter != buttons.rend();
         --i, ++iter)
    {
        float x2 = x;
        float y2 = y + 20 * i;
        for (auto b : *iter)
        {
            // Sooooo inefficient!
            if (b.held)
                glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
            else
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            font.drawChar(x2, y2, b.button);
        }
    }

    glPopMatrix();
}
