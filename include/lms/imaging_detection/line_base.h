#ifndef IMAGE_LINE_BASE_H
#define IMAGE_LINE_BASE_H

#include <lms/imaging_detection/line_point.h>
#include <deque>
#include <lms/imaging/draw_debug.h>
#include "image_object.h"
#include <functional>
namespace lms{
namespace imaging{
namespace detection{

class LineBase: public ImageObject{
public:

    typedef float parameterType;
    virtual bool find(DRAWDEBUG_PARAM_N) = 0;
    virtual bool findPoint(LinePoint &pointToFind,LinePoint::LinePointParam linePointParam DRAWDEBUG_PARAM) = 0;

    float length();
    const std::deque<LinePoint> &points() const;
    std::deque<LinePoint> &points();
protected:
    std::deque<LinePoint> m_points;
};

} //namepsace find
} //namespace imaging
} //namespace lms
#endif // IMAGE_LINE
