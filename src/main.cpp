#include "Stopwatch.hpp"

#include "Font.hpp"
#include "LineGraph.hpp"
#include "ButtonSpectrum.hpp"
#include "StringUtils.hpp"

#include <GLFW/glfw3.h>

#include <unistd.h>

#include <iostream>

#include <vector>
#include <functional>
#include <string>

void sendCommand(int fd, std::string command)
{
    command.push_back('\n');
    write(fd, command.c_str(), command.length());
}

std::string readCommandOutput(int fd)
{
    const int bufsize = 256;
    char buffer[bufsize];

    buffer[read(fd, buffer, bufsize)] = 0;

    return std::string(buffer);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: spectrum <pid> <address>" << std::endl;
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

    if (!fork()) // Child
    {
        const char *argv[] =
            {
                "scanmem",
                "-b",
                "-p",
                pid.c_str(),
                0
            };

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

        // // Read Relevant(hopefully) addresses.
        // FILE* addressFile = fopen("address.txt", "r");

        // fclose(addressFile);

        std::string scanMemVersion = readCommandOutput(infd[0]);
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
                                              "My Game Window",
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

            sendCommand(outfd[1], "dump " + address + " 2");

            // Big-endian hexadecimal string
            std::string hexStrRaw = readCommandOutput(infd[0]);
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

        glfwTerminate();
        return 0;
    }
}
