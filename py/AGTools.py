#!/usr/bin/python
#
# Functions for querying and controlling local Access Grid venue clients.
#
# Created November 2, 2010
# @author Andrew Ford

try:
    import agversion
    agversion.select(3)

    def GetVenueClients():
        from AccessGrid.VenueClient import GetVenueClientUrls
        return GetVenueClientUrls()

    def GetClient(clientURL):
        from AccessGrid.interfaces.VenueClient_client import VenueClientIW

        client = VenueClientIW(clientURL)
        if not client.IsValid():
            print "Error: venue client at ", clientURL, " not valid"
            return {}
        else:
            return client

    def GetExits(clientURL):
        client = GetClient(clientURL)
        if client == None:
            print "GetExits(): Error: getclient failed"
            return {}

        try:
            exits = client.GetConnections()
        except:
            print "Error: GetConnections() failed, maybe client is not in a venue?"
            return {}

        exitNames = dict([(exit.GetName(), exit.GetURI()) for exit in exits])
        return exitNames

    def EnterVenue(clientURL, venueURL):
        client = GetClient(clientURL)
        if client == None:
            print "EnterVenue(): Error: getclient failed"
            return

        client.EnterVenue(venueURL)

except:
    import traceback
    traceback.print_exc()

#if __name__ == '__main__':
#    urls = GetVenueClients()
#    if len(urls) > 0:
#        exits = GetExits(urls[0])
#        print "Exits: "
#        import pprint
#        pprint.pprint(exits)
#        if len(exits) > 0:
#            print "exit key 0 type ", type(exits.keys()[0])
#            print "Entering first venue"
#            EnterVenue(urls[0], exits[exits.keys()[0]])
#        else:
#            print "No venue connections?"
#    else:
#        print "No Venue Clients found"
