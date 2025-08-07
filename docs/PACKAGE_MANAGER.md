# Tesseract Package Manager (TPM)

A minimal package manager for Tesseract written in C. Packages are C source files that extend Tesseract's functionality.

## Building TPM

### Linux/macOS
```bash
make tpm
```

### Windows
```cmd
build_tpm.bat
```

## Usage

### Install a Package
```bash
./tpm install <package_name> <source_file.c>
```

Example:
```bash
./tpm install math_utils sample_package.c
```

### List Installed Packages
```bash
./tpm list
```

### Uninstall a Package
```bash
./tpm uninstall <package_name>
```

### Help
```bash
./tpm help
```