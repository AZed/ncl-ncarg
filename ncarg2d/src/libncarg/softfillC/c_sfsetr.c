/*
 *	$Id: c_sfsetr.c,v 1.5 2008/07/23 16:17:00 haley Exp $
 */
/************************************************************************
*                                                                       *
*                Copyright (C)  2000                                    *
*        University Corporation for Atmospheric Research                *
*                All Rights Reserved                                    *
*                                                                       *
*    The use of this Software is governed by a License Agreement.       *
*                                                                       *
************************************************************************/

#include <ncarg/ncargC.h>

extern void NGCALLF(sfsetr,SFSETR)(NGstring,float*,int);

void c_sfsetr
#ifdef NeedFuncProto
(
    char *cnp,
    float rvp
)
#else
(cnp,rvp)
    char *cnp;
    float rvp;
#endif
{
    int len;
    NGstring cnp2;
/*
 * Make sure parameter name is not NULL
 */
    if( !cnp ) {
        fprintf( stderr, "c_sfsetr:  illegal parameter name (NULL)\n");
        return;
    }

    len = NGSTRLEN(cnp);
    cnp2 = NGCstrToFstr(cnp,len);
    NGCALLF(sfsetr,SFSETR)(cnp2,&rvp,len);
}
