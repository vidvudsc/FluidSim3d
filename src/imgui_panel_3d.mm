#include "imgui_panel_3d.h"

#include "imgui.h"
#include "rlImGui.h"

#include <math.h>
#include <stdio.h>

static int ClampInt(int value, int minValue, int maxValue)
{
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}

static void ApplyGasSimTheme(void)
{
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.WindowPadding = ImVec2(14.0f, 14.0f);
    style.FramePadding = ImVec2(12.0f, 8.0f);
    style.ItemSpacing = ImVec2(10.0f, 10.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.93f, 0.96f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.58f, 0.69f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.05f, 0.08f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.08f, 0.12f, 0.90f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.07f, 0.11f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.16f, 0.27f, 0.44f, 0.80f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.11f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.34f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.28f, 0.49f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.12f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.22f, 0.42f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.10f, 0.18f, 0.95f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.55f, 0.78f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.62f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.58f, 0.82f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.12f, 0.22f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.32f, 0.54f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.40f, 0.70f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.11f, 0.21f, 0.36f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.16f, 0.30f, 0.51f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.38f, 0.66f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.17f, 0.29f, 0.47f, 0.85f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.31f, 0.56f, 0.90f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.42f, 0.69f, 0.98f, 0.78f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.76f, 1.00f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.13f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.31f, 0.52f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.14f, 0.24f, 0.41f, 1.00f);
}

