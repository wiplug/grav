/*
 * @file SessionManager.cpp
 *
 * Implementation of the session manager.
 *
 * Created on: Sep 28, 2010
 * @author Andrew Ford
 */

#include <VPMedia/VPMSession.h>
#include <VPMedia/VPMSessionFactory.h>
#include <VPMedia/thread_helper.h>
#include <VPMedia/random_helper.h>

#include <wx/utils.h>

#include <stdio.h>

#include "SessionManager.h"
#include "VideoListener.h"
#include "AudioManager.h"
#include "grav.h"

SessionManager::SessionManager( VideoListener* vl, AudioManager* al )
    : videoSessionListener( vl ), audioSessionListener( al )
{
    sessionMutex = mutex_create();

    videoSessionCount = 0;
    audioSessionCount = 0;
    lockCount = 0;
    pause = false;

    rotatePos = -1;
}

SessionManager::~SessionManager()
{
    mutex_free( sessionMutex );
    for ( unsigned int i = 0; i < sessions.size(); i++ )
    {
        delete sessions[i].session;
    }
}

bool SessionManager::initSession( std::string address, bool audio )
{
    lockSessions();

    SessionEntry entry;
    VPMSession* session;
    VPMSessionFactory* factory = VPMSessionFactory::getInstance();
    std::string type = std::string( audio ? "audio" : "video" );
    int* counter = audio ? &audioSessionCount : &videoSessionCount;

    if ( !audio )
    {
        session = factory->createSession( address.c_str(),
                                        *videoSessionListener );

        session->enableVideo( true );
        session->enableAudio( false );
        session->enableOther( false );
    }
    else
    {
        session = factory->createSession( address.c_str(),
                                        *audioSessionListener);

        session->enableVideo(false);
        session->enableAudio(true);
        session->enableOther(false);
    }

    if ( !session->initialise() )
    {
        fprintf(stderr, "error: failed to initialise session\n");
        unlockSessions();
        return false;
    }

    printf( "SessionManager::initialized %s session on %s\n", type.c_str(),
                address.c_str() );
    (*counter)++;
    entry.sessionTS = random32();
    entry.address = address;
    entry.audio = audio;
    entry.enabled = true;
    entry.session = session;
    sessions.push_back( entry );

    unlockSessions();
    return true;
}

bool SessionManager::removeSession( std::string addr )
{
    lockSessions();

    printf( "SessionManager::attempting to remove session %s\n", addr.c_str() );
    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( it != sessions.end() && (*it).address.compare( addr ) != 0 )
        ++it;

    if ( it == sessions.end() )
    {
        unlockSessions();
        printf( "SessionManager::ERROR: session %s not found\n", addr.c_str() );
        return false;
    }

    int* counter = (*it).audio ? &audioSessionCount : &videoSessionCount;
    (*counter)--;
    delete (*it).session;
    sessions.erase( it );
    unlockSessions();
    return true;
}

void SessionManager::addRotatedSession( std::string addr, bool audio )
{
    lockSessions();
    videoRotateList.push_back( addr );
    unlockSessions();
}

void SessionManager::removeRotatedSession( std::string addr, bool audio )
{
    lockSessions();
    int i = 0;
    std::vector<std::string>::iterator vrlit = videoRotateList.begin();
    while ( vrlit != videoRotateList.end() && (*vrlit).compare( addr ) != 0 )
    {
        ++vrlit;
        i++;
    }
    if ( vrlit == videoRotateList.end() )
    {
        // if addr not found, exit
        unlockSessions();
        return;
    }
    else
    {
        videoRotateList.erase( vrlit );
        // shift rotate position back if what we're removing if before or at it,
        // so we don't skip any
        if ( i <= rotatePos )
            rotatePos--;
        // find session & remove it from main list if it's active
        std::vector<SessionEntry>::iterator it2 = sessions.begin();
        while ( it2 != sessions.end() && (*it2).address.compare( addr ) != 0 )
            ++it2;
        if ( it2 == sessions.end() )
        {
            unlockSessions();
            return;
        }
        else
        {
            unlockSessions();
            removeSession( (*it2).address );
        }
    }
}

