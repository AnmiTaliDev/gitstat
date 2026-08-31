#include "gitstat/app/context.h"
#include "gitstat/app/app.h"

int main(int argc, char **argv) {
    gitstat_context_t ctx;
    gitstat_context_init(&ctx);

    gitstat_error_t err = gitstat_context_bootstrap(&ctx, argc, argv);
    if (err != GITSTAT_OK) {
        gitstat_context_free(&ctx);
        return 1;
    }

    int result = gitstat_app_run(&ctx);

    gitstat_context_free(&ctx);
    return result;
}
