#include "../droid.h"
void droid_list_intents(const droid_bundle_t *bundle, FILE *fp) {
    char ** intents = manifest_get_all_intents(bundle->manifest);

    if (fp==stdout||fp==stderr)
        printf("listing intents:\n");
    for (size_t i=0; intents[i]; i++) {
        fprintf(fp, "%s\n", intents[i]);
        free(intents[i]);
    }
    free(intents);
}