#include <windows.h>
#include <stdint.h>

struct shim_ctx {
    HANDLE barrier;
    int (*proc)(void*);
    void* ctx;
};

static unsigned int WINAPI crt_thread_shim(void* outer_ctx)
{
    struct shim_ctx* sctx = outer_ctx;
    int (*proc)(void*);
    void* inner_ctx;

    proc = sctx->proc;
    inner_ctx = sctx->ctx;

    SetEvent(sctx->barrier);

    return proc(inner_ctx);
}

int crt_thread_create(
    int (*proc)(void*), void* ctx, uint32_t stack_sz, unsigned int priority)
{
    struct shim_ctx sctx;
    uintptr_t thread_id;

    sctx.barrier = CreateEvent(NULL, TRUE, FALSE, NULL);
    sctx.proc = proc;
    sctx.ctx = ctx;

    thread_id = _beginthreadex(NULL, stack_sz, crt_thread_shim, &sctx, 0, NULL);

    WaitForSingleObject(sctx.barrier, INFINITE);
    CloseHandle(sctx.barrier);

    return (int)thread_id;
}

void crt_thread_destroy(int thread_id)
{
    CloseHandle((HANDLE)(uintptr_t)thread_id);
}

void crt_thread_join(int thread_id, int* result)
{
    WaitForSingleObject((HANDLE)(uintptr_t)thread_id, INFINITE);

    if (result) {
        GetExitCodeThread((HANDLE)(uintptr_t)thread_id, (DWORD*)result);
    }
}