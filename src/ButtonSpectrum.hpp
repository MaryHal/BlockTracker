#ifndef _ButtonSpectrum_hpp_
#define _ButtonSpectrum_hpp_

#include <string>
#include <vector>

namespace fgen { class OpenGLFont; }

class JoystickInput;

class ButtonSpectrum
{
    private:
        std::vector<std::wstring> buttons;

    public:
        ButtonSpectrum();
        ~ButtonSpectrum();

        void clear();
        void addButton(const JoystickInput& joystick);
        void newSection();
        void draw(float x, float y, const fgen::OpenGLFont& font) const;
};

#endif /* _ButtonSpectrum_hpp_ */
