#!/bin/bash
# Quick Setup Script for macOS/Linux

set -e

echo "RTL-SDR VST3 Plugin - Quick Setup"
echo "===================================="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check Git
echo -e "${YELLOW}[1/5] Checking Git...${NC}"
if command -v git &> /dev/null; then
    echo -e "${GREEN}✓ Git found${NC}"
else
    echo -e "${RED}✗ Git not found. Please install Git.${NC}"
    exit 1
fi

# Check CMake
echo -e "\n${YELLOW}[2/5] Checking CMake...${NC}"
if command -v cmake &> /dev/null; then
    echo -e "${GREEN}✓ CMake found${NC}"
else
    echo -e "${RED}✗ CMake not found.${NC}"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "Install with: brew install cmake"
    else
        echo "Install with: sudo apt-get install cmake"
    fi
    exit 1
fi

# Initialize JUCE
echo -e "\n${YELLOW}[3/5] Setting up JUCE...${NC}"
if [ -d "JUCE/.git" ]; then
    echo -e "${GREEN}✓ JUCE already initialized${NC}"
else
    echo "Cloning JUCE framework..."
    git submodule add https://github.com/juce-framework/JUCE.git JUCE
    git submodule update --init --recursive
    echo -e "${GREEN}✓ JUCE cloned successfully${NC}"
fi

# Check for RTL-SDR
echo -e "\n${YELLOW}[4/5] Checking RTL-SDR library...${NC}"
SIMULATION_MODE=false

if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    if brew list librtlsdr &> /dev/null; then
        echo -e "${GREEN}✓ RTL-SDR found (via Homebrew)${NC}"
    else
        echo -e "${YELLOW}⚠ RTL-SDR not found${NC}"
        echo "Install with: brew install librtlsdr"
        read -p "Build in simulation mode? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            SIMULATION_MODE=true
            echo -e "${GREEN}✓ Will build in simulation mode${NC}"
        else
            exit 1
        fi
    fi
else
    # Linux
    if ldconfig -p | grep librtlsdr &> /dev/null; then
        echo -e "${GREEN}✓ RTL-SDR found${NC}"
    else
        echo -e "${YELLOW}⚠ RTL-SDR not found${NC}"
        echo "Install with: sudo apt-get install librtlsdr-dev"
        read -p "Build in simulation mode? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            SIMULATION_MODE=true
            echo -e "${GREEN}✓ Will build in simulation mode${NC}"
        else
            exit 1
        fi
    fi
fi

# Build
echo -e "\n${YELLOW}[5/5] Building project...${NC}"

rm -rf build
mkdir build
cd build

echo "Running CMake configuration..."

if [ "$SIMULATION_MODE" = true ]; then
    cmake .. -DCMAKE_BUILD_TYPE=Release -DHAS_RTLSDR=0
else
    cmake .. -DCMAKE_BUILD_TYPE=Release
fi

echo -e "${GREEN}✓ Configuration successful${NC}"

echo "Building..."
cmake --build . --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo -e "\n${GREEN}✓ Build successful!${NC}"

# Find VST3
VST3_PATH=$(find . -name "*.vst3" -type d | head -n 1)

if [ -n "$VST3_PATH" ]; then
    echo -e "\n${GREEN}✓ VST3 plugin built at:${NC}"
    echo -e "  ${CYAN}$VST3_PATH${NC}"

    if [[ "$OSTYPE" == "darwin"* ]]; then
        INSTALL_PATH="$HOME/Library/Audio/Plug-Ins/VST3"
    else
        INSTALL_PATH="$HOME/.vst3"
    fi

    echo -e "\nTo install, copy to:"
    echo -e "  ${CYAN}$INSTALL_PATH${NC}"

    read -p "Install plugin now? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        mkdir -p "$INSTALL_PATH"
        cp -r "$VST3_PATH" "$INSTALL_PATH/"
        echo -e "${GREEN}✓ Plugin installed!${NC}"
    fi
fi

cd ..

echo -e "\n===================================="
echo -e "${GREEN}Setup complete!${NC}\n"
echo "Next steps:"
echo "  1. Open Ableton Live"
echo "  2. Add RTL-SDR Radio plugin to a track"
echo "  3. Select your RTL-SDR device and click 'Start SDR'"
echo "  4. Tune to a frequency and enjoy!"
echo ""
echo "Documentation: README.md"
echo "Architecture details: ARCHITECTURE.md"
echo "Build guide: BUILD.md"
echo ""
