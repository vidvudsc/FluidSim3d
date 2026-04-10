CC := clang
CXX := clang++
TARGET := build/gassim3d

C_SRC := src/main_3d.c
MM_SRC := src/imgui_panel_3d.mm
CPP_SRC := \
	third_party/rlImGui/rlImGui.cpp \
	third_party/imgui/imgui.cpp \
	third_party/imgui/imgui_draw.cpp \
	third_party/imgui/imgui_tables.cpp \
	third_party/imgui/imgui_widgets.cpp

OBJS := \
	build/main_3d.o \
	build/imgui_panel_3d.o \
	build/rlImGui.o \
	build/imgui.o \
	build/imgui_draw.o \
	build/imgui_tables.o \
	build/imgui_widgets.o

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib)
RAYLIB_LIBS := $(shell pkg-config --libs raylib)
INCLUDES := $(RAYLIB_CFLAGS) -Isrc -Ithird_party/imgui -Ithird_party/rlImGui -Ithird_party/rlImGui/extras

CFLAGS := -std=c11 -x objective-c -fobjc-arc -O3 -ffast-math -fblocks -Wall -Wextra -Wpedantic $(INCLUDES)
MMFLAGS := -std=c++17 -x objective-c++ -fobjc-arc -O3 -ffast-math -fblocks -Wall -Wextra -Wpedantic $(INCLUDES)
THIRD_PARTY_CXXFLAGS := -std=c++17 -O3 -ffast-math -fblocks $(INCLUDES)
CFLAGS_3D := $(CFLAGS)
LDFLAGS := $(RAYLIB_LIBS) -lm -framework Foundation -framework AppKit -framework Metal

.PHONY: all run run3d clean

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p build
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

build/main_3d.o: $(C_SRC)
	mkdir -p build
	$(CC) $(CFLAGS_3D) -c $< -o $@

build/rlImGui.o: third_party/rlImGui/rlImGui.cpp
	mkdir -p build
	$(CXX) $(THIRD_PARTY_CXXFLAGS) -c $< -o $@

build/imgui.o: third_party/imgui/imgui.cpp
	mkdir -p build
	$(CXX) $(THIRD_PARTY_CXXFLAGS) -c $< -o $@

build/imgui_draw.o: third_party/imgui/imgui_draw.cpp
	mkdir -p build
	$(CXX) $(THIRD_PARTY_CXXFLAGS) -c $< -o $@

build/imgui_tables.o: third_party/imgui/imgui_tables.cpp
	mkdir -p build
	$(CXX) $(THIRD_PARTY_CXXFLAGS) -c $< -o $@

build/imgui_widgets.o: third_party/imgui/imgui_widgets.cpp
	mkdir -p build
	$(CXX) $(THIRD_PARTY_CXXFLAGS) -c $< -o $@

build/imgui_panel_3d.o: $(MM_SRC)
	mkdir -p build
	$(CXX) $(MMFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

run3d: run

clean:
	rm -rf build
