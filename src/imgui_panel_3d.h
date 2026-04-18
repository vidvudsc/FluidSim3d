#ifndef IMGUI_PANEL_3D_H
#define IMGUI_PANEL_3D_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum Ui3DSimMode {
    UI_3D_MODE_WATER_TANK = 0,
    UI_3D_MODE_GAS_TANK,
    UI_3D_MODE_WIND_TUNNEL,
    UI_3D_MODE_COUNT
} Ui3DSimMode;

typedef enum Ui3DBakeStatus {
    UI_3D_BAKE_IDLE = 0,
    UI_3D_BAKE_BAKING,
    UI_3D_BAKE_STOPPED,
    UI_3D_BAKE_COMPLETE,
    UI_3D_BAKE_PLAYBACK
} Ui3DBakeStatus;

#define UI_3D_WATER_CYLINDER_MAX 16

typedef enum Ui3DWaterObstacleShape {
    UI_3D_WATER_OBSTACLE_CYLINDERS = 0,
    UI_3D_WATER_OBSTACLE_RECTANGLES,
    UI_3D_WATER_OBSTACLE_SHAPE_COUNT
} Ui3DWaterObstacleShape;

typedef struct Ui3DCaptureState {
    bool wantsMouse;
    bool wantsKeyboard;
} Ui3DCaptureState;

typedef struct Ui3DPanelState {
    int backend;
    int targetParticleCount;
    int actualParticleCount;
    int mode;
    int obstacleModel;
    bool waterObstacleEnabled;
    int waterObstacleShape;
    int waterCylinderCount;
    int waterCylinderSelected;
    float waterCylinderX;
    float waterCylinderY;
    float waterCylinderZ;
    float waterCylinderRadius;
    float waterCylinderDepth;
    float waterRectangleWidth;
    float waterRectangleHeight;
    float waterRectangleAngleDegrees;
    float obstacleAngleDegrees;
    float obstacleRectWidth;
    float obstacleRectHeight;
    bool importedObstacleLoaded;
    const char *importedObstacleLabel;
    float importedScale;
    float importedOffsetX;
    float importedOffsetY;
    float importedOffsetZ;
    float importedRotationX;
    float importedRotationY;
    float importedRotationZ;
    int viewMode;
    int colorMode;
    bool flyCamera;
    bool paused;
    bool gpuBackendAvailable;
    float windSpeedScale;
    float flowMach;
    bool showParticles;
    bool showSlicePanels;
    bool sliceXEnabled;
    float sliceXNormalized;
    bool sliceYEnabled;
    float sliceYNormalized;
    bool sliceZEnabled;
    float sliceZNormalized;
    bool showStreamlines;
    int streamlineDensity;
    int streamlineStepCount;
    bool showPathlines;
    int pathlineDensity;
    int pathlineTrailLength;
    bool acousticsAvailable;
    bool acousticsEnabled;
    float speakerFrequency;
    float speakerAmplitude;
    float effectiveSpeakerAmplitude;
    float speakerWidth;
    float speakerHeight;
    float speakerDepth;
    float micRadius;
    float micSignal;
    float acousticSoundSpeed;
    float acousticMachLimit;
    float acousticViscosityScale;
    float acousticDragScale;
    float audioMonitorPitchHz;
    bool audioOutputAvailable;
    bool audioOutputEnabled;
    bool acousticAudioLoaded;
    const char *acousticAudioLabel;
    float acousticAudioDuration;
    float acousticAudioBandwidthHz;
    float acousticResolvedHz;
    float acousticDriverHz;
    float acousticPropagationDelay;
    float acousticSlowdownFactor;
    float acousticRestoredBandwidthHz;
    float acousticSlowBakeDuration;
    int bakeStatus;
    bool bakeSimulationLocked;
    float bakeDuration;
    int bakeParticleCount;
    float bakeProgress;
    float bakeBakedTime;
    float bakeEtaSeconds;
    float bakePlaybackTime;
    bool bakePlaybackPlaying;
    bool bakeHasCache;
    bool bakeCanExportMic;
    const char *bakeNotice;
    const float *micWaveform;
    int micWaveformCount;
    float fps;
    double lastSimStepMs;
    float cameraDistance;
} Ui3DPanelState;

