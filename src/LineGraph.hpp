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

    public:
        LineGraph();
        ~LineGraph();

        void clear();
        void addPoint(int level, float time);
        void draw(float x, float y, const fgen::OpenGLFont& font) const;
};

#endif /* _LineGraph_hpp_ */
