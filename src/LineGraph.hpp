#ifndef _LineGraph_hpp_
#define _LineGraph_hpp_

#include <vector>

namespace fgen { class OpenGLFont; }

class LineGraph
{
    public:
        struct DataPoint
        {
            public:
                int level;
                float time;

                DataPoint(int level, float time)
                    : level(level), time(time)
                {
                }
        };

    private:
        std::vector<std::vector<DataPoint> > data;
        float graphWidth;
        float graphHeight;

        unsigned int scaleIndex;
        static const std::vector<float> SCALES;

    public:
        LineGraph(float width=300.0f, float height=300.0f);
        ~LineGraph();

        void clear();
        void addPoint(int level, float time);
        void cycleXScale();
        void draw(float x, float y, const fgen::OpenGLFont& font) const;
};

#endif /* _LineGraph_hpp_ */
