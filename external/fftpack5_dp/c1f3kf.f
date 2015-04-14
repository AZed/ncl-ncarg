CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C   FFTPACK 5.0
C   Copyright (C) 1995-2004, Scientific Computing Division,
C   University Corporation for Atmospheric Research
C   Licensed under the GNU General Public License (GPL)
C
C   Authors:  Paul N. Swarztrauber and Richard A. Valent
C
C   $Id: c1f3kf.f,v 1.2 2006/11/21 01:10:15 haley Exp $
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

      SUBROUTINE DC1F3KF(IDO,L1,NA,CC,IN1,CH,IN2,WA)
      DOUBLE PRECISION TAUR
      DOUBLE PRECISION TAUI
      DOUBLE PRECISION SN
      DOUBLE PRECISION TR2
      DOUBLE PRECISION CR2
      DOUBLE PRECISION TI2
      DOUBLE PRECISION CI2
      DOUBLE PRECISION CR3
      DOUBLE PRECISION CI3
      DOUBLE PRECISION DR2
      DOUBLE PRECISION DR3
      DOUBLE PRECISION DI2
      DOUBLE PRECISION DI3
      DOUBLE PRECISION CC(IN1,L1,IDO,3),CH(IN2,L1,3,IDO),WA(IDO,2,2)
      DATA TAUR,TAUI/-.5D0,-.866025403784439D0/
C
      IF (IDO.GT.1) GO TO 102
      SN = 1.D0/DBLE(3*L1)
      IF (NA.EQ.1) GO TO 106
      DO 101 K = 1,L1
          TR2 = CC(1,K,1,2) + CC(1,K,1,3)
          CR2 = CC(1,K,1,1) + TAUR*TR2
          CC(1,K,1,1) = SN* (CC(1,K,1,1)+TR2)
          TI2 = CC(2,K,1,2) + CC(2,K,1,3)
          CI2 = CC(2,K,1,1) + TAUR*TI2
          CC(2,K,1,1) = SN* (CC(2,K,1,1)+TI2)
          CR3 = TAUI* (CC(1,K,1,2)-CC(1,K,1,3))
          CI3 = TAUI* (CC(2,K,1,2)-CC(2,K,1,3))
          CC(1,K,1,2) = SN* (CR2-CI3)
          CC(1,K,1,3) = SN* (CR2+CI3)
          CC(2,K,1,2) = SN* (CI2+CR3)
          CC(2,K,1,3) = SN* (CI2-CR3)
  101 CONTINUE
      RETURN
  106 DO 107 K = 1,L1
          TR2 = CC(1,K,1,2) + CC(1,K,1,3)
          CR2 = CC(1,K,1,1) + TAUR*TR2
          CH(1,K,1,1) = SN* (CC(1,K,1,1)+TR2)
          TI2 = CC(2,K,1,2) + CC(2,K,1,3)
          CI2 = CC(2,K,1,1) + TAUR*TI2
          CH(2,K,1,1) = SN* (CC(2,K,1,1)+TI2)
          CR3 = TAUI* (CC(1,K,1,2)-CC(1,K,1,3))
          CI3 = TAUI* (CC(2,K,1,2)-CC(2,K,1,3))
          CH(1,K,2,1) = SN* (CR2-CI3)
          CH(1,K,3,1) = SN* (CR2+CI3)
          CH(2,K,2,1) = SN* (CI2+CR3)
          CH(2,K,3,1) = SN* (CI2-CR3)
  107 CONTINUE
      RETURN
  102 DO 103 K = 1,L1
          TR2 = CC(1,K,1,2) + CC(1,K,1,3)
          CR2 = CC(1,K,1,1) + TAUR*TR2
          CH(1,K,1,1) = CC(1,K,1,1) + TR2
          TI2 = CC(2,K,1,2) + CC(2,K,1,3)
          CI2 = CC(2,K,1,1) + TAUR*TI2
          CH(2,K,1,1) = CC(2,K,1,1) + TI2
          CR3 = TAUI* (CC(1,K,1,2)-CC(1,K,1,3))
          CI3 = TAUI* (CC(2,K,1,2)-CC(2,K,1,3))
          CH(1,K,2,1) = CR2 - CI3
          CH(1,K,3,1) = CR2 + CI3
          CH(2,K,2,1) = CI2 + CR3
          CH(2,K,3,1) = CI2 - CR3
  103 CONTINUE
      DO 105 I = 2,IDO
          DO 104 K = 1,L1
              TR2 = CC(1,K,I,2) + CC(1,K,I,3)
              CR2 = CC(1,K,I,1) + TAUR*TR2
              CH(1,K,1,I) = CC(1,K,I,1) + TR2
              TI2 = CC(2,K,I,2) + CC(2,K,I,3)
              CI2 = CC(2,K,I,1) + TAUR*TI2
              CH(2,K,1,I) = CC(2,K,I,1) + TI2
              CR3 = TAUI* (CC(1,K,I,2)-CC(1,K,I,3))
              CI3 = TAUI* (CC(2,K,I,2)-CC(2,K,I,3))
              DR2 = CR2 - CI3
              DR3 = CR2 + CI3
              DI2 = CI2 + CR3
              DI3 = CI2 - CR3
              CH(2,K,2,I) = WA(I,1,1)*DI2 - WA(I,1,2)*DR2
              CH(1,K,2,I) = WA(I,1,1)*DR2 + WA(I,1,2)*DI2
              CH(2,K,3,I) = WA(I,2,1)*DI3 - WA(I,2,2)*DR3
              CH(1,K,3,I) = WA(I,2,1)*DR3 + WA(I,2,2)*DI3
  104     CONTINUE
  105 CONTINUE
      RETURN
      END
