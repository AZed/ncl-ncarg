/*
 *	$Id: c_mpsetc.c,v 1.2 2008/07/23 16:16:54 haley Exp $
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

extern void NGCALLF(mpsetc,MPSETC)(NGstring,NGstring,int,int);

void c_mpsetc
#ifdef NeedFuncProto
(
    char *whch,
    char *cval
)
#else
(whch,cval)
    char *whch;
    char *cval;
#endif
{
    NGstring whch2;
    NGstring cval2;
    int len1, len2;
/*
 * Make sure parameter and return string is not NULL
 */
    if( !whch ) { 
        fprintf( stderr, "c_mpsetc:  illegal parameter string (NULL)\n" );
        return;
    }
    if( !cval ) { 
        fprintf( stderr, "c_mpsetc:  illegal return string (NULL)\n" );
        return;
    }
    len1 = NGSTRLEN(whch);
    len2 = NGSTRLEN(cval);
    whch2 = NGCstrToFstr(whch,len1);
    cval2 = NGCstrToFstr(cval,len2);
    NGCALLF(mpsetc,MPSETC)(whch2,cval2,len1,len2);
}

