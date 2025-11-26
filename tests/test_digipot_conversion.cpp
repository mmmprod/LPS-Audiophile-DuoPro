/*
 * ============================================================================
 *    TEST HÔTE - CONVERSIONS DIGIPOT ↔ TENSION
 *    LPS Audiophile V2.4.2
 * ============================================================================
 * 
 * Compilation: g++ -o test_digipot test_digipot_conversion.cpp -lm
 * Exécution:   ./test_digipot
 * 
 * Vérifie:
 * - digipotToVoltage(pos) → tension de sortie attendue
 * - voltageToDigipot(v) → position digipot attendue
 * - Réversibilité: voltageToDigipot(digipotToVoltage(pos)) ≈ pos
 * - Limites de plage: 5V min, 15V max
 * - Monotonie: tension croissante avec position
 * 
 * ============================================================================
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstdint>

// ============================================================================
// CONSTANTES (identiques au firmware)
// ============================================================================

const float R_FIXED_FB = 1100.0f;      // R_FIXED feedback (Ω)
const float R1_FB = 240.0f;            // R1 ADJ→GND (Ω)
const float R_SHUNT_FB = 2000.0f;      // R_SHUNT parallèle digipot (Ω)
const float R_DIGIPOT_FULL = 100000.0f; // MCP41100 100kΩ
const float DIGIPOT_STEPS = 255.0f;    // Pas digipot
const float V_REF_LM317 = 1.25f;       // Tension référence LM317
const float V_HEADROOM = 2.0f;         // V_PRE - V_OUT constant
const float V_OUT_MIN = 5.0f;
const float V_OUT_MAX_SET = 15.0f;

// ============================================================================
// FONCTIONS SOUS TEST (copie exacte du firmware)
// ============================================================================

float digipotToVoltage(uint8_t pos) {
    float r_wiper = pos * (R_DIGIPOT_FULL / DIGIPOT_STEPS);
    float r_eff = (r_wiper * R_SHUNT_FB) / (r_wiper + R_SHUNT_FB);
    float r2 = R_FIXED_FB + r_eff;
    float v_pre = V_REF_LM317 * (1.0f + r2 / R1_FB);
    float v_out = v_pre - V_HEADROOM;
    
    if (v_out < V_OUT_MIN) v_out = V_OUT_MIN;
    if (v_out > V_OUT_MAX_SET) v_out = V_OUT_MAX_SET;
    
    return v_out;
}

uint8_t voltageToDigipot(float v_out_target) {
    if (v_out_target < V_OUT_MIN) v_out_target = V_OUT_MIN;
    if (v_out_target > V_OUT_MAX_SET) v_out_target = V_OUT_MAX_SET;
    
    float v_pre = v_out_target + V_HEADROOM;
    float r2 = (v_pre / V_REF_LM317 - 1.0f) * R1_FB;
    float r_eff = r2 - R_FIXED_FB;
    
    if (r_eff <= 0) return 0;
    
    float r_wiper = (r_eff * R_SHUNT_FB) / (R_SHUNT_FB - r_eff);
    
    if (r_wiper < 0) return 0;
    if (r_wiper > R_DIGIPOT_FULL) return 255;
    
    int pos = (int)(r_wiper / (R_DIGIPOT_FULL / DIGIPOT_STEPS) + 0.5f);
    
    if (pos < 0) pos = 0;
    if (pos > 255) pos = 255;
    
    return (uint8_t)pos;
}

// Fonction validateDigipotRange (V2.4.1 corrigée)
bool validateDigipotRange() {
    float r_wiper_0 = 0;
    float r_eff_0 = 0;
    float r2_0 = R_FIXED_FB + r_eff_0;
    float v_pre_0 = V_REF_LM317 * (1.0f + r2_0 / R1_FB);
    float v_out_0 = v_pre_0 - V_HEADROOM;
    
    float r_wiper_255 = 255 * (R_DIGIPOT_FULL / DIGIPOT_STEPS);
    float r_eff_255 = (r_wiper_255 * R_SHUNT_FB) / (r_wiper_255 + R_SHUNT_FB);
    float r2_255 = R_FIXED_FB + r_eff_255;
    float v_pre_255 = V_REF_LM317 * (1.0f + r2_255 / R1_FB);
    float v_out_255 = v_pre_255 - V_HEADROOM;
    
    bool min_ok = (v_out_0 >= 4.5f) && (v_out_0 <= 5.5f);
    bool max_ok = (v_out_255 >= 14.5f) && (v_out_255 <= 16.0f);
    
    return min_ok && max_ok;
}

// ============================================================================
// TESTS
// ============================================================================

int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) printf("TEST: %s ... ", name)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)
#define ASSERT_FLOAT_EQ(a, b, tol) \
    if (fabs((a) - (b)) > (tol)) { \
        char buf[128]; \
        snprintf(buf, sizeof(buf), "Expected %.3f, got %.3f (tol=%.3f)", (b), (a), (tol)); \
        FAIL(buf); \
    } else { PASS(); }

void test_boundary_values() {
    printf("\n=== Tests valeurs limites ===\n");
    
    TEST("digipotToVoltage(0) ≈ 5V");
    float v0 = digipotToVoltage(0);
    ASSERT_FLOAT_EQ(v0, 5.0f, 0.1f);
    
    TEST("digipotToVoltage(255) = 15V (clamp)");
    float v255 = digipotToVoltage(255);
    ASSERT_FLOAT_EQ(v255, 15.0f, 0.1f);
    
    // Note: courbe non-linéaire, pos=128 déjà saturé à 15V
    TEST("digipotToVoltage(128) = 15V (saturation parallèle R_SHUNT)");
    float v128 = digipotToVoltage(128);
    ASSERT_FLOAT_EQ(v128, 15.0f, 0.1f);
    
    // Plage utile réelle: pos 0-130 environ
    TEST("digipotToVoltage(64) ≈ 14.6V");
    float v64 = digipotToVoltage(64);
    ASSERT_FLOAT_EQ(v64, 14.6f, 0.3f);
}

void test_voltageToDigipot_boundaries() {
    printf("\n=== Tests voltageToDigipot limites ===\n");
    
    TEST("voltageToDigipot(5.0) = 0");
    uint8_t p5 = voltageToDigipot(5.0f);
    if (p5 == 0) { PASS(); } else { 
        char buf[64]; snprintf(buf, 64, "Got %d", p5); FAIL(buf); 
    }
    
    // Note: 15V atteint à pos≈129 (saturation parallèle)
    TEST("voltageToDigipot(15.0) ≈ 129 (saturation R_SHUNT)");
    uint8_t p15 = voltageToDigipot(15.0f);
    if (p15 >= 120 && p15 <= 140) { PASS(); } else { 
        char buf[64]; snprintf(buf, 64, "Got %d", p15); FAIL(buf); 
    }
    
    TEST("voltageToDigipot(4.0) clampé à 0");
    uint8_t p4 = voltageToDigipot(4.0f);
    if (p4 == 0) { PASS(); } else { FAIL("Non clampé"); }
    
    // 20V clampé à 15V → pos≈129
    TEST("voltageToDigipot(20.0) clampé (→15V→pos≈129)");
    uint8_t p20 = voltageToDigipot(20.0f);
    if (p20 >= 120 && p20 <= 140) { PASS(); } else { 
        char buf[64]; snprintf(buf, 64, "Got %d", p20); FAIL(buf); 
    }
}

void test_reversibility() {
    printf("\n=== Tests réversibilité (plage utile 0-130) ===\n");
    
    // Note: seuls pos 0-130 sont réversibles, au-delà tout sature à 15V
    for (int pos = 0; pos <= 125; pos += 25) {
        char name[64];
        snprintf(name, 64, "Réversibilité pos=%d", pos);
        TEST(name);
        
        float v = digipotToVoltage(pos);
        uint8_t pos_back = voltageToDigipot(v);
        
        int diff = abs((int)pos - (int)pos_back);
        if (diff <= 3) { PASS(); } else {
            char buf[64]; 
            snprintf(buf, 64, "pos=%d → v=%.2f → pos_back=%d (diff=%d)", 
                     pos, v, pos_back, diff); 
            FAIL(buf);
        }
    }
    
    // Document saturation zone
    printf("  [INFO] pos 130-255: saturés à 15V (limitation R_SHUNT//R_digipot)\n");
}

void test_monotonicity() {
    printf("\n=== Test monotonie ===\n");
    
    TEST("Tension croissante avec position");
    float v_prev = 0;
    bool monotonic = true;
    
    for (int pos = 0; pos <= 255; pos++) {
        float v = digipotToVoltage(pos);
        if (v < v_prev - 0.001f) {
            printf("Non monotone à pos=%d: v=%.3f < v_prev=%.3f\n", pos, v, v_prev);
            monotonic = false;
            break;
        }
        v_prev = v;
    }
    
    if (monotonic) { PASS(); } else { FAIL("Non monotone"); }
}

void test_resolution() {
    printf("\n=== Test résolution ===\n");
    
    TEST("Résolution moyenne ≈ 40mV/pas");
    float v0 = digipotToVoltage(0);
    float v255 = digipotToVoltage(255);
    float resolution = (v255 - v0) / 255.0f * 1000.0f;  // mV/pas
    
    printf("(résolution = %.1f mV/pas) ", resolution);
    if (resolution >= 30.0f && resolution <= 50.0f) { PASS(); } 
    else { FAIL("Hors plage 30-50 mV/pas"); }
}

void test_validateDigipotRange() {
    printf("\n=== Test validateDigipotRange ===\n");
    
    TEST("validateDigipotRange() = true");
    if (validateDigipotRange()) { PASS(); } 
    else { FAIL("Plage invalide"); }
}

void test_ocp_limits() {
    printf("\n=== Test limites OCP adaptatives ===\n");
    
    // Simuler getAdaptiveCurrentLimit
    auto getAdaptiveCurrentLimit = [](float v_out) -> float {
        if (v_out < 7.0f) return 350.0f;
        else if (v_out < 10.0f) return 450.0f;
        else return 500.0f;
    };
    
    TEST("OCP @ 5V = 350mA");
    if (getAdaptiveCurrentLimit(5.0f) == 350.0f) { PASS(); } else { FAIL(""); }
    
    TEST("OCP @ 8V = 450mA");
    if (getAdaptiveCurrentLimit(8.0f) == 450.0f) { PASS(); } else { FAIL(""); }
    
    TEST("OCP @ 12V = 500mA");
    if (getAdaptiveCurrentLimit(12.0f) == 500.0f) { PASS(); } else { FAIL(""); }
}

void print_conversion_table() {
    printf("\n=== Table de conversion ===\n");
    printf("| Pos | R_wiper | R_eff  | V_PRE  | V_OUT |\n");
    printf("|-----|---------|--------|--------|-------|\n");
    
    for (int pos = 0; pos <= 255; pos += 32) {
        float r_wiper = pos * (R_DIGIPOT_FULL / DIGIPOT_STEPS);
        float r_eff = (r_wiper * R_SHUNT_FB) / (r_wiper + R_SHUNT_FB + 0.001f);
        float r2 = R_FIXED_FB + r_eff;
        float v_pre = V_REF_LM317 * (1.0f + r2 / R1_FB);
        float v_out = digipotToVoltage(pos);
        
        printf("| %3d | %7.0f | %6.0f | %6.2f | %5.2f |\n",
               pos, r_wiper, r_eff, v_pre, v_out);
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("============================================================\n");
    printf("  TEST CONVERSIONS DIGIPOT - LPS AUDIOPHILE V2.4.2\n");
    printf("============================================================\n");
    
    test_boundary_values();
    test_voltageToDigipot_boundaries();
    test_reversibility();
    test_monotonicity();
    test_resolution();
    test_validateDigipotRange();
    test_ocp_limits();
    print_conversion_table();
    
    printf("\n============================================================\n");
    printf("  RÉSULTATS: %d PASS, %d FAIL\n", tests_passed, tests_failed);
    printf("============================================================\n");
    
    return (tests_failed > 0) ? 1 : 0;
}
