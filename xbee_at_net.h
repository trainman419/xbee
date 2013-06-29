
// AT network commands
//  net short address (ATMY)
//  net short parent  (ATMP)
//
//  net ID node (ATNI)
//  net ID cluster (ATCI)
//
//  net max-hops unicast (ATNH)
//  net max-hops broadcast (ATBH)
//
//  net PAN-ID operating (ATOP) - current PAN ID
//  net PAN-ID long (ATID) - PAN ID to join (0 for any)
//  net PAN-ID short (ATOI)
//
//  net node-discovery timeout (ATNT)
//  net node-discovery options (ATNO)
//
//  net join notification (ATJN)
//  net join time (ATNJ)
//
//  net max-payload (ATNP)
//  net destination (ATDL + ATDH)
//  net children (ATNC)
//  net serial (ATSL + ATSH)


#include "xbee_at_cmd.h"

command ** net();
