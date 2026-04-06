#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void        udp_init(void);
void        udp_snapshot(void);
void        udp_shutdown(void);
int         udp_connected(void);
const char *udp_status(void);
int         udp_get_room_user_count(void);

#ifdef __cplusplus
}
#endif
