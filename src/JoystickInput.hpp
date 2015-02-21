#ifndef _JoystickInput_hpp_
#define _JoystickInput_hpp_

#include <GLFW/glfw3.h>

#include <vector>

// Currently hard-coded :(
// TODO: Button profiles

// Weak enums with less naming conflicts.
namespace myButtons
{
    enum { D = 0, A = 1, B = 2, C = 3, RESET = 5, TOGGLE = 4 };
}

namespace myAxis
{
    enum { HORI = 6, VERT = 7 };
}

class JoystickInput
{
    private:
        int joystick;

        std::vector<unsigned char> prevButtons;
        std::vector<float> prevAxis;

        std::vector<unsigned char> buttons;
        std::vector<float> axis;

    public:
        // Initialize joystick and update button data.
        // Throws a runtime_error exception if joystickNum does not exist.
        JoystickInput(int joystickNum=GLFW_JOYSTICK_1);

        void updateButtons();

        unsigned char getButton(int buttonId) const;
        float getAxis(int axisId) const;
        bool buttonChange(int buttonId) const;
        bool axisChange(int axisId) const;
};

#endif /* _JoystickInput_hpp_ */
