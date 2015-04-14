C
C       $Id: drwvec.f,v 1.6 2008-07-27 00:17:34 haley Exp $
C                                                                      
C                Copyright (C)  2000
C        University Corporation for Atmospheric Research
C                All Rights Reserved
C
C The use of this Software is governed by a License Agreement.
C
      SUBROUTINE DRWVEC (M1,M2,M3,M4,LABEL,NC)
C
C THIS ROUTINE IS CALLED TO DRAW A SINGLE ARROW.  IT HAS ARGUMENTS AS
C FOLLOWS -
C
C     (M1,M2)  -  COORDINATE OF ARROW BASE, ON A 2**15 X 2**15 GRID.
C     (M3,M4)  -  COORDINATE OF ARROW HEAD, ON A 2**15 X 2**15 GRID.
C     LABEL    -  CHARACTER LABEL TO BE PUT ABOVE ARROW.
C     NC       -  NUMBER OF CHARACTERS IN LABEL.
C
        SAVE
C
C
      COMMON /VEC1/   ASH        ,EXT        ,ICTRFG     ,ILAB       ,
     +                IOFFD      ,IOFFM      ,ISX        ,ISY        ,
     +                RMN        ,RMX        ,SIDE       ,SIZE       ,
     +                XLT        ,YBT        ,ZMN        ,ZMX
        CHARACTER*10 LABEL
C
C SOME LOCAL PARAMETERS ARE THE FOLLOWING -
C
C     CL     -  ARROW HEAD LENGTH SCALE FACTOR - EACH SIDE OF THE ARROW
C               HEAD IS THIS LONG RELATIVE TO THE LENGTH OF THE ARROW
C     ST,CT  -  SIN AND COS OF THE ARROW HEAD ANGLE
C     PI     -  THE CONSTANT PI
C     TWOPI  -  TWO TIMES PI
C     OHOPI  -  ONE HALF OF PI
C     FHOPI  -  FIVE HALVES OF PI
C
      DATA    CL / .25 /
      DATA    ST / .382683432365090 /
      DATA    CT / .923879532511287 /
      DATA    PI / 3.14159265358979 /
      DATA TWOPI / 6.28318530717959 /
      DATA OHOPI / 1.57079632679489 /
      DATA FHOPI / 7.85398163397448 /
C
      DIST(X,Y) = SQRT(X*X+Y*Y)
C
C TRANSFER ARGUMENTS TO LOCAL VARIABLES AND COMPUTE THE VECTOR LENGTH.
C
      N1 = M1
      N2 = M2
      N3 = M3
      N4 = M4
      DX = N3-N1
      DY = N4-N2
      R = DIST(DX,DY)
C
C SORT OUT POSSIBLE CASES, DEPENDING ON VECTOR LENGTH.
C
      IF (R .LE. ZMN) RETURN
C
      IF (R .LE. ZMX) GO TO 101
C
C PLOT A POINT FOR VECTORS WHICH ARE TOO LONG.
C
      CALL PLOTIT (N1,N2,0)
      CALL PLOTIT (N1,N2,1)
      CALL PLOTIT (N1,N2,0)
      RETURN
C
C ADJUST THE COORDINATES OF THE VECTOR ENDPOINTS AS IMPLIED BY THE
C CENTERING OPTION.
C
  101 IF (ICTRFG) 102,103,104
C
  102 N3 = N1
      N4 = N2
      N1 = REAL(N1)-DX
      N2 = REAL(N2)-DY
      GO TO 104
C
  103 N1 = REAL(N1)-.5*DX
      N2 = REAL(N2)-.5*DY
      N3 = REAL(N3)-.5*DX
      N4 = REAL(N4)-.5*DY
C
C DETERMINE THE COORDINATES OF THE POINTS USED TO DRAW THE ARROWHEAD.
C
  104 C1 = CL
C
C SHORT ARROWS HAVE HEADS OF A FIXED MINIMUM SIZE.
C
      IF (R .LT. RMN) C1 = RMN*CL/R
C
C LONG ARROWS HAVE HEADS OF A FIXED MAXIMUM SIZE.
C
      IF (R .GT. RMX) C1 = RMX*CL/R
C
C COMPUTE THE COORDINATES OF THE HEAD.
C
      N5 = REAL(N3)-C1*(CT*DX-ST*DY)
      N6 = REAL(N4)-C1*(CT*DY+ST*DX)
      N7 = REAL(N3)-C1*(CT*DX+ST*DY)
      N8 = REAL(N4)-C1*(CT*DY-ST*DX)
C
C PLOT THE ARROW.
C
      CALL PLOTIT (N1,N2,0)
      CALL PLOTIT (N3,N4,1)
      CALL PLOTIT (N5,N6,0)
      CALL PLOTIT (N3,N4,1)
      CALL PLOTIT (N7,N8,1)
      CALL PLOTIT (0,0,0)
C
C IF REQUESTED, PUT THE VECTOR MAGNITUDE ABOVE THE ARROW.
C
      IF (NC .EQ. 0) RETURN
      PHI = ATAN2(DY,DX)
      IF (MOD(PHI+FHOPI,TWOPI) .GT. PI) PHI = PHI+PI
      IX = 1+INT(.5*REAL(N1+N3)+1.25*
     +            REAL(ISX*MAX(INT(SIZE)/ISX,8))*COS(PHI+OHOPI))/ISX
      IY = 1+INT(.5*REAL(N2+N4)+1.25*
     +            REAL(ISX*MAX(INT(SIZE)/ISX,8))*SIN(PHI+OHOPI))/ISY
C
C
        XC = CPUX(IX)
        YC = CPUY(IY)
      CALL WTSTR(XC,YC,
     +           LABEL,MAX(INT(SIZE)/ISX,8),
     +                                     INT(57.2957795130823*PHI),0)
C
      RETURN
      END
