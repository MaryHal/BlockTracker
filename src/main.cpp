#include "Stopwatch.hpp"

#include "Font.hpp"
#include "LineGraph.hpp"
#include "ButtonSpectrum.hpp"
#include "StringUtils.hpp"

#include <GLFW/glfw3.h>

#include <unistd.h>
#include <sys/wait.h>

#include <iostream>

#include <vector>
#include <string>

#include <mutex>
#include <memory>

class JoystickInput
{
    private:
        std::vector<unsigned char> prevButtons;
        std::vector<float> prevAxis;

        std::vector<unsigned char> buttons;
        std::vector<float> axis;

    public:
        JoystickInput()
            : prevButtons{},
              prevAxis{}
        {
        }

        void updateButtons(int joystickNum=GLFW_JOYSTICK_1)
        {
            prevButtons = std::move(buttons);
            prevAxis = std::move(axis);

            int buttonCount = 0;
            auto* buttonStates = glfwGetJoystickButtons(joystickNum, &buttonCount);

            for (int i = 0; i < buttonCount; ++i)
                buttons.push_back(buttonStates[i]);

            int axisCount = 0;
            auto* axisStates = glfwGetJoystickAxes(joystickNum, &axisCount);

            for (int i = 0; i < axisCount; ++i)
                axis.push_back(axisStates[i]);
        }

        unsigned char getButton(int buttonId)
        {
            return buttons[buttonId];
        }

        float getAxis(int axisId)
        {
            return axis[axisId];
        }

        bool buttonChange(int buttonId)
        {
            return buttons[buttonId] == prevButtons[buttonId];
        }

        bool axisChange(int axisId)
        {
            return axis[axisId] == prevAxis[axisId];
        }
};

class ScanMem
{
    private:
        int infd;
        int outfd;

        std::recursive_mutex scanLock;

    public:
        ScanMem(int in, int out)
            : infd{in}, outfd{out}
        {
        }

        void sendCommand(std::string command)
        {
            scanLock.lock();

            command.push_back('\n');
            write(outfd, command.c_str(), command.length());

            scanLock.unlock();
        }

        void dumpRegion(const std::string& address, int length)
        {
            std::string command { "dump " + address + " " + std::to_string(length) };
            sendCommand(command);
        }

        void exit()
        {
            sendCommand("exit");
        }

