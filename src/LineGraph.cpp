#include "LineGraph.hpp"

#include <GL/glew.h>

#include "Font.hpp"
// #include "StringUtils.hpp"

LineGraph::LineGraph()
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
        data.emplace_back();
    }

    data.back().emplace_back(level, time);
}

void LineGraph::draw(float x, float y, const Font& font) const
{
    const float graphWidth = 300.0f;
    const float graphHeight = 300.0f;

    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    // Draw graph line
    glColor4f(0.8f, 0.0f, 0.8f, 1.0f);
    {
        float x = 0.0f;
        float y = 0.0f;

        glBegin(GL_LINE_STRIP);
        {
            for (DataPoint d : data.back())
            {
                x = graphWidth * ((d.time - data.back().front().time) / 60.0f);
                y = graphHeight - graphHeight * (d.level % 100) / 100;

                glVertex2f(x, y);
            }
        }
        glEnd();

        if (!data.back().empty())
            font.draw(x, y, std::to_string(data.back().back().level));

        // glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
        // glBegin(GL_LINES);
        // {
        //     glVertex2f(x, y);
        //     glVertex2f(x, graphHeight + 20.0f);
        // }
        // glEnd();

        // if (!data.back().empty())
        // {
        //     DataPoint d = data.back().back();
        //     font.draw(x, y, std::to_string(d.level));
        //     font.draw(x, graphHeight + 20.0f, std::to_string(d.time));
        // }
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

    glPopMatrix();
}
