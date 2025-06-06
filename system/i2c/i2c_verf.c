/****************************************************************************
 * apps/system/i2c/i2c_verf.c
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

#include <stdlib.h>
#include <unistd.h>

#include <nuttx/i2c/i2c_master.h>

#include "i2ctool.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: i2ccmd_verf
 ****************************************************************************/

int i2ccmd_verf(FAR struct i2ctool_s *i2ctool, int argc, FAR char **argv)
{
  FAR char *ptr;
  uint16_t rdvalue;
  uint8_t regaddr;
  bool addrinaddr;
  long wrvalue;
  long repetitions;
  int nargs;
  int argndx;
  int ret;
  int fd;
  int i;

  /* Parse any command line arguments */

  for (argndx = 1; argndx < argc; )
    {
      /* Break out of the look when the last option has been parsed */

      ptr = argv[argndx];
      if (*ptr != '-')
        {
          break;
        }

      /* Otherwise, check for common options */

      nargs = i2ctool_common_args(i2ctool, &argv[argndx]);
      if (nargs < 0)
        {
          return ERROR;
        }

      argndx += nargs;
    }

  /* The options may be followed by the optional wrvalue to be written.
   * If omitted, then the register address will be used as the wrvalue,
   * providing an address-in-address test.
   */

  addrinaddr = true;
  wrvalue    = 0;

  if (argndx < argc)
    {
      wrvalue = strtol(argv[argndx], NULL, 16);
      if (i2ctool->width == 8)
        {
          if (wrvalue < 0 || wrvalue > 255)
            {
              i2ctool_printf(i2ctool, g_i2cargrange, argv[0]);
              return ERROR;
            }
        }
      else if (wrvalue < 0 || wrvalue > 65535)
        {
          i2ctool_printf(i2ctool, g_i2cargrange, argv[0]);
          return ERROR;
        }

      addrinaddr = false;
      argndx++;
    }

  /* There may be one more thing on the command line:  The repetition
   * count.
   */

  repetitions = 1;
  if (argndx < argc)
    {
      repetitions = strtol(argv[argndx], NULL, 16);
      if (repetitions < 1)
        {
          i2ctool_printf(i2ctool, g_i2cargrange, argv[0]);
          return ERROR;
        }

      argndx++;
    }

  if (argndx != argc)
    {
      i2ctool_printf(i2ctool, g_i2ctoomanyargs, argv[0]);
      return ERROR;
    }

  /* Get a handle to the I2C bus */

  fd = i2cdev_open(i2ctool->bus);
  if (fd < 0)
    {
      i2ctool_printf(i2ctool, "Failed to get bus %d\n", i2ctool->bus);
      return ERROR;
    }

  /* Loop for the requested number of repetitions */

  regaddr = i2ctool->regaddr;
  ret = OK;

  for (i = 0; i < repetitions; i++)
    {
      /* If we are performing an address-in-address test, then use the
       * register address as the value to write.
       */

      if (addrinaddr)
        {
          wrvalue = regaddr;
        }

      /* Write to the I2C bus */

      ret = i2ctool_set(i2ctool, fd, regaddr, (uint16_t)wrvalue);
      if (ret == OK)
        {
          /* Read the value back from the I2C bus */

          ret = i2ctool_get(i2ctool, fd, regaddr, &rdvalue);
        }

      /* Display the result */

      if (ret == OK)
        {
          i2ctool_printf(i2ctool,
                         "VERIFY Bus: %d Addr: %02x Subaddr: %02x Wrote: ",
                         i2ctool->bus, i2ctool->addr, i2ctool->regaddr);

          if (i2ctool->width == 8)
            {
              i2ctool_printf(i2ctool,
                             "%02x Read: %02x",
                             (int)wrvalue, (int)rdvalue);
            }
          else
            {
              i2ctool_printf(i2ctool,
                             "%04x Read: %04x",
                             (int)wrvalue, (int)rdvalue);
            }

          if (wrvalue != rdvalue)
            {
              i2ctool_printf(i2ctool, "  <<< FAILURE\n");
            }
          else
            {
              i2ctool_printf(i2ctool, "\n");
            }
        }
      else
        {
          i2ctool_printf(i2ctool, g_i2cxfrerror, argv[0], -ret);
          break;
        }

      /* Auto-increment the address if so configured */

      if (i2ctool->autoincr)
        {
          regaddr++;
        }
    }

  close(fd);
  return ret;
}
