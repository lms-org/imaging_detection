#ifndef IMAGING_DETECTION_STREET_UTILS_H
#define IMAGING_DETECTION_STREET_UTILS_H

#include "line_point.h"
#include "line.h"
#include "lms/imaging/warp.h"
namespace lms {
namespace imaging {
namespace detection {
/**
 * @brief vecToLinePointParam
 * @param start
 * @param end
 * @param lpp sets the searchANgle and the searchLength of the given LinePointParam
 */
inline bool vecToLinePointParam(const lms::math::vertex2f &start,const lms::math::vertex2f &end, LinePoint::LinePointParam &lpp){

    lms::math::vertex2i targetBotImage;
    lms::math::vertex2i targetTopImage;

    if(!lms::imaging::V2C(&start,&targetBotImage)){
        return false;
    }
    if(!lms::imaging::V2C(&end,&targetTopImage)){
        return false;
    }
    //create hint
    float imageSearchDistance = (targetTopImage-targetBotImage).length();
    float searchAngle = (targetTopImage-targetBotImage).angle();
    lpp.x = targetBotImage.x;
    lpp.y = targetBotImage.y;
    lpp.searchAngle = searchAngle;
    lpp.searchLength = imageSearchDistance;
    return true;
}

/**
 * @brief vecToLineParam
 * @param start
 * @param end
 * @param endMiddle
 * @param lp sets the searchANgle, searchLength and maxLength of the given LineParam
 */
inline bool vecToLineParam(const lms::math::vertex2f &start,const lms::math::vertex2f &end, const lms::math::vertex2f& endMiddle,
                           Line::LineParam &lp){

    lms::math::vertex2i targetBotImage;
    lms::math::vertex2i targetTopImage;
    lms::math::vertex2i targetEndMiddle;

    if(!lms::imaging::V2C(&start,&targetBotImage)){
        return false;
    }
    if(!lms::imaging::V2C(&end,&targetTopImage)){
         return false;
     }
    if(!lms::imaging::V2C(&endMiddle, &targetEndMiddle)){
            return false;
    }
    //create hint
    float imageSearchDistance = (targetTopImage-targetBotImage).length();
    float searchAngle = (targetTopImage-targetBotImage).angle();
    lp.x = targetBotImage.x;
    lp.y = targetBotImage.y;
    lp.searchAngle = searchAngle;
    lp.searchLength = imageSearchDistance;
    lp.maxLength = targetTopImage.distance(targetEndMiddle);
    return true;
}
}
}
}

#endif //IMAGING_DETECTION_STREET_UTILS_H
