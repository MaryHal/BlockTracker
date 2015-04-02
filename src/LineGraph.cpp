#include "LineGraph.hpp"

#include <GL/glew.h>

#include <fontgen/OpenGLFont.hpp>

const std::vector<float> LineGraph::SCALES{30.0f, 45.0f, 60.0f, 75.0f};

LineGraph::LineGraph(float width, float height)
    : graphWidth{width},
      graphHeight{height},
      scaleIndex{0}
{
    data.emplace_back();
}

LineGraph::~LineGraph()
{
}

void LineGraph::clear()
{
    data.clear();
    data.emplace_back();
}

void LineGraph::addPoint(int level, float time)
{
    // Create a new section
    if (!data.back().empty() &&
        data.back().back().level / 100 < level / 100)
    {
        data.back().emplace_back(100, time);
        data.emplace_back();
    }

    data.back().emplace_back(level, time);
}

void LineGraph::cycleXScale()
{
    scaleIndex++;
    if (scaleIndex >= SCALES.size())
        scaleIndex = 0;
}

void LineGraph::draw(float x, float y, const fgen::OpenGLFont& font) const
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    {
        // Gridlines
        glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
        for (int i = 0; i < 10; ++i)
        {
            // Vertical lines
            glBegin(GL_LINES);
            {
                glVertex2f(graphWidth * (i + 1) / 10.0f, graphHeight);
                glVertex2f(graphWidth * (i + 1) / 10.0f, 0.0f);
            }
            glEnd();

            // Horizontal lines
            glBegin(GL_LINES);
            {
                glVertex2f(0.0f, graphHeight * i / 10.0f);
                glVertex2f(graphWidth, graphHeight * i / 10.0f);
            }
            glEnd();
        }

        float x{0.0f};
        float y{0.0f};

        float sectionAlpha{1.0f};

        float colors[4][3] {
            { 0.8f, 0.8f, 0.8f }, // Nothing / Single
            { 0.3f, 0.0f, 0.8f }, // Double
            { 0.5f, 0.2f, 0.9f }, // Triple
            { 1.0f, 1.0f, 0.0f }  // Tetris
        };

        int prevLevel{0};

        // Section Lines
        for (auto sectionData = data.rbegin(); sectionData != data.rend(); ++sectionData)
        {
            glColor4f(0.8f, 0.0f, 0.8f, sectionAlpha);
            glBegin(GL_LINE_STRIP);
            {
                for (DataPoint d : *sectionData)
                {
                    x = graphWidth * ((d.time - sectionData->front().time) / SCALES[scaleIndex]);
                    y = graphHeight - graphHeight * (d.level % 100) / 100;

                    // Oh God
                    if (sectionData->size() > 5 && d.level == 100)
                        y = 0.0f;

                    int levelDiff{d.level - prevLevel};
                    if (levelDiff == 1)
                        glColor4f(colors[0][0], colors[0][1], colors[0][2], sectionAlpha);
                    else if (levelDiff == 2)
                        glColor4f(colors[1][0], colors[1][1], colors[1][2], sectionAlpha);
                    else if (levelDiff == 3)
                        glColor4f(colors[2][0], colors[2][1], colors[2][2], sectionAlpha);
                    else if (levelDiff >= 4)
                        glColor4f(colors[3][0], colors[3][1], colors[3][2], sectionAlpha);
                    prevLevel = d.level;

                    glVertex2f(x, y);
                }
            }
            glEnd();

            sectionAlpha /= 4.0f;
        }

        // Draw current level.
        if (!data.empty() && !data.back().empty())
        {
            DataPoint last = data.back().back();
            x = graphWidth * ((last.time - data.back().front().time) / SCALES[scaleIndex]);
            y = graphHeight - graphHeight * (last.level % 100) / 100;

            glColor4f(0.8f, 0.8f, 0.8f, 0.8f);
            if (!data.back().empty())
                font.draw(x, y, std::to_wstring(data.back().back().level));
        }

        // Draw axis
        glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
        glBegin(GL_LINES);
        {
            glVertex2f(0.0f, 0.0f);
            glVertex2f(0.0f, graphHeight);
        }
        glEnd();

        glBegin(GL_LINES);
        {
            glVertex2f(0.0f, graphHeight);
            glVertex2f(graphWidth, graphHeight);
        }
        glEnd();

        font.draw(graphWidth - 18.0f, graphHeight + 16.0f, std::to_wstring(SCALES[scaleIndex]));

        font.draw(graphWidth / 2.0f - 50.0f, graphHeight + 16.0f, L"Section Time");

        glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
        font.draw(20.0f, 20.0f, L"Level");
    }

    glPopMatrix();
}
