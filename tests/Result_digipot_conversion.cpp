============================================================
  TEST CONVERSIONS DIGIPOT - LPS AUDIOPHILE V2.4.2
============================================================

=== Tests valeurs limites ===
TEST: digipotToVoltage(0) Ôëê 5V ... PASS
TEST: digipotToVoltage(255) = 15V (clamp) ... PASS
TEST: digipotToVoltage(128) = 15V (saturation parall├¿le R_SHUNT) ... PASS
TEST: digipotToVoltage(64) Ôëê 14.6V ... PASS

=== Tests voltageToDigipot limites ===
TEST: voltageToDigipot(5.0) = 0 ... PASS
TEST: voltageToDigipot(15.0) Ôëê 129 (saturation R_SHUNT) ... PASS
TEST: voltageToDigipot(4.0) clamp├® ├á 0 ... PASS
TEST: voltageToDigipot(20.0) clamp├® (ÔåÆ15VÔåÆposÔëê129) ... PASS

=== Tests r├®versibilit├® (plage utile 0-130) ===
TEST: R├®versibilit├® pos=0 ... PASS
TEST: R├®versibilit├® pos=25 ... PASS
TEST: R├®versibilit├® pos=50 ... PASS
TEST: R├®versibilit├® pos=75 ... PASS
TEST: R├®versibilit├® pos=100 ... PASS
TEST: R├®versibilit├® pos=125 ... PASS
  [INFO] pos 130-255: satur├®s ├á 15V (limitation R_SHUNT//R_digipot)

=== Test monotonie ===
TEST: Tension croissante avec position ... PASS

=== Test r├®solution ===
TEST: R├®solution moyenne Ôëê 40mV/pas ... (r├®solution = 39.2 mV/pas) PASS

=== Test validateDigipotRange ===
TEST: validateDigipotRange() = true ... PASS

=== Test limites OCP adaptatives ===
TEST: OCP @ 5V = 350mA ... PASS
TEST: OCP @ 8V = 450mA ... PASS
TEST: OCP @ 12V = 500mA ... PASS

=== Table de conversion ===
| Pos | R_wiper | R_eff  | V_PRE  | V_OUT |
|-----|---------|--------|--------|-------|
|   0 |       0 |      0 |   6.98 |  5.00 |
|  32 |   12549 |   1725 |  15.96 | 13.96 |
|  64 |   25098 |   1852 |  16.63 | 14.63 |
|  96 |   37647 |   1899 |  16.87 | 14.87 |
| 128 |   50196 |   1923 |  17.00 | 15.00 |
| 160 |   62745 |   1938 |  17.07 | 15.00 |
| 192 |   75294 |   1948 |  17.13 | 15.00 |
| 224 |   87843 |   1955 |  17.16 | 15.00 |

============================================================
  R├ëSULTATS: 20 PASS, 0 FAIL
============================================================
