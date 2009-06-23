
#ifndef EDGE_ACTOR_H
#define EDGE_ACTOR_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtGame/gameactor.h>
#include <osg/Geode>
#include <osg/Vec3>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class EdgeActor : public dtGame::GameActor
{
    public:
        EdgeActor(dtGame::GameActorProxy& gameActorProxy);

        virtual void TickRemote(const dtGame::Message &tickMessage);
        virtual void TickLocal(const dtGame::Message &tickMessage);
        void DrawEdge();

        inline osg::Vec3 GetHeadPos() { return headPos; }
        inline osg::Vec3 GetTailPos() { return tailPos; }
        inline void SetHeadPos(const osg::Vec3& newHeadPos) { headPos = newHeadPos; DrawEdge(); }
        inline void SetTailPos(const osg::Vec3& newTailPos) { tailPos = newTailPos; DrawEdge(); }

    protected:
        virtual ~EdgeActor();

    private:
        osg::Geode *geode;
        osg::Vec3 headPos;
        osg::Vec3 tailPos;
        DECLARE_LOGGER();
};

#endif // EDGE_ACTOR_H