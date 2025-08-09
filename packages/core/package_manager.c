#include "package_manager.h"
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

// Normalize path separators
void normalize_path(char *path) {
    for (char *p = path; *p; p++) {
        if (*p == '\\') *p = '/';
    }
}

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
    
    // Ensure stdlib directory exists
    char stdlib_dir[512];
    snprintf(stdlib_dir, sizeof(stdlib_dir), "%s/stdlib", pm->packages_dir);
    mkdir(stdlib_dir, 0755);
    
    char dest_path[512];
    snprintf(dest_path, sizeof(dest_path), "%s/stdlib/%s.c", pm->packages_dir, package_name);
    
    // Check if backup exists first
    char backup_path[512];
    snprintf(backup_path, sizeof(backup_path), "%s/.backup/%s.c", pm->packages_dir, package_name);
    
    FILE *src = fopen(source_path, "rb");
    int using_backup = 0;
    if (!src) {
        // Try backup if source not found
        src = fopen(backup_path, "rb");
        if (!src) {
            printf("Error: Cannot find source file '%s' or backup\n", source_path);
            return -1;
        }
        printf("Using backup file for '%s'\n", package_name);
        using_backup = 1;
    }
    
    FILE *dst = fopen(dest_path, "wb");
    
    if (!dst) {
        printf("Error: Cannot create destination file '%s'\n", dest_path);
        fclose(src);
        return -1;
    }
    
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }
    
    fclose(src);
    fclose(dst);
    
    // Verify the file was copied successfully
    struct stat st;
    if (stat(dest_path, &st) != 0 || st.st_size == 0) {
        printf("Error: Package installation failed - file is empty or missing\n");
        remove(dest_path);
        return -1;
    }
    
    Package *pkg = malloc(sizeof(Package));
    pkg->name = malloc(strlen(package_name) + 1);
    strcpy(pkg->name, package_name);
    pkg->version = malloc(6);
    strcpy(pkg->version, "1.0.0");
    pkg->path = malloc(strlen(dest_path) + 1);
    strcpy(pkg->path, dest_path);
    pkg->next = pm->packages;
    pm->packages = pkg;
    
    if (pm_save_registry(pm) != 0) {
        printf("Warning: Failed to update registry\n");
    }
    
    // Delete backup file if it was used
    if (using_backup) {
        remove(backup_path);
    }
    
    printf("Package '%s' installed successfully\n", package_name);
    return 0;
}

int pm_uninstall(PackageManager *pm, const char *package_name) {
    Package **current = &pm->packages;
    
    while (*current) {
        if (strcmp((*current)->name, package_name) == 0) {
            Package *to_remove = *current;
            *current = (*current)->next;
            
            // Move file to backup instead of deleting
            char backup_dir[512], backup_path[512];
            snprintf(backup_dir, sizeof(backup_dir), "%s/.backup", pm->packages_dir);
            mkdir(backup_dir, 0755);
            snprintf(backup_path, sizeof(backup_path), "%s/%s.c", backup_dir, package_name);
            
            char normalized_path[512];
            strcpy(normalized_path, to_remove->path);
            normalize_path(normalized_path);
            
            if (rename(normalized_path, backup_path) != 0) {
                printf("Warning: Could not backup file '%s'\n", normalized_path);
            }
            
            free(to_remove->name);
            free(to_remove->version);
            free(to_remove->path);
            free(to_remove);
            
            // Update registry
            if (pm_save_registry(pm) != 0) {
                printf("Warning: Failed to update registry\n");
            }
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
        // Remove Windows line endings
        char *newline = strchr(line, '\r');
        if (newline) *newline = '\0';
        newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        char *name = strtok(line, "|");
        char *version = strtok(NULL, "|");
        char *path = strtok(NULL, "|");
        
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
    if (!f) {
        printf("Error: Cannot open registry file for writing: %s\n", registry_path);
        return -1;
    }
    
    Package *current = pm->packages;
    while (current) {
        if (fprintf(f, "%s|%s|%s\n", current->name, current->version, current->path) < 0) {
            printf("Error: Failed to write to registry\n");
            fclose(f);
            return -1;
        }
        current = current->next;
    }
    
    if (fflush(f) != 0 || fclose(f) != 0) {
        printf("Error: Failed to save registry file\n");
        return -1;
    }
    return 0;
}

void pm_functions(PackageManager *pm, const char *package_name) {
    Package *pkg = pm_find(pm, package_name);
    if (!pkg) {
        printf("Package '%s' not found\n", package_name);
        return;
    }
    
    FILE *f = fopen(pkg->path, "r");
    if (!f) {
        printf("Error: Cannot open package file '%s'\n", pkg->path);
        return;
    }
    
    printf("Functions in package '%s':\n", package_name);
    
    char line[512];
    int found_functions = 0;
    
    while (fgets(line, sizeof(line), f)) {
        // Look for function definitions starting with "ASTNode *tesseract_"
        if (strstr(line, "ASTNode *tesseract_") == line) {
            // Extract function name
            char *start = line + strlen("ASTNode *tesseract_");
            char *end = strchr(start, '(');
            if (end) {
                *end = '\0';
                printf("  %s\n", start);
                found_functions = 1;
            }
        }
    }
    
    if (!found_functions) {
        printf("  No functions found in this package\n");
    }
    
    fclose(f);
}