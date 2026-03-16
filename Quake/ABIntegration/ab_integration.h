/*
 * AccelByte SDK Integration for QuakeSpasm
 * C wrapper for AccelByte C++ SDK
 */

#ifndef AB_INTEGRATION_H
#define AB_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Matchmaking status enum */
typedef enum {
    AB_MM_IDLE,
    AB_MM_SEARCHING,
    AB_MM_FOUND,
    AB_MM_JOINING,
    AB_MM_WAITING_FOR_DS,
    AB_MM_JOINED_AS_LEADER,
    AB_MM_JOINED_AS_CLIENT,
    AB_MM_HOSTING,
    AB_MM_CONNECTING,
    AB_MM_CANCELLED,
    AB_MM_ERROR
} ab_matchmake_status_t;

/* Login status enum */
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

/* Called on the main thread after a successful login.
 * user_id and display_name are valid only for the duration of the call. */
typedef void (*ab_login_success_callback_t)(const char* user_id, const char* display_name, void* userdata);

void ab_set_server_url   (ab_instance_t* instance, const char* url);
void ab_set_client_id    (ab_instance_t* instance, const char* id);
void ab_set_client_secret(ab_instance_t* instance, const char* secret);

void ab_login_with_device_id(ab_instance_t* instance, ab_login_success_callback_t on_success, void* userdata);
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
/*
 * Get error message if login failed
 * Returns NULL if no error
 */
const char* AB_GetErrorMessage(const ab_instance_t* instance);

/*
 * Update a user stat item value.
 * Calls AccelByte's update_user_stat_item_value_v2.
 * stat_code: the stat code identifier (e.g., "kills", "headshots")
 * value: the value to apply
 * strategy: 0 = OVERRIDE, 1 = INCREMENT, 2 = MAX, 3 = MIN
 */
void AB_UpdateUserStatItemValue(ab_instance_t* instance, const char* stat_code, float value, int strategy);

/*
 * Create a match ticket — starts searching for a match
 */
void AB_CreateMatchTicket(ab_instance_t* instance);

/*
 * Cancel the current match ticket
 */
void AB_CancelMatchTicket(ab_instance_t* instance);

/*
 * Get current matchmaking status
 */
ab_matchmake_status_t AB_GetMatchmakingStatus(const ab_instance_t* instance);

/*
 * Set matchmaking status (for menu use)
 */
void AB_SetMatchmakingStatus(ab_instance_t* instance, ab_matchmake_status_t status);

/*
 * Get matchmaking error message (NULL if no error)
 */
const char* AB_GetMatchmakingErrorMessage(const ab_instance_t* instance);

/*
 * Get match ticket ID (NULL if none)
 */
const char* AB_GetMatchTicketId(const ab_instance_t* instance);

/*
 * Get match ID after match found (NULL if none)
 */
const char* AB_GetMatchId(const ab_instance_t* instance);

/*
 * Get match pool name (NULL if none)
 */
const char* AB_GetMatchPoolName(const ab_instance_t* instance);

/*
 * Get number of players in the match
 */
int AB_GetMatchNumPlayers(const ab_instance_t* instance);

/*
 * Get number of teams in the match
 */
int AB_GetMatchNumTeams(const ab_instance_t* instance);

/*
 * Check if we are the session leader
 */
int AB_IsSessionLeader(const ab_instance_t* instance);

void ab_leaderboard_invalidate_cache    (ab_instance_t* instance);

/* -----------------------------------------------------------------------
 * Achievement API
 * ----------------------------------------------------------------------- */

/*
 * C-compatible struct describing a single achievement entry.
 * unlocked: 1 if the current user has unlocked it, 0 otherwise.
 */
typedef struct {
    char achievement_code[64];
    char name[256];
    char description[512];
    int unlocked;
} ab_achievement_t;

/*
 * Callback invoked on the main thread (via AB_Update) when
 * ab_achievement_query finishes.
 * achievements : pointer to an array of ab_achievement_t (may be NULL on error)
 * count        : number of elements in the array (0 on error)
 */
typedef void (*ab_achievements_callback_t)(const ab_achievement_t* achievements, int count);

/*
 * Callback invoked on the main thread when ab_achievement_unlock finishes.
 * success       : 1 on success, 0 on failure
 * error_message : human-readable error string (NULL on success)
 */
typedef void (*ab_unlock_achievement_callback_t)(int success, const char* error_message);

/*
 * Query all namespace achievements and the current user's unlock status.
 * Results are delivered asynchronously via on_done on the next AB_Update()
 * tick after the request completes.
 * The achievement list is cached after the first fetch; only user progress
 * is re-queried on subsequent calls.
 */
void ab_achievement_query(ab_instance_t* instance, ab_achievements_callback_t on_done);

/*
 * Unlock a specific achievement for the logged-in user.
 * on_done is called on the main thread via AB_Update() when the
 * request completes (success or failure).
 */
void ab_achievement_unlock(
    ab_instance_t* instance, const char* achievement_code, ab_unlock_achievement_callback_t on_done);

#ifdef __cplusplus
}
#endif

#endif /* AB_INTEGRATION_H */
