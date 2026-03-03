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

#ifdef __cplusplus
}
#endif

#endif /* AB_INTEGRATION_H */
