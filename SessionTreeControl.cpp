/**
 * @file SessionTreeControl.cpp
 *
 * Implementation of the SessionTreeControl - see SessionTreeControl.h
 *
 * Created on: Sep 13, 2010
 * @author Andrew Ford
 */

#include <wx/menu.h>

#include "SessionTreeControl.h"
#include "SessionManager.h"

BEGIN_EVENT_TABLE(SessionTreeControl, wxTreeCtrl)
EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, SessionTreeControl::itemRightClick)
END_EVENT_TABLE()

int SessionTreeControl::toggleEnableID = wxNewId();
int SessionTreeControl::removeID = wxNewId();

SessionTreeControl::SessionTreeControl() :
    wxTreeCtrl( NULL, wxID_ANY )
{ }

SessionTreeControl::SessionTreeControl( wxWindow* parent ) :
    wxTreeCtrl( parent, wxID_ANY, parent->GetPosition(), parent->GetSize() )
{
    rootID = AddRoot( _("Sessions"), -1, -1, new wxTreeItemData() );
    videoNodeID = AppendItem( rootID, _("Video") );
    audioNodeID = AppendItem( rootID, _("Audio") );
    Expand( rootID );
}

void SessionTreeControl::setSessionManager( SessionManager* s )
{
    sessionManager = s;
}

void SessionTreeControl::addSession( std::string address, bool audio )
{
    if ( !audio )
    {
        AppendItem( videoNodeID, wxString( address.c_str(), wxConvUTF8 ) );
        Expand( videoNodeID );
    }
    else
    {
        AppendItem( audioNodeID, wxString( address.c_str(), wxConvUTF8 ) );
        Expand( audioNodeID );
    }
}

void SessionTreeControl::removeSession( std::string address )
{
    wxTreeItemId item = findSession( rootID, address );
    if ( !item.IsOk() )
    {
        printf( "SessionTreeControl::removeObject: ERROR: item %s not found?\n",
                    address.c_str() );
        return;
    }

    Delete( item );
}

wxTreeItemId SessionTreeControl::findSession( wxTreeItemId root,
                                                std::string address )
{
    wxTreeItemIdValue temp; // unused var, needed in getchild
    wxTreeItemId targetItem;
    wxTreeItemId current = GetFirstChild( root, temp );

    while ( current.IsOk() )
    {
        wxString text = GetItemText( current );
        std::string target = std::string( text.char_str() );
        if ( target.compare( address ) == 0 )
            return current;

        if ( ItemHasChildren( current ) )
        {
            targetItem = findSession( current, address );
            if ( targetItem.IsOk() )
                return targetItem;
        }
        current = GetNextChild( root, temp );
    }

    wxTreeItemId none;
    return none; // return default value if not found
}

void SessionTreeControl::itemRightClick( wxTreeEvent& evt )
{
    std::string text = std::string( GetItemText( evt.GetItem() ).char_str() );
    {
        if ( text.compare( "Video" ) != 0 && text.compare( "Audio" ) != 0 &&
                text.compare( "Sessions" ) != 0 )
        {
            wxMenu rightClickMenu;
            wxString toggleLabel = sessionManager->isSessionEnabled( text ) ?
                    _("Disable") : _("Enable");
            rightClickMenu.Append( toggleEnableID, toggleLabel );
            rightClickMenu.Append( removeID, _("Remove") );
            // TODO static evt_menu binding (at top) doesn't seem to work for
            // these, so doing runtime connect
            Connect( toggleEnableID, wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler( SessionTreeControl::toggleEnableEvent) );
            Connect( removeID, wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler( SessionTreeControl::removeEvent ) );

            PopupMenu( &rightClickMenu, evt.GetPoint() );
        }
    }
}

void SessionTreeControl::toggleEnableEvent( wxCommandEvent& evt )
{
    std::string selectedAddress = std::string(
                                     GetItemText( GetSelection() ).char_str() );
    if ( sessionManager->setSessionEnable( selectedAddress,
            !sessionManager->isSessionEnabled( selectedAddress ) ) )
    {
        wxColour col = sessionManager->isSessionEnabled( selectedAddress ) ?
                        *wxWHITE : *wxLIGHT_GREY;
        SetItemBackgroundColour( GetSelection(), col );
    }
}

void SessionTreeControl::removeEvent( wxCommandEvent& evt )
{
    std::string selectedAddress = std::string(
                                     GetItemText( GetSelection() ).char_str() );
    if ( sessionManager->removeSession( selectedAddress ) )
    {
        removeSession( selectedAddress );
    }
}
