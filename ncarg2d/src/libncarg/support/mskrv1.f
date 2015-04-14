C
C	$Id: mskrv1.f,v 1.5 2008/07/27 00:17:30 haley Exp $
C                                                                      
C                Copyright (C)  2000
C        University Corporation for Atmospheric Research
C                All Rights Reserved
C
C The use of this Software is governed by a License Agreement.
C
      SUBROUTINE MSKRV1 (N,X,Y,SLP1,SLPN,XP,YP,TEMP,S,SIGMA,ISLPSW)
C
      DIMENSION X(N),Y(N),XP(N),YP(N),TEMP(N),S(N)
C
C ---------------------------------------------------------------------
C Note:  This routine comes from a proprietary package called FITPACK.
C It is used in the NCAR graphics package by permission of the author,
C Alan Cline.
C ---------------------------------------------------------------------
C
C                                            CODED BY ALAN KAYLOR CLINE
C                                         FROM FITPACK -- JUNE 22, 1986
C                                   A CURVE AND SURFACE FITTING PACKAGE
C                                 A PRODUCT OF PLEASANT VALLEY SOFTWARE
C                             8603 ALTUS COVE, AUSTIN, TEXAS 78759, USA
C
C ---------------------------------------------------------------------
C
C THIS SUBROUTINE DETERMINES THE PARAMETERS NECESSARY TO
C COMPUTE A SPLINE UNDER TENSION FORMING A CURVE IN THE
C PLANE AND PASSING THROUGH A SEQUENCE OF PAIRS (X(1),Y(1)),
C ...,(X(N),Y(N)). FOR ACTUAL COMPUTATION OF POINTS ON THE
C CURVE IT IS NECESSARY TO CALL THE SUBROUTINE MSKRV2.
C
C ON INPUT--
C
C   N IS THE NUMBER OF POINTS TO BE INTERPOLATED (N.GE.2).
C
C   X IS AN ARRAY CONTAINING THE N X-COORDINATES OF THE
C   POINTS.
C
C   Y IS AN ARRAY CONTAINING THE N Y-COORDINATES OF THE
C   POINTS. (ADJACENT X-Y PAIRS MUST BE DISTINCT, I. E.
C   EITHER X(I) .NE. X(I+1) OR Y(I) .NE. Y(I+1), FOR
C   I = 1,...,N-1.)
C
C   SLP1 AND SLPN CONTAIN THE DESIRED VALUES FOR THE ANGLES
C   (IN DEGREES) OF THE SLOPE AT (X(1),Y(1)) AND (X(N),Y(N))
C   RESPECTIVELY. THE ANGLES ARE MEASURED COUNTER-CLOCK-
C   WISE FROM THE X-AXIS AND THE POSITIVE SENSE OF THE CURVE
C   IS ASSUMED TO BE THAT MOVING FROM POINT 1 TO POINT N.
C   THE USER MAY OMIT VALUES FOR EITHER OR BOTH OF THESE
C   PARAMETERS AND SIGNAL THIS WITH ISLPSW.
C
C   XP AND YP ARE ARRAYS OF LENGTH AT LEAST N.
C
C   TEMP IS AN ARRAY OF LENGTH AT LEAST N WHICH IS USED
C   FOR SCRATCH STORAGE.
C
C   S IS AN ARRAY OF LENGTH AT LEAST N.
C
C   SIGMA CONTAINS THE TENSION FACTOR. THIS VALUE INDICATES
C   THE CURVINESS DESIRED. IF ABS(SIGMA) IS NEARLY ZERO
C   (E.G. .001) THE RESULTING CURVE IS APPROXIMATELY A CUBIC
C   SPLINE. IF ABS(SIGMA) IS LARGE (E. G. 50.) THE RESULTING
C   CURVE IS NEARLY A POLYGONAL LINE. IF SIGMA EQUALS ZERO A
C   CUBIC SPLINE RESULTS. A STANDARD VALUE FOR SIGMA IS
C   APPROXIMATELY 1. IN ABSOLUTE VALUE.
C
C   ISLPSW CONTAINS A SWITCH INDICATING WHICH SLOPE DATA
C   SHOULD BE USED AND WHICH SHOULD BE ESTIMATED BY THIS
C   SUBROUTINE,
C          = 0 IF SLP1 AND SLPN ARE TO BE USED,
C          = 1 IF SLP1 IS TO BE USED BUT NOT SLPN,
C          = 2 IF SLPN IS TO BE USED BUT NOT SLP1,
C          = 3 IF BOTH SLP1 AND SLPN ARE TO BE ESTIMATED
C              INTERNALLY.
C          = 4 IF BOTH SLP1 AND SLPN ARE TO BE ESTIMATED
C              INTERNALLY, SUBJECT TO THE CONDITIONS THAT
C              N.GE.3, X(N)=X(1), AND Y(N)=Y(1).
C
C ON OUTPUT--
C
C   XP AND YP CONTAIN INFORMATION ABOUT THE CURVATURE OF THE
C   CURVE AT THE GIVEN NODES.
C
C   S CONTAINS THE POLYGONAL ARCLENGTHS OF THE CURVE.
C
C   N, X, Y, SLP1, SLPN, ISLPSW, AND SIGMA ARE UNALTERED.
C
C THIS SUBROUTINE REFERENCES PACKAGE MODULES MSCEEZ, MSTRMS,
C AND MSSHCH.
C
C-----------------------------------------------------------
C
      NM1 = N-1
      NP1 = N+1
