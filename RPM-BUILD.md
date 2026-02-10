# Building LANGUAGE RPM Package

## Quick Start

1. **Make the build script executable:**
   ```bash
   chmod +x build-rpm.sh
   ```

2. **Run the build script:**
   ```bash
   ./build-rpm.sh
   ```

3. **Install the RPM:**
   ```bash
   sudo dnf install ~/rpmbuild/RPMS/x86_64/LANGUAGE-0.1.0-1.*.rpm
   ```

4. **Test it:**
   ```bash
   LANGUAGE --version
   LANGUAGE test.LANGUAGE
   ```

## Manual Steps (if you want to understand what's happening)

1. **Install RPM build tools:**
   ```bash
   sudo dnf install rpm-build rpmdevtools
   ```

2. **Setup RPM build directory structure:**
   ```bash
   rpmdev-setuptree
   ```
   This creates `~/rpmbuild/` with folders: BUILD, RPMS, SOURCES, SPECS, SRPMS

3. **Create source tarball:**
   ```bash
   cd ..
   tar --exclude='*/build*' --exclude='*/.git' -czf LANGUAGE-0.1.0.tar.gz LANGUAGE/
   mv LANGUAGE-0.1.0.tar.gz ~/rpmbuild/SOURCES/
   ```

4. **Copy spec file:**
   ```bash
   cp LANGUAGE.spec ~/rpmbuild/SPECS/
   ```

5. **Build the RPM:**
   ```bash
   cd ~/rpmbuild/SPECS
   rpmbuild -ba LANGUAGE.spec
   ```

6. **Find your RPM:**
   ```bash
   ls ~/rpmbuild/RPMS/x86_64/
   ```

## After Installing

Once installed, `LANGUAGE` will be in `/usr/bin/` and available system-wide:

```bash
LANGUAGE script.LANGUAGE
LANGUAGE --version
LANGUAGE --help
```

## Uninstalling

```bash
sudo dnf remove LANGUAGE
```

## Distributing

Share the RPM file from:
```
~/rpmbuild/RPMS/x86_64/LANGUAGE-0.1.0-1.fc*.x86_64.rpm
```

Users can install it with:
```bash
sudo dnf install ./LANGUAGE-0.1.0-1.fc*.x86_64.rpm
```
