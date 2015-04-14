C
C $Id: pj22sp.f,v 1.5 2008/07/27 00:17:12 haley Exp $
C
C                Copyright (C)  2000
C        University Corporation for Atmospheric Research
C                All Rights Reserved
C
C The use of this Software is governed by a License Agreement.
C
      SUBROUTINE PJ22SP (COORD,CRDIO,INDIC)
C
C -- S P A C E   O B L I Q U E   M E R C A T O R
C
      IMPLICIT REAL (A-Z)
      INTEGER PATH,LAND,NN,L
      INTEGER INDIC
      DIMENSION GEOG(2),PROJ(2),COORD(2),CRDIO(2)
C **** PARAMETERS **** A,E,ES,LON0,LATC,X0,Y0,MCS,TCS,FAC,IND **********
      COMMON /ERRMZ0/ IERR
        INTEGER IERR
      SAVE   /ERRMZ0/
      COMMON /PRINZ0/ IPEMSG,IPELUN,IPPARM,IPPLUN
        INTEGER IPEMSG,IPELUN,IPPARM,IPPLUN
      SAVE   /PRINZ0/
      COMMON /NORMSP/ Q,T,U,W,ES,P22,SA,CA,XJ
      COMMON /PC22SP/ A,X0,Y0,A2,A4,B,C1,C3,LAND,PATH
      DATA TOL /1.0E-7/
      DATA DG1 /0.01745329252E0/
      DATA PI /3.14159265358979323846E0/
      DATA ZERO,ONE,TWO /0.0E0,1.0E0,2.0E0/
C
C -- F O R W A R D   . . .
C
      IF (INDIC .EQ. 0) THEN
C
         GEOG(1) = COORD(1)
         GEOG(2) = COORD(2)
         IERR = 0
         IF (LAND.GE.4) GO TO 225
         LON=GEOG(1)-128.87E0*DG1+PI*TWO/251.E0*PATH
         GO TO 230
  225    LON=GEOG(1)-129.30E0*DG1+PI*TWO/233.E0*PATH
  230    LAT=GEOG(2)
C
C        TEST FOR LAT. AND LONG. APPROACHING 90 DEGREES.
C
         IF (LAT.GT.1.570796E0) LAT=1.570796E0
         IF (LAT.LT.-1.570796E0) LAT =-1.570796E0
         IF (LAT.GE.0) LAMPP=PI/TWO
         IF (LAT.LT.0) LAMPP=1.5E0*PI
         NN=0
  231    SAV=LAMPP
         L=0
         LAMTP=LON+P22*LAMPP
         CL=COS(LAMTP)
         IF (ABS(CL).LT.TOL) LAMTP=LAMTP-TOL
         FAC=LAMPP-(SIGN(ONE,CL))*SIN(LAMPP)*PI/TWO
  232    LAMT=LON+P22*SAV
         C=COS(LAMT)
         IF (ABS(C).LT.TOL) THEN
            LAMDP = SAV
            GO TO 233
         END IF
         XLAM=((ONE-ES)*TAN(LAT)*SA+SIN(LAMT)*CA)/C
         LAMDP=ATAN(XLAM)
         LAMDP=LAMDP+FAC
         DIF=ABS(SAV)-ABS(LAMDP)
         IF (ABS(DIF).LT.TOL) GO TO 233
         SAV=LAMDP
         L=L+1
         IF (L.GT.50) GO TO 234
         GO TO 232
C
C        ADJUST FOR LANDSAT ORIGIN.
C
  233    RLM=PI*(16.E0/31.E0+ONE/248.E0)
         RLM2=RLM+TWO*PI
         NN=NN+1
         IF (NN.GE.3) GO TO 236
         IF (LAMDP.GT.RLM.AND.LAMDP.LT.RLM2) GO TO 236
         IF (LAMDP.LE.RLM) LAMPP=2.5E0*PI
         IF (LAMDP.GE.RLM2) LAMPP=PI/TWO
         GO TO 231
  234    IF (IPEMSG .EQ. 0) WRITE (IPELUN,235)
  235    FORMAT (/' ERROR PJ22SP'/
     .            ' 50 ITERATIONS WITHOUT CONVERGENCE.')
         IERR = 223
  236    CONTINUE
C
C        LAMDP COMPUTED.  NOW COMPUTE PHIDP.
C
         SP=SIN(LAT)
         PHIDP=ASINSP(((ONE-ES)*CA*SP-SA*COS(LAT)*SIN(LAMT))/SQRT(ONE
     .     -ES*SP*SP))