void SessionManager::rotate( bool audio )
{
    lockSessions();

    int numSessions = (int)videoRotateList.size();
    int lastRotatePos = rotatePos;
    if ( lastRotatePos != -1 )
        lastRotateSession = videoRotateList[ rotatePos ];
    if ( numSessions == 0 )
    {
        unlockSessions();
        return;
    }
    if ( ++rotatePos >= numSessions )
    {
        rotatePos = 0;
    }
    printf( "about to rotate: pos %i, session %s, last session %s\n", rotatePos,
            videoRotateList[ rotatePos ].c_str(), lastRotateSession.c_str() );
    unlockSessions();

    // only remove & rotate if there is a valid old one & it isn't the same as
    // current
    if ( lastRotateSession.compare( "" ) != 0 &&
            lastRotateSession.compare( videoRotateList[ rotatePos ] ) != 0 )
    {
        removeSession( lastRotateSession );
        initSession( videoRotateList[ rotatePos ], false );
    }
    // case for first rotate
    else if ( lastRotatePos == -1 )
    {
        initSession( videoRotateList[ rotatePos ], false );
    }
}

std::string SessionManager::getCurrentRotateSession()
{
    if ( rotatePos != -1 && rotatePos < (int)videoRotateList.size() )
        return videoRotateList[ rotatePos ];
    else
        return "";
}

std::string SessionManager::getLastRotateSession()
{
    return lastRotateSession;
}

bool SessionManager::setSessionEnable( std::string addr, bool set )
{
    lockSessions();

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( it != sessions.end() && (*it).address.compare( addr ) != 0 )
        ++it;
    if ( it == sessions.end() )
    {
        unlockSessions();
        return false;
    }

    (*it).enabled = set;
    unlockSessions();
    return true;
}

bool SessionManager::isSessionEnabled( std::string addr )
{
    lockSessions();

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( it != sessions.end() && (*it).address.compare( addr ) != 0 )
        ++it;
    if ( it == sessions.end() )
    {
        unlockSessions();
        return false;
    }

    bool ret = (*it).enabled;
    unlockSessions();
    return ret;
}

bool SessionManager::setEncryptionKey( std::string addr, std::string key )
{
    lockSessions();

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( it != sessions.end() && (*it).address.compare( addr ) != 0 )
        ++it;
    if ( it == sessions.end() )
    {
        unlockSessions();
        return false;
    }

    (*it).encryptionKey = key;
    (*it).encryptionEnabled = true;
    (*it).session->setEncryptionKey( key.c_str() );

    unlockSessions();
    return true;
}

bool SessionManager::disableEncryption( std::string addr )
{
    lockSessions();

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( it != sessions.end() && (*it).address.compare( addr ) != 0 )
        ++it;
    if ( it == sessions.end() )
    {
        unlockSessions();
        return false;
    }

    (*it).encryptionEnabled = false;
    (*it).session->setEncryptionKey( NULL );

    unlockSessions();
    return true;
}

bool SessionManager::isEncryptionEnabled( std::string addr )
{
    lockSessions();

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( it != sessions.end() && (*it).address.compare( addr ) != 0 )
        ++it;
    if ( it == sessions.end() )
    {
        unlockSessions();
        return false; // this doesn't quite make sense - should throw some other
                      // kind of error for session not found?
    }

    bool ret = (*it).encryptionEnabled;
    unlockSessions();
    return ret;
}

bool SessionManager::iterateSessions()
{
    // kind of a hack to force this to wait, when removing etc.
    // mutex should do this but this thread seems way too eager
    if ( pause )
    {
        //printf( "Sessions temporarily paused...\n" );
        wxMicroSleep( 10 );
    }

    // note: iterate doesn't do lockSessions() since it shouldn't affect pause
    mutex_lock( sessionMutex );
    lockCount++;

    bool haveSessions = false;
    for ( unsigned int i = 0; i < sessions.size(); i++ )
    {
        if ( sessions[i].enabled )
            sessions[i].session->iterate( sessions[i].sessionTS++ );
        haveSessions = haveSessions || sessions[i].enabled;
    }
    if ( sessions.size() >= 1 && gravApp::threadDebug )
    {
        if ( sessions[0].sessionTS % 1000 == 0 )
        {
            printf( "SessionManager::iterate: have %u sessions, TS=%u\n",
                    sessions.size(), sessions[0].sessionTS );
        }
    }

    mutex_unlock( sessionMutex );
    lockCount--;
    return haveSessions;
}

int SessionManager::getVideoSessionCount()
{
    return videoSessionCount;
}

int SessionManager::getAudioSessionCount()
{
    return audioSessionCount;
}

void SessionManager::lockSessions()
{
    //printf( "SessionManager::lock::attempting to get lock...\n" );
    pause = true;
    mutex_lock( sessionMutex );
    lockCount++;
    //printf( "SessionManager::lock::lock gained, count now %i\n", lockCount );
}

void SessionManager::unlockSessions()
{
    //printf( "SessionManager::lock::unlocking, count %i\n", lockCount );
    pause = false;
    mutex_unlock( sessionMutex );
    lockCount--;
    //printf( "SessionManager::lock::lock released, count now %i\n", lockCount );
}