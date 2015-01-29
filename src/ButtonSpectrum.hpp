#ifndef _ButtonSpectrum_hpp_
#define _ButtonSpectrum_hpp_

#include <string>
#include <vector>

class Font;

class ButtonSpectrum
{
    private:
        unsigned char prevButtonStates[16];
        float prevAxisStates[16];

        std::vector<std::string> buttons;

    public:
        ButtonSpectrum();
        ~ButtonSpectrum();

        void clear();
        void addButton(int buttonCount, const unsigned char* buttonStates,
                       int axisCount, const float* axisStates);
        void newSection();
        void draw(float x, float y, const Font& font) const;
};

#endif /* _ButtonSpectrum_hpp_ */