static void DrawCombo(const char *label, const char *preview, const char *const *items, int itemCount,
    int currentItem, int *selectedItem, bool *changed, bool disabled)
{
    if (disabled) {
        ImGui::BeginDisabled(true);
    }

    if (ImGui::BeginCombo(label, preview)) {
        for (int i = 0; i < itemCount; ++i) {
            bool isSelected = (i == currentItem);
            if (ImGui::Selectable(items[i], isSelected)) {
                *selectedItem = i;
                *changed = true;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (disabled) {
        ImGui::EndDisabled();
    }
}

void Ui3DPanelSetup(void)
{
    rlImGuiSetup(true);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    Vector2 dpiScale = GetWindowScaleDPI();
    const float scale = (dpiScale.x > 0.0f) ? dpiScale.x : 1.0f;
    if (scale > 1.01f) {
        ImGui::GetStyle().ScaleAllSizes(scale);
        io.FontGlobalScale = 1.0f / scale;
    }

    ApplyGasSimTheme();
}

void Ui3DPanelShutdown(void)
{
    rlImGuiShutdown();
}

void Ui3DPanelBegin(float deltaTime)
{
    rlImGuiBeginDelta(deltaTime);
}

void Ui3DPanelEnd(void)
{
    rlImGuiEnd();
}

Ui3DCaptureState Ui3DPanelGetCaptureState(void)
{
    ImGuiIO &io = ImGui::GetIO();
    Ui3DCaptureState capture = {
        io.WantCaptureMouse ||
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
            ImGui::IsAnyItemHovered() ||
            ImGui::IsAnyItemActive(),
        io.WantCaptureKeyboard ||
            ImGui::IsAnyItemActive() ||
            ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow),
    };
    return capture;
}

void Ui3DPanelDraw(const Ui3DPanelState *state, Ui3DPanelActions *actions)
{
    static const char *const backendLabels[] = {"CPU", "GPU"};
    static const char *const cpuCountLabels[] = {"2k", "4k", "6k", "8k", "10k"};
    static const int cpuCounts[] = {2000, 4000, 6000, 8000, 10000};
    static const char *const gpuCountLabels[] = {"16k", "32k", "64k", "96k", "120k"};
    static const int gpuCounts[] = {16000, 32000, 64000, 96000, 120000};
    static const char *const bakeCountLabels[] = {"120k", "250k", "500k", "750k", "1M"};
    static const int bakeCounts[] = {120000, 250000, 500000, 750000, 1000000};
    static const char *const modeLabels[] = {"Water tank", "Gas tank", "Wind tunnel"};
    static const char *const obstacleLabels[] = {"Cylinder", "Airfoil", "Car", "Rectangle", "Imported"};
    static const char *const waterObstacleShapeLabels[] = {"Cylinders", "Rectangles"};
    static const char *const viewLabels[] = {"Particles", "Smoothing"};
    static const char *const colorLabels[] = {"Material", "Temperature", "Pressure", "Speed", "Density", "Tracer dye", "Vorticity"};
    static float importedRotationStepDegrees = 10.0f;
    const int colorLabelCount = (int)(sizeof(colorLabels) / sizeof(colorLabels[0]));
    const bool controlsLocked = state->bakeSimulationLocked;

    *actions = {};

    int backend = state->backend;
    int mode = state->mode;
    int obstacleModel = state->obstacleModel;
    bool waterObstacleEnabled = state->waterObstacleEnabled;
    int waterObstacleShape = state->waterObstacleShape;
    int waterCylinderCount = state->waterCylinderCount;
    int waterCylinderSelected = state->waterCylinderSelected;
    float waterCylinderX = state->waterCylinderX;
    float waterCylinderY = state->waterCylinderY;
    float waterCylinderZ = state->waterCylinderZ;
    float waterCylinderRadius = state->waterCylinderRadius;
    float waterCylinderDepth = state->waterCylinderDepth;
    float waterRectangleWidth = state->waterRectangleWidth;
    float waterRectangleHeight = state->waterRectangleHeight;
    float waterRectangleAngleDegrees = state->waterRectangleAngleDegrees;
    float obstacleAngleDegrees = state->obstacleAngleDegrees;
    float obstacleRectWidth = state->obstacleRectWidth;
    float obstacleRectHeight = state->obstacleRectHeight;
    float importedScale = state->importedScale;
    float importedOffsetX = state->importedOffsetX;
    float importedOffsetY = state->importedOffsetY;
    float importedOffsetZ = state->importedOffsetZ;
    int viewMode = state->viewMode;
    int colorMode = state->colorMode;
    bool flyCamera = state->flyCamera;
    float windSpeedScale = state->windSpeedScale;
    bool showParticles = state->showParticles;
    bool showSlicePanels = state->showSlicePanels;
    bool sliceXEnabled = state->sliceXEnabled;
    float sliceXNormalized = state->sliceXNormalized;
    bool sliceYEnabled = state->sliceYEnabled;
    float sliceYNormalized = state->sliceYNormalized;
    bool sliceZEnabled = state->sliceZEnabled;
    float sliceZNormalized = state->sliceZNormalized;
    bool showStreamlines = state->showStreamlines;
    int streamlineDensity = state->streamlineDensity;
    int streamlineStepCount = state->streamlineStepCount;
    bool showPathlines = state->showPathlines;
    int pathlineDensity = state->pathlineDensity;
    int pathlineTrailLength = state->pathlineTrailLength;
    bool acousticsEnabled = state->acousticsEnabled;
    float speakerFrequency = state->speakerFrequency;
    float speakerAmplitude = state->speakerAmplitude;
    float acousticSoundSpeed = state->acousticSoundSpeed;
    float acousticMachLimit = state->acousticMachLimit;
    float acousticViscosityScale = state->acousticViscosityScale;
    float acousticDragScale = state->acousticDragScale;
    float acousticSlowdownFactor = state->acousticSlowdownFactor;
    float audioMonitorPitchHz = state->audioMonitorPitchHz;
    float speakerWidth = state->speakerWidth;
    float speakerHeight = state->speakerHeight;
    float speakerDepth = state->speakerDepth;
    float micRadius = state->micRadius;
    bool audioOutputEnabled = state->audioOutputEnabled;
    float bakeDuration = state->bakeDuration;
    float bakePlaybackTime = state->bakePlaybackTime;
    bool bakePlaybackPlaying = state->bakePlaybackPlaying;

    const bool gpuAvailable = state->gpuBackendAvailable;
    const int *countValues = (backend == 1) ? gpuCounts : cpuCounts;
    const char *const *countLabels = (backend == 1) ? gpuCountLabels : cpuCountLabels;
    const int countLabelCount = 5;
    int countIndex = 0;
    for (int i = 0; i < countLabelCount; ++i) {
        if (countValues[i] == state->targetParticleCount) {
            countIndex = i;
            break;
        }
    }

    ImGuiIO &io = ImGui::GetIO();
    const float panelWidth = 418.0f;
    ImGui::SetNextWindowPos(ImVec2(fmaxf(18.0f, io.DisplaySize.x - panelWidth - 18.0f), 18.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, 700.0f), ImGuiCond_Always);

    if (ImGui::Begin("Controls")) {
        ImGui::SetWindowFontScale(1.38f);
        ImGui::Text("Particles %d / %d", state->actualParticleCount, state->targetParticleCount);
        ImGui::Text("FPS %.1f", state->fps);
        ImGui::Text("Step %.2f ms", (float)state->lastSimStepMs);
        if (state->flowMach > 0.0f) {
            ImGui::Text("Mach %.2f", state->flowMach);
        }
        ImGui::SetWindowFontScale(1.18f);

        if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool backendChanged = false;
            DrawCombo("Backend", backendLabels[backend], backendLabels, 2, backend, &backend, &backendChanged, controlsLocked);
            if (!gpuAvailable) {
                ImGui::TextDisabled("GPU backend unavailable in this build.");
            }
            if (backendChanged) {
                if (backend == 0 || gpuAvailable) {
                    actions->setBackend = true;
                    actions->backend = backend;
                } else {
                    backend = state->backend;
                }
            }

            countValues = (backend == 1) ? gpuCounts : cpuCounts;
            countLabels = (backend == 1) ? gpuCountLabels : cpuCountLabels;
            countIndex = 0;
            for (int i = 0; i < countLabelCount; ++i) {
                if (countValues[i] == state->targetParticleCount) {
                    countIndex = i;
                    break;
                }
            }

            bool countChanged = false;
            DrawCombo("Particle count", countLabels[countIndex], countLabels, countLabelCount, countIndex, &countIndex,
                &countChanged, controlsLocked);
            if (countChanged) {
                actions->setTargetParticleCount = true;
                actions->targetParticleCount = countValues[countIndex];
            }

            bool paused = state->paused;
            if (controlsLocked) {
                ImGui::BeginDisabled(true);
            }
            if (ImGui::Checkbox("Paused", &paused)) {
                actions->setPaused = true;
                actions->paused = paused;
            }
            ImGui::SameLine();
            if (ImGui::Button("Respawn")) {
                actions->requestReset = true;
            }
            if (controlsLocked) {
                ImGui::EndDisabled();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset camera")) {
                actions->requestCameraReset = true;
            }
        }

        if (ImGui::CollapsingHeader("Mode", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool modeChanged = false;
            DrawCombo("Scene", modeLabels[mode], modeLabels, UI_3D_MODE_COUNT, mode, &mode, &modeChanged, controlsLocked);
            if (modeChanged) {
                actions->setMode = true;
                actions->mode = mode;
            }

            if (mode == UI_3D_MODE_WIND_TUNNEL) {
                bool obstacleChanged = false;
                DrawCombo("Wind shape", obstacleLabels[obstacleModel], obstacleLabels, 5, obstacleModel, &obstacleModel,
                    &obstacleChanged, controlsLocked);
                if (obstacleChanged) {
                    actions->setObstacleModel = true;
                    actions->obstacleModel = obstacleModel;
                }

                if (controlsLocked) {
                    ImGui::BeginDisabled(true);
                }
                if (ImGui::SliderFloat("Tunnel speed", &windSpeedScale, 0.35f, 2.50f, "%.2fx")) {
                    actions->setWindSpeedScale = true;
                    actions->windSpeedScale = windSpeedScale;
                }
                ImGui::TextDisabled("Current Mach %.2f", state->flowMach);

                if (ImGui::Button("Open OBJ...")) {
                    actions->requestImportObstacle = true;
                }
                if (state->importedObstacleLoaded) {
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s", state->importedObstacleLabel);
                } else {
                    ImGui::SameLine();
                    ImGui::TextDisabled("No OBJ loaded");
                }

                if (obstacleModel != 0 && obstacleModel != 4) {
                    if (ImGui::SliderFloat("Obstacle angle", &obstacleAngleDegrees, -180.0f, 180.0f, "%.0f deg")) {
                        actions->setObstacleAngleDegrees = true;
                        actions->obstacleAngleDegrees = obstacleAngleDegrees;
                    }
                }

                if (obstacleModel == 3) {
                    if (ImGui::SliderFloat("Rect width", &obstacleRectWidth, 40.0f, 140.0f, "%.0f px")) {
                        actions->setObstacleRectWidth = true;
                        actions->obstacleRectWidth = obstacleRectWidth;
                    }
                    if (ImGui::SliderFloat("Rect height", &obstacleRectHeight, 24.0f, 96.0f, "%.0f px")) {
                        actions->setObstacleRectHeight = true;
                        actions->obstacleRectHeight = obstacleRectHeight;
                    }
                }

                if (obstacleModel == 4) {
                    if (ImGui::SliderFloat("Import scale", &importedScale, 0.35f, 3.00f, "%.2fx")) {
                        actions->setImportedScale = true;
                        actions->importedScale = importedScale;
                    }
                    if (ImGui::SliderFloat("Import X", &importedOffsetX, -160.0f, 160.0f, "%.1f")) {
                        actions->setImportedOffsetX = true;
                        actions->importedOffsetX = importedOffsetX;
                    }
                    if (ImGui::SliderFloat("Import Y", &importedOffsetY, -90.0f, 90.0f, "%.1f")) {
                        actions->setImportedOffsetY = true;
                        actions->importedOffsetY = importedOffsetY;
                    }
                    if (ImGui::SliderFloat("Import Z", &importedOffsetZ, -90.0f, 90.0f, "%.1f")) {
                        actions->setImportedOffsetZ = true;
                        actions->importedOffsetZ = importedOffsetZ;
                    }
                    ImGui::SliderFloat("Rotation step", &importedRotationStepDegrees, 1.0f, 30.0f, "%.0f deg");

                    if (ImGui::Button("Rotate X -")) {
                        actions->setImportedRotationX = true;
                        actions->importedRotationX = -importedRotationStepDegrees;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Rotate X +")) {
                        actions->setImportedRotationX = true;
                        actions->importedRotationX = importedRotationStepDegrees;
                    }

                    if (ImGui::Button("Rotate Y -")) {
                        actions->setImportedRotationY = true;
                        actions->importedRotationY = -importedRotationStepDegrees;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Rotate Y +")) {
                        actions->setImportedRotationY = true;
                        actions->importedRotationY = importedRotationStepDegrees;
                    }

                    if (ImGui::Button("Rotate Z -")) {
                        actions->setImportedRotationZ = true;
                        actions->importedRotationZ = -importedRotationStepDegrees;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Rotate Z +")) {
                        actions->setImportedRotationZ = true;
                        actions->importedRotationZ = importedRotationStepDegrees;
                    }

                    if (ImGui::Button("Reset import rotation")) {
                        actions->requestResetImportedRotation = true;
                    }
                    ImGui::TextDisabled("Incremental world-axis rotation to avoid Euler coupling.");
                }
                if (controlsLocked) {
                    ImGui::EndDisabled();
                }
            }

            if (mode == UI_3D_MODE_WATER_TANK) {
                if (controlsLocked) {
                    ImGui::BeginDisabled(true);
                }

                if (ImGui::Checkbox("Immovable obstacles", &waterObstacleEnabled)) {
                    actions->setWaterObstacleEnabled = true;
                    actions->waterObstacleEnabled = waterObstacleEnabled;
                }

                if (waterObstacleEnabled) {
                    waterObstacleShape = ClampInt(waterObstacleShape, 0, UI_3D_WATER_OBSTACLE_SHAPE_COUNT - 1);
                    bool waterShapeChanged = false;
                    DrawCombo("Obstacle type", waterObstacleShapeLabels[waterObstacleShape], waterObstacleShapeLabels,
                        UI_3D_WATER_OBSTACLE_SHAPE_COUNT, waterObstacleShape, &waterObstacleShape,
                        &waterShapeChanged, false);
                    if (waterShapeChanged) {
                        actions->setWaterObstacleShape = true;
                        actions->waterObstacleShape = waterObstacleShape;
                    }

                    waterCylinderCount = ClampInt(waterCylinderCount, 1, UI_3D_WATER_CYLINDER_MAX);
                    waterCylinderSelected = ClampInt(waterCylinderSelected, 0, waterCylinderCount - 1);
                    int selectedDisplay = waterCylinderSelected + 1;
                    const bool editingRectangles = waterObstacleShape == UI_3D_WATER_OBSTACLE_RECTANGLES;
                    const char *countLabel = editingRectangles ? "Rectangle count" : "Cylinder count";
                    const char *selectedLabel = editingRectangles ? "Edit rectangle" : "Edit cylinder";

                    if (ImGui::SliderInt(countLabel, &waterCylinderCount, 1, UI_3D_WATER_CYLINDER_MAX)) {
                        actions->setWaterCylinderCount = true;
                        actions->waterCylinderCount = waterCylinderCount;
                    }
                    if (ImGui::SliderInt(selectedLabel, &selectedDisplay, 1, waterCylinderCount)) {
                        actions->setWaterCylinderSelected = true;
                        actions->waterCylinderSelected = selectedDisplay - 1;
                    }

                    if (ImGui::SliderFloat(editingRectangles ? "Rectangle X" : "Cylinder X", &waterCylinderX, -70.0f, 70.0f, "%.1f")) {
                        actions->setWaterCylinderX = true;
                        actions->waterCylinderX = waterCylinderX;
                    }
                    if (ImGui::SliderFloat(editingRectangles ? "Rectangle Y" : "Cylinder Y", &waterCylinderY, 6.0f, 78.0f, "%.1f")) {
                        actions->setWaterCylinderY = true;
                        actions->waterCylinderY = waterCylinderY;
                    }
                    if (ImGui::SliderFloat(editingRectangles ? "Rectangle Z" : "Cylinder Z", &waterCylinderZ, -40.0f, 40.0f, "%.1f")) {
                        actions->setWaterCylinderZ = true;
                        actions->waterCylinderZ = waterCylinderZ;
                    }

                    if (editingRectangles) {
                        if (ImGui::SliderFloat("Width", &waterRectangleWidth, 5.0f, 90.0f, "%.1f")) {
                            actions->setWaterRectangleWidth = true;
                            actions->waterRectangleWidth = waterRectangleWidth;
                        }
                        if (ImGui::SliderFloat("Height", &waterRectangleHeight, 5.0f, 70.0f, "%.1f")) {
                            actions->setWaterRectangleHeight = true;
                            actions->waterRectangleHeight = waterRectangleHeight;
                        }
                        if (ImGui::SliderFloat("Angle", &waterRectangleAngleDegrees, -180.0f, 180.0f, "%.0f deg")) {
                            actions->setWaterRectangleAngleDegrees = true;
                            actions->waterRectangleAngleDegrees = waterRectangleAngleDegrees;
                        }
                    } else {
                        if (ImGui::SliderFloat("Radius", &waterCylinderRadius, 3.0f, 24.0f, "%.1f")) {
                            actions->setWaterCylinderRadius = true;
                            actions->waterCylinderRadius = waterCylinderRadius;
                        }
                    }

                    if (ImGui::SliderFloat("Depth", &waterCylinderDepth, 8.0f, 88.0f, "%.1f")) {
                        actions->setWaterCylinderDepth = true;
                        actions->waterCylinderDepth = waterCylinderDepth;
                    }
                    ImGui::TextDisabled(editingRectangles
                        ? "Use rotated rectangles as walls, gates, and channels."
                        : "Use multiple cylinders to form barriers, bends, and restrictions.");
                }

                if (controlsLocked) {
                    ImGui::EndDisabled();
                }
            }
        }

        if (ImGui::CollapsingHeader("Bake", ImGuiTreeNodeFlags_DefaultOpen)) {
            int bakeCountIndex = 0;
            for (int i = 0; i < 5; ++i) {
                if (bakeCounts[i] == state->bakeParticleCount) {
                    bakeCountIndex = i;
                    break;
                }
            }

            bool bakeCountChanged = false;
            DrawCombo("Bake particles", bakeCountLabels[bakeCountIndex], bakeCountLabels, 5, bakeCountIndex,
                &bakeCountIndex, &bakeCountChanged, state->bakeStatus == UI_3D_BAKE_BAKING);
            if (bakeCountChanged) {
                actions->setBakeParticleCount = true;
                actions->bakeParticleCount = bakeCounts[bakeCountIndex];
            }

            if (state->acousticAudioLoaded) {
                ImGui::TextDisabled("Slow audio bake: %.0fx, %.1fs clip -> %.1fs sim",
                    state->acousticSlowdownFactor, state->acousticAudioDuration, state->acousticSlowBakeDuration);
                ImGui::TextDisabled("Target: SPH %.1f Hz slow-time -> %.0f Hz after export.",
                    state->acousticResolvedHz, state->acousticRestoredBandwidthHz);
            } else {
                if (state->bakeStatus == UI_3D_BAKE_BAKING) {
                    ImGui::BeginDisabled(true);
                }
                if (ImGui::SliderFloat("Bake duration", &bakeDuration, 1.0f, 60.0f, "%.1f s")) {
                    actions->setBakeDuration = true;
                    actions->bakeDuration = bakeDuration;
                }
                if (state->bakeStatus == UI_3D_BAKE_BAKING) {
                    ImGui::EndDisabled();
                }
            }

            ImGui::TextDisabled("Preview cache: 24 fps, max 200k visible particles.");
            if (state->bakeStatus == UI_3D_BAKE_BAKING) {
                ImGui::Text("Simulating %d / %d", state->actualParticleCount, state->bakeParticleCount);
            } else if (state->bakeHasCache) {
                ImGui::TextDisabled("Playback shows cached preview particles, not the full keyframe count.");
            }
            if (state->mode == UI_3D_MODE_GAS_TANK) {
                if (state->bakeStatus == UI_3D_BAKE_BAKING) {
                    ImGui::BeginDisabled(true);
                }
                if (ImGui::Button("Open WAV/MP3...")) {
                    actions->requestLoadAcousticAudio = true;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("%s", state->acousticAudioLoaded ? state->acousticAudioLabel : "procedural speaker");
                if (state->bakeStatus == UI_3D_BAKE_BAKING) {
                    ImGui::EndDisabled();
                }
            }

            if (state->bakeStatus == UI_3D_BAKE_BAKING) {
                if (ImGui::Button("Stop bake")) {
                    actions->requestBakeStop = true;
                }
            } else {
                if (ImGui::Button("Start GPU bake")) {
                    actions->requestBakeStart = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Load latest")) {
                actions->requestBakeLoadLatest = true;
            }
            if (state->bakeCanExportMic) {
                ImGui::SameLine();
                if (ImGui::Button(state->acousticAudioLoaded ? "Export simulated audio" : "Export mic WAV")) {
                    actions->requestBakeExportMic = true;
                }
            }
            if (state->bakeNotice != nullptr && state->bakeNotice[0] != '\0') {
                ImGui::TextWrapped("%s", state->bakeNotice);
            }
        }

        if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool viewChanged = false;
            DrawCombo("View", viewLabels[viewMode], viewLabels, 2, viewMode, &viewMode, &viewChanged, false);
            if (viewChanged) {
                actions->setViewMode = true;
                actions->viewMode = viewMode;
            }

            bool colorChanged = false;
            DrawCombo("Color", colorLabels[colorMode], colorLabels, colorLabelCount, colorMode, &colorMode, &colorChanged, false);
            if (colorChanged) {
                actions->setColorMode = true;
                actions->colorMode = colorMode;
            }

            if (ImGui::Checkbox("Fly camera", &flyCamera)) {
                actions->setFlyCamera = true;
                actions->flyCamera = flyCamera;
            }
            ImGui::TextDisabled(flyCamera
                ? "LMB look, WASD move, Q/E down/up, Shift fast, Alt slow."
                : "LMB orbit, RMB repulse, wheel zoom, 0 resets view.");
            ImGui::TextDisabled("Camera distance %.1f", state->cameraDistance);
        }

        if (ImGui::CollapsingHeader("3D Debug", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Checkbox("Show particles", &showParticles)) {
                actions->setShowParticles = true;
                actions->showParticles = showParticles;
            }

            const bool sliceScene = (mode == UI_3D_MODE_WATER_TANK || mode == UI_3D_MODE_GAS_TANK ||
                mode == UI_3D_MODE_WIND_TUNNEL);
            if (sliceScene) {
                if (ImGui::Checkbox("Slice panels", &showSlicePanels)) {
                    actions->setShowSlicePanels = true;
                    actions->showSlicePanels = showSlicePanels;
                }
                if (showSlicePanels) {
                    if (ImGui::Checkbox("X##slice_enabled", &sliceXEnabled)) {
                        actions->setSliceXEnabled = true;
                        actions->sliceXEnabled = sliceXEnabled;
                    }
                    ImGui::SameLine();
                    if (ImGui::SliderFloat("Slice X", &sliceXNormalized, 0.05f, 0.95f, "%.2f")) {
                        actions->setSliceXNormalized = true;
                        actions->sliceXNormalized = sliceXNormalized;
                    }
                    if (ImGui::Checkbox("Y##slice_enabled", &sliceYEnabled)) {
                        actions->setSliceYEnabled = true;
                        actions->sliceYEnabled = sliceYEnabled;
                    }
                    ImGui::SameLine();
                    if (ImGui::SliderFloat("Slice Y", &sliceYNormalized, 0.05f, 0.95f, "%.2f")) {
                        actions->setSliceYNormalized = true;
                        actions->sliceYNormalized = sliceYNormalized;
                    }
                    if (ImGui::Checkbox("Z##slice_enabled", &sliceZEnabled)) {
                        actions->setSliceZEnabled = true;
                        actions->sliceZEnabled = sliceZEnabled;
                    }
                    ImGui::SameLine();
                    if (ImGui::SliderFloat("Slice Z", &sliceZNormalized, 0.05f, 0.95f, "%.2f")) {
                        actions->setSliceZNormalized = true;
                        actions->sliceZNormalized = sliceZNormalized;
                    }
                }
            }

            if (mode == UI_3D_MODE_WIND_TUNNEL) {
                if (ImGui::Checkbox("Streamlines", &showStreamlines)) {
                    actions->setShowStreamlines = true;
                    actions->showStreamlines = showStreamlines;
                }
                if (showStreamlines) {
                    if (ImGui::SliderInt("Stream density", &streamlineDensity, 2, 8)) {
                        actions->setStreamlineDensity = true;
                        actions->streamlineDensity = streamlineDensity;
                    }
                    if (ImGui::SliderInt("Stream steps", &streamlineStepCount, 6, 40)) {
                        actions->setStreamlineStepCount = true;
                        actions->streamlineStepCount = streamlineStepCount;
                    }
                }

                if (ImGui::Checkbox("Pathlines", &showPathlines)) {
                    actions->setShowPathlines = true;
                    actions->showPathlines = showPathlines;
                }
                if (showPathlines) {
                    if (ImGui::SliderInt("Path density", &pathlineDensity, 2, 8)) {
                        actions->setPathlineDensity = true;
                        actions->pathlineDensity = pathlineDensity;
                    }
                    if (ImGui::SliderInt("Trail length", &pathlineTrailLength, 4, 32)) {
                        actions->setPathlineTrailLength = true;
                        actions->pathlineTrailLength = pathlineTrailLength;
                    }
                }
            }
        }

        if (state->acousticsAvailable && ImGui::CollapsingHeader("Acoustics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BeginDisabled(controlsLocked);

            if (ImGui::Checkbox("Enable speaker + mic", &acousticsEnabled)) {
                actions->setAcousticsEnabled = true;
                actions->acousticsEnabled = acousticsEnabled;
            }

            if (ImGui::SliderFloat("Frequency", &speakerFrequency, 0.2f, 40.0f, "%.2f Hz")) {
                actions->setSpeakerFrequency = true;
                actions->speakerFrequency = speakerFrequency;
            }

            if (ImGui::SliderFloat("Amplitude", &speakerAmplitude, 1.0f, 42.0f, "%.1f px")) {
                actions->setSpeakerAmplitude = true;
                actions->speakerAmplitude = speakerAmplitude;
            }
            ImGui::Text("Effective amp %.2f px", state->effectiveSpeakerAmplitude);
            if (ImGui::Button("Open WAV/MP3 audio...")) {
                actions->requestLoadAcousticAudio = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("%s", state->acousticAudioLoaded ? state->acousticAudioLabel : "procedural");
            if (state->acousticAudioLoaded) {
                ImGui::TextDisabled("Driver %.0f Hz slowed %.0fx; restored bandwidth estimate %.0f Hz.",
                    state->acousticDriverHz, state->acousticSlowdownFactor, state->acousticRestoredBandwidthHz);
                ImGui::TextDisabled("Export is the compressed SPH mic pressure, not the original source mixed back in.");
                ImGui::TextDisabled("If the slider snaps upward, the bake needs that slowdown for the 20 kHz target.");
            } else {
                ImGui::TextDisabled("Procedural mode drives a moving speaker slab through the gas.");
            }

            if (ImGui::SliderFloat("Acoustic c", &acousticSoundSpeed, 52.0f, 420.0f, "%.0f")) {
                actions->setAcousticSoundSpeed = true;
                actions->acousticSoundSpeed = acousticSoundSpeed;
            }

            if (ImGui::SliderFloat("Mach cap", &acousticMachLimit, 0.10f, 0.60f, "%.2f c")) {
                actions->setAcousticMachLimit = true;
                actions->acousticMachLimit = acousticMachLimit;
            }

            if (ImGui::SliderFloat("Viscosity x", &acousticViscosityScale, 0.05f, 1.00f, "%.2f")) {
                actions->setAcousticViscosityScale = true;
                actions->acousticViscosityScale = acousticViscosityScale;
            }

            if (ImGui::SliderFloat("Drag x", &acousticDragScale, 0.00f, 1.00f, "%.2f")) {
                actions->setAcousticDragScale = true;
                actions->acousticDragScale = acousticDragScale;
            }

            if (state->acousticAudioLoaded) {
                if (ImGui::SliderFloat("Audio slowdown", &acousticSlowdownFactor, 100.0f, 2400.0f, "%.0fx")) {
                    actions->setAcousticSlowdownFactor = true;
                    actions->acousticSlowdownFactor = acousticSlowdownFactor;
                }
            }

            if (ImGui::SliderFloat("Speaker width", &speakerWidth, 8.0f, 80.0f, "%.0f px")) {
                actions->setSpeakerWidth = true;
                actions->speakerWidth = speakerWidth;
            }

            if (ImGui::SliderFloat("Speaker height", &speakerHeight, 24.0f, 220.0f, "%.0f px")) {
                actions->setSpeakerHeight = true;
                actions->speakerHeight = speakerHeight;
            }

            if (ImGui::SliderFloat("Speaker depth", &speakerDepth, 8.0f, 56.0f, "%.0f px")) {
                actions->setSpeakerDepth = true;
                actions->speakerDepth = speakerDepth;
            }

            if (ImGui::SliderFloat("Mic radius", &micRadius, 8.0f, 80.0f, "%.0f px")) {
                actions->setMicRadius = true;
                actions->micRadius = micRadius;
            }

            if (ImGui::Checkbox("Output mic to speakers", &audioOutputEnabled)) {
                actions->setAudioOutputEnabled = true;
                actions->audioOutputEnabled = audioOutputEnabled;
            }
            if (!state->audioOutputAvailable) {
                ImGui::TextDisabled("Speaker output unavailable.");
            }
            if (ImGui::SliderFloat("Monitor pitch", &audioMonitorPitchHz, 0.0f, 1000.0f, "%.0f Hz")) {
                actions->setAudioMonitorPitchHz = true;
                actions->audioMonitorPitchHz = audioMonitorPitchHz;
            }
            ImGui::TextDisabled("0 Hz plays the raw low-frequency mic signal.");

            ImGui::Text("Mic signal %.3f", state->micSignal);
            if (state->micWaveform != nullptr && state->micWaveformCount > 1) {
                ImGui::PlotLines("Waveform", state->micWaveform, state->micWaveformCount, 0, NULL, -1.0f, 1.0f,
                    ImVec2(0.0f, 96.0f));
            }
            ImGui::TextDisabled("The 3D gas tank renders the speaker slab and microphone probe in-world.");

            ImGui::EndDisabled();
        }
    }
    ImGui::End();

    if (state->bakeStatus != UI_3D_BAKE_IDLE || state->bakeHasCache) {
        const char *statusLabel = "Idle";
        if (state->bakeStatus == UI_3D_BAKE_BAKING) {
            statusLabel = "Baking";
        } else if (state->bakeStatus == UI_3D_BAKE_STOPPED) {
            statusLabel = "Stopped";
        } else if (state->bakeStatus == UI_3D_BAKE_COMPLETE) {
            statusLabel = "Complete";
        } else if (state->bakeStatus == UI_3D_BAKE_PLAYBACK) {
            statusLabel = "Playback";
        }

        const bool isBaking = state->bakeStatus == UI_3D_BAKE_BAKING;
        char etaText[32] = "--";
        if (state->bakeEtaSeconds > 0.0f && isfinite(state->bakeEtaSeconds)) {
            const int totalSeconds = (int)ceilf(state->bakeEtaSeconds);
            const int minutes = totalSeconds / 60;
            const int seconds = totalSeconds % 60;
            if (minutes > 0) {
                snprintf(etaText, sizeof(etaText), "%dm %02ds", minutes, seconds);
            } else {
                snprintf(etaText, sizeof(etaText), "%ds", seconds);
            }
        }
        const float height = isBaking ? 82.0f : 56.0f;
        const float x = 24.0f;
        const float y = io.DisplaySize.y - height - 16.0f;
        const float width = fmaxf(460.0f, io.DisplaySize.x - 48.0f);

        ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 10.0f));
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground;
        if (ImGui::Begin("##bake_timeline_overlay", nullptr, flags)) {
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            const ImVec2 min = ImGui::GetWindowPos();
            const ImVec2 max = ImVec2(min.x + ImGui::GetWindowSize().x, min.y + ImGui::GetWindowSize().y);
            drawList->AddRectFilled(min, max, IM_COL32(7, 12, 21, 218), 0.0f);
            drawList->AddRect(min, max, IM_COL32(72, 118, 170, 160), 0.0f, 0, 1.0f);

            ImGui::SetWindowFontScale(1.12f);
            ImGui::Text("%s  %.1f / %.1fs", statusLabel, state->bakeBakedTime, state->bakeDuration);
            ImGui::SameLine();
            ImGui::TextDisabled("ETA %s  |  sim %d / %d  |  preview 200k @ 24fps",
                etaText, state->actualParticleCount, state->bakeParticleCount);
            if (isBaking) {
                ImGui::ProgressBar(state->bakeProgress, ImVec2(-1.0f, 0.0f));
            }
            ImGui::SetWindowFontScale(1.0f);
            const bool hasPlayableCache = state->bakeHasCache && state->bakeBakedTime > 0.0f;
            ImGui::BeginDisabled(!hasPlayableCache || state->bakeStatus == UI_3D_BAKE_BAKING);
            if (ImGui::Button(bakePlaybackPlaying ? "Pause" : "Play")) {
                actions->setBakePlaybackPlaying = true;
                actions->bakePlaybackPlaying = !bakePlaybackPlaying;
            }
            ImGui::SameLine();
            if (ImGui::Button("Start")) {
                actions->setBakePlaybackTime = true;
                actions->bakePlaybackTime = 0.0f;
            }
            ImGui::SameLine();
            if (ImGui::Button("End")) {
                actions->setBakePlaybackTime = true;
                actions->bakePlaybackTime = state->bakeBakedTime;
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::SliderFloat("##bake_scrub", &bakePlaybackTime, 0.0f,
                    (state->bakeBakedTime > 0.0f) ? state->bakeBakedTime : state->bakeDuration, "%.2f s")) {
                actions->setBakePlaybackTime = true;
                actions->bakePlaybackTime = bakePlaybackTime;
            }
            ImGui::EndDisabled();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}
