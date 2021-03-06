#include "lms/imaging_detection/street_crossing.h"
#include "lms/imaging_detection/line_point.h"
#include "lms/imaging_detection/line.h"
#include "lms/math/vertex.h"
#include "lms/imaging_detection/street_utils.h"

#include <random>
#include <algorithm>
#include <iterator>

namespace lms {
namespace imaging {
namespace detection {

bool StreetCrossing::find(StreetCrossing::StreetCrossingParam param DRAWDEBUG_PARAM){
    searchParam = param;
    searchParam.fixedSearchAngle = true;
    return find(DRAWDEBUG_ARG_N);
}

bool StreetCrossing::find(DRAWDEBUG_PARAM_N){
    foundStartLine = false;
    foundCrossing = false;
    oppositeStopLineFound = false;
    using lms::math::vertex2f;
    using lms::math::vertex2i;

    float laneWidth = 0.4;//width of one
    float tangentStartLineOffset = 0.1;
    float tangentRightCrossingLineStart = 0.15;
    float tangentRightCrossingLineEnd = 0.15;
    //try to find Stop-line
    LinePoint::LinePointParam lpp;
    Line::LineParam linePar;
    linePar = searchParam;
    // ToDo find valid settings
    lpp = searchParam;

    //Not smart at all but it may work :)
    for(int i = 1; i < (int)searchParam.middleLine.points().size(); i++) {
        vertex2f top = searchParam.middleLine.points()[i];
        vertex2f bot = searchParam.middleLine.points()[i-1];
        vertex2f tangentDir = top-bot;
        tangentDir = tangentDir.normalize();
        //create hint
        vertex2f norm = tangentDir.rotateClockwise90deg();

        vertex2f targetBot = bot+norm*laneWidth/2;
        vertex2f targetTop = top+norm*laneWidth/2;

        if(!vecToLineParam(targetBot, targetTop, top, linePar)){
            continue;
        }
        if(stopLine.find(linePar DRAWDEBUG_ARG)) {
            if(stopLine.points().size() < 3)
                break;

            vertex2f foundStopLine;
            vertex2i tmp;
            //trying to get one straight line
            double m,b;
            if(lineFitRansac(m, b)){
                tmp.x = stopLine.points().at(0).getX() +
                        0.5*(stopLine.points()[stopLine.points().size()-1].getX() - stopLine.points().at(0).getX());
                tmp.y = m*tmp.x + b;
            }else{
                tmp=stopLine.getAveragePoint();
            }

            lms::imaging::C2V(&tmp,&foundStopLine);

            //check if it's not a start line
            targetBot = foundStopLine-tangentDir*tangentStartLineOffset-norm*laneWidth;
            targetTop = foundStopLine+tangentDir*2*tangentStartLineOffset-norm*laneWidth;
            if(!vecToLineParam(targetBot,targetTop,top,linePar)){
                continue;
            }

            // search start line
            if(!leftPartStartLine.find(linePar DRAWDEBUG_ARG)){
                //it's not a Start-Line
                //get the opposite stop lane
                targetBot = foundStopLine + tangentDir*laneWidth - norm*laneWidth;
                targetTop = targetBot + tangentDir *laneWidth;
                vecToLineParam(targetBot,targetTop, top, linePar);
                linePar.lineWidthTransMultiplier = 1.5; //TODO warum braucht man das hier? #HACK
                oppositeStopLineFound = oppositeStopLine.find(linePar DRAWDEBUG_ARG);

                //get the crossing right lane
                targetBot = foundStopLine-tangentDir*tangentRightCrossingLineStart+norm*laneWidth;
                targetTop = foundStopLine+tangentDir*tangentRightCrossingLineEnd+norm*laneWidth;
                if(!vecToLinePointParam(targetBot,targetTop,lpp)){
                    continue;
                }
                bool rightCrossingLineFound = rightCrossingLine.find(lpp DRAWDEBUG_ARG);

                if(rightCrossingLineFound){
                    //store Position
                    m_x = tmp.x;
                    m_y = tmp.y;
                    //we found the crossing <3
                    foundCrossing = true;

                    //check if the crossing is blocked
                    /*
                    targetBot = foundStopLine+tangentDir*0.5;
                    targetTop = foundStopLine+tangentDir*0.5+norm*streetWidth;
                    vecToLinePointParam(targetBot,targetTop,lpp);
                    */
                    //TODO Write street_obstacle class to find obstacles along a given path
                    targetBot = foundStopLine-norm*laneWidth+tangentDir*(laneWidth*1.5+searchParam.obstacleRightOffset);
                    targetTop = targetBot + norm*laneWidth*3.0;
                    if(!vecToLinePointParam(targetBot,targetTop,lpp)){
                        continue;
                    }
                    Line::LineParam obstacleLineParam = searchParam;
                    obstacleLineParam.sobelThreshold = searchParam.obstacleSobelThreshold;
                    obstacleLineParam.x = lpp.x;
                    obstacleLineParam.y = lpp.y;
                    obstacleLineParam.searchAngle = lpp.searchAngle;
                    obstacleLineParam.searchLength = lpp.searchLength;
                    obstacleLineParam.lineWidthMax = searchParam.boxDepthSearchLength;
                    blocked = isBlocked(obstacleLineParam DRAWDEBUG_ARG);

                    if(!blocked){
                        //search on the left side
                        targetBot = foundStopLine+norm*laneWidth+tangentDir*(laneWidth*0.5+searchParam.obstacleLeftOffset);
                        targetTop = targetBot - norm*laneWidth*2.5; //TODO der punkt liegt leicht außerhalb des bildes
                        if(!vecToLinePointParam(targetBot,targetTop,lpp)){
                            continue;
                        }
                        obstacleLineParam.sobelThreshold = searchParam.obstacleSobelThreshold;
                        obstacleLineParam.x = lpp.x;
                        obstacleLineParam.y = lpp.y;
                        obstacleLineParam.searchAngle = lpp.searchAngle;
                        obstacleLineParam.searchLength = lpp.searchLength;
                        obstacleLineParam.lineWidthMax = searchParam.boxDepthSearchLength;
                        blocked = isBlocked(obstacleLineParam DRAWDEBUG_ARG);
                    }
                    return true;
                }

            }else{
                //TODO we found a Start-line!
                tmp = stopLine.getAveragePoint()+leftPartStartLine.getAveragePoint();
                tmp /= 2;
                m_x = tmp.x;
                m_y = tmp.y;
                foundStartLine = true;
                return false;
            }
        }

    }
    return false;
}

bool StreetCrossing::isBlocked(Line::LineParam lParam DRAWDEBUG_PARAM){
    lParam.edge = true;
    lParam.fixedSearchAngle = true;
    lParam.validPoint = [lParam,this](lms::imaging::detection::LinePoint &lp DRAWDEBUG_PARAM)->bool{
        lms::imaging::detection::EdgePoint check = lp.low_high;
        check.searchParam().x = check.x;
        check.searchParam().y = check.y;
        check.searchParam().searchLength = searchParam.boxDepthSearchLength;
        check.searchParam().searchType = lms::imaging::detection::EdgePoint::EdgeType::HIGH_LOW;
        bool found = check.find(DRAWDEBUG_ARG_N);
        //std::cout<<"VALIDATE OBSTACLE IN CROSSING: "<<!found;

        return !found;
    };
    Line obst;
    if(obst.find(lParam DRAWDEBUG_ARG)){
        //std::cout<<"POINTS FOUND: "<<obst.points().size()<<std::endl;
        return (int)obst.points().size() >= searchParam.boxPointsNeeded;//TODO Not that smart
    }
    return false;
}


int StreetCrossing::getType() const{
    return StreetCrossing::TYPE;
}

bool StreetCrossing::lineFitRansac(double& m, double& b)
{
    if(stopLine.points().size() <= 2)
        return false;

    int bestInliers = 0;
    std::deque<LinePoint> inlierPoints;
    std::deque<LinePoint> bestInlierPoints;

    for(int i=0; i<searchParam.maxIterationsRANSAC; i++)
    {

        int inliers = 0;
        int idx1 = 0;
        int idx2 = 0;

        inlierPoints.clear();
        if(getRandomSample(idx1, idx2))
        {

            LinePoint p1 = stopLine.points().at(idx1);
            LinePoint p2 = stopLine.points().at(idx2);

            double aTemp = p1.getY() - p2.getY();
            double bTemp = p2.getX() - p1.getX();
            double cTemp = p1.getX()*p2.getY() - p2.getX()*p1.getY();

            for(const LinePoint& p : stopLine.points())
            {
                if(computeDistance(aTemp, bTemp, cTemp, p) < searchParam.inlierThresholdRANSAC)
                {
                    inliers++;
                    inlierPoints.push_back(p);
                }
            }

            if(inliers > bestInliers)
            {
                m = -aTemp/bTemp;
                b = -cTemp/bTemp;
                bestInliers = inliers;
                bestInlierPoints = inlierPoints;

            }

        }
        else
        {
            m = 0;
            b = 0;
            return false;
        }
    }

    stopLine.points() = bestInlierPoints;
    return true;

}

bool StreetCrossing::getRandomSample(int& idx1, int& idx2)
{
    int loopCnt = 10;
    idx1 = 0;
    idx2 = 0;

    while(idx1 == idx2)
    {
        idx1 = std::rand() % (int)(stopLine.points().size() );
        idx2 = std::rand() % (int)(stopLine.points().size() );

        loopCnt--;

        if(loopCnt < 0)
        {
            return false;
        }
    }
    return true;
}

double StreetCrossing::computeDistance(const double& a, const double& b, const double& c, const LinePoint& p)
{
    return fabs(a*static_cast<double>(p.getX()) + b*static_cast<double>(p.getY()) + c)/sqrt(pow(a,2) + pow(b,2));
}

} //namepsace find
} //namespace imaging
} //namespace lms
