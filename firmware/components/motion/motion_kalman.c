#include "motion_kalman.h"

#include <math.h>
#include <string.h>

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void kalman_set_tunables(kalman_t *k, float Q_pos, float Q_vel, float R_base) {
    if (Q_pos > 0) k->Q_pos = Q_pos;
    if (Q_vel > 0) k->Q_vel = Q_vel;
    if (R_base > 0) k->R_base = R_base;
}

void kalman_reset(kalman_t *k, float z) {
    k->x_pos = z;
    k->x_vel = 0.f;
    /* Initial covariance: high uncertainty in velocity (we don't know
     * which direction the target is moving), moderate in position
     * (radar gave us a measurement). 100 cm²/s² lets the filter learn
     * the velocity within ~5 samples; tighter than that overconstrains
     * the early estimates. */
    k->P[0][0] = (k->R_base > 0 ? k->R_base : 4.f);
    k->P[0][1] = 0.f;
    k->P[1][0] = 0.f;
    k->P[1][1] = 100.f;

    k->dir_committed = 0;
    k->dir_pending = 0;
    k->dir_agree = 0;
    k->initialized = true;
}

float kalman_step(kalman_t *k, float z, uint8_t energy, float dt, float *vel_out) {
    if (!k->initialized) kalman_reset(k, z);
    dt = clampf(dt, 0.001f, 1.0f);

    /* ---- Predict ---- */
    /* x = F x ; F = [[1, dt], [0, 1]] */
    float x0 = k->x_pos + k->x_vel * dt;
    float x1 = k->x_vel;

    /* P = F P F^T + Q;  Q = diag(Q_pos*dt, Q_vel*dt). The Q_pos*dt and
     * Q_vel*dt scaling makes the noise integral over the timestep, which
     * is what discrete KF theory requires. */
    float P00 = k->P[0][0] + dt * (k->P[1][0] + k->P[0][1]) + dt * dt * k->P[1][1] + k->Q_pos * dt;
    float P01 = k->P[0][1] + dt * k->P[1][1];
    float P10 = k->P[1][0] + dt * k->P[1][1];
    float P11 = k->P[1][1] + k->Q_vel * dt;

    /* ---- Update ---- */
    /* H = [1, 0] — we observe position only.
     * y = z - H x = z - x0
     * S = H P H^T + R = P00 + R
     * K = P H^T / S = [P00 / S; P10 / S]
     * x = x + K y
     * P = (I - K H) P
     */
    float R = k->R_base;
    if (energy < 30) R *= 4.f;   /* deweight low-confidence observations */
    if (R < 0.5f) R = 0.5f;

    float S = P00 + R;
    float K0 = P00 / S;
    float K1 = P10 / S;
    float y = z - x0;

    k->x_pos = x0 + K0 * y;
    k->x_vel = x1 + K1 * y;

    k->P[0][0] = (1.f - K0) * P00;
    k->P[0][1] = (1.f - K0) * P01;
    k->P[1][0] = P10 - K1 * P00;
    k->P[1][1] = P11 - K1 * P01;

    /* ---- Bound velocity (humans walking on stairs are <200 cm/s) ---- */
    k->x_vel = clampf(k->x_vel, -200.f, 200.f);

    if (vel_out) *vel_out = k->x_vel;
    return k->x_pos;
}

float kalman_predict_ahead(const kalman_t *k, float predict_ms) {
    return k->x_pos + k->x_vel * (predict_ms / 1000.f);
}

int8_t kalman_direction(kalman_t *k, float vel, float vel_threshold_cm_s) {
    int8_t sign = (vel > vel_threshold_cm_s) ? 1 :
                  (vel < -vel_threshold_cm_s) ? -1 : 0;
    if (sign == k->dir_committed) {
        k->dir_pending = 0;
        k->dir_agree = 0;
        return k->dir_committed;
    }
    /* Different from committed → require N consecutive ticks to flip.
     * The motion task ticks at ~50 Hz, so each agree count is ~20 ms.
     * Bumped from 3 (60 ms) to 10 (200 ms) on 2026-05-07 — at 3, a
     * brief velocity blip from breathing or jitter would flip the
     * chip; 200 ms requires the user to be genuinely moving for ~⅕ s
     * before the indicator commits. Combined with the 10 cm/s
     * velocity threshold (raised from 4 cm/s), this kills the rapid
     * still ↔ closer ↔ away oscillation users were seeing. */
    if (sign == k->dir_pending) {
        if (k->dir_agree < 255) k->dir_agree++;
    } else {
        k->dir_pending = sign;
        k->dir_agree = 1;
    }
    if (k->dir_agree >= 10) {
        k->dir_committed = k->dir_pending;
        k->dir_agree = 0;
    }
    return k->dir_committed;
}
