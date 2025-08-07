#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Package {
    char *name;
    char *version;
    char *path;
    struct Package *next;
} Package;

typedef struct {
    Package *packages;
    char *packages_dir;
} PackageManager;

// Core functions
PackageManager *pm_init(const char *packages_dir);
void pm_free(PackageManager *pm);
int pm_install(PackageManager *pm, const char *package_name, const char *source_path);
int pm_uninstall(PackageManager *pm, const char *package_name);
Package *pm_find(PackageManager *pm, const char *package_name);
void pm_list(PackageManager *pm);
int pm_load_registry(PackageManager *pm);
int pm_save_registry(PackageManager *pm);

#endif