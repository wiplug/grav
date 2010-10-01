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

SessionManager::SessionManager( VideoListener* vl, AudioManager* al )
    : videoSessionListener( vl ), audioSessionListener( al )
{
    sessionMutex = mutex_create();

    videoSessionCount = 0;
    audioSessionCount = 0;
    lockCount = 0;
    pause = false;
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
    *counter++;
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
    pause = true;
    lockSessions();

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( it != sessions.end() && (*it).address.compare( addr ) != 0 )
        it++;

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
    pause = false;
    unlockSessions();
    return true;
}

bool SessionManager::setSessionEnable( std::string addr, bool set )
{
    lockSessions();

    std::vector<SessionEntry>::iterator it = sessions.begin();
    while ( (*it).address.compare( addr ) != 0 ) it++;
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
    while ( (*it).address.compare( addr ) != 0 ) it++;
    if ( it == sessions.end() )
    {
        unlockSessions();
        return false;
    }

    bool ret = (*it).enabled;
    unlockSessions();
    return ret;
}

bool SessionManager::iterateSessions()
{
    // kind of a hack to force this to wait, when removing etc.
    // mutex should do this but this thread seems way too eager
    if ( pause )
    {
        printf( "Sessions temporarily paused...\n" );
        wxMicroSleep( 10 );
    }

    lockSessions();

    bool haveSessions = false;
    for ( unsigned int i = 0; i < sessions.size(); i++ )
    {
        if ( sessions[i].enabled )
            sessions[i].session->iterate( sessions[i].sessionTS++ );
        haveSessions = haveSessions || sessions[i].enabled;
    }

    unlockSessions();
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
    mutex_lock( sessionMutex );
    lockCount++;
    //printf( "SessionManager::lock::lock gained, count now %i\n", lockCount );
}

void SessionManager::unlockSessions()
{
    //printf( "SessionManager::lock::unlocking, count %i\n", lockCount );
    mutex_unlock( sessionMutex );
    lockCount--;
    //printf( "SessionManager::lock::lock released, count now %i\n", lockCount );
}
