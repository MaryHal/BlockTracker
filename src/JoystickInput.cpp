#include "JoystickInput.hpp"
#include <stdexcept>

JoystickInput::JoystickInput(int joystickNum)
    : joystick{joystickNum},
      prevButtons{},
      prevAxis{},
      buttons{},
      axis{}
      {
          if (!glfwJoystickPresent(joystickNum))
          {
              throw std::runtime_error{"Joystick " + std::to_string(joystickNum) + " not found."};
          }

          updateButtons();
      }

void JoystickInput::updateButtons()
{
    // Store all the previous button states. This will also clear the current saved button states.
    prevButtons = std::move(buttons);
    prevAxis = std::move(axis);

    int buttonCount = 0;
    auto* buttonStates = glfwGetJoystickButtons(joystick, &buttonCount);

    for (int i = 0; i < buttonCount; ++i)
        buttons.push_back(buttonStates[i]);

    int axisCount = 0;
    auto* axisStates = glfwGetJoystickAxes(joystick, &axisCount);

    for (int i = 0; i < axisCount; ++i)
        axis.push_back(axisStates[i]);
}

unsigned char JoystickInput::getButton(int buttonId) const
{
    return buttons[buttonId];
}

float JoystickInput::getAxis(int axisId) const
{
    return axis[axisId];
}

bool JoystickInput::buttonChange(int buttonId) const
{
    return buttons[buttonId] != prevButtons[buttonId];
}

bool JoystickInput::axisChange(int axisId) const
{
    return axis[axisId] != prevAxis[axisId];
}
