#ifndef _ButtonSpectrum_hpp_
#define _ButtonSpectrum_hpp_

#include <string>
#include <vector>

class Font;
class JoystickInput;

class ButtonSpectrum
{
    private:
        std::vector<std::string> buttons;

    public:
        ButtonSpectrum();
        ~ButtonSpectrum();

        void clear();
        void addButton(const JoystickInput& joystick);
        void newSection();
        void draw(float x, float y, const Font& font) const;
};

#endif /* _ButtonSpectrum_hpp_ */
