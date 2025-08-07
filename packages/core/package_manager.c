#include "package_manager.h"
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

PackageManager *pm_init(const char *packages_dir) {
    PackageManager *pm = malloc(sizeof(PackageManager));
    if (!pm) return NULL;
    
    pm->packages = NULL;
    pm->packages_dir = malloc(strlen(packages_dir) + 1);
    strcpy(pm->packages_dir, packages_dir);
    
    mkdir(packages_dir, 0755);
    pm_load_registry(pm);
    return pm;
}

void pm_free(PackageManager *pm) {
    if (!pm) return;
    
    Package *current = pm->packages;
    while (current) {
        Package *next = current->next;
        free(current->name);
        free(current->version);
        free(current->path);
        free(current);
        current = next;
    }
    
    free(pm->packages_dir);
    free(pm);
}

int pm_install(PackageManager *pm, const char *package_name, const char *source_path) {
    if (pm_find(pm, package_name)) {
        printf("Package '%s' already installed\n", package_name);
        return 0;
    }
    
    char dest_path[512];
    snprintf(dest_path, sizeof(dest_path), "%s/%s.c", pm->packages_dir, package_name);
    
    FILE *src = fopen(source_path, "rb");
    FILE *dst = fopen(dest_path, "wb");
    
    if (!src || !dst) {
        if (src) fclose(src);
        if (dst) fclose(dst);
        return -1;
    }
    
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }
    
    fclose(src);
    fclose(dst);
    
    Package *pkg = malloc(sizeof(Package));
    pkg->name = malloc(strlen(package_name) + 1);
    strcpy(pkg->name, package_name);
    pkg->version = malloc(6);
    strcpy(pkg->version, "1.0.0");
    pkg->path = malloc(strlen(dest_path) + 1);
    strcpy(pkg->path, dest_path);
    pkg->next = pm->packages;
    pm->packages = pkg;
    
    pm_save_registry(pm);
    printf("Package '%s' installed successfully\n", package_name);
    return 0;
}

int pm_uninstall(PackageManager *pm, const char *package_name) {
    Package **current = &pm->packages;
    
    while (*current) {
        if (strcmp((*current)->name, package_name) == 0) {
            Package *to_remove = *current;
            *current = (*current)->next;
            
            remove(to_remove->path);
            
            free(to_remove->name);
            free(to_remove->version);
            free(to_remove->path);
            free(to_remove);
            
            pm_save_registry(pm);
            printf("Package '%s' uninstalled\n", package_name);
            return 0;
        }
        current = &(*current)->next;
    }
    
    printf("Package '%s' not found\n", package_name);
    return -1;
}

Package *pm_find(PackageManager *pm, const char *package_name) {
    Package *current = pm->packages;
    while (current) {
        if (strcmp(current->name, package_name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void pm_list(PackageManager *pm) {
    Package *current = pm->packages;
    if (!current) {
        printf("No packages installed\n");
        return;
    }
    
    printf("Installed packages:\n");
    while (current) {
        printf("  %s v%s\n", current->name, current->version);
        current = current->next;
    }
}

int pm_load_registry(PackageManager *pm) {
    char registry_path[512];
    snprintf(registry_path, sizeof(registry_path), "%s/registry.txt", pm->packages_dir);
    
    FILE *f = fopen(registry_path, "r");
    if (!f) return 0;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *name = strtok(line, "|");
        char *version = strtok(NULL, "|");
        char *path = strtok(NULL, "|\n");
        
        if (name && version && path) {
            Package *pkg = malloc(sizeof(Package));
            pkg->name = malloc(strlen(name) + 1);
            strcpy(pkg->name, name);
            pkg->version = malloc(strlen(version) + 1);
            strcpy(pkg->version, version);
            pkg->path = malloc(strlen(path) + 1);
            strcpy(pkg->path, path);
            pkg->next = pm->packages;
            pm->packages = pkg;
        }
    }
    
    fclose(f);
    return 0;
}

int pm_save_registry(PackageManager *pm) {
    char registry_path[512];
    snprintf(registry_path, sizeof(registry_path), "%s/registry.txt", pm->packages_dir);
    
    FILE *f = fopen(registry_path, "w");
    if (!f) return -1;
    
    Package *current = pm->packages;
    while (current) {
        fprintf(f, "%s|%s|%s\n", current->name, current->version, current->path);
        current = current->next;
    }
    
    fclose(f);
    return 0;
}