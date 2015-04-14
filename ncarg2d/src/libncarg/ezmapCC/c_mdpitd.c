/*
 *      $Id: c_mdpitd.c,v 1.2 2008/07/23 16:16:50 haley Exp $
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

extern void NGCALLF(mdpitd,MDPITD)(double*,double*,int*);

void c_mdpitd
#ifdef NeedFuncProto
(
    double xlat,
    double xlon,
    int ifst
)
#else
(xlat,xlon,ifst)
    double xlat;
    double xlon;
    int ifst;
#endif
{
      NGCALLF(mdpitd,MDPITD)(&xlat,&xlon,&ifst);
}
