#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <windows.h>

enum {
    CHECK_SIMILAR,
    CHECK_EXACT
};

bool init(char *dir, char *target, int8_t *op_type, bool *case_sensitive, int argc, char **argv) {
    
    const char *short_opts = "d:t:cs";
    const struct option long_opts[] = {
        {"dir", required_argument, NULL, 'd'},
        {"target", required_argument, NULL, 't'},
        {"case_sensitive", no_argument, NULL, 'c'},
        {"similar_check", no_argument, NULL, 's'},
        {NULL, 0, NULL, 0}
    };

    *case_sensitive = false;
    *op_type = CHECK_EXACT;

    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (opt) {
            case 'd':
                strcpy(dir, optarg);
                break;
            case 't':
                strcpy(target, optarg);
                break;
            case 'c':
                *case_sensitive = true;
                break;
            case 's':
                *op_type = CHECK_SIMILAR;
                break;
            default:
                fprintf(stderr, "Usage: %s [-d directory] [-t target] [-c] [-s]\n", argv[0]);
                return false;
        }
    }

    if(strcmp(dir, "") == 0) {
        GetCurrentDirectory(2048, dir);
    }

    if(strcmp(target, "") == 0) {
        fprintf(stderr, "Usage: %s [-d directory] [-t target] [-c] [-s]\n", argv[0]);
    }

    return true;
}

bool search(const char *dir, const char *target, const bool *case_sensitive, const int8_t* op_type) {
    char sPath[2048];

    WIN32_FIND_DATA file;
    HANDLE handle = NULL;
    
    sprintf(sPath, "%s\\*.*", dir);

    if((handle = FindFirstFile(sPath, &file)) == INVALID_HANDLE_VALUE) {
        printf("ERR: Path not found: \"%s\"\n", dir);
        return false;
    }
    
    do {
        if(strcmp(file.cFileName, ".") == 0 || strcmp(file.cFileName, "..") == 0) {
            continue;
        }

        // printf("TEST: %s | %s\n", file.cFileName, target);
        sprintf(sPath, "%s\\%s", dir, file.cFileName);
        if(file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            search(sPath, target, case_sensitive, op_type);
        } else {
            char file_name[1024];

            if(*case_sensitive == true) {
                strcpy(file_name, file.cFileName);
            } else {
                strcpy(file_name, strlwr(file.cFileName));
            }

            if(*op_type == CHECK_SIMILAR) {
                if(strstr(file_name, target) != 0) {
                    printf("Found (similar) file at: %s\n", sPath);
                }
            } else {
                if(strcmp(file_name, target) == 0) {
                    printf("Found file at: %s\n", sPath);
                }
            }
        }
    } while(FindNextFile(handle, &file));

    FindClose(handle);
    return true;
}

int main(int argc, char **argv) {
    int8_t op_type;
    bool case_sensitive;
    char target[1024] = "";
    char dir[2048] = "";
    if(init(dir, target, &op_type, &case_sensitive, argc, argv) == false) {
        return 1;
    }

    if(case_sensitive == false) {
        strcpy(target, strlwr(target));
    }

    if(search(dir, target, &case_sensitive, &op_type) == false) {
        return 1;
    }
    return 0;
}