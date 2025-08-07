#include "package_manager.h"

void print_usage(const char *prog_name) {
    printf("Tesseract Package Manager (TPM)\n");
    printf("Usage: %s <command> [args]\n\n", prog_name);
    printf("Commands:\n");
    printf("  install <package_name> <source_file>  Install a package\n");
    printf("  uninstall <package_name>              Uninstall a package\n");
    printf("  list                                  List installed packages\n");
    printf("  help                                  Show this help\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    PackageManager *pm = pm_init("packages");
    if (!pm) {
        fprintf(stderr, "Failed to initialize package manager\n");
        return 1;
    }
    
    const char *command = argv[1];
    
    if (strcmp(command, "install") == 0) {
        if (argc != 4) {
            printf("Usage: %s install <package_name> <source_file>\n", argv[0]);
            pm_free(pm);
            return 1;
        }
        pm_install(pm, argv[2], argv[3]);
    }
    else if (strcmp(command, "uninstall") == 0) {
        if (argc != 3) {
            printf("Usage: %s uninstall <package_name>\n", argv[0]);
            pm_free(pm);
            return 1;
        }
        pm_uninstall(pm, argv[2]);
    }
    else if (strcmp(command, "list") == 0) {
        pm_list(pm);
    }
    else if (strcmp(command, "help") == 0) {
        print_usage(argv[0]);
    }
    else {
        printf("Unknown command: %s\n", command);
        print_usage(argv[0]);
        pm_free(pm);
        return 1;
    }
    
    pm_free(pm);
    return 0;
}