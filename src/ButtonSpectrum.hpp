#ifndef _ButtonSpectrum_hpp_
#define _ButtonSpectrum_hpp_

#include <string>
#include <vector>

#include "JoystickInput.hpp"

namespace fgen { class OpenGLFont; }

class ButtonSpectrum
{
    private:
        struct ButtonID
        {
            public:
                ButtonID(wchar_t b)
                    : button{b}, held{true}
                {
                }
                wchar_t button;
                bool held;
        };

        std::vector<std::vector<ButtonID> > buttons;

    public:
        ButtonSpectrum();

        void clear();
        void addButton(const JoystickInput& joystick);
        void unhold(wchar_t button);

        void newSection();
        void draw(float x, float y, const fgen::OpenGLFont& font) const;
};

#endif /* _ButtonSpectrum_hpp_ */
