#include <windows.h>

typedef void (*log_formatter_t)(const char* module, const char* fmt, ...);

typedef int (*thread_create_t)(
    int (*proc)(void*), void* ctx, uint32_t stack_sz, unsigned int priority);
typedef void (*thread_join_t)(int thread_id, int* result);
typedef void (*thread_destroy_t)(int thread_id);

#ifdef __cplusplus
extern "C" {
#endif

int crt_thread_create(int (*proc)(void*), void* ctx, uint32_t stack_sz, unsigned int priority);
void crt_thread_join(int thread_id, int* result);
void crt_thread_destroy(int thread_id);

#ifdef __cplusplus
}
#endif