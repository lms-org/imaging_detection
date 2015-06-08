#ifndef IMAGE_EDGE_POINT_H

#define IMAGE_EDGE_POINT_H

#include <math.h>
#include <lms/imaging/draw_debug.h>
#include "lms/deprecated.h"
#include "lms/math/vertex.h"
#include "lms/type/module_config.h"
namespace lms{
namespace imaging{
namespace find{
class EdgePoint: public lms::math::vertex2f{

public:
    enum class EdgeType {LOW_HIGH, HIGH_LOW, PLANE};

    struct EdgePointParam{
        EdgePointParam():x(0),y(0),target(nullptr),searchLength(0),searchAngle(0),searchType(EdgeType::PLANE),sobelThreshold(0),gaussBuffer(nullptr),verify(false),preferVerify(false){
        }

        virtual void fromConfig(const lms::type::ModuleConfig *config){
            searchAngle = config->get<float>("searchAngle",0);
            searchLength = config->get<float>("searchLength",30);
            x = config->get<float>("x",180);
            y = config->get<float>("y",120);
            sobelThreshold = config->get<float>("sobelThreshold",150);
            verify = config->get<bool>("verify",true);
            preferVerify = config->get<bool>("preferVerify",false);
            //TODO searchType = config->get<EdgeType>("searchType",EdgeType::PLANE);
        }

        int x;
        int y;
        const Image *target;
        float searchLength;
        float searchAngle;
        EdgeType searchType;
        int sobelThreshold;
        Image *gaussBuffer;
        /**
         * @brief verify if true find will try to find it with the old values if the new one don't find anything
         */
        bool verify;
        bool preferVerify;
    };

    typedef EdgePointParam parameterType;

private:
    EdgePointParam m_searchParam;
    int m_sobelX;
    int m_sobelY;
    float m_sobelNormal;
    float m_sobelTangent;
    EdgeType m_type;

    /**
     * @brief setType "calculates" the type high_low/low_high edge
     * @return the found type
     */
    EdgePoint::EdgeType setType();

    public:
        void setSearchParam(const EdgePointParam &searchParam);
        bool find(DRAWDEBUG_PARAM_N);
        bool find(const EdgePointParam &searchParam DRAWDEBUG_PARAM);
        int sobelX();
        int sobelY();
        /**
         * @brief sobelAngle
         * @return the angle from -PI to PI
         */
        float sobelTangent();
        float sobelNormal();
        EdgeType type();

};

} //namepsace find
} //namespace imaging
} //namespace lms

#endif // IMAGE_EDGE_POINT
