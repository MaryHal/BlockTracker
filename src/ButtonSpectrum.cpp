#include "ButtonSpectrum.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>

#include "Font.hpp"

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

void ButtonSpectrum::addButton(int buttonCount, const unsigned char* buttonStates,
                               int axisCount, const float* axisStates)
{
    for (int i = 0; i < buttonCount; ++i)
    {
        // Only care if we switch TO the pressed state.
        if (prevButtonStates[i] != buttonStates[i] && buttonStates[i] == GLFW_PRESS)
        {
            if (i == 0)
                buttons.back() += 'D';
            if (i == 1)
                buttons.back() += 'A';
            if (i == 2)
                buttons.back() += 'B';
            if (i == 3)
                buttons.back() += 'C';
        }
        prevButtonStates[i] = buttonStates[i];
    }

    for (int i = 0; i < axisCount; ++i)
    {
        if (prevAxisStates[i] != axisStates[i])
        {
            if (i == 7 && axisStates[i] < -0.9f)
                buttons.back() += 'U';
            if (i == 7 && axisStates[i] > 0.9f)
                buttons.back() += 'D';
            if (i == 6 && axisStates[i] < -0.9f)
                buttons.back() += 'L';
            if (i == 6 && axisStates[i] > 0.9f)
                buttons.back() += 'R';
        }
        prevAxisStates[i] = axisStates[i];
    }
}

void ButtonSpectrum::newSection()
{
    buttons.emplace_back();
}

void ButtonSpectrum::draw(float x, float y, const Font& font) const
{
    glPushMatrix();

    glColor4f(0.8f, 0.8f, 0.8f, 1.0f);

    int i = 10;
    for (auto iter = buttons.rbegin();
         i > 0 && iter != buttons.rend();
         --i, ++iter)
    {
        font.draw(x, y + 20 * i, *iter);
    }

    glPopMatrix();
}
