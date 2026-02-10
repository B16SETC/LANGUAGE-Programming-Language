#!/bin/bash
# Script to build LANGUAGE RPM package

echo "Building LANGUAGE RPM package..."

# Install required tools if not present
if ! command -v rpmbuild &> /dev/null; then
    echo "Installing rpm-build tools..."
    sudo dnf install -y rpm-build rpmdevtools
fi

# Setup RPM build environment
rpmdev-setuptree

# Get current directory and project name
CURRENT_DIR=$(pwd)
PROJECT_DIR=$(basename "$CURRENT_DIR")
VERSION="0.1.0"

# Create source tarball
echo "Creating source tarball..."
cd ..
tar --transform="s|^$PROJECT_DIR|LANGUAGE-${VERSION}|" \
    --exclude='*/build*' --exclude='*/.git' --exclude='*/cmake-build-*' \
    -czf LANGUAGE-${VERSION}.tar.gz "$PROJECT_DIR/"
mv LANGUAGE-${VERSION}.tar.gz ~/rpmbuild/SOURCES/

# Copy spec file
cd "$CURRENT_DIR"
cp LANGUAGE.spec ~/rpmbuild/SPECS/

# Build the RPM
echo "Building RPM..."
cd ~/rpmbuild/SPECS
rpmbuild -ba LANGUAGE.spec

echo ""
echo "Build complete!"
echo "RPM location: ~/rpmbuild/RPMS/x86_64/LANGUAGE-${VERSION}-1.*.rpm"
echo ""
echo "To install:"
echo "  sudo dnf install ~/rpmbuild/RPMS/x86_64/LANGUAGE-${VERSION}-1.*.rpm"
