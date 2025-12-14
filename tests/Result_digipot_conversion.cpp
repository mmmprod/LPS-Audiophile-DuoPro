============================================================
  DIGIPOT CONVERSION TESTS - LPS AUDIOPHILE V2.4.2
============================================================

=== Boundary tests ===
TEST: digipotToVoltage(0) ≈ 5V ... PASS
TEST: digipotToVoltage(255) = 15V (clamp) ... PASS
TEST: digipotToVoltage(128) = 15V (R_SHUNT parallel saturation) ... PASS
TEST: digipotToVoltage(64) ≈ 14.6V ... PASS

=== voltageToDigipot limits ===
TEST: voltageToDigipot(5.0) = 0 ... PASS
TEST: voltageToDigipot(15.0) ≈ 129 (R_SHUNT saturation) ... PASS
TEST: voltageToDigipot(4.0) clamped to 0 ... PASS
TEST: voltageToDigipot(20.0) clamped (→15V→pos≈129) ... PASS

=== Reversibility tests (useful range 0-130) ===
TEST: Reversibility pos=0 ... PASS
TEST: Reversibility pos=25 ... PASS
TEST: Reversibility pos=50 ... PASS
TEST: Reversibility pos=75 ... PASS
TEST: Reversibility pos=100 ... PASS
TEST: Reversibility pos=125 ... PASS
  [INFO] pos 130-255: saturated at 15V (R_SHUNT//R_digipot limitation)

=== Monotonicity test ===
TEST: Voltage increases with position ... PASS

=== Resolution test ===
TEST: Average resolution ≈ 40mV/step ... (resolution = 39.2 mV/step) PASS

=== validateDigipotRange test ===
TEST: validateDigipotRange() = true ... PASS

=== Adaptive OCP limit test ===
TEST: OCP @ 5V = 350mA ... PASS
TEST: OCP @ 8V = 450mA ... PASS
TEST: OCP @ 12V = 500mA ... PASS

=== Conversion table ===
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
  RESULTS: 20 PASS, 0 FAIL
============================================================
