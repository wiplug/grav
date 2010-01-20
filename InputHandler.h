#ifndef INPUTHANDLER_H_
#define INPUTHANDLER_H_

/**
 * @file InputHandler.h
 * Processes keyboard and mouse input (passed from GLUT) and controls object
 * selection and movement accordingly.
 * @author Andrew Ford
 */

#include <vector>
#include <map>
#include <string>

#include <wx/wx.h>

class VideoSource;
class RectangleBase;
class Earth;
class Group;
class gravManager;

typedef double GLdouble;

class InputHandler : public wxEvtHandler
{
    
public:
    InputHandler( std::vector<VideoSource*>* source,
                  std::vector<RectangleBase*>* drawn,
                  std::vector<RectangleBase*>* selected,
                  std::map<std::string,Group*>* sites,
                  Earth* e, gravManager* g );
    ~InputHandler();
    
    void wxKeyPress( wxKeyEvent& evt );
    void wxMouseMove( wxMouseEvent& evt );
    void wxMouseLDown( wxMouseEvent& evt );
    void wxMouseLUp( wxMouseEvent& evt );
    
    void processKeyboard( unsigned char key, int x, int y );
    void processSpecialKey( int key, int x, int y );
    void processMouse( int button, int state, int x, int y );
    void processActiveMotion( int x, int y );
    
    void leftClick( int x, int y );
    void leftRelease( int x, int y );
    void mouseLeftHeldMove( int x, int y );
    
    bool selectVideos();
    
    /*
     * Various accessors.
     */
    bool isLeftButtonHeld();
    float getDragStartX(); float getDragStartY();
    float getDragEndX(); float getDragEndY();
    
private:
    std::vector<VideoSource*>* sources;
    std::vector<RectangleBase*>* drawnObjects;
    std::vector<RectangleBase*>* selectedObjects;
    std::vector<RectangleBase*>* tempSelectedObjects;
    std::map<std::string,Group*>* siteIDGroups;
    Earth* earth;
    
    // parent class
    gravManager* grav;
    
    // special input modifiers like CTRL
    int special;
    
    GLdouble mouseX, mouseY, mouseZ;
    
    // start & end pos for click-and-dragging
    float dragStartX;
    float dragStartY;
    float dragEndX;
    float dragEndY;
    // the mouse pos from a previous activemotion call, for calculating drag
    // distance
    float dragPrevX;
    float dragPrevY;
    
    bool leftButtonHeld;
    bool clickedInside;
    bool dragging;
    
    DECLARE_EVENT_TABLE()
    
};

#endif /*INPUTHANDLER_H_*/