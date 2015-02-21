#include <fontgen/OpenGLFont.hpp>
#include <GLFW/glfw3.h>

#include "Stopwatch.hpp"

#include "LineGraph.hpp"
#include "JoystickInput.hpp"
#include "ButtonSpectrum.hpp"
#include "StringUtils.hpp"

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

class Window
{
    private:
        const std::string title;
        const unsigned int width;
        const unsigned int height;
        GLFWwindow* window;

    public:
        Window(const std::string& windowTitle,
               unsigned int windowWidth,
               unsigned int windowHeight,
               Window* share)
            : title{windowTitle},
              width{windowWidth},
              height{windowHeight}
        {
            createWindowContext(share);
            initOpenGL();
        }

        ~Window()
        {
            glfwDestroyWindow(window);
        }

        Window(const Window& other) = delete;
        Window& operator=(const Window& other) = delete;

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

        void makeContextCurrent()
        {
            glfwMakeContextCurrent(window);
        }

        GLFWwindow* getGLFWwindow()
        {
            return window;
        }

    private:
        void createWindowContext(Window* share)
        {
            glfwWindowHint(GLFW_RESIZABLE, false);
            glfwWindowHint(GLFW_SAMPLES, 4);

            window = glfwCreateWindow(width, height,
                                      title.c_str(),
                                      nullptr, share == nullptr ? nullptr : share->getGLFWwindow());

            glfwMakeContextCurrent(window);
        }

        void initOpenGL()
        {
            // OpenGL 2d perspective
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

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

class ScanMemController
{
    private:
        int infd;
        int outfd;

        std::recursive_mutex scanLock;

    public:
        ScanMemController(int in, int out)
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
            std::string command{ "dump " + address + " " + std::to_string(length) };
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

        ScanMemController scanMem(infd[0], outfd[1]);

        // Scanmem immediately outputs its version number
        std::string scanMemVersion = scanMem.readCommandOutput();

        scanMem.setProcess(pid);
        std::string procMaps = scanMem.readCommandOutput();

        if (!glfwInit())
        {
            throw std::runtime_error{"Error initializing glfw."};;
        }

        Window graphWindow{"Blocktracker Graph", 640, 480, nullptr};
        Window spectrumWindow{"Blocktracker Spectrum", 120, 300, &graphWindow};

        // Create Font
        fgen::OpenGLFont font{"DroidSansFallback.ttf",
            {
                {20.0f, fgen::charset::ascii},
                {20.0f, { 0x21E6, 0x21E9 } }
            }
        };

        JoystickInput joystick(GLFW_JOYSTICK_1);

        // struct Button
        // {
        //     public:
        //         std::string buttonStr;
        //         int buttonOrAxis;
        //         int state;
        //         int x;
        //         int y;
        //  };

        // auto checkButton = [](const JoystickInput& joystick, const Button& b)
        // {
        //     return joystick.getButton(b.buttonOrAxis) == b.state;
        // };

        // auto checkAxis = [](const JoystickInput& joystick, const Button& b)
        // {
        //     return joystick.getAxis(b.buttonOrAxis) <= b.state;
        // };

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
        while (!graphWindow.shouldWindowClose() && running)
        {
            graphWindow.makeContextCurrent();
            graphWindow.clear();

            spectrumWindow.makeContextCurrent();
            spectrumWindow.clear();

            joystick.updateButtons();

            spectrum.addButton(joystick);

            if (joystick.getButton(myButtons::RESET) == GLFW_PRESS)
            {
                // level = 0;
                prevLevel = 0;

                graph.clear();
                spectrum.clear();

                timer.start();
            }

            // Parse scanmem output for level string
            scanMem.dumpRegion(address, 2);

            // Big-endian hexadecimal string
            std::string hexStrRaw = scanMem.readCommandOutput();
            std::string hexStr = hexStrRaw.substr(3, 2) + hexStrRaw.substr(0, 2);
            level = std::stoi(hexStr, nullptr, 16);

            graphWindow.makeContextCurrent();

            float gameTime{timer.getFloatTime() - 1.7f};
            font.draw(20, 20, std::to_wstring(gameTime));

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

            spectrumWindow.makeContextCurrent();
            spectrum.draw(10.0f, 10.0f, font);

            graphWindow.swapBuffers();
            spectrumWindow.swapBuffers();

            graphWindow.pollEvents();
            spectrumWindow.pollEvents();
        }

        glfwTerminate();

        scanMem.exit();

        int childReturnStatus{};
        wait(&childReturnStatus);

        return 0;
    }
}
