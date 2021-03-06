#include "lms/imaging_detection/line.h"
#include "lms/imaging_detection/line_point.h"
#include <lms/imaging/graphics.h>
#include <cmath>
#include <lms/imaging/draw_debug.h>
#include <algorithm>
#include <iostream>
#include "lms/math/vertex.h"
#include "lms/imaging_detection/edge_point.h"
namespace lms{
namespace imaging{
namespace detection{

lms::math::vertex2i Line::getAveragePoint() const{
    if(points().size() == 0){
        return lms::math::vertex2i(0,0);
    }
    lms::math::vertex2i m(0,0);
    for(const LinePoint& lp : points()) {
         lms::math::vertex2i p(lp.getX(), lp.getY());
         m = m + p;
    }
    m /= points().size();
    return m;
}

bool Line::findPoint(LinePoint &pointToFind,LinePoint::LinePointParam linePointParam DRAWDEBUG_PARAM){
    //find first point
    //Draw red cross
    DRAWCROSS(linePointParam.x,linePointParam.y,255,0,0);
    bool found =  pointToFind.find(linePointParam DRAWDEBUG_ARG);
    if(found && m_LineParam.validPoint){
        found = m_LineParam.validPoint(pointToFind DRAWDEBUG_ARG);
        //TODO
        //float diffAngle = m_LineParam.searchAngle-;
    }
    return found;
}
bool Line::find(LineParam lineParam DRAWDEBUG_PARAM){
    setParam(lineParam);
    return find(DRAWDEBUG_ARG_N);
}

void Line::setParam(const LineParam &lineParam){
    m_LineParam = lineParam;
}
bool Line::find(DRAWDEBUG_PARAM_N){
    LinePoint lp;
    LinePoint::LinePointParam lParam = m_LineParam;
    lParam.sobelThreshold = 50;
    bool found = findPoint(lp,lParam DRAWDEBUG_ARG);
    m_points.clear();
    //std::cout<<"found::::: "<<found<<" "<<lParam.edge<<std::endl;

    if(!found){
        return false;
    }
    m_points.push_front(lp);
    //found receptor point -> try to extend the line
    extend(true DRAWDEBUG_ARG);
    extend(false DRAWDEBUG_ARG);
    return true;
}

bool Line::getSearchPoint(int &x, int &y, float &angle){
    float totalLength = length();
    float res = 0;
    if(points().size() == 1){
        x = points()[0].low_high.x;
        y = points()[0].low_high.y;
        angle = points()[0].param().searchAngle;
        return true;
    }
    for(int i = 0; i+1 < (int)points().size();i++){
        res += points()[i].low_high.distance(points()[i].low_high);
        if(res >= totalLength/2.0){
            x = points()[i].low_high.x;
            y = points()[i].low_high.y;
            angle = points()[i].param().searchAngle;
            return true;
        }
    }
    return false;
}


/**
  * TODO es kommt auf die suchrichtung an, was direction==true oder false tut, ab dem winkel > 90 dreht sich die suchrichtung im Bild um. Veranschaulicht wird das durch einen Kreis wobei man in den beiden oberen Quadranten sucht.
  */
void Line::extend(bool direction DRAWDEBUG){
    lms::math::vertex2i pixel;

    //std::cout<<"EXXXXXXXXXXXXTEND"<<std::endl;
    float searchStepX;
    float searchStepY;
    float currentLength = 0;
    float currentStepLength = m_LineParam.stepLengthMax;

    bool found = false;
    //search as long as the searchLength isn't reached
    while(currentLength < m_LineParam.maxLength){
        found = false;
        //Create new point using the last data
        LinePoint searchPoint;
        if(direction){
            searchPoint = m_points[m_points.size()-1];
        }else{
            searchPoint = m_points[0];
        }

        //get needed stuff
        //float lineWidth = searchPoint.distance();

        pixel.x = searchPoint.low_high.x;
        pixel.y = searchPoint.low_high.y;
        float oldSearchAngle = m_LineParam.searchAngle;
        float oldSearchAngleOrth;
        if(m_points.size() > 1){
            //get angle between last two points
            EdgePoint *top;
            EdgePoint *bot;
            if(!m_LineParam.fixedSearchAngle){
                if(direction){
                    top = &m_points[m_points.size()-1].low_high;
                    bot = &m_points[m_points.size()-2].low_high;
                }else{
                    top = &m_points[1].low_high;
                    bot = &m_points[0].low_high;
                }
                oldSearchAngle = atan2(top->y - bot->y,top->x-bot->x);
                oldSearchAngle -= M_PI_2;
            }
        }

        if(direction){
            oldSearchAngleOrth = oldSearchAngle+M_PI_2;
        }else{
            oldSearchAngleOrth = oldSearchAngle-M_PI_2;
        }
        //move the point along the tangent of the line and afterwards move it from the line so the point isn't already on the line
        searchStepX = cos(oldSearchAngleOrth)*currentStepLength-m_LineParam.lineWidthTransMultiplier*currentStepLength*cos(oldSearchAngle);
        searchStepY = sin(oldSearchAngleOrth)*currentStepLength-m_LineParam.lineWidthTransMultiplier*currentStepLength*sin(oldSearchAngle);
        //try to find a new point
        //calculate new searchPoint
        if(m_LineParam.target->inside(pixel.x+searchStepX,pixel.y+searchStepY)){
            //move pixel
            pixel += lms::math::vertex2i(searchStepX,searchStepY);
            //TODO that could be made more efficient
            LinePoint::LinePointParam param = m_LineParam;
            param.x = pixel.x;
            param.y = pixel.y;
            param.searchLength = 2*m_LineParam.lineWidthTransMultiplier*currentStepLength;
            param.searchAngle = oldSearchAngle;

            if(findPoint(searchPoint,param DRAWDEBUG_ARG)){
                found = true;
                if(direction){
                    currentLength += searchPoint.low_high.distance(m_points[m_points.size()-1].low_high);
                    m_points.push_back(searchPoint);
                }else{
                    currentLength += searchPoint.low_high.distance(m_points[0].low_high);
                    m_points.push_front(searchPoint);
                }


            }
        }
        //std::cout<<"FOUND "<<found<<" "<<currentLength<<std::endl;
        if(!found){
            //found no point, decrease length
            //TODO add some better algo.
            currentStepLength *= 0.5;
            if(currentStepLength < m_LineParam.stepLengthMin){
                //stop searching, no more points can be found on this line
                //TODO return
                //std::cout<<"BREAK "<<currentLength<<std::endl;
                break;
            }
        }
    }
    //std::cout<<"ENDE "<< currentLength<<" max: "<<m_LineParam.maxLength<<std::endl;
}
int Line::getType() const{
    return Line::TYPE;
}
} //namepsace find
} //namespace imaging
} //namespace lms