        std::string readCommandOutput()
        {
            scanLock.lock();

            const int bufsize = 256;
            char buffer[bufsize];

            buffer[read(infd, buffer, bufsize)] = 0;

            scanLock.unlock();

            return std::string(buffer);
        }
};

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: # blocktracker <pid> <address>" << std::endl;
        return -1;
    }

    std::string pid = argv[1];
    std::string address = argv[2];

    int outfd[2];
    int infd[2];

    int oldstdin, oldstdout;

    pipe(outfd); // Where the parent is going to write to
    pipe(infd); // From where parent is going to read

    oldstdin = dup(0); // Save current stdin
    oldstdout = dup(1); // Save stdout

    close(0);
    close(1);

    dup2(outfd[0], 0); // Make the read end of outfd pipe as stdin
    dup2(infd[1], 1); // Make the write end of infd as stdout

    int subprocessPid = fork();

    if (!subprocessPid) // Child
    {
        const char *argv[] { "/usr/bin/scanmem", "-b", "-p", pid.c_str(), nullptr };

        close(outfd[0]); // Not required for the child
        close(outfd[1]);
        close(infd[0]);
        close(infd[1]);

        execv(argv[0], const_cast<char**>(argv));
    }
    else // Parent
    {
        // Restore the original std fds of parent
        close(0);
        close(1);
        dup2(oldstdin, 0);
        dup2(oldstdout, 1);

        close(outfd[0]); // These are being used by the child
        close(infd[1]);

        ScanMem scanMem(infd[0], outfd[1]);

        // Scanmem immediately outputs its version number
        std::string scanMemVersion = scanMem.readCommandOutput();
        printf("Version: %s\n", scanMemVersion.c_str());

        // sendCommand(outfd[1], "help");
        // printf("Output:\n%s\n", readCommandOutput(infd[0]).c_str());

        if (!glfwInit())
        {
            std::cout << "Error initializing glfw." << std::endl;
            return 1;
        }

        glfwWindowHint(GLFW_RESIZABLE, false);
        GLFWwindow* window = glfwCreateWindow(640, 480,
                                              "BlockTracker",
                                              nullptr, nullptr);

        glfwMakeContextCurrent(window);

        // OpenGL 2d perspective
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0f, 640.0f, 480.0f, 0.0f, -1.0f, 1.0f);

        // Initialize modelview matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glEnable(GL_LINE_SMOOTH);
        glLineWidth(2.0f);

        Stopwatch timer;

        // Create Font
        Font font("DroidSansFallback.ttf");

        if (!glfwJoystickPresent(GLFW_JOYSTICK_1))
        {
            std::cout << "Joystick 1 not found." << std::endl;
            return -1;
        }

        struct ButtonDisplay
        {
            public:
                int id;
                std::string button;
                int xPos, yPos;

                bool test(const unsigned char* buttonStates)
                {
                    return buttonStates[id] == GLFW_PRESS;
                }
        };

        struct AxisDisplay
        {
            public:
                int id;
                std::string button;
                int xPos, yPos;

                bool test(const float* axisStates)
                {
                    if (id < 0)
                    {
                        int index = -id;
                        return axisStates[index] < -0.9f;
                    }
                    else
                    {
                        int index = id;
                        return axisStates[index] > 0.9f;
                    }
                }
        };

        std::vector<ButtonDisplay> buttonMap;
        buttonMap.push_back({ 0, "D", 140, 70 });
        buttonMap.push_back({ 1, "A", 140, 50 });
        buttonMap.push_back({ 2, "B", 160, 50 });
        buttonMap.push_back({ 3, "C", 180, 50 });

        std::vector<AxisDisplay> axisMap;
        axisMap.push_back({ -7, "U", 100, 40 });
        axisMap.push_back({  7, "D", 100, 80 });
        axisMap.push_back({ -6, "L",  80, 60 });
        axisMap.push_back({  6, "R", 120, 60 });

        ButtonSpectrum spectrum;
        LineGraph graph;

        int level = 0;
        int prevLevel = 0;

        bool running = true;
        while (!glfwWindowShouldClose(window) && running)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glColor4f(0.8f, 0.8f, 0.8f, 1.0f);

            int buttonCount = 0;
            const unsigned char* buttonStates = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

            for (ButtonDisplay& b : buttonMap)
            {
                if (b.test(buttonStates))
                {
                    font.draw(b.xPos, b.yPos, b.button);
                }
            }

            int axisCount = 0;
            const float* axisStates = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axisCount);

            for (AxisDisplay& b : axisMap)
            {
                if (b.test(axisStates))
                {
                    font.draw(b.xPos, b.yPos, b.button);
                }
            }

            spectrum.addButton(buttonCount, buttonStates, axisCount, axisStates);

            if (buttonStates[5] == GLFW_PRESS)
            {
                level = 0;
                prevLevel = 0;

                graph.clear();
                spectrum.clear();

                timer.start();
            }

            scanMem.dumpRegion(address, 2);
            // scanMem.sendCommand("dump " + address + " 2");

            // Big-endian hexadecimal string
            std::string hexStrRaw = scanMem.readCommandOutput();
            std::string hexStr = hexStrRaw.substr(3, 2) + hexStrRaw.substr(0, 2);
            level = std::stoi(hexStr, nullptr, 16);

            float gameTime = timer.getFloatTime() - 1.7f;
            font.draw(20, 20, strformat("time: %.2f", gameTime));
            // font.draw(20, 40, strformat("level: %d", level));

            // Level-up!
            if (level > prevLevel)
            {
                prevLevel = level;
                graph.addPoint(level, gameTime);
                spectrum.newSection();
            }

            // We died.
            if (prevLevel > level && level == 0)
            {
                timer.stop();
            }

            graph.draw(80.0f, 80.0f, font);
            spectrum.draw(400.0f, 10.0f, font);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        scanMem.exit();

        int childReturnStatus{};
        wait(&childReturnStatus);

        glfwTerminate();
        return 0;
    }
}
