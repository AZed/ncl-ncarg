C
C       $Id: curvs2.f,v 1.3 2008-07-27 03:10:11 haley Exp $
C
C                Copyright (C)  2000
C        University Corporation for Atmospheric Research
C                All Rights Reserved
C
C The use of this Software is governed by a License Agreement.
C
C NOTE: If you make any changes to this software, please remember to
C make the same changes to the corresponding double precision routine.
C
      SUBROUTINE CURVS2(T,N,PARAM,X,Y,XP,YP,SIGMA,XO,YO)
C
C  This subroutine is a companion to CURV1S which must be
C  called before calling this subroutine.  CURV2S calculates
C  an interpolated value for a smoothing spline under tension 
C  at a given point.  CURVS1 must be called before CURVS2 to 
C  determine certain necessary parameters.
C
C  On input:
C
C    T 
C      A real value to be mapped onto the interpolating curve.
C      Values of T between zero and one interpolate the original
C      data; any values of T outside this range result in
C      extrapolation.
C
C    N 
C      The number of points which were specified to
C      determine the curve.
C
C    PARAM
C      The arc lengths computed by CURVS1.
C
C    X 
C      An array containing the X-coordinate values of the specified points.
C
C    Y 
C      An array containing the Y-coordinate values of the specified points.
C
C    XP 
C      An array of second derivative values as calculated by CURVS1.
C
C    YP 
C      An array of second derivative values as calculated by CURVS1.
C
C    SIGMA 
C      The tension factor (its sign is ignored).
C
C  On output:
C
C    XO
C      The interpolated X value.
C
C    YO
C      The interpolated Y value.
C
C  The parameters N, X, Y, XP, YP, and SIGMA should be input
C  unaltered from the output of CURVS1.
C
      REAL T,PARAM(N),X(N),Y(N),XP(N),YP(N),SIGMA
C
      XO = CURV2(T,N,PARAM,X,XP,SIGMA)
      YO = CURV2(T,N,PARAM,Y,YP,SIGMA)
C     
      RETURN
      END
