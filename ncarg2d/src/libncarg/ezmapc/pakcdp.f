C
C $Id: pakcdp.f,v 1.5 2008/07/27 00:17:09 haley Exp $
C
C                Copyright (C)  2000
C        University Corporation for Atmospheric Research
C                All Rights Reserved
C
C The use of this Software is governed by a License Agreement.
C
      DOUBLE PRECISION FUNCTION PAKCDP (PAK)
C
C FUNCTION TO CONVERT 2 DIGIT PACKED DMS TO 3 DIGIT PACKED DMS ANGLE.
C
C SGNA : SIGN OF ANGLE
C DEGS : DEGREES PORTION OF ANGLE
C MINS : MINUTES PORTION OF ANGLE
C SECS : SECONDS PORTION OF ANGLE
C
      IMPLICIT DOUBLE PRECISION (A-Z)
      INTEGER DEGS,MINS
      CHARACTER*1 SGNA,IBLANK,NEG
      DATA ZERO,CON1,CON2 /0.0D0,10000.0D0,100.0D0/
      DATA CON3,CON4 /1000000.0D0,1000.0D0/
      DATA TOL /1.0D-3/
      DATA IBLANK,NEG /' ','-'/
C
      SGNA = IBLANK
      IF (PAK .LT. ZERO) SGNA = NEG
      CON = ABS (PAK)
      DEGS = INT ((CON / CON1) + TOL)
      CON = MOD ( CON , CON1)
      MINS = INT ((CON / CON2) + TOL)
      SECS = MOD (CON , CON2)
C
      CON = DEGS * CON3 + MINS * CON4 + SECS
      IF (SGNA .EQ. NEG) CON = - CON
      PAKCDP = CON
      RETURN
C
      END
