CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C   FFTPACK 5.0
C   Copyright (C) 1995-2004, Scientific Computing Division,
C   University Corporation for Atmospheric Research
C   Licensed under the GNU General Public License (GPL)
C
C   Authors:  Paul N. Swarztrauber and Richard A. Valent
C
C   $Id: rffti1.f,v 1.2 2006/11/21 01:10:19 haley Exp $
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

      SUBROUTINE DRFFTI1(N,WA,FAC)
      DOUBLE PRECISION FI
      DOUBLE PRECISION WA(N),FAC(15)
      INTEGER NTRYH(4)
C*PT*WARNING* Already double-precision
      DOUBLE PRECISION TPI,ARGH,ARGLD,ARG
      DATA NTRYH(1),NTRYH(2),NTRYH(3),NTRYH(4)/4,2,3,5/
C
      NL = N
      NF = 0
      J = 0
  101 J = J + 1
      IF (J-4) 102,102,103
  102 NTRY = NTRYH(J)
      GO TO 104
  103 NTRY = NTRY + 2
  104 NQ = NL/NTRY
      NR = NL - NTRY*NQ
      IF (NR) 101,105,101
  105 NF = NF + 1
      FAC(NF+2) = NTRY
      NL = NQ
      IF (NTRY.NE.2) GO TO 107
      IF (NF.EQ.1) GO TO 107
      DO 106 I = 2,NF
          IB = NF - I + 2
          FAC(IB+2) = FAC(IB+1)
  106 CONTINUE
      FAC(3) = 2
  107 IF (NL.NE.1) GO TO 104
      FAC(1) = N
      FAC(2) = NF
C*PT*WARNING* Constant already double-precision
C*PT*WARNING* Constant already double-precision
      TPI = 8.D0*DATAN(1.D0)
      ARGH = TPI/DBLE(N)
      IS = 0
      NFM1 = NF - 1
      L1 = 1
      IF (NFM1.EQ.0) RETURN
      DO 110 K1 = 1,NFM1
          IP = FAC(K1+2)
          LD = 0
          L2 = L1*IP
          IDO = N/L2
          IPM = IP - 1
          DO 109 J = 1,IPM
              LD = LD + L1
              I = IS
              ARGLD = DBLE(LD)*ARGH
              FI = 0.D0
              DO 108 II = 3,IDO,2
                  I = I + 2
                  FI = FI + 1.D0
                  ARG = FI*ARGLD
                  WA(I-1) = DCOS(ARG)
                  WA(I) = DSIN(ARG)
  108         CONTINUE
              IS = IS + IDO
  109     CONTINUE
          L1 = L2
  110 CONTINUE
      RETURN
      END
