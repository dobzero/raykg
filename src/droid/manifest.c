#include "libaxml/axml_document.h"
#include "libaxml/axml_xml.h"
#include "core/zip_io.h"
#include "manifest.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>


manifest_t * manifest_from_archive(zip_t *z) {
    zip_stat_t stat;
    zip_stat(z, "AndroidManifest.xml", 0, &stat);

    if (!stat.size)
        return nullptr;

    manifest_t * man = calloc(1, sizeof(manifest_t));
    man->origin_file = malloc(stat.size);
    man->origin_size=stat.size;


    zip_file_t * m = zip_fopen(z, "AndroidManifest.xml", 0);

    char package_name[1000]={0};
    zip_get_pattern_from_file(z, "AndroidManifest.xml", package_name, "^[a-z_][a-z0-9_]*(\\.[a-z_][a-z0-9_]*)*$");
    man->is_xml_binary=strlen(package_name)==0;

    if (zip_fread(m, man->origin_file, stat.size) != stat.size) {

    }
    if (!man->is_xml_binary) {
        man->plain_file=man->origin_file;
    }

    zip_fclose(m);

    return man;
}

static const char* skip_whitespace(const char* ptr) {
    while (*ptr && isspace((unsigned char)*ptr)) {
        ptr++;
    }
    return ptr;
}

static void manifest_xml_get_attrs(const char* xml_string, const char* target_tag, char **attr_nval_list) {
    const char* ptr = xml_string;
    const size_t target_len = strlen(target_tag);

    char ** attr_val = attr_nval_list;
    while ((ptr = strchr(ptr, '<')) != nullptr) {
        ptr++;
        if (*ptr == '/' || *ptr == '!' || *ptr == '?') {
            continue;
        }
        if (strncmp(ptr, target_tag, target_len) == 0 && (isspace((unsigned char)ptr[target_len]) || ptr[target_len] == '>' || ptr[target_len] == '/')) {
            ptr += target_len;
            while (*ptr && *ptr != '>' && *ptr != '/') {
                ptr = skip_whitespace(ptr);

                if (*ptr == '>' || *ptr == '/' || *ptr == '\0') {
                    break;
                }
                char attr_name[64] = {0};
                int i = 0;
                while (*ptr && !isspace((unsigned char)*ptr) && *ptr != '=' && *ptr != '>' && *ptr != '/') {
                    if (i < 63) attr_name[i++] = *ptr;
                    ptr++;
                }

                ptr = skip_whitespace(ptr);
                if (*ptr != '=') {
                    continue;
                }
                ptr++;
                ptr = skip_whitespace(ptr);
                char quote_type = '\0';
                if (*ptr == '"' || *ptr == '\'') {
                    quote_type = *ptr;
                    ptr++;
                } else {
                    continue;
                }
                char attr_value[128] = {0};
                int j = 0;
                while (*ptr && *ptr != quote_type) {
                    if (j < 127) attr_value[j++] = *ptr;
                    ptr++;
                }

                if (*ptr == quote_type) {
                    ptr++;
                }

                *attr_val++=strdup(attr_name);
                *attr_val++=strdup(attr_value);
            }
        }
    }
}

char * manifest_get(const manifest_t *man, const char *ns_list) {
    const char * package = strstr(man->plain_file, ns_list);
    const char *begin = strchr(package, '\"')+1;
    const char * end = strchr(begin, '\"');


    char * value = calloc(1, end-begin+1);
    strncpy(value, begin, end-begin);
    return value;
}

char ** manifest_get_all_intents(const manifest_t *man) {
    char ** intent = calloc(sizeof(char*), 100);
    manifest_xml_get_attrs(man->plain_file, "action", intent);

    char **intent_filter = calloc(sizeof(char*), 50);
    for (size_t i=0,inf=0;i<100&&intent[i];i+=2) {
        if (strcmp(intent[i], "android:name")==0)
            intent_filter[inf++]=intent[i+1];
        free(intent[i]);
    }
    free(intent);

    return intent_filter;
}

void manifest_prepare(manifest_t *man) {
    if (!man->is_xml_binary)
        return;

    man->plain_file = calloc(1, man->origin_size*3);
    FILE * fp = fmemopen(man->plain_file, man->origin_size*3, "w");
    manifest_write(man, fp);

    fclose(fp);

}

void manifest_write(const manifest_t *man, FILE *fp) {
    axml_document_t binary_xml;
    axml_document_init(&binary_xml);
    axml_document_decode(man->origin_file, man->origin_size, &binary_xml);

    axml_document_write_xml(&binary_xml, fp);
    axml_document_free(&binary_xml);
}

void manifest_destroy(manifest_t *man) {
    if (man->plain_file&&
            man->plain_file!=man->origin_file)
        free(man->plain_file);
    free(man->origin_file);
    free(man);
}
