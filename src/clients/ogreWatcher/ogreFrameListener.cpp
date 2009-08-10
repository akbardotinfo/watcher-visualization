/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ogreFrameListener.h"

using namespace Ogre;

namespace ogreWatcher
{
    INIT_LOGGER(OgreFrameListener, "OgreFrameListener");

    void OgreFrameListener::updateStats(void)
    {
        TRACE_ENTER();
        static String currFps = "Current FPS: ";
        static String avgFps = "Average FPS: ";
        static String bestFps = "Best FPS: ";
        static String worstFps = "Worst FPS: ";
        static String tris = "Triangle Count: ";
        static String batches = "Batch Count: ";

        // update stats when necessary
        try {
            OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
            OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
            OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
            OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

            const RenderTarget::FrameStats& stats = mWindow->getStatistics();
            guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
            guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
            guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS) +" "+StringConverter::toString(stats.bestFrameTime)+" ms");
            guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS) +" "+StringConverter::toString(stats.worstFrameTime)+" ms");

            OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
            guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

            OverlayElement* guiBatches = OverlayManager::getSingleton().getOverlayElement("Core/NumBatches");
            guiBatches->setCaption(batches + StringConverter::toString(stats.batchCount));

            OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
            guiDbg->setCaption(mDebugText);
        }
        catch(...) { 
            LOG_ERROR("Caught ... excpetion in updateStats()");
        }
        TRACE_EXIT();
    }

    OgreFrameListener::OgreFrameListener(RenderWindow* win, Camera* cam, SceneManager *sceneManager, bool bufferedKeys, bool bufferedMouse, bool bufferedJoy) :
        mCamera(cam), mTranslateVector(Vector3::ZERO), mWindow(win), mSceneMgr(sceneManager), mStatsOn(true), mNumScreenShots(0),
        mMoveScale(0.0f), mRotScale(0.0f), mTimeUntilNextToggle(0), mFiltering(TFO_BILINEAR),
        mAniso(1), mSceneDetailIndex(0), mMoveSpeed(100), mRotateSpeed(36), mDebugOverlay(0),
        mInputManager(0), mMouse(0), mKeyboard(0), mJoy(0)
    {
        TRACE_ENTER();
        using namespace OIS;

        mDebugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");

        LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
        ParamList pl;
        size_t windowHnd = 0;
        std::ostringstream windowHndStr;

        win->getCustomAttribute("WINDOW", &windowHnd);
        windowHndStr << windowHnd;
        pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

        mInputManager = InputManager::createInputSystem( pl );

        //Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
        mKeyboard = static_cast<Keyboard*>(mInputManager->createInputObject( OISKeyboard, bufferedKeys ));
        mMouse = static_cast<Mouse*>(mInputManager->createInputObject( OISMouse, bufferedMouse ));
        try {
            mJoy = static_cast<JoyStick*>(mInputManager->createInputObject( OISJoyStick, bufferedJoy ));
        }
        catch(...) {
            LOG_ERROR("Caught ... excpetion while creating joystick input.");
            mJoy = 0;
        }

        //Set initial mouse clipping size
        windowResized(mWindow);

        showDebugOverlay(true);

        mRaySceneQuery=mSceneMgr->createRayQuery(Ray());

        //Register as a Window listener
        WindowEventUtilities::addWindowEventListener(mWindow, this);
        TRACE_EXIT();
    }

    //Adjust mouse clipping area
    // virtual 
    void OgreFrameListener::windowResized(RenderWindow* rw)
    {
        TRACE_ENTER();
        unsigned int width, height, depth;
        int left, top;
        rw->getMetrics(width, height, depth, left, top);

        const OIS::MouseState &ms = mMouse->getMouseState();
        ms.width = width;
        ms.height = height;
        TRACE_EXIT();
    }

    //Unattach OIS before window shutdown (very important under Linux)
    // virtual 
    void OgreFrameListener::windowClosed(RenderWindow* rw)
    {
        TRACE_ENTER();
        //Only close for window that created OIS (the main window in these demos)
        if( rw == mWindow )
        {
            if( mInputManager )
            {
                mInputManager->destroyInputObject( mMouse );
                mInputManager->destroyInputObject( mKeyboard );
                mInputManager->destroyInputObject( mJoy );

                OIS::InputManager::destroyInputSystem(mInputManager);
                mInputManager = 0;
            }
        }
        TRACE_EXIT();
    }

    // virtual 
    OgreFrameListener::~OgreFrameListener()
    {
        TRACE_ENTER();

        if (mRaySceneQuery && mSceneMgr)
            mSceneMgr->destroyQuery(mRaySceneQuery);

        //Remove ourself as a Window listener
        WindowEventUtilities::removeWindowEventListener(mWindow, this);
        windowClosed(mWindow);
        TRACE_EXIT();
    }

    // virtual 
    bool OgreFrameListener::processUnbufferedKeyInput(const FrameEvent& evt)
    {
        using namespace OIS;
        TRACE_ENTER();

        if(mKeyboard->isKeyDown(KC_A))
            mTranslateVector.x = -mMoveScale;	// Move camera left

        if(mKeyboard->isKeyDown(KC_D))
            mTranslateVector.x = mMoveScale;	// Move camera RIGHT

        if(mKeyboard->isKeyDown(KC_UP) || mKeyboard->isKeyDown(KC_W) )
            mTranslateVector.z = -mMoveScale;	// Move camera forward

        if(mKeyboard->isKeyDown(KC_DOWN) || mKeyboard->isKeyDown(KC_S) )
            mTranslateVector.z = mMoveScale;	// Move camera backward

        if(mKeyboard->isKeyDown(KC_PGUP))
            mTranslateVector.y = mMoveScale;	// Move camera up

        if(mKeyboard->isKeyDown(KC_PGDOWN))
            mTranslateVector.y = -mMoveScale;	// Move camera down

        if(mKeyboard->isKeyDown(KC_RIGHT))
            mCamera->yaw(-mRotScale);

        if(mKeyboard->isKeyDown(KC_LEFT))
            mCamera->yaw(mRotScale);

        if( mKeyboard->isKeyDown(KC_ESCAPE) || mKeyboard->isKeyDown(KC_Q) )
            return false;

        if( mKeyboard->isKeyDown(KC_F) && mTimeUntilNextToggle <= 0 )
        {
            mStatsOn = !mStatsOn;
            showDebugOverlay(mStatsOn);
            mTimeUntilNextToggle = 1;
        }

        if( mKeyboard->isKeyDown(KC_T) && mTimeUntilNextToggle <= 0 )
        {
            switch(mFiltering)
            {
                case TFO_BILINEAR:
                    mFiltering = TFO_TRILINEAR;
                    mAniso = 1;
                    break;
                case TFO_TRILINEAR:
                    mFiltering = TFO_ANISOTROPIC;
                    mAniso = 8;
                    break;
                case TFO_ANISOTROPIC:
                    mFiltering = TFO_BILINEAR;
                    mAniso = 1;
                    break;
                default: break;
            }
            MaterialManager::getSingleton().setDefaultTextureFiltering(mFiltering);
            MaterialManager::getSingleton().setDefaultAnisotropy(mAniso);

            showDebugOverlay(mStatsOn);
            mTimeUntilNextToggle = 1;
        }

        if(mKeyboard->isKeyDown(KC_F1) && mTimeUntilNextToggle <= 0)
        {
            // GTL - doesn't work, screenshot is scrambled a bit
            std::ostringstream ss;
            ss << "screenshot_" << ++mNumScreenShots << ".png";
            LOG_INFO("Writing screenshot to file: " << ss.str());
            mWindow->writeContentsToFile(ss.str());
            mTimeUntilNextToggle = 0.5;
            mDebugText = "Saved: " + ss.str();
        }

        if(mKeyboard->isKeyDown(KC_R) && mTimeUntilNextToggle <=0)
        {
            mSceneDetailIndex = (mSceneDetailIndex+1)%3 ;
            switch(mSceneDetailIndex) {
                case 0 : mCamera->setPolygonMode(PM_SOLID); break;
                case 1 : mCamera->setPolygonMode(PM_WIREFRAME); break;
                case 2 : mCamera->setPolygonMode(PM_POINTS); break;
            }
            mTimeUntilNextToggle = 0.5;
        }

        static bool displayCameraDetails = false;
        if(mKeyboard->isKeyDown(KC_P) && mTimeUntilNextToggle <= 0)
        {
            displayCameraDetails = !displayCameraDetails;
            mTimeUntilNextToggle = 0.5;
            if (!displayCameraDetails)
                mDebugText = "";
        }

        // Print camera details
        if(displayCameraDetails)
            mDebugText = "P: " + StringConverter::toString(mCamera->getDerivedPosition()) +
                " " + "O: " + StringConverter::toString(mCamera->getDerivedOrientation());

        // Return true to continue rendering
        TRACE_EXIT();
        return true;
    }

    bool OgreFrameListener::processUnbufferedMouseInput(const FrameEvent& evt)
    {
        using namespace OIS;
        TRACE_ENTER();

        // Rotation factors, may not be used if the second mouse button is pressed
        // 2nd mouse button - slide, otherwise rotate
        const MouseState &ms = mMouse->getMouseState();
        if( ms.buttonDown( MB_Right ) )
        {
            mTranslateVector.x += ms.X.rel * 0.13;
            mTranslateVector.y -= ms.Y.rel * 0.13;
        }
        else
        {
            mRotX = Degree(-ms.X.rel * 0.13);
            mRotY = Degree(-ms.Y.rel * 0.13);
        }

        TRACE_EXIT();
        return true;
    }

    void OgreFrameListener::moveCamera()
    {
        TRACE_ENTER();
        // Make all the changes to the camera
        // Note that YAW direction is around a fixed axis (freelook style) rather than a natural YAW
        //(e.g. airplane)
        mCamera->yaw(mRotX);
        mCamera->pitch(mRotY);
        mCamera->moveRelative(mTranslateVector);

        // Don't let the camera dip beneath the terrain
        Vector3 camPos = mCamera->getPosition();
        Ray cameraRay(Vector3(camPos.x, 5000.0f, camPos.z), Vector3::NEGATIVE_UNIT_Y);
        mRaySceneQuery->setRay(cameraRay);
        
        // Perform the scene query
        RaySceneQueryResult &result = mRaySceneQuery->execute();
        RaySceneQueryResult::iterator itr = result.begin();
        
        if (itr!=result.end() && itr->worldFragment)
        {
            Real terrainHgt=itr->worldFragment->singleIntersection.y;
            if ((terrainHgt+10.0)>camPos.y)
                mCamera->setPosition(camPos.x, terrainHgt+10.0, camPos.z);
        }

        TRACE_EXIT();
    }

    void OgreFrameListener::showDebugOverlay(bool show)
    {
        TRACE_ENTER();
        if (mDebugOverlay)
        {
            if (show)
                mDebugOverlay->show();
            else
                mDebugOverlay->hide();
        }
        TRACE_EXIT();
    }

    // Override frameStarted event to process that (don't care about frameEnded)
    bool OgreFrameListener::frameStarted(const FrameEvent& evt)
    {
        using namespace OIS;
        TRACE_ENTER();

        if(mWindow->isClosed())	return false;

        //Need to capture/update each device
        mKeyboard->capture();
        mMouse->capture();
        if( mJoy ) mJoy->capture();

        bool buffJ = (mJoy) ? mJoy->buffered() : true;

        //Check if one of the devices is not buffered
        if( !mMouse->buffered() || !mKeyboard->buffered() || !buffJ )
        {
            // one of the input modes is immediate, so setup what is needed for immediate movement
            if (mTimeUntilNextToggle >= 0)
                mTimeUntilNextToggle -= evt.timeSinceLastFrame;

            // If this is the first frame, pick a speed
            if (evt.timeSinceLastFrame == 0)
            {
                mMoveScale = 1;
                mRotScale = 0.1;
            }
            // Otherwise scale movement units by time passed since last frame
            else
            {
                // Move about 100 units per second,
                mMoveScale = mMoveSpeed * evt.timeSinceLastFrame;
                // Take about 10 seconds for full rotation
                mRotScale = mRotateSpeed * evt.timeSinceLastFrame;
            }
            mRotX = 0;
            mRotY = 0;
            mTranslateVector = Ogre::Vector3::ZERO;
        }

        //Check to see which device is not buffered, and handle it
        if( !mKeyboard->buffered() )
            if( processUnbufferedKeyInput(evt) == false )
                return false;
        if( !mMouse->buffered() )
            if( processUnbufferedMouseInput(evt) == false )
                return false;

        if( !mMouse->buffered() || !mKeyboard->buffered() || !buffJ )
            moveCamera();

        TRACE_EXIT();
        return true;
    }

    bool OgreFrameListener::frameEnded(const FrameEvent& evt)
    {
        TRACE_ENTER();
        updateStats();
        TRACE_EXIT();
        return true;
    }

} // end of namespace


