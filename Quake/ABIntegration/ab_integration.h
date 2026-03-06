/*
 * AccelByte SDK Integration for QuakeSpasm
 * C wrapper for AccelByte C++ SDK
 */

#ifndef AB_INTEGRATION_H
#define AB_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AB_LOGIN_IDLE,
    AB_LOGIN_IN_PROGRESS,
    AB_LOGIN_QUEUED,
    AB_LOGIN_SUCCESS,
    AB_LOGIN_FAILED
} ab_login_status_t;

typedef struct ab_instance_t ab_instance_t;

ab_instance_t* ab_create(void);
void           ab_destroy(ab_instance_t* instance);

void ab_set_server_url   (ab_instance_t* instance, const char* url);
void ab_set_client_id    (ab_instance_t* instance, const char* id);
void ab_set_client_secret(ab_instance_t* instance, const char* secret);

void ab_login_with_device_id(ab_instance_t* instance);
void ab_update              (ab_instance_t* instance);

ab_login_status_t ab_get_login_status  (const ab_instance_t* instance);
const char*       ab_get_user_id       (const ab_instance_t* instance);
const char*       ab_get_display_name  (const ab_instance_t* instance);
const char*       ab_get_error_message (const ab_instance_t* instance);

void ab_update_user_stat(ab_instance_t* instance, const char* stat_code, float value, int strategy);

typedef enum {
    AB_STAT_STRATEGY_OVERRIDE  = 0,
    AB_STAT_STRATEGY_INCREMENT = 1,
    AB_STAT_STRATEGY_MAX       = 2,
    AB_STAT_STRATEGY_MIN       = 3
} ab_stat_strategy_t;

void ab_stat_update          (ab_instance_t* instance, const char* stat_code, float value, ab_stat_strategy_t strategy);
void ab_stat_fetch           (ab_instance_t* instance, const char* const* stat_codes, int count);
void ab_stat_bulk_update     (ab_instance_t* instance, const char* const* stat_codes, const float* values, int count, ab_stat_strategy_t strategy);
int  ab_stat_get_cached      (const ab_instance_t* instance, const char* stat_code, float* out_value);
void ab_stat_invalidate_cache(ab_instance_t* instance);

void ab_cycle_fetch_items     (ab_instance_t* instance, const char* cycle_id,
                                const char* const* stat_codes, int count);
int  ab_cycle_get_cached      (const ab_instance_t* instance, const char* cycle_id,
                                const char* stat_code, float* out_value);
void ab_cycle_invalidate_cache(ab_instance_t* instance);

/* Rank entry returned from the leaderboard cache.
 * user_id is NUL-terminated and truncated to 63 chars if the server ID is longer. */
typedef struct {
    char  user_id[64];
    float point;
    int   rank;        /* 1-based position in the ranking list */
} ab_rank_entry_t;

void ab_leaderboard_fetch_rankings      (ab_instance_t* instance,
                                         const char* leaderboard_code,
                                         int limit, int offset);
void ab_leaderboard_fetch_cycle_rankings(ab_instance_t* instance,
                                         const char* leaderboard_code,
                                         const char* cycle_id,
                                         int limit, int offset);
void ab_leaderboard_fetch_user_rank     (ab_instance_t* instance,
                                         const char* leaderboard_code);

int  ab_leaderboard_get_rankings        (const ab_instance_t* instance,
                                         const char* leaderboard_code,
                                         ab_rank_entry_t* out, int max_entries);
int  ab_leaderboard_get_cycle_rankings  (const ab_instance_t* instance,
                                         const char* leaderboard_code,
                                         const char* cycle_id,
                                         ab_rank_entry_t* out, int max_entries);

int  ab_leaderboard_get_user_rank       (const ab_instance_t* instance,
                                         const char* leaderboard_code,
                                         long* out_rank, float* out_point);
int  ab_leaderboard_get_user_cycle_rank (const ab_instance_t* instance,
                                         const char* leaderboard_code,
                                         const char* cycle_id,
                                         long* out_rank, float* out_point);

void ab_leaderboard_invalidate_cache    (ab_instance_t* instance);

#ifdef __cplusplus
}
#endif

#endif /* AB_INTEGRATION_H */
