#include "ButtonSpectrum.hpp"
#include "JoystickInput.hpp"

#include <cmath>

#include <fontgen/OpenGLFont.hpp>

ButtonSpectrum::ButtonSpectrum()
{
    buttons.emplace_back();
}

ButtonSpectrum::~ButtonSpectrum()
{
}

void ButtonSpectrum::clear()
{
    buttons.clear();
    buttons.emplace_back();
}

void ButtonSpectrum::addButton(const JoystickInput& joystick)
{
    if (joystick.buttonChange(myButtons::D) &&
        joystick.getButton(myButtons::D) == GLFW_PRESS)
        buttons.back() += L"D";
    if (joystick.buttonChange(myButtons::A) &&
        joystick.getButton(myButtons::A) == GLFW_PRESS)
        buttons.back() += L"A";
    if (joystick.buttonChange(myButtons::B) &&
        joystick.getButton(myButtons::B) == GLFW_PRESS)
        buttons.back() += L"B";
    if (joystick.buttonChange(myButtons::C) &&
        joystick.getButton(myButtons::C) == GLFW_PRESS)
        buttons.back() += L"C";

    if (joystick.axisChange(myAxis::HORI) &&
        joystick.getAxis(myAxis::HORI) <= -1.0f)
        buttons.back() += L"⇦";
    if (joystick.axisChange(myAxis::HORI) &&
        joystick.getAxis(myAxis::HORI) >= 1.0f)
        buttons.back() += L"⇨";
    if (joystick.axisChange(myAxis::VERT) &&
        joystick.getAxis(myAxis::VERT) <= -1.0f)
        buttons.back() += L"⇧";
    if (joystick.axisChange(myAxis::VERT) &&
        joystick.getAxis(myAxis::VERT) >= 1.0f)
        buttons.back() += L"⇩";
}

void ButtonSpectrum::newSection()
{
    buttons.emplace_back();
}

void ButtonSpectrum::draw(float x, float y, const fgen::OpenGLFont& font) const
{
    glPushMatrix();

    glColor4f(0.8f, 0.8f, 0.8f, 1.0f);

    // Display the pressed buttons for the last 10 pieces.
    int i = 10;
    for (auto iter = buttons.rbegin();
         i > 0 && iter != buttons.rend();
         --i, ++iter)
    {
        font.draw(x, y + 20 * i, *iter);
    }

    glPopMatrix();
}