C
C DETERMINE POLYGONAL ARCLENGTHS
C
      S(1) = 0.
      DO 1 I = 2,N
        IM1 = I-1
    1   S(I) = S(IM1)+SQRT((X(I)-X(IM1))**2+
     *         (Y(I)-Y(IM1))**2)
C
C DENORMALIZE TENSION FACTOR
C
      SIGMAP = ABS(SIGMA)*REAL(N-1)/S(N)
C
C COMPUTE SLOPES WHEN ISLPSW = 4, USING SECOND ORDER INTERPOLATION.
C
      IF (ISLPSW.EQ.4) THEN
        DELS1=S(2)-S(1)
        DELN=S(N)-S(NM1)
        DELNN=DELS1+DELN
        C1 = -(DELS1/DELN)/DELNN
        C2 = ((DELS1-DELN)/DELS1)/DELN
        C3 = (DELN/DELNN)/DELS1
        SX = C1*X(NM1)+C2*X(1)+C3*X(2)
        SY = C1*Y(NM1)+C2*Y(1)+C3*Y(2)
        IF (SX.EQ.0..AND.SY.EQ.0.) SX = 1.
        DELT=SQRT(SX*SX+SY*SY)
        SLPP1X=SX/DELT
        SLPP1Y=SY/DELT
        SLPPNX=SLPP1X
        SLPPNY=SLPP1Y
        GO TO 7
      END IF
C
C APPROXIMATE END SLOPES
C
      IF (ISLPSW .GE. 2) GO TO 2
      SLPP1X = COS(.017453292519943*SLP1)
      SLPP1Y = SIN(.017453292519943*SLP1)
      GO TO 4
    2 DELS1 = S(2)-S(1)
      DELS2 = DELS1+DELS1
      IF (N .GT. 2) DELS2 = S(3)-S(1)
      CALL MSCEEZ (DELS1,DELS2,SIGMAP,C1,C2,C3,N)
      SX = C1*X(1)+C2*X(2)
      SY = C1*Y(1)+C2*Y(2)
      IF (N .EQ. 2) GO TO 3
      SX = SX+C3*X(3)
      SY = SY+C3*Y(3)
    3 DELT = SQRT(SX*SX+SY*SY)
      SLPP1X = SX/DELT
      SLPP1Y = SY/DELT
    4 IF (ISLPSW .EQ. 1 .OR. ISLPSW .EQ. 3) GO TO 5
      SLPPNX = COS(.017453292519943*SLPN)
      SLPPNY = SIN(.017453292519943*SLPN)
      GO TO 7
    5 DELSN = S(N)-S(NM1)
      DELSNM = DELSN+DELSN
      IF (N .GT. 2) DELSNM = S(N)-S(N-2)
      CALL MSCEEZ (-DELSN,-DELSNM,SIGMAP,C1,C2,C3,N)
      SX = C1*X(N)+C2*X(NM1)
      SY = C1*Y(N)+C2*Y(NM1)
      IF (N .EQ. 2) GO TO 6
      SX = SX+C3*X(N-2)
      SY = SY+C3*Y(N-2)
    6 DELT = SQRT(SX*SX+SY*SY)
      SLPPNX = SX/DELT
      SLPPNY = SY/DELT
C
C SET UP RIGHT HAND SIDES AND TRIDIAGONAL SYSTEM FOR XP AND
C YP AND PERFORM FORWARD ELIMINATION
C
    7 DX1 = (X(2)-X(1))/S(2)
      DY1 = (Y(2)-Y(1))/S(2)
      CALL MSTRMS (DIAG1,SDIAG1,SIGMAP,S(2))
      XP(1) = (DX1-SLPP1X)/DIAG1
      YP(1) = (DY1-SLPP1Y)/DIAG1
      TEMP(1) = SDIAG1/DIAG1
      IF (N .EQ. 2) GO TO 9
      DO 8 I = 2,NM1
        DELS2 = S(I+1)-S(I)
        DX2 = (X(I+1)-X(I))/DELS2
        DY2 = (Y(I+1)-Y(I))/DELS2
        CALL MSTRMS (DIAG2,SDIAG2,SIGMAP,DELS2)
        DIAG = DIAG1+DIAG2-SDIAG1*TEMP(I-1)
        DIAGIN = 1./DIAG
        XP(I) = (DX2-DX1-SDIAG1*XP(I-1))*DIAGIN
        YP(I) = (DY2-DY1-SDIAG1*YP(I-1))*DIAGIN
        TEMP(I) = SDIAG2*DIAGIN
        DX1 = DX2
        DY1 = DY2
        DIAG1 = DIAG2
    8   SDIAG1 = SDIAG2
    9 DIAG = DIAG1-SDIAG1*TEMP(NM1)
      XP(N) = (SLPPNX-DX1-SDIAG1*XP(NM1))/DIAG
      YP(N) = (SLPPNY-DY1-SDIAG1*YP(NM1))/DIAG
C
C PERFORM BACK SUBSTITUTION
C
      DO 10 I = 2,N
        IBAK = NP1-I
        XP(IBAK) = XP(IBAK)-TEMP(IBAK)*XP(IBAK+1)
   10   YP(IBAK) = YP(IBAK)-TEMP(IBAK)*YP(IBAK+1)
      RETURN
C
      END
