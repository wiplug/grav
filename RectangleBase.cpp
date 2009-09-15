/*
 * @file RectangleBase.cpp
 * Implementation of the RectangleBase class from RectangleBase.h
 * @author Andrew Ford
 */

#include "RectangleBase.h"

#include <cmath>

RectangleBase::RectangleBase( float _x, float _y )
{
    scaleX = 5.0f; scaleY = 5.0f;
    destX = x; destY = y;
    destScaleX = scaleX; destScaleY = scaleY;
    selected = false;
    destBColor.R = 1.0f; destBColor.G = 1.0f;
    destBColor.B = 1.0f; destBColor.A = 0.5f;
    borderColor = destBColor;
    animated = true;
    name = "";
    
    font = new FTBufferFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf");
    font->FaceSize(100);
}

float RectangleBase::getWidth()
{
    return scaleX;
}

float RectangleBase::getHeight()
{
    return scaleY;
}

void RectangleBase::move( float _x, float _y )
{
    destX = _x;
    if ( !animated ) x = _x;
    destY = _y;
    if ( !animated ) y = _y;
}

void RectangleBase::setPos( float _x, float _y )
{
    destX = _x; x = _x;
    destY = _y; y = _y;
}

void RectangleBase::setScale( float xs, float ys )
{
    destScaleX = xs; destScaleY = ys;
    if ( !animated) { scaleX = xs; scaleY = ys; }
}

float RectangleBase::getX()
{
    return x;
}

float RectangleBase::getY()
{
    return y;
}

float RectangleBase::getZ()
{
    return z;
}

float RectangleBase::getScaleX()
{
    return scaleX;
}

float RectangleBase::getScaleY()
{
    return scaleY;
}

bool RectangleBase::isSelected()
{
    return selected;
}

void RectangleBase::setSelect( bool select )
{
    selected = select;
    if ( select )
    {
        destBColor.R = 1.0f; destBColor.G = 1.0f;
        destBColor.B = 0.0f, destBColor.A = 0.8f;
    }
    else
    {
        destBColor.R = 1.0f; destBColor.G = 1.0f;
        destBColor.B = 1.0f, destBColor.A = 0.5f;
    }
    
    if ( !animated )
        borderColor = destBColor;
}

bool RectangleBase::intersect( float L, float R, float U, float D )
{
    // find the bounds of this object
    float left = getX() - getWidth()/2;
    float right = getX() + getWidth()/2;
    float bottom = getY() - getHeight()/2;
    float top = getY() + getHeight()/2;
    
    return !(L > right || R < left || D > top || U < bottom);
}

bool RectangleBase::intersect( RectangleBase* other )
{
    // find the bounds of the other object
    float left = other->getX() - other->getWidth()/2;
    float right = other->getX() + other->getWidth()/2;
    float bottom = other->getY() - other->getHeight()/2;
    float top = other->getY() + other->getHeight()/2;
    
    return intersect( left, right, top, bottom );
}

void RectangleBase::draw()
{
    // note that the position should be set before calling this
    
    // draw the border first
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    
    // set the border color
    glColor4f( borderColor.R, borderColor.G, 
                borderColor.B, borderColor.A );
    
    glVertex3f(-getWidth()/2.0-0.3, -getHeight()/2.0-0.3, 0.0);
    glVertex3f(-getWidth()/2.0-0.3, getHeight()/2.0+0.3, 0.0);
    glVertex3f(getWidth()/2.0+0.3, getHeight()/2.0+0.3, 0.0);
    glVertex3f(getWidth()/2.0+0.3, -getHeight()/2.0-0.3, 0.0);
    
    glEnd();
    glDisable(GL_BLEND);
    
    glPushMatrix();
    
    glTranslatef( -getWidth()/2.0f, getHeight()/2.0f+0.5f, 0.0f );
    glRasterPos2f( -getWidth()/2.0f, getHeight()/2.0f+0.5f );
    float scaleFactor = 0.006f * scaleX / 10.0f;
    glScalef( scaleFactor, scaleFactor, scaleFactor );
    
    glColor3f( 1.0f, 1.0f, 1.0f );
    
    const char* nc = name.c_str();
    font->Render(nc);
    
    glPopMatrix();
    
    /*float spacing = 0.20f;
    //float i = -name.length()*spacing/2.0f;
    float i = -getWidth();
    for (; *nc != '\0'; nc++)
    {
        glRasterPos2f( i*spacing, getHeight()/2.0f+0.2f );
        //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10,*nc);
        i++;
    }
    //glutStrokeString(GLUT_STROKE_MONO_ROMAN, uc);*/
}

void RectangleBase::animateValues()
{
    // movement animation
    x += (destX-x)/10.0f;
    y += (destY-y)/10.0f;
    scaleX += (destScaleX-scaleX)/10.0f;
    scaleY += (destScaleY-scaleY)/10.0f;
    
    borderColor.R += (destBColor.R-borderColor.R)/3.0f;
    borderColor.G += (destBColor.G-borderColor.G)/3.0f;
    borderColor.B += (destBColor.B-borderColor.B)/3.0f;
    borderColor.A += (destBColor.A-borderColor.A)/3.0f;
    
    // snap to the destination, since we'll never actually get there via
    // the above lines due to roundoff errors
    if ( fabs(destX-x) < 0.01f ) x = destX;
    if ( fabs(destY-y) < 0.01f ) y = destY;
    if ( fabs(destScaleX-scaleX) < 0.01f ) scaleX = destScaleX;
    if ( fabs(destScaleY-scaleY) < 0.01f ) scaleY = destScaleY;
    if ( fabs(destBColor.R-borderColor.R) < 0.01f ) 
        borderColor.R = destBColor.R;
    if ( fabs(destBColor.G-borderColor.G) < 0.01f ) 
        borderColor.G = destBColor.G;
    if ( fabs(destBColor.B-borderColor.B) < 0.01f ) 
        borderColor.B = destBColor.B;
    if ( fabs(destBColor.A-borderColor.A) < 0.01f ) 
        borderColor.A = destBColor.A;
}
