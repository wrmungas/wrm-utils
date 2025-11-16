#include "wrm/common.h"

/* Internal helper: prints an error message formatted like printf, supplied via explicit varargs */
void wrm_va_error(const char *module, const char *function, const char *format, va_list args);

/*
Function definitions
*/
void wrm_error(const char *module, const char *function, const char *format, ... )
{
    va_list args;
    va_start(args, format);
    wrm_va_error(module, function, format, args);
    va_end(args);
}

void wrm_fail(u8 code, const char *module, const char *function, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    wrm_va_error(module, function, format, args);
    va_end(args);

    exit(code);
}

bool wrm_cstrn_eq(size_t n, const char *str1, const char *str2) 
{
    return strncmp(str1, str2, n) == 0;
}

int wrm_cstr_match(const char *src, const char **opts, size_t n_opts)
{
    for(u8 i = 0; i < n_opts; i++) {
        if(strcmp(src, opts[i]) == 0) {
            return i + 1;
        }
    }
    return 0;
}

int wrm_cstrn_match(size_t len, const char *src, const char **opts, size_t n_opts)
{
    for(u8 i = 0; i < n_opts; i++) {
        if(wrm_cstrn_eq(len, src, opts[i])) {
            return i + 1;
        }
    }
    return 0;
}

void wrm_va_error(const char *module, const char *function, const char *format, va_list args)
{
    fprintf(stderr, "ERROR: %s: %s: ", module, function);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
}

char *wrm_readFile(const char *path)
{
    FILE *fp = fopen(path, "r");
    if(!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long bytes = ftell(fp);
    if(bytes < 1) {
        fclose(fp); 
        return NULL;
    }

    char *dest = malloc(bytes + 1);
    if(!dest) {
        fclose(fp);
        return NULL;
    }

    rewind(fp);
    size_t copied = fread(dest, 1, bytes, fp);
    if(copied != bytes) {
        fclose(fp);
        free(dest);
        return NULL;
    }

    dest[bytes] = '\0';
    fclose(fp);
    return dest;
}