C
C	$Id: pwrtm.f,v 1.6 2008/07/27 00:23:04 haley Exp $
C                                                                      
C                Copyright (C)  2000
C        University Corporation for Atmospheric Research
C                All Rights Reserved
C
C The use of this Software is governed by a License Agreement.
C
      SUBROUTINE PWRTM (X,Y,CHARS,JSIZE,JOR,JCNTR)
C
C
C PWRTM CAN BE CALLED TO DRAW CHARACTERS AND MARK THE REGIONS WHERE THE
C CHARACTERS HAVE BEEN DRAWN IN THE MODEL PICTURE.
C
C THE PARAMETERS TO PWRTM ARE EXACTLY THE SAME AS THE PARAMETERS TO
C THE UTILITY SUPPORT ROUTINE WTSTR.
C
C THE COORDINATES X AND Y ARE IN THE USER'S RESOLUTION, SINCE PWRTM
C IS A USER ENTRY POINT
C
      CHARACTER*(*) CHARS
C
C ISKIP IS INITIALIZED AND DESCRIBED IN BLOCKDATA DASHBDX.
C
      COMMON /DSUP1/ ISKIP
C
      COMMON/INTPR/IPAU,FPART,TENSN,NP,SMALL,L1,ADDLR,ADDTB,MLLINE,
     1    ICLOSE
      SAVE
C
C
C A CONSTANT USED TO COMPUTE THE NUMBER OF LINES TO BE MARKED.
C SQTDTH = SQRT(2)/3
C
      DATA SQTDTH /.47133/
C
C COPY INTO LOCAL VARIABLES
C
      ICNTR = JCNTR
      ISIZE = JSIZE
      IOR = JOR
C
C RETRIEVE THE USER'S RESOLUTION.
C
         CALL GETUSV('XF',LXSAVE)
         CALL GETUSV('YF',LYSAVE)
C
C DRAW THE CHARACTER STRING
C
      CALL WTSTR (X,Y,CHARS,ISIZE,IOR,ICNTR)
C
C RETURN IF STRING IS OF LENGTH 0
C
      IC = LEN(CHARS)
      IF (IC .EQ. 0) RETURN
C
C TRANSFORM COORDINATES INTO RESOLUTION 15.
C
      CALL FL2INT (X,Y,MXS,MYS)
C
C ADJUST ADDLR AND ADDTB TO 15 BIT RESOLUTION.
C
      IFIFTE = ISHIFT (1,15-10)
      ATBAD = ADDTB*REAL(IFIFTE)
      ALRAD = ADDLR*REAL(IFIFTE)
C
C COMPUTE THE SIZE OF A CHARACTER IN THE STRING DRAWN
C
      FISIZE = ISIZE*ISHIFT(1,15-LXSAVE)
      IF (ISIZE .GT. 3) GO TO 30
      FISIZE = 256. + REAL(ISIZE)*128.
      IF (ISIZE .EQ. 3) FISIZE = 768.
   30 CONTINUE
      LNT = INT(FISIZE*REAL(IC)+2.*ALRAD+.5)
C
C COMPUTE THE ANGLE IN WHICH THE STRING WAS DRAWN
C
      ANGLE = (REAL(IOR)*2.*3.1415)/360.
C
      SA = SIN(ANGLE)
      CA = COS(ANGLE)
C
C CORRECT FOR CENTERING OPTION.
C
      IF (ICNTR .EQ. 0) SHIFT = .5*REAL(LNT)
      IF (ICNTR .EQ. -1) SHIFT = ALRAD
      IF (ICNTR .EQ. 1) SHIFT = REAL(LNT)-.5*FISIZE-ALRAD
C
C ADD FREE SPACE AT TOP AND BOTTOM
C
      FISIZE = FISIZE + 2.*ATBAD
C
C FIND THE LEFT AND THE RIGHT END OF THE AREA TO BE MARKED.
C
      MXS1 = REAL(MXS)-SHIFT*CA+.5*FISIZE*SA
      MYS1 = REAL(MYS)-SHIFT*SA-.5*FISIZE*CA
      MXS2 = REAL(MXS)+(REAL(LNT)-SHIFT)*CA+.5*FISIZE*SA
      MYS2 = REAL(MYS)+(REAL(LNT)-SHIFT)*SA-.5*FISIZE*CA
C
C COMPUTE THE NUMBER OF LINES TO BE MARKED.
C
      NLINES = FISIZE*SQTDTH
C
      R = REAL(MAX(NLINES-1,1))/FISIZE
C
C MARK THE AREA WHERE THE STRING WAS DRAWN
C
      DO 90 I=1,NLINES,ISKIP
         P = REAL(I-1)/R
         MX1 = REAL(MXS1)-P*SA
         MY1 = REAL(MYS1)+P*CA
         MX2 = REAL(MXS2)-P*SA
         MY2 = REAL(MYS2)+P*CA
         CALL MARKL (MX1,MY1,MX2,MY2)
   90 CONTINUE
C
      RETURN
C
C------REVISION HISTORY
C
C JUNE 1984          CONVERTED TO FORTRAN77 AND GKS
C
C JUNE 1988          CHANGED THE NAME OF A COMMON BLOCK TO GET RID OF A
C                    WARNING FROM SEGLDR.  (DJK)
C
C NOVEMBER 1988      REMOVED KURV1S AND KURV2S.  IMPLEMENTED CALLS TO
C                    MSKRV1 AND MSKRV2 INSTEAD.
C
C-----------------------------------------------------------------------
C
      END
