/****************************************************************************
 * apps/netutils/netlib/netlib_getnodeaddr.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "nuttx/wireless/pktradio.h"
#include "netutils/netlib.h"

#if defined(CONFIG_NET_6LOWPAN) || defined(CONFIG_NET_IEEE802154)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: netlib_getnodnodeaddr
 *
 * Description:
 *   Get the non-IEEE802.15.4 packet radio node address
 *
 * Parameters:
 *   ifname  - The name of the interface to use
 *   nodeadd - Location to return the node address
 *
 * Return:
 *   0 on success; -1 on failure.  errno will be set on failure.
 *
 ****************************************************************************/

int netlib_getnodnodeaddr(FAR const char *ifname,
                          FAR struct pktradio_addr_s *nodeaddr)
{
  struct pktradio_ifreq_s req;
  int ret = ERROR;

  DEBUGASSERT(ifname != NULL && nodeaddr != NULL);
  if (ifname != NULL)
    {
      /* Get a socket (only so that we get access to the INET subsystem) */

      int sockfd = socket(NET_SOCK_FAMILY, NET_SOCK_TYPE, NET_SOCK_PROTOCOL);
      if (sockfd >= 0)
        {
          /* Copy the network interface name */

          strlcpy(req.pifr_name, ifname, IFNAMSIZ);

          /* And perform the IOCTL command */

          ret = ioctl(sockfd, SIOCPKTRADIOGNODE,
                      (unsigned long)((uintptr_t)&req));
          if (ret >= 0)
            {
              /* Copy the node address from the request */

              memcpy(nodeaddr, &req.pifr_hwaddr,
                     sizeof(struct pktradio_addr_s));
            }

          close(sockfd);
        }
    }

  return ret;
}

#endif /* CONFIG_NET_6LOWPAN || CONFIG_NET_IEEE802154 */
