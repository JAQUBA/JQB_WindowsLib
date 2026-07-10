#pragma once

#include <Core.h>
#include <string>

struct WorkerContext {
    HWND notifyHwnd;
    volatile LONG cancelRequested;
    int progress;
    std::string result;
};

static DWORD WINAPI WorkerProc(LPVOID param) {
    WorkerContext* ctx = reinterpret_cast<WorkerContext*>(param);

    for (int i = 0; i <= 100; ++i) {
        if (InterlockedCompareExchange(&ctx->cancelRequested, 0, 0) != 0) {
            return 1;
        }
        ctx->progress = i;
        PostMessageW(ctx->notifyHwnd, WM_APP_WORKER_PROGRESS, (WPARAM)i, 0);
    }

    ctx->result = "ok";
    PostMessageW(ctx->notifyHwnd, WM_APP_WORKER_DONE, 0, 0);
    return 0;
}