typedef struct Ui3DPanelActions {
    bool setBackend;
    int backend;
    bool setTargetParticleCount;
    int targetParticleCount;
    bool setMode;
    int mode;
    bool setObstacleModel;
    int obstacleModel;
    bool setWaterObstacleEnabled;
    bool waterObstacleEnabled;
    bool setWaterObstacleShape;
    int waterObstacleShape;
    bool setWaterCylinderCount;
    int waterCylinderCount;
    bool setWaterCylinderSelected;
    int waterCylinderSelected;
    bool setWaterCylinderX;
    float waterCylinderX;
    bool setWaterCylinderY;
    float waterCylinderY;
    bool setWaterCylinderZ;
    float waterCylinderZ;
    bool setWaterCylinderRadius;
    float waterCylinderRadius;
    bool setWaterCylinderDepth;
    float waterCylinderDepth;
    bool setWaterRectangleWidth;
    float waterRectangleWidth;
    bool setWaterRectangleHeight;
    float waterRectangleHeight;
    bool setWaterRectangleAngleDegrees;
    float waterRectangleAngleDegrees;
    bool setObstacleAngleDegrees;
    float obstacleAngleDegrees;
    bool setObstacleRectWidth;
    float obstacleRectWidth;
    bool setObstacleRectHeight;
    float obstacleRectHeight;
    bool requestImportObstacle;
    bool setImportedScale;
    float importedScale;
    bool setImportedOffsetX;
    float importedOffsetX;
    bool setImportedOffsetY;
    float importedOffsetY;
    bool setImportedOffsetZ;
    float importedOffsetZ;
    bool setImportedRotationX;
    float importedRotationX;
    bool setImportedRotationY;
    float importedRotationY;
    bool setImportedRotationZ;
    float importedRotationZ;
    bool requestResetImportedRotation;
    bool setViewMode;
    int viewMode;
    bool setColorMode;
    int colorMode;
    bool setFlyCamera;
    bool flyCamera;
    bool setPaused;
    bool paused;
    bool setWindSpeedScale;
    float windSpeedScale;
    bool setShowParticles;
    bool showParticles;
    bool setShowSlicePanels;
    bool showSlicePanels;
    bool setSliceXEnabled;
    bool sliceXEnabled;
    bool setSliceXNormalized;
    float sliceXNormalized;
    bool setSliceYEnabled;
    bool sliceYEnabled;
    bool setSliceYNormalized;
    float sliceYNormalized;
    bool setSliceZEnabled;
    bool sliceZEnabled;
    bool setSliceZNormalized;
    float sliceZNormalized;
    bool setShowStreamlines;
    bool showStreamlines;
    bool setStreamlineDensity;
    int streamlineDensity;
    bool setStreamlineStepCount;
    int streamlineStepCount;
    bool setShowPathlines;
    bool showPathlines;
    bool setPathlineDensity;
    int pathlineDensity;
    bool setPathlineTrailLength;
    int pathlineTrailLength;
    bool setAcousticsEnabled;
    bool acousticsEnabled;
    bool setSpeakerFrequency;
    float speakerFrequency;
    bool setSpeakerAmplitude;
    float speakerAmplitude;
    bool setAcousticSoundSpeed;
    float acousticSoundSpeed;
    bool setAcousticMachLimit;
    float acousticMachLimit;
    bool setAcousticViscosityScale;
    float acousticViscosityScale;
    bool setAcousticDragScale;
    float acousticDragScale;
    bool setAcousticSlowdownFactor;
    float acousticSlowdownFactor;
    bool setAudioMonitorPitchHz;
    float audioMonitorPitchHz;
    bool setSpeakerWidth;
    float speakerWidth;
    bool setSpeakerHeight;
    float speakerHeight;
    bool setSpeakerDepth;
    float speakerDepth;
    bool setMicRadius;
    float micRadius;
    bool setAudioOutputEnabled;
    bool audioOutputEnabled;
    bool requestLoadAcousticAudio;
    bool setBakeDuration;
    float bakeDuration;
    bool setBakeParticleCount;
    int bakeParticleCount;
    bool requestBakeStart;
    bool requestBakeStop;
    bool requestBakeLoadLatest;
    bool requestBakeExportMic;
    bool setBakePlaybackTime;
    float bakePlaybackTime;
    bool setBakePlaybackPlaying;
    bool bakePlaybackPlaying;
    bool requestReset;
    bool requestCameraReset;
} Ui3DPanelActions;

void Ui3DPanelSetup(void);
void Ui3DPanelShutdown(void);
void Ui3DPanelBegin(float deltaTime);
void Ui3DPanelEnd(void);
Ui3DCaptureState Ui3DPanelGetCaptureState(void);
void Ui3DPanelDraw(const Ui3DPanelState *state, Ui3DPanelActions *actions);

#ifdef __cplusplus
}
#endif

#endif