C
C        COMPUTE X AND Y
C
         TANPH=LOG(TAN(PI/4.0E0+PHIDP/TWO))
         SD=SIN(LAMDP)
         SDSQ=SD*SD
         S=P22*SA*COS(LAMDP)*SQRT((ONE+T*SDSQ)/((ONE+W*SDSQ)*(ONE
     .     +Q*SDSQ)))
         D=SQRT(XJ*XJ+S*S)
         X=B*LAMDP+A2*SIN(TWO*LAMDP)+A4*SIN(4.0E0*LAMDP)-TANPH*S/D
         X=A*X
         Y=C1*SD+C3*SIN(3.0E0*LAMDP)+TANPH*XJ/D
         Y=A*Y
         PROJ(1)=X+X0
         PROJ(2)=Y+Y0
         CRDIO(1) = PROJ(1)
         CRDIO(2) = PROJ(2)
         RETURN
      END IF
C
C -- I N V E R S E   . . .
C
      IF (INDIC .EQ. 1) THEN
C
         PROJ(1) = COORD(1)
         PROJ(2) = COORD(2)
         IERR = 0
         X = PROJ(1) -X0
         Y = PROJ(2) -Y0
C
C        COMPUTE TRANSFORMED LAT/LONG AND GEODETIC LAT/LONG, GIVEN X,Y.
C
C        BEGIN INVERSE COMPUTATION WITH APPROXIMATION FOR LAMDP.  SOLVE
C        FOR TRANSFORMED LONG.
C
         LAMDP=X/A/B
         NN=0
  325    SAV=LAMDP
         SD=SIN(LAMDP)
         SDSQ=SD*SD
         S=P22*SA*COS(LAMDP)*SQRT((ONE+T*SDSQ)/((ONE+W*SDSQ)*(ONE+Q
     .     *SDSQ)))
         LAMDP=X/A+Y/A*S/XJ-A2*SIN(TWO*LAMDP)-A4*SIN(4.0E0*LAMDP)
     .     -(S/XJ)*(C1*SIN(LAMDP)+C3*SIN(3.0E0*LAMDP))
         LAMDP=LAMDP/B
         DIF=LAMDP-SAV
         IF (ABS(DIF).LT.TOL) GO TO 330
         NN=NN+1
         IF (NN.EQ.50) GO TO 330
         GO TO 325
C
C        COMPUTE TRANSFORMED LAT.
C
  330    SL=SIN(LAMDP)
         FAC=EXP(SQRT(ONE+S*S/XJ/XJ)*(Y/A-C1*SL-C3*SIN(3.0E0*LAMDP)))
         ACTAN=ATAN(FAC)
         PHIDP=TWO*(ACTAN-PI/4.0E0)
C
C        COMPUTE GEODETIC LATITUDE.
C
         DD=SL*SL
         IF (ABS(COS(LAMDP)).LT.TOL) LAMDP=LAMDP-TOL
         SPP=SIN(PHIDP)
         SPPSQ=SPP*SPP
         LAMT=ATAN(((ONE-SPPSQ/(ONE-ES))*TAN(LAMDP)*CA-SPP*SA*SQRT((
     .   ONE+Q*DD)*(ONE-SPPSQ)-SPPSQ*U)/COS(LAMDP))/(ONE-SPPSQ*(ONE+U))
     .   )
C
C        CORRECT INVERSE QUADRANT.
C
         IF (LAMT.GE.0) SL=ONE
         IF (LAMT.LT.0) SL=-ONE
         IF (COS(LAMDP).GE.0) SCL=ONE
         IF (COS(LAMDP).LT.0) SCL=-ONE
         LAMT=LAMT-PI/TWO*(ONE-SCL)*SL
         LON=LAMT-P22*LAMDP
C
C        COMPUTE GEODETIC LATITUDE.
C
         IF (ABS(SA).LT.TOL) LAT=ASINSP(SPP/SQRT((ONE-ES)*(ONE-ES)
     .      +ES*SPPSQ))
         IF (ABS(SA).LT.TOL) GO TO 335
         LAT=ATAN((TAN(LAMDP)*COS(LAMT)-CA*SIN(LAMT))/((ONE-ES)*SA))
  335    CONTINUE
         IF (LAND.GE.4) GO TO 370
         GEOG(1)=LON+128.87E0*DG1-PI*TWO/251.E0*PATH
         GO TO 380
  370    GEOG(1)=LON+129.30E0*DG1-PI*TWO/233.E0*PATH
  380    GEOG(2)=LAT
         CRDIO(1) = GEOG(1)
         CRDIO(2) = GEOG(2)
         RETURN
      END IF
C
      END
