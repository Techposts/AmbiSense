#pragma once

/*
 * AmbiSense v6 — 1-D Kalman filter for radar distance smoothing.
 *
 * State: x = [position_cm, velocity_cm_per_s]
 * Process model:  position_{k+1}   = position_k + velocity_k * dt
 *                 velocity_{k+1}   = velocity_k                          (constant velocity)
 * Observation:    z_k              = position_k + R noise
 *
 * Process noise:
 *   Q_pos drives "how much can position spontaneously wander between
 *   ticks" — small (~0.5 cm²/s) for steady targets.
 *   Q_vel drives "how aggressively the velocity can change". Big values
 *   make the filter snappier; small values make it calmer.
 *
 * Observation noise R:
 *   Trust the radar more when it reports high energy (target lock is
 *   strong) and less when energy is low. We multiply the base R by 4×
 *   when energy < 30 — empirically the radar's reported distance jumps
 *   most when it's struggling to lock.
 *
 * Direction hysteresis (3-sample agreement) and ±200 cm/s velocity
 * clamp are applied at the API boundary, not inside the filter math —
 * the filter itself stays a clean linear KF.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* State + covariance. */
    float x_pos;
    float x_vel;
    float P[2][2];

    /* Tunables. */
    float Q_pos;
    float Q_vel;
    float R_base;

    /* Derived: direction hysteresis state. dir_pending holds the
     * candidate direction sign; dir_agree counts consecutive samples
     * agreeing with the candidate. */
    int8_t  dir_committed;
    int8_t  dir_pending;
    uint8_t dir_agree;

    bool initialized;
} kalman_t;

/* Initialize at first observation; sets x_pos = z, x_vel = 0,
 * P = diag(R_base, 100). Tunables left untouched if non-zero. */
void kalman_reset(kalman_t *k, float z);

/* Set tunables. Pass 0 for any to keep current. */
void kalman_set_tunables(kalman_t *k, float Q_pos, float Q_vel, float R_base);

/* Run one predict+update step. Returns the smoothed position and writes
 * the velocity through *vel_out. `energy` 0-255 from radar; <30 widens
 * R to deweight low-confidence observations. dt in seconds, clamped
 * internally to (0.001, 1.0). */
float kalman_step(kalman_t *k, float z, uint8_t energy, float dt, float *vel_out);

/* Look ahead `predict_ms` from the latest filtered state; useful for the
 * UI's "look-ahead-ms" predict slider. Bounded to ±max_dist_cm shift. */
float kalman_predict_ahead(const kalman_t *k, float predict_ms);

/* Run direction hysteresis on the latest velocity estimate. Returns the
 * committed direction (-1, 0, +1). Three consecutive same-sign samples
 * are required to flip; below `vel_threshold_cm_s` direction is 0. */
int8_t kalman_direction(kalman_t *k, float vel, float vel_threshold_cm_s);

#ifdef __cplusplus
}
#endif
