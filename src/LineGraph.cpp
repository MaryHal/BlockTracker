#include "LineGraph.hpp"

#include <GL/glew.h>

#include <fontgen/OpenGLFont.hpp>

LineGraph::LineGraph(float width, float height)
    : graphWidth{width},
      graphHeight{height},
      xScale{30.0f}
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

void LineGraph::toggleXScale()
{
    // Is this the worst thing ever?
    if (xScale > 29.0f && xScale < 31.0f)
        xScale = 60.0f;
    else if (xScale > 59.0f && xScale < 61.0f)
        xScale = 30.0f;
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

        float sectionAlpha = 1.0f;

        // Section Lines
        for (auto sectionData = data.rbegin(); sectionData != data.rend(); ++sectionData)
        {
            glColor4f(0.8f, 0.0f, 0.8f, sectionAlpha);
            glBegin(GL_LINE_STRIP);
            {
                for (DataPoint d : *sectionData)
                {
                    x = graphWidth * ((d.time - sectionData->front().time) / xScale);
                    y = graphHeight - graphHeight * (d.level % 100) / 100;

                    // Oh God
                    if (sectionData->size() > 5 && d.level == 100)
                        y = 0.0f;

                    glVertex2f(x, y);
                }
            }
            glEnd();

            sectionAlpha /= 3.0f;
        }

        if (!data.empty() && !data.back().empty())
        {
            DataPoint last = data.back().back();
            x = graphWidth * ((last.time - data.back().front().time) / xScale);
            y = graphHeight - graphHeight * (last.level % 100) / 100;

            glColor4f(0.8f, 0.0f, 0.8f, 1.0f);
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

        font.draw(graphWidth - 18.0f, graphHeight + 16.0f, std::to_wstring(xScale));

        font.draw(graphWidth / 2.0f - 50.0f, graphHeight + 16.0f, L"Section Time");

        glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
        font.draw(20.0f, 20.0f, L"Level");
    }

    glPopMatrix();
}
