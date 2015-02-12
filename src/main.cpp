#include "Stopwatch.hpp"

#include "Font.hpp"
#include "LineGraph.hpp"
#include "JoystickInput.hpp"
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

#include <thread>
#include <chrono>

#include <stdexcept>

class Application
{
    private:
        GLFWwindow* window;

    public:
        Application()
        {
            initGLFW();
            initOpenGL();
        }

        ~Application()
        {
            glfwTerminate();
        }

        void clear()
        {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void pollEvents()
        {
            glfwPollEvents();
        }

        void swapBuffers()
        {
            glfwSwapBuffers(window);
        }

        bool shouldWindowClose()
        {
            return glfwWindowShouldClose(window);
        }

    private:
        void initGLFW()
        {
            if (!glfwInit())
            {
                throw std::runtime_error{"Error initializing glfw."};;
            }

            glfwWindowHint(GLFW_RESIZABLE, false);
            window = glfwCreateWindow(640, 480,
                                      "BlockTracker",
                                      nullptr, nullptr);

            glfwMakeContextCurrent(window);
        }

        void initOpenGL()
        {
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

        void setProcess(const std::string& pid)
        {
            sendCommand("pid " + pid);
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

    // TODO: validate command line arguments
    std::string pid{argv[1]}; // processid to scan.
    std::string address{argv[2]}; // specific memory address to scan.

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
        const char *argv[] { "/usr/bin/scanmem", "-b", nullptr };

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

        scanMem.setProcess(pid);
        std::string procMaps = scanMem.readCommandOutput();

        Application app;

        // Create Font
        Font font("DroidSansFallback.ttf");

        JoystickInput joystick(GLFW_JOYSTICK_1);

        // std::vector<ButtonDisplay> buttonMap;
        // buttonMap.push_back({ 0, "D", 140, 70 });
        // buttonMap.push_back({ 1, "A", 140, 50 });
        // buttonMap.push_back({ 2, "B", 160, 50 });
        // buttonMap.push_back({ 3, "C", 180, 50 });

        // std::vector<AxisDisplay> axisMap;
        // axisMap.push_back({ -7, "U", 100, 40 });
        // axisMap.push_back({  7, "D", 100, 80 });
        // axisMap.push_back({ -6, "L",  80, 60 });
        // axisMap.push_back({  6, "R", 120, 60 });

        Stopwatch timer;
        ButtonSpectrum spectrum;
        LineGraph graph;

        int level{};
        int prevLevel{};

        bool running = true;
        while (!app.shouldWindowClose() && running)
        {
            app.clear();

            joystick.updateButtons();

            spectrum.addButton(joystick);

            if (joystick.getButton(myButtons::RESET) == GLFW_PRESS)
            {
                // std::cout << "\n\n\n\n\n\n\n\n" << std::endl;
                
                // level = 0;
                prevLevel = 0;

                graph.clear();
                spectrum.clear();

                timer.start();
            }

            scanMem.dumpRegion(address, 2);

            // Big-endian hexadecimal string
            std::string hexStrRaw = scanMem.readCommandOutput();
            std::string hexStr = hexStrRaw.substr(3, 2) + hexStrRaw.substr(0, 2);
            level = std::stoi(hexStr, nullptr, 16);

            float gameTime = timer.getFloatTime() - 1.7f;
            font.draw(20, 20, strformat("time: %.2f", gameTime));

            // if (joystick.buttonChange(myButtons::D) &&
            //     joystick.getButton(myButtons::D) == GLFW_PRESS)
            //     std::cout << "D" << std::flush;
            // if (joystick.buttonChange(myButtons::A) &&
            //     joystick.getButton(myButtons::A) == GLFW_PRESS)
            //     std::cout << "A" << std::flush;
            // if (joystick.buttonChange(myButtons::B) &&
            //     joystick.getButton(myButtons::B) == GLFW_PRESS)
            //     std::cout << "B" << std::flush;
            // if (joystick.buttonChange(myButtons::C) &&
            //     joystick.getButton(myButtons::C) == GLFW_PRESS)
            //     std::cout << "C" << std::flush;

            // if (joystick.axisChange(myAxis::HORI) &&
            //     joystick.getAxis(myAxis::HORI) < -0.9f)
            //     std::cout << "←" << std::flush;
            // if (joystick.axisChange(myAxis::HORI) &&
            //     joystick.getAxis(myAxis::HORI) > 0.9f)
            //     std::cout << "→" << std::flush;
            // if (joystick.axisChange(myAxis::VERT) &&
            //     joystick.getAxis(myAxis::VERT) < -0.9f)
            //     std::cout << "↑" << std::flush;
            // if (joystick.axisChange(myAxis::VERT) &&
            //     joystick.getAxis(myAxis::VERT) > 0.9f)
            //     std::cout << "↓" << std::flush;

            // Level-up!
            if (level > prevLevel)
            {
                std::cout << std::endl;
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

            app.swapBuffers();
            app.pollEvents();
        }

        scanMem.exit();

        int childReturnStatus{};
        wait(&childReturnStatus);

        return 0;
    }
}
