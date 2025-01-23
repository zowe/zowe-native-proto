/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#pragma pack(packed)

#ifndef __ssib__
#define __ssib__

struct ssib
{
  unsigned char ssibid[4];   /* CONTROL BLOCK IDENTIFIER              */
  unsigned short ssiblen;    /* SSIB LENGTH                           */
  unsigned char ssibflg1;    /* FLAGS                                 */
  unsigned char ssibssid;    /* SUBSYSTEM IDENTIFIER. SET    @YC01974 */
  unsigned char ssibssnm[4]; /* Subsystem name to which a        @P1C */
  unsigned char ssibjbid[8]; /* Job Identifier or Subsystem name @P1C */
  unsigned char ssibdest[8]; /* DEFAULT USERID FOR SYSOUT DESTINATION */
  int ssibrsv1;              /* RESERVED                              */
  int ssibsuse;              /* RESERVED FOR SUBSYSTEM USAGE          */
};

/* Values for field "ssibflg1" */
#define ssibpjes 0x80 /* THIS SSIB IS USED TO START THE        */
#define ssibnsvc 0x40 /* NO SVC INDICATOR             @G38RP2Q */

/* Values for field "ssibssid" */
#define ssibunkn 0x00 /* UNKNOWN SUBSYSTEM ID         @YA01974 */
#define ssibjes2 0x02 /* JES2 SUBSYSTEM ID            @YA01974 */
#define ssibjes3 0x03 /* JES3 SUBSYSTEM ID            @YA01974 */

/* Values for field "ssibsuse" */
#define ssibsize 0x24 /* SSIB LENGTH                           */

#endif

#pragma pack(reset)
