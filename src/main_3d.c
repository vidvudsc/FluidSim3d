    #include "raylib.h"
    #include "raymath.h"
    #include "imgui_panel_3d.h"
    #include "rlgl.h"

    #include <dispatch/dispatch.h>
    #include <ctype.h>
    #include <errno.h>
    #include <float.h>
    #include <math.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <limits.h>
    #include <stddef.h>
    #include <sys/stat.h>
    #include <time.h>
    #include <unistd.h>

    #if defined(__APPLE__)
    #ifndef GL_SILENCE_DEPRECATION
    #define GL_SILENCE_DEPRECATION
    #endif
    #import <AppKit/AppKit.h>
    #import <Foundation/Foundation.h>
    #import <Metal/Metal.h>
    #import <OpenGL/gl3.h>
    #include <mach-o/dyld.h>
    #endif

    #define WINDOW_WIDTH 1400
    #define WINDOW_HEIGHT 820
    #define MAX_PARTICLES_3D 120000
    #define DEFAULT_TARGET_PARTICLES_3D 24000
    #define CPU_COUNT_PRESET_1_3D 2000
    #define CPU_COUNT_PRESET_2_3D 4000
    #define CPU_COUNT_PRESET_3_3D 6000
    #define CPU_COUNT_PRESET_4_3D 8000
    #define CPU_COUNT_PRESET_5_3D 10000
    #define GPU_COUNT_PRESET_1_3D 16000
    #define GPU_COUNT_PRESET_2_3D 32000
    #define GPU_COUNT_PRESET_3_3D 64000
    #define GPU_COUNT_PRESET_4_3D 96000
    #define GPU_COUNT_PRESET_5_3D 120000
    #define MAX_SIM_STEPS_PER_FRAME 8
    #define PARALLEL_THRESHOLD 3072
    #define DIAGNOSTIC_REFRESH_INTERVAL 4
    #define PARTICLE_VIEW_FULL_DRAW_LIMIT 12000
    #define GPU_PARTICLE_VIEW_FULL_DRAW_LIMIT 24000
    #define SMOOTH_VIEW_FULL_DRAW_LIMIT 24000
    #define GPU_SMOOTH_VIEW_FULL_DRAW_LIMIT 160000
    #define PARTICLE_SORT_DRAW_LIMIT 60000
    #define MIC_WAVEFORM_SAMPLES 192
    #define AUDIO_OUTPUT_SAMPLE_RATE 48000
    #define AUDIO_OUTPUT_BUFFER_FRAMES 4096
    #define AUDIO_OUTPUT_GAIN 0.55f
    #define PI_F 3.14159265358979323846f
    #define TEMP_MIN 0.35f
    #define TEMP_MAX 2.50f
    #define RECT_WIDTH_MIN_3D 40.0f
    #define RECT_WIDTH_MAX_3D 140.0f
    #define RECT_HEIGHT_MIN_3D 24.0f
    #define RECT_HEIGHT_MAX_3D 96.0f
    #define FLOW_LINE_DENSITY_MIN 2
    #define FLOW_LINE_DENSITY_MAX 8
    #define FLOW_PATHLINE_COUNT_MAX (FLOW_LINE_DENSITY_MAX * FLOW_LINE_DENSITY_MAX)
    #define FLOW_PATHLINE_TRAIL_MAX 32
    #define FLOW_PATHLINE_TRAIL_MIN 4
    #define STREAMLINE_STEPS_MIN 6
    #define STREAMLINE_STEPS_MAX 40
    #define IMPORTED_SDF_MAX_RES 36
    #define IMPORTED_SDF_BAKE_MAX_RES 96
    #define IMPORTED_SDF_MIN_RES 14
    #define IMPORTED_OBSTACLE_PATH_MAX 1024
    #define CAR_SHAPE_SCALE_X 0.82f
    #define CAR_SHAPE_SCALE_Y 1.08f
    #define BAKE_DEFAULT_DURATION_SECONDS 5.0f
    #define BAKE_MIN_DURATION_SECONDS 1.0f
    #define BAKE_MAX_DURATION_SECONDS 60.0f
    #define BAKE_DEFAULT_PARTICLE_COUNT 120000
    #define BAKE_MAX_PARTICLE_COUNT 1000000
    #define BAKE_PREVIEW_FPS 24
    #define BAKE_PREVIEW_PARTICLE_CAP 200000
    #define BAKE_KEYFRAME_INTERVAL_SECONDS 1.0f
    #define BAKE_CHUNK_BUDGET_SECONDS 0.014
    #define BAKE_MIC_EXPORT_SAMPLE_RATE 48000
    #define BAKE_CACHE_VERSION 1
    #define BAKE_PATH_MAX 1024
    #define ACOUSTIC_AUDIO_PREROLL_SECONDS 1.0f
    #define ACOUSTIC_AUDIO_POSTROLL_SECONDS 1.0f
    #define ACOUSTIC_AUDIO_DRIVER_SAMPLE_RATE 48000.0f
    #define ACOUSTIC_AUDIO_SLOWDOWN_FACTOR 600.0f
    #define ACOUSTIC_AUDIO_MIN_SLOWDOWN_FACTOR 100.0f
    #define ACOUSTIC_AUDIO_MAX_SLOWDOWN_FACTOR 2400.0f
    #define ACOUSTIC_AUDIO_TARGET_BANDWIDTH_HZ 20000.0f
    #define ACOUSTIC_AUDIO_MIN_TARGET_BANDWIDTH_HZ 1000.0f
    #define ACOUSTIC_AUDIO_BANDWIDTH_HEADROOM 1.15f
    #define ACOUSTIC_AUDIO_ANALYSIS_FFT_SIZE 4096
    #define ACOUSTIC_AUDIO_ANALYSIS_MAX_WINDOWS 96
    #define ACOUSTIC_AUDIO_ANALYSIS_ROLLOFF 0.997f
    #define ACOUSTIC_AUDIO_ANALYSIS_GATE_RATIO 0.00003f
    #define ACOUSTIC_AUDIO_BAKE_SUBSTEPS_PER_SAMPLE 4.0f
    #define ACOUSTIC_AUDIO_BAKE_MIN_SUBSTEPS_PER_SAMPLE 16.0f
    #define WATER_CYLINDER_MAX UI_3D_WATER_CYLINDER_MAX

    typedef enum MaterialPreset {
        MATERIAL_WATER = 0,
        MATERIAL_GAS,
        MATERIAL_COUNT
    } MaterialPreset;

    typedef enum ViewMode {
        VIEW_PARTICLES = 0,
        VIEW_SMOOTHING_RADIUS,
        VIEW_MODE_COUNT
    } ViewMode;

    typedef enum ColorMode {
        COLOR_MATERIAL = 0,
        COLOR_TEMPERATURE,
        COLOR_PRESSURE,
        COLOR_SPEED,
        COLOR_DENSITY,
        COLOR_DYE,
        COLOR_VORTICITY,
        COLOR_MODE_COUNT
    } ColorMode;

    typedef enum SimulationBackend {
        SIM_BACKEND_CPU = 0,
        SIM_BACKEND_GPU,
        SIM_BACKEND_COUNT
    } SimulationBackend;

    typedef enum SimulationScene {
        SCENE_TANK = 0,
        SCENE_WIND_TUNNEL,
        SCENE_COUNT
    } SimulationScene;

    typedef enum ObstacleModel {
        OBSTACLE_CIRCLE = 0,
        OBSTACLE_AIRFOIL,
        OBSTACLE_CAR,
        OBSTACLE_RECTANGLE,
        OBSTACLE_IMPORTED,
        OBSTACLE_MODEL_COUNT
    } ObstacleModel;

    typedef struct MaterialParams {
        const char *name;
        float spacing;
        float supportRadius;
        float particleRadius;
        float restDensity;
        float soundSpeed;
        float kinematicViscosity;
        float gravity;
        float buoyancy;
        float temperatureDiffusion;
        float thermalExpansion;
        float ambientTemperature;
        float initialTemperatureGradient;
        float globalDrag;
        float wallBounce;
        float wallFriction;
        float timeStep;
    } MaterialParams;

    typedef struct Diagnostics {
        float minDensity;
        float maxDensity;
        float minPressure;
        float maxPressure;
        float minSpeed;
        float maxSpeed;
        float minTemperature;
        float maxTemperature;
        float avgDensity;
        float avgPressure;
        float avgSpeed;
        float avgTemperature;
        float minVorticity;
        float maxVorticity;
        float avgVorticity;
    } Diagnostics;

    typedef struct OrbitCameraState {
        Vector3 target;
        float yaw;
        float pitch;
        float distance;
        bool flyMode;
        Vector3 flyPosition;
        float flyYaw;
        float flyPitch;
    } OrbitCameraState;

    typedef struct ParticleDrawItem {
        int index;
        float depth;
    } ParticleDrawItem;

    typedef struct ParticleRenderInstance {
        float x;
        float y;
        float z;
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
        float diameter;
    } ParticleRenderInstance;

    typedef struct FlowPathline {
        Vector3 current;
        Vector3 trail[FLOW_PATHLINE_TRAIL_MAX];
        int count;
    } FlowPathline;

    typedef enum BakeStatus3D {
        BAKE_STATUS_IDLE = 0,
        BAKE_STATUS_BAKING,
        BAKE_STATUS_STOPPED,
        BAKE_STATUS_COMPLETE,
        BAKE_STATUS_PLAYBACK
    } BakeStatus3D;

    typedef struct BakePreviewParticle3D {
        float x;
        float y;
        float z;
        float vx;
        float vy;
        float vz;
        float temperature;
        float density;
        float pressure;
        float dye;
        float vorticity;
    } BakePreviewParticle3D;

    typedef struct BakeMicSample3D {
        float time;
        float value;
    } BakeMicSample3D;

    typedef struct BakeSession3D {
        BakeStatus3D status;
        float duration;
        int particleTarget;
        float startSimulationTime;
        float bakedTime;
        float progress;
        float playbackTime;
        bool playbackPlaying;
        bool hasCache;
        bool canExportMic;
        bool audioSlowMotion;
        float audioSlowdownFactor;
        char cacheDir[BAKE_PATH_MAX];
        char latestPath[BAKE_PATH_MAX];
        char notice[256];
        int previewFrameCount;
        int keyframeCount;
        int nextPreviewFrame;
        int nextKeyframeIndex;
        int previewParticleCount;
        BakePreviewParticle3D *previewParticles;
        int previewParticleCapacity;
        int loadedPreviewFrame;
        BakeMicSample3D *micSamples;
        int micSampleCount;
        int micSampleCapacity;
        double lastChunkMs;
        double wallStartTime;
        float etaSeconds;
        float lastBakedTimeForEta;
        double lastEtaWallTime;
    } BakeSession3D;

    typedef struct ImportedTriangle {
        Vector3 a;
        Vector3 b;
        Vector3 c;
    } ImportedTriangle;

    typedef struct ParticleSystem3D {
        int maxParticles;
        int particleCount;
        int targetParticleCount;
        MaterialPreset preset;
        MaterialPreset tankPreset;
        MaterialParams params;
        ViewMode viewMode;
        ColorMode colorMode;
        SimulationBackend activeBackend;
        bool bakeCapacityMode;
        SimulationScene scene;
        ObstacleModel obstacleModel;
        bool waterObstacleEnabled;
        int waterObstacleShape;
        bool waterCylindersInitialized;
        int waterCylinderCount;
        int waterCylinderSelected;
        float waterCylinderX[WATER_CYLINDER_MAX];
        float waterCylinderY[WATER_CYLINDER_MAX];
        float waterCylinderZ[WATER_CYLINDER_MAX];
        float waterCylinderRadius[WATER_CYLINDER_MAX];
        float waterCylinderDepth[WATER_CYLINDER_MAX];
        float waterRectangleWidth[WATER_CYLINDER_MAX];
        float waterRectangleHeight[WATER_CYLINDER_MAX];
        float waterRectangleAngleDegrees[WATER_CYLINDER_MAX];
        bool paused;
        bool gpuBackendAvailable;
        bool acousticsEnabled;
        bool acousticHandlesInitialized;
        float timeScale;
        float accumulator;
        float simulationTime;
        float mass;
        float supportRadiusSquared;
        float pressureStiffness;
        float densityKernel;
        float pressureKernelGrad;
        float viscosityKernelLap;
        float interactionRadius;
        float interactionRadiusSquared;
        float interactionStrength;
        Vector3 interactionPoint;
        bool interactionActive;
        float worldWidth;
        float worldHeight;
        float worldDepth;
        Vector3 boundsMin;
        Vector3 boundsMax;
        Vector3 boundsCenter;
        Vector3 boundsSize;
        float flowSpeedScale;
        Vector3 obstacleCenter;
        float obstacleRadius;
        float obstacleAngleDegrees;
        float obstacleRectWidth;
        float obstacleRectHeight;
        float obstacleDepth;
        float obstacleShell;
        float obstacleStrength;
        float obstacleDamping;
        float flowTargetSpeed;
        float flowDrive;
        int gridWidth;
        int gridHeight;
        int gridDepth;
        int cellCount;
        float cellSize;
        float invCellSize;
        float *x;
        float *y;
        float *z;
        float *vx;
        float *vy;
        float *vz;
        float *ax;
        float *ay;
        float *az;
        float *density;
        float *pressure;
        float *temperature;
        float *temperatureRate;
        float *dye;
        float *vorticity;
        float *xsphVX;
        float *xsphVY;
        float *xsphVZ;
        float *sortedX;
        float *sortedY;
        float *sortedZ;
        float *sortedVX;
        float *sortedVY;
        float *sortedVZ;
        float *sortedTemperature;
        float *sortedDensity;
        float *sortedPressure;
        int *particleCells;
        int *sortedCellIndices;
        int *sortedIndices;
        int *cellCounts;
        int *cellStarts;
        int *cellOffsets;
        int *cellNeighborCounts;
        int *cellNeighbors;
        Texture2D particleTexture;
        Shader particleShader;
        int particleShaderSoftPowerLoc;
        Shader particleInstanceShader;
        int particleInstanceMvpLoc;
        int particleInstanceRightLoc;
        int particleInstanceUpLoc;
        int particleInstanceSoftPowerLoc;
        unsigned int particleQuadVao;
        unsigned int particleQuadVbo;
        unsigned int particleInstanceVbo;
        int particleInstanceCapacity;
        ParticleRenderInstance *particleInstances;
        ParticleDrawItem *drawItems;
        int drawItemCount;
        Diagnostics stats;
        double lastSimStepMs;
        float lastStepDt;
        int framesUntilDiagnostics;
        int framesUntilVorticity;
        bool scalarFieldsDirty;
        bool sortedStateDirty;
        bool vorticityDirty;
        bool playbackPreviewMode;
        int playbackPreviewSourceCount;
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
        float pathlineAccumulator;
        FlowPathline pathlines[FLOW_PATHLINE_COUNT_MAX];
        bool importedObstacleLoaded;
        Model importedObstacleModel;
        char importedObstaclePath[IMPORTED_OBSTACLE_PATH_MAX];
        Vector3 importedObstacleRawCenter;
        float importedObstacleScale;
        float importedObstacleUserScale;
        Vector3 importedObstacleOffset;
        Quaternion importedObstacleOrientation;
        Vector3 importedObstacleRotationDegrees;
        Vector3 importedObstacleLocalMin;
        Vector3 importedObstacleLocalMax;
        int importedSdfWidth;
        int importedSdfHeight;
        int importedSdfDepth;
        int importedSdfVoxelCount;
        int importedSdfMaxResolution;
        bool importedSdfBakeQuality;
        float *importedSdfValues;
        Vector3 speakerBaseCenter;
        float speakerWidth;
        float speakerHeight;
        float speakerDepth;
        float speakerFrequency;
        float speakerAmplitude;
        float speakerShell;
        float speakerStrength;
        float speakerDamping;
        Vector3 micPosition;
        float micRadius;
        float micSignal;
        float micBaseline;
        float micWaveform[MIC_WAVEFORM_SAMPLES];
        float micWaveformDisplay[MIC_WAVEFORM_SAMPLES];
        int micWaveformHead;
        int micWaveformCount;
        float baseSoundSpeed;
        float baseKinematicViscosity;
        float baseGlobalDrag;
        float acousticSoundSpeed;
        float acousticMachLimit;
        float acousticViscosityScale;
        float acousticDragScale;
        bool acousticAudioLoaded;
        char acousticAudioPath[BAKE_PATH_MAX];
        char acousticAudioLabel[128];
        float *acousticEnvelope;
        int acousticEnvelopeCount;
        float acousticEnvelopeSampleRate;
        float acousticEnvelopeDuration;
        float *acousticWaveform;
        int acousticWaveformCount;
        float acousticWaveformSampleRate;
        float acousticAudioBandwidthHz;
        float acousticAudioSlowdownFactor;
        bool audioOutputEnabled;
        bool audioOutputReady;
        AudioStream micAudioStream;
        volatile float audioOutputSignal;
        volatile float audioOutputState;
        volatile float audioMonitorPitchHz;
        volatile float audioMonitorPhase;
        const char *backendNotice;
        double backendNoticeUntil;
        void *gpuBackend;
    } ParticleSystem3D;

    typedef struct GpuSimParams3D {
        uint32_t particleCount;
        uint32_t gridWidth;
        uint32_t gridHeight;
        uint32_t gridDepth;
        uint32_t cellCount;
        uint32_t preset;
        uint32_t scene;
        uint32_t obstacleModel;
        uint32_t obstacleEnabled;
        uint32_t waterCylinderCount;
        uint32_t waterObstacleShape;
        uint32_t interactionActive;
        uint32_t acousticsEnabled;
        uint32_t importedSdfWidth;
        uint32_t importedSdfHeight;
        uint32_t importedSdfDepth;
        float waterCylinderX[WATER_CYLINDER_MAX];
        float waterCylinderY[WATER_CYLINDER_MAX];
        float waterCylinderZ[WATER_CYLINDER_MAX];
        float waterCylinderRadius[WATER_CYLINDER_MAX];
        float waterCylinderHalfDepth[WATER_CYLINDER_MAX];
        float waterRectangleHalfWidth[WATER_CYLINDER_MAX];
        float waterRectangleHalfHeight[WATER_CYLINDER_MAX];
        float waterRectangleAngleCos[WATER_CYLINDER_MAX];
        float waterRectangleAngleSin[WATER_CYLINDER_MAX];
        float boundsMinX;
        float boundsMinY;
        float boundsMinZ;
        float boundsSizeX;
        float boundsSizeY;
        float boundsSizeZ;
        float supportRadius;
        float supportRadiusSquared;
        float particleRadius;
        float restDensity;
        float soundSpeed;
        float kinematicViscosity;
        float gravity;
        float buoyancy;
        float temperatureDiffusion;
        float thermalExpansion;
        float ambientTemperature;
        float globalDrag;
        float wallBounce;
        float wallFriction;
        float mass;
        float pressureStiffness;
        float densityKernel;
        float pressureKernelGrad;
        float viscosityKernelLap;
        float interactionRadius;
        float interactionRadiusSquared;
        float interactionStrength;
        float interactionX;
        float interactionY;
        float interactionZ;
        float obstacleCenterX;
        float obstacleCenterY;
        float obstacleCenterZ;
        float obstacleRadius;
        float obstacleAngleCos;
        float obstacleAngleSin;
        float obstacleRectHalfWidth;
        float obstacleRectHalfHeight;
        float obstacleHalfDepth;
        float importedSdfMinX;
        float importedSdfMinY;
        float importedSdfMinZ;
        float importedSdfSizeX;
        float importedSdfSizeY;
        float importedSdfSizeZ;
        float importedCenterX;
        float importedCenterY;
        float importedCenterZ;
        float importedScale;
        float importedInvRotXX;
        float importedInvRotXY;
        float importedInvRotXZ;
        float importedInvRotYX;
        float importedInvRotYY;
        float importedInvRotYZ;
        float importedInvRotZX;
        float importedInvRotZY;
        float importedInvRotZZ;
        float obstacleShell;
        float obstacleStrength;
        float obstacleDamping;
        float speakerCenterX;
        float speakerCenterY;
        float speakerCenterZ;
        float speakerVelocityX;
        float speakerVelocityY;
        float speakerVelocityZ;
        float speakerHalfWidth;
        float speakerHalfHeight;
        float speakerHalfDepth;
        float speakerShell;
        float speakerStrength;
        float speakerDamping;
        float flowTargetSpeed;
        float flowDrive;
        float simulationTime;
        float substepDt;
    } GpuSimParams3D;

    #if defined(__APPLE__)
    @interface MetalGpuBackend3D : NSObject

    - (instancetype)initWithSystem:(ParticleSystem3D *)system error:(NSString * _Nullable * _Nullable)error;
    - (void)invalidateGridBindings;
    - (BOOL)prepareForSystem:(ParticleSystem3D *)system error:(NSString * _Nullable * _Nullable)error;
    - (BOOL)runBuildDensityForceForSystem:(ParticleSystem3D *)system includeForce:(BOOL)includeForce error:(NSString * _Nullable * _Nullable)error;
    - (BOOL)runStepForSystem:(ParticleSystem3D *)system dt:(float)dt error:(NSString * _Nullable * _Nullable)error;

    @end
    #endif

    static bool InitializeGpuBackend(ParticleSystem3D *system);
    static void ShutdownGpuBackend(ParticleSystem3D *system);
    static void InvalidateGpuBackendState(ParticleSystem3D *system);
    static bool StepSimulationGPU(ParticleSystem3D *system, float frameDelta);
    static void BuildGrid(ParticleSystem3D *system);
    static void SetSimulationScene(ParticleSystem3D *system, SimulationScene scene);
    static void ResetSimulation(ParticleSystem3D *system, MaterialPreset preset);
    static void BakeSessionInit(BakeSession3D *bake);
    static void BakeSessionFree(BakeSession3D *bake);
    static void BakeStart(ParticleSystem3D *system, BakeSession3D *bake);
    static void BakeStop(BakeSession3D *bake);
    static void BakeUpdate(ParticleSystem3D *system, BakeSession3D *bake);
    static void BakePlaybackUpdate(ParticleSystem3D *system, BakeSession3D *bake, float frameDelta);
    static bool BakeSetPlaybackTime(ParticleSystem3D *system, BakeSession3D *bake, float playbackTime);
    static bool BakeLoadLatest(ParticleSystem3D *system, BakeSession3D *bake);
    static bool BakeExportMicWav(const ParticleSystem3D *system, BakeSession3D *bake);
    static bool LoadAcousticAudioFile(ParticleSystem3D *system);

    static const MaterialParams MATERIAL_PRESETS[MATERIAL_COUNT] = {
        [MATERIAL_WATER] = {
            .name = "Water-like",
            .spacing = 6.1f,
            .supportRadius = 13.0f,
            .particleRadius = 1.9f,
            .restDensity = 1.0f,
            .soundSpeed = 92.0f,
            .kinematicViscosity = 18.0f,
            .gravity = 950.0f,
            .buoyancy = 70.0f,
            .temperatureDiffusion = 0.18f,
            .thermalExpansion = 0.010f,
            .ambientTemperature = 1.0f,
            .initialTemperatureGradient = 0.02f,
            .globalDrag = 0.16f,
            .wallBounce = 0.03f,
            .wallFriction = 0.82f,
            .timeStep = 1.0f / 240.0f,
        },
        [MATERIAL_GAS] = {
            .name = "Gas-like",
            .spacing = 8.0f,
            .supportRadius = 16.0f,
            .particleRadius = 1.5f,
            .restDensity = 1.0f,
            .soundSpeed = 52.0f,
            .kinematicViscosity = 0.22f,
            .gravity = 0.0f,
            .buoyancy = 0.0f,
            .temperatureDiffusion = 0.65f,
            .thermalExpansion = 0.020f,
            .ambientTemperature = 1.0f,
            .initialTemperatureGradient = 0.0f,
            .globalDrag = 0.14f,
            .wallBounce = 0.35f,
            .wallFriction = 0.995f,
            .timeStep = 1.0f / 240.0f,
        },
    };

    static ParticleSystem3D *gAudioOutputSystem3D = NULL;

    static const Vector2 AIRFOIL_OUTLINE_POINTS[] = {
        {1.9919f, 0.0067f}, {1.9415f, 0.0171f}, {1.8914f, 0.0272f}, {1.7439f, 0.0560f},
        {1.6478f, 0.0738f}, {1.4138f, 0.1149f}, {1.2779f, 0.1372f}, {0.9748f, 0.1828f},
        {0.8103f, 0.2051f}, {0.4608f, 0.2467f}, {0.2788f, 0.2653f}, {-0.0905f, 0.2950f},
        {-0.2748f, 0.3054f}, {-0.6368f, 0.3150f}, {-0.8115f, 0.3130f}, {-1.1373f, 0.2944f},
        {-1.2856f, 0.2785f}, {-1.5474f, 0.2348f}, {-1.6584f, 0.2079f}, {-1.8354f, 0.1478f},
        {-1.9000f, 0.1157f}, {-1.9791f, 0.0496f}, {-1.9899f, -0.0149f}, {-1.9699f, -0.0445f},
        {-1.8805f, -0.0946f}, {-1.8122f, -0.1148f}, {-1.6312f, -0.1454f}, {-1.5198f, -0.1557f},
        {-1.2613f, -0.1671f}, {-1.1163f, -0.1686f}, {-0.7994f, -0.1646f}, {-0.6299f, -0.1598f},
        {-0.2757f, -0.1467f}, {-0.0941f, -0.1383f}, {0.2717f, -0.1185f}, {0.4525f, -0.1078f},
        {0.8006f, -0.0861f}, {0.9650f, -0.0755f}, {1.2690f, -0.0554f}, {1.4058f, -0.0463f},
        {1.6419f, -0.0302f}, {1.7392f, -0.0235f}, {1.8889f, -0.0130f}, {1.9399f, -0.0093f},
        {1.9916f, 0.0019f},
    };
    static const int AIRFOIL_OUTLINE_POINT_COUNT = (int)(sizeof(AIRFOIL_OUTLINE_POINTS) / sizeof(AIRFOIL_OUTLINE_POINTS[0]));

    static const Vector2 CAR_OUTLINE_POINTS[] = {
        {-2.1600f, 0.1150f}, {-2.0600f, -0.0550f}, {-1.9800f, -0.1250f}, {-1.8750f, -0.1750f},
        {-1.5950f, -0.2300f}, {-1.2500f, -0.2850f}, {-1.0700f, -0.3350f}, {-0.8850f, -0.3950f},
        {-0.4900f, -0.5150f}, {-0.0500f, -0.5600f}, {0.1700f, -0.5600f}, {0.3850f, -0.5450f},
        {0.7900f, -0.4800f}, {1.1300f, -0.4000f}, {1.2700f, -0.3600f}, {1.4000f, -0.3250f},
        {1.6400f, -0.2850f}, {1.8800f, -0.3250f}, {2.0000f, -0.3750f}, {2.1050f, -0.2550f},
        {2.1600f, -0.1100f}, {2.1150f, 0.1200f}, {2.0250f, 0.1600f}, {1.8800f, 0.1850f},
        {1.4550f, 0.2000f}, {0.9500f, 0.2000f}, {0.6900f, 0.2000f}, {0.4300f, 0.2000f},
        {-0.0900f, 0.2000f}, {-0.6150f, 0.2000f}, {-0.8850f, 0.2000f}, {-1.1400f, 0.2000f},
        {-1.6000f, 0.1950f}, {-1.9700f, 0.1750f}, {-2.1100f, 0.1650f},
    };
    static const int CAR_OUTLINE_POINT_COUNT = (int)(sizeof(CAR_OUTLINE_POINTS) / sizeof(CAR_OUTLINE_POINTS[0]));

    static float ClampFloat(float value, float minValue, float maxValue)
    {
        if (value < minValue) {
            return minValue;
        }
        if (value > maxValue) {
            return maxValue;
        }
        return value;
    }

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

    static float Saturate(float value)
    {
        return ClampFloat(value, 0.0f, 1.0f);
    }

    static float Pow7(float x)
    {
        const float x2 = x * x;
        const float x4 = x2 * x2;
        return x4 * x2 * x;
    }

    static float HashNoise(int index)
    {
        uint32_t x = (uint32_t)index * 747796405u + 2891336453u;
        x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
        x = (x >> 22u) ^ x;
        return (float)(x & 0x00FFFFFFu) / (float)0x00FFFFFFu;
    }

    static uint32_t HashUint32(uint32_t x)
    {
        x = x * 747796405u + 2891336453u;
        x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
        return (x >> 22u) ^ x;
    }

    static Color ColorRamp(float value)
    {
        const float hue = 220.0f - 220.0f * Saturate(value);
        return ColorFromHSV(hue, 0.85f, 1.0f);
    }

    static float RangeLerp(float minValue, float maxValue, float value)
    {
        const float denom = maxValue - minValue;
        if (fabsf(denom) < 1e-6f) {
            return 0.0f;
        }
        return (value - minValue) / denom;
    }

    static float LerpFloat(float minValue, float maxValue, float t)
    {
        return minValue + (maxValue - minValue) * t;
    }

    static float MinFloat3(float a, float b, float c)
    {
        return fminf(a, fminf(b, c));
    }

    static bool ShouldParallelize(int count)
    {
        return count >= PARALLEL_THRESHOLD;
    }

    static bool ColorModeNeedsFreshDensity(ColorMode colorMode)
    {
        return colorMode == COLOR_PRESSURE || colorMode == COLOR_DENSITY;
    }

    static bool ColorModeNeedsVorticity(ColorMode colorMode)
    {
        return colorMode == COLOR_VORTICITY;
    }

    static int DiagnosticRefreshIntervalForBackend(SimulationBackend backend)
    {
        return (backend == SIM_BACKEND_GPU) ? 10 : DIAGNOSTIC_REFRESH_INTERVAL;
    }

    static int VorticityRefreshIntervalForBackend(SimulationBackend backend)
    {
        return (backend == SIM_BACKEND_GPU) ? 4 : 2;
    }

    static bool SceneIsWindTunnel(const ParticleSystem3D *system)
    {
        return system->scene == SCENE_WIND_TUNNEL;
    }

    static bool SceneIsWaterTank(const ParticleSystem3D *system)
    {
        return system->scene == SCENE_TANK && system->preset == MATERIAL_WATER;
    }

    static bool WaterObstaclesActive(const ParticleSystem3D *system)
    {
        return SceneIsWaterTank(system) && system->waterObstacleEnabled && system->waterCylinderCount > 0;
    }

    static bool WaterCylindersActive(const ParticleSystem3D *system)
    {
        return WaterObstaclesActive(system) &&
            system->waterObstacleShape == UI_3D_WATER_OBSTACLE_CYLINDERS;
    }

    static bool WaterRectanglesActive(const ParticleSystem3D *system)
    {
        return WaterObstaclesActive(system) &&
            system->waterObstacleShape == UI_3D_WATER_OBSTACLE_RECTANGLES;
    }

    static bool SolidObstacleActive(const ParticleSystem3D *system)
    {
        return SceneIsWindTunnel(system) || WaterObstaclesActive(system);
    }

    static void SetDefaultWaterCylinderSlot(ParticleSystem3D *system, int index, int totalCount)
    {
        const float t = ((float)index + 0.5f) / fmaxf((float)totalCount, 1.0f);
        const float stagger = ((index & 1) == 0) ? -0.07f : 0.07f;
        system->waterCylinderX[index] = system->boundsMin.x + system->boundsSize.x * (0.40f + stagger);
        system->waterCylinderY[index] = system->boundsMin.y + system->boundsSize.y * (0.15f + 0.54f * t);
        system->waterCylinderZ[index] = system->boundsCenter.z;
        system->waterCylinderRadius[index] = fmaxf(4.0f, fminf(system->boundsSize.x, system->boundsSize.y) * 0.060f);
        system->waterCylinderDepth[index] = system->boundsSize.z * 0.92f;
    }

    static void SetDefaultWaterRectangleSlot(ParticleSystem3D *system, int index, int totalCount)
    {
        const float t = ((float)index + 0.5f) / fmaxf((float)totalCount, 1.0f);
        const float stagger = ((index & 1) == 0) ? -0.08f : 0.08f;
        system->waterCylinderX[index] = system->boundsMin.x + system->boundsSize.x * (0.43f + stagger);
        system->waterCylinderY[index] = system->boundsMin.y + system->boundsSize.y * (0.18f + 0.50f * t);
        system->waterCylinderZ[index] = system->boundsCenter.z;
        system->waterCylinderDepth[index] = system->boundsSize.z * 0.88f;
        system->waterRectangleWidth[index] = fmaxf(10.0f, system->boundsSize.x * 0.12f);
        system->waterRectangleHeight[index] = fmaxf(12.0f, system->boundsSize.y * 0.18f);
        system->waterRectangleAngleDegrees[index] = ((index & 1) == 0) ? -24.0f : 24.0f;
    }

    static void InitializeDefaultWaterCylinders(ParticleSystem3D *system)
    {
        if (system->waterCylindersInitialized) {
            return;
        }

        system->waterCylinderCount = 4;
        system->waterCylinderSelected = 0;
        for (int i = 0; i < WATER_CYLINDER_MAX; ++i) {
            SetDefaultWaterCylinderSlot(system, i, WATER_CYLINDER_MAX);
            SetDefaultWaterRectangleSlot(system, i, WATER_CYLINDER_MAX);
        }
        system->waterCylindersInitialized = true;
    }

    static void ClampWaterCylinders(ParticleSystem3D *system)
    {
        system->waterObstacleShape = ClampInt(system->waterObstacleShape, 0, UI_3D_WATER_OBSTACLE_SHAPE_COUNT - 1);
        system->waterCylinderCount = ClampInt(system->waterCylinderCount, 1, WATER_CYLINDER_MAX);
        system->waterCylinderSelected = ClampInt(system->waterCylinderSelected, 0, system->waterCylinderCount - 1);

        const float maxRadius = fmaxf(3.0f, fminf(system->boundsSize.x, system->boundsSize.y) * 0.24f);
        const float maxDepth = fmaxf(8.0f, system->boundsSize.z);
        const float maxRectWidth = fmaxf(5.0f, system->boundsSize.x * 0.72f);
        const float maxRectHeight = fmaxf(5.0f, system->boundsSize.y * 0.72f);
        for (int i = 0; i < WATER_CYLINDER_MAX; ++i) {
            float radius = ClampFloat(system->waterCylinderRadius[i], 2.5f, maxRadius);
            float depth = ClampFloat(system->waterCylinderDepth[i], 6.0f, maxDepth);
            float rectWidth = ClampFloat(system->waterRectangleWidth[i], 5.0f, maxRectWidth);
            float rectHeight = ClampFloat(system->waterRectangleHeight[i], 5.0f, maxRectHeight);
            const float obstacleMargin = (system->waterObstacleShape == UI_3D_WATER_OBSTACLE_RECTANGLES)
                ? 0.5f * sqrtf(rectWidth * rectWidth + rectHeight * rectHeight)
                : radius;
            const float xMargin = fminf(obstacleMargin + system->params.particleRadius, system->boundsSize.x * 0.45f);
            const float yMargin = fminf(obstacleMargin + system->params.particleRadius, system->boundsSize.y * 0.45f);
            const float zMargin = fminf(depth * 0.5f, system->boundsSize.z * 0.50f);
            system->waterCylinderRadius[i] = radius;
            system->waterCylinderDepth[i] = depth;
            system->waterRectangleWidth[i] = rectWidth;
            system->waterRectangleHeight[i] = rectHeight;
            system->waterRectangleAngleDegrees[i] = ClampFloat(system->waterRectangleAngleDegrees[i], -180.0f, 180.0f);
            system->waterCylinderX[i] = ClampFloat(system->waterCylinderX[i],
                system->boundsMin.x + xMargin, system->boundsMax.x - xMargin);
            system->waterCylinderY[i] = ClampFloat(system->waterCylinderY[i],
                system->boundsMin.y + yMargin, system->boundsMax.y - yMargin);
            system->waterCylinderZ[i] = ClampFloat(system->waterCylinderZ[i],
                system->boundsMin.z + zMargin, system->boundsMax.z - zMargin);
        }
    }

    static bool AcousticsAvailable(const ParticleSystem3D *system)
    {
        return system->scene == SCENE_TANK && system->preset == MATERIAL_GAS;
    }

    static bool AcousticsActive(const ParticleSystem3D *system)
    {
        return AcousticsAvailable(system) && system->acousticsEnabled;
    }

    static bool VisualizationNeedsGrid(const ParticleSystem3D *system)
    {
        return system->showStreamlines || system->showPathlines || ColorModeNeedsVorticity(system->colorMode);
    }

    static bool UseParticleForWindVorticityScale(const ParticleSystem3D *system, int index)
    {
        if (!SceneIsWindTunnel(system)) {
            return true;
        }

        const float inletCutoffX = system->boundsMin.x + system->boundsSize.x * 0.20f;
        return system->x[index] >= inletCutoffX;
    }

    static Ui3DSimMode CurrentUiMode(const ParticleSystem3D *system)
    {
        if (system->scene == SCENE_WIND_TUNNEL) {
            return UI_3D_MODE_WIND_TUNNEL;
        }
        return (system->preset == MATERIAL_GAS) ? UI_3D_MODE_GAS_TANK : UI_3D_MODE_WATER_TANK;
    }

    static int BackendMinTargetParticleCount(SimulationBackend backend)
    {
        return (backend == SIM_BACKEND_GPU) ? GPU_COUNT_PRESET_1_3D : CPU_COUNT_PRESET_1_3D;
    }

    static int BackendMaxTargetParticleCount(const ParticleSystem3D *system, SimulationBackend backend)
    {
        const int backendLimit = (backend == SIM_BACKEND_GPU) ? GPU_COUNT_PRESET_5_3D : CPU_COUNT_PRESET_5_3D;
        return ClampInt(backendLimit, 1, system->maxParticles);
    }

    static int CountPresetForBackend(SimulationBackend backend, int presetIndex)
    {
        switch (presetIndex) {
            case 0: return (backend == SIM_BACKEND_GPU) ? GPU_COUNT_PRESET_1_3D : CPU_COUNT_PRESET_1_3D;
            case 1: return (backend == SIM_BACKEND_GPU) ? GPU_COUNT_PRESET_2_3D : CPU_COUNT_PRESET_2_3D;
            case 2: return (backend == SIM_BACKEND_GPU) ? GPU_COUNT_PRESET_3_3D : CPU_COUNT_PRESET_3_3D;
            case 3: return (backend == SIM_BACKEND_GPU) ? GPU_COUNT_PRESET_4_3D : CPU_COUNT_PRESET_4_3D;
            case 4:
            default:
                return (backend == SIM_BACKEND_GPU) ? GPU_COUNT_PRESET_5_3D : CPU_COUNT_PRESET_5_3D;
        }
    }

    static int EffectiveTargetParticleCount(const ParticleSystem3D *system)
    {
        if (system->bakeCapacityMode) {
            return ClampInt(system->targetParticleCount, 1, system->maxParticles);
        }

        return ClampInt(system->targetParticleCount,
            BackendMinTargetParticleCount(system->activeBackend),
            BackendMaxTargetParticleCount(system, system->activeBackend));
    }

    static float SmoothViewCoverageScale(const ParticleSystem3D *system)
    {
        if (system->playbackPreviewMode && system->playbackPreviewSourceCount > system->particleCount) {
            const float ratio = (float)system->playbackPreviewSourceCount / (float)fmaxf(system->particleCount, 1);
            return ClampFloat(1.0f + 0.10f * log2f(fmaxf(ratio, 1.0f)), 1.0f, 1.30f);
        }
        if (system->viewMode != VIEW_SMOOTHING_RADIUS || system->particleCount <= SMOOTH_VIEW_FULL_DRAW_LIMIT) {
            return 1.0f;
        }

        const float ratio = (float)system->particleCount / (float)SMOOTH_VIEW_FULL_DRAW_LIMIT;
        return ClampFloat(1.0f + 0.025f * log2f(fmaxf(ratio, 1.0f)), 1.0f, 1.10f);
    }

    static float ParticleViewCoverageScale(const ParticleSystem3D *system)
    {
        if (system->playbackPreviewMode && system->playbackPreviewSourceCount > system->particleCount) {
            const float ratio = (float)system->playbackPreviewSourceCount / (float)fmaxf(system->particleCount, 1);
            return ClampFloat(1.0f + 0.12f * log2f(fmaxf(ratio, 1.0f)), 1.0f, 1.35f);
        }
        const int fullDrawLimit = (system->activeBackend == SIM_BACKEND_GPU)
            ? GPU_PARTICLE_VIEW_FULL_DRAW_LIMIT
            : PARTICLE_VIEW_FULL_DRAW_LIMIT;
        if (system->viewMode != VIEW_PARTICLES || system->particleCount <= fullDrawLimit) {
            return 1.0f;
        }

        const float ratio = (float)system->particleCount / (float)fullDrawLimit;
        return ClampFloat(1.0f + 0.030f * log2f(fmaxf(ratio, 1.0f)), 1.0f, 1.16f);
    }

    static int SmoothViewDrawLimit(const ParticleSystem3D *system)
    {
        if (system->playbackPreviewMode) {
            return ClampInt(system->particleCount, 1, BAKE_PREVIEW_PARTICLE_CAP);
        }
        return (system->activeBackend == SIM_BACKEND_GPU)
            ? GPU_SMOOTH_VIEW_FULL_DRAW_LIMIT
            : SMOOTH_VIEW_FULL_DRAW_LIMIT;
    }

    static int ParticleViewDrawLimit(const ParticleSystem3D *system)
    {
        if (system->playbackPreviewMode) {
            return ClampInt(system->particleCount, 1, BAKE_PREVIEW_PARTICLE_CAP);
        }
        return (system->activeBackend == SIM_BACKEND_GPU)
            ? GPU_PARTICLE_VIEW_FULL_DRAW_LIMIT
            : PARTICLE_VIEW_FULL_DRAW_LIMIT;
    }

    static bool ShouldDrawParticleSample(int index, int drawStride)
    {
        if (drawStride <= 1) {
            return true;
        }

        return (HashUint32((uint32_t)index + 0x9E3779B9u) % (uint32_t)drawStride) == 0u;
    }

    static float LocalRestDensity(const ParticleSystem3D *system, float temperature)
    {
        const float delta = temperature - system->params.ambientTemperature;
        const float adjusted = system->params.restDensity * (1.0f - system->params.thermalExpansion * delta);
        return fmaxf(0.35f * system->params.restDensity, adjusted);
    }

    static float DensityFloor(const ParticleSystem3D *system)
    {
        return (system->preset == MATERIAL_GAS)
            ? 0.10f * system->params.restDensity
            : 0.50f * system->params.restDensity;
    }

    static float BoundaryMassScale(const ParticleSystem3D *system)
    {
        (void)system;
        return 1.12f;
    }

    static float XsphBlend(const ParticleSystem3D *system)
    {
        return (system->preset == MATERIAL_GAS) ? 0.0f : 0.055f;
    }

    static bool ShortcutAllowedWhileUiFocused(int key)
    {
        return key == KEY_SPACE || key == KEY_C || key == KEY_V;
    }

    static float PolygonSignedArea(const Vector2 *points, int pointCount)
    {
        float area = 0.0f;
        for (int i = 0; i < pointCount; ++i) {
            const Vector2 a = points[i];
            const Vector2 b = points[(i + 1) % pointCount];
            area += a.x * b.y - b.x * a.y;
        }
        return 0.5f * area;
    }

    static bool PointInTriangle(Vector2 p, Vector2 a, Vector2 b, Vector2 c)
    {
        const float ab = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
        const float bc = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
        const float ca = (a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x);
        const bool hasNeg = (ab < -1e-5f) || (bc < -1e-5f) || (ca < -1e-5f);
        const bool hasPos = (ab > 1e-5f) || (bc > 1e-5f) || (ca > 1e-5f);
        return !(hasNeg && hasPos);
    }

    static bool PolygonVertexIsEar(const Vector2 *points, const int *indices, int remainingCount, int vertex,
        float orientationSign)
    {
        const int prevIndex = indices[(vertex + remainingCount - 1) % remainingCount];
        const int currIndex = indices[vertex];
        const int nextIndex = indices[(vertex + 1) % remainingCount];
        const Vector2 a = points[prevIndex];
        const Vector2 b = points[currIndex];
        const Vector2 c = points[nextIndex];
        const float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

        if (cross * orientationSign <= 1e-5f) {
            return false;
        }

        for (int i = 0; i < remainingCount; ++i) {
            const int testIndex = indices[i];
            if (testIndex == prevIndex || testIndex == currIndex || testIndex == nextIndex) {
                continue;
            }
            if (PointInTriangle(points[testIndex], a, b, c)) {
                return false;
            }
        }
        return true;
    }

    static float DistanceSquaredToSegment(Vector2 p, Vector2 a, Vector2 b)
    {
        const Vector2 ab = {b.x - a.x, b.y - a.y};
        const Vector2 ap = {p.x - a.x, p.y - a.y};
        const float abLengthSquared = ab.x * ab.x + ab.y * ab.y;
        const float t = (abLengthSquared > 1e-8f)
            ? ClampFloat((ap.x * ab.x + ap.y * ab.y) / abLengthSquared, 0.0f, 1.0f)
            : 0.0f;
        const Vector2 closest = {a.x + ab.x * t, a.y + ab.y * t};
        const float dx = p.x - closest.x;
        const float dy = p.y - closest.y;
        return dx * dx + dy * dy;
    }

    static float PolygonSignedDistanceScaled(const Vector2 *points, int pointCount, Vector2 p, float scale)
    {
        float minDistanceSquared = FLT_MAX;
        bool inside = false;

        for (int i = 0, j = pointCount - 1; i < pointCount; j = i++) {
            const Vector2 a = {points[i].x * scale, points[i].y * scale};
            const Vector2 b = {points[j].x * scale, points[j].y * scale};
            minDistanceSquared = fminf(minDistanceSquared, DistanceSquaredToSegment(p, a, b));

            const float yDelta = b.y - a.y;
            const bool intersects = ((a.y > p.y) != (b.y > p.y)) &&
                (p.x < (b.x - a.x) * (p.y - a.y) / (fabsf(yDelta) > 1e-6f ? yDelta : 1e-6f) + a.x);
            if (intersects) {
                inside = !inside;
            }
        }

        const float distance = sqrtf(fmaxf(minDistanceSquared, 0.0f));
        return inside ? -distance : distance;
    }

    static float PolygonSignedDistanceScaledXY(const Vector2 *points, int pointCount, Vector2 p, Vector2 scale)
    {
        float minDistanceSquared = FLT_MAX;
        bool inside = false;

        for (int i = 0, j = pointCount - 1; i < pointCount; j = i++) {
            const Vector2 a = {points[i].x * scale.x, points[i].y * scale.y};
            const Vector2 b = {points[j].x * scale.x, points[j].y * scale.y};
            minDistanceSquared = fminf(minDistanceSquared, DistanceSquaredToSegment(p, a, b));

            const float yDelta = b.y - a.y;
            const bool intersects = ((a.y > p.y) != (b.y > p.y)) &&
                (p.x < (b.x - a.x) * (p.y - a.y) / (fabsf(yDelta) > 1e-6f ? yDelta : 1e-6f) + a.x);
            if (intersects) {
                inside = !inside;
            }
        }

        const float distance = sqrtf(fmaxf(minDistanceSquared, 0.0f));
        return inside ? -distance : distance;
    }

    static float RectangleSignedDistance(Vector2 p, Vector2 halfSize)
    {
        const Vector2 d = {fabsf(p.x) - halfSize.x, fabsf(p.y) - halfSize.y};
        const float ox = fmaxf(d.x, 0.0f);
        const float oy = fmaxf(d.y, 0.0f);
        return sqrtf(ox * ox + oy * oy) + fminf(fmaxf(d.x, d.y), 0.0f);
    }

    static float BoxSignedDistance(Vector3 p, Vector3 halfSize)
    {
        const Vector3 d = {
            fabsf(p.x) - halfSize.x,
            fabsf(p.y) - halfSize.y,
            fabsf(p.z) - halfSize.z,
        };
        const Vector3 outside = {
            fmaxf(d.x, 0.0f),
            fmaxf(d.y, 0.0f),
            fmaxf(d.z, 0.0f),
        };
        return Vector3Length(outside) + fminf(fmaxf(d.x, fmaxf(d.y, d.z)), 0.0f);
    }

    static Vector2 RotateVector(Vector2 p, float radians)
    {
        const float c = cosf(radians);
        const float s = sinf(radians);
        return (Vector2){
            c * p.x - s * p.y,
            s * p.x + c * p.y,
        };
    }

    static float ObstacleAngleRadians(const ParticleSystem3D *system)
    {
        return system->obstacleAngleDegrees * (PI_F / 180.0f);
    }

    static Vector3 ObstacleWorldToLocal(const ParticleSystem3D *system, float x, float y, float z)
    {
        const Vector2 rotated = RotateVector((Vector2){
            x - system->obstacleCenter.x,
            y - system->obstacleCenter.y,
        }, -ObstacleAngleRadians(system));

        return (Vector3){
            rotated.x,
            rotated.y,
            z - system->obstacleCenter.z,
        };
    }

    static Vector3 ObstacleLocalToWorld(const ParticleSystem3D *system, Vector3 local)
    {
        const Vector2 rotated = RotateVector((Vector2){local.x, local.y}, ObstacleAngleRadians(system));
        return (Vector3){
            system->obstacleCenter.x + rotated.x,
            system->obstacleCenter.y + rotated.y,
            system->obstacleCenter.z + local.z,
        };
    }

    static Vector3 ClampVector3ToBox(Vector3 p, Vector3 minBox, Vector3 maxBox)
    {
        return (Vector3){
            ClampFloat(p.x, minBox.x, maxBox.x),
            ClampFloat(p.y, minBox.y, maxBox.y),
            ClampFloat(p.z, minBox.z, maxBox.z),
        };
    }

    static bool OpenObjFileDialog(char *outPath, size_t outPathSize)
    {
    #if defined(__APPLE__)
        @autoreleasepool {
            NSOpenPanel *panel = [NSOpenPanel openPanel];
            [panel setCanChooseFiles:YES];
            [panel setCanChooseDirectories:NO];
            [panel setAllowsMultipleSelection:NO];
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
            [panel setAllowedFileTypes:@[@"obj"]];
    #pragma clang diagnostic pop
            if ([panel runModal] == NSModalResponseOK && panel.URL.path != nil) {
                snprintf(outPath, outPathSize, "%s", panel.URL.path.UTF8String);
                return true;
            }
        }
    #else
        (void)outPath;
        (void)outPathSize;
    #endif
        return false;
    }

    static bool OpenAudioFileDialog(char *outPath, size_t outPathSize)
    {
    #if defined(__APPLE__)
        @autoreleasepool {
            NSOpenPanel *panel = [NSOpenPanel openPanel];
            [panel setCanChooseFiles:YES];
            [panel setCanChooseDirectories:NO];
            [panel setAllowsMultipleSelection:NO];
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
            [panel setAllowedFileTypes:@[@"wav", @"mp3"]];
    #pragma clang diagnostic pop
            if ([panel runModal] == NSModalResponseOK && panel.URL.path != nil) {
                snprintf(outPath, outPathSize, "%s", panel.URL.path.UTF8String);
                return true;
            }
        }
    #else
        (void)outPath;
        (void)outPathSize;
    #endif
        return false;
    }

static Vector3 ImportedObstacleWorldCenter(const ParticleSystem3D *system)
{
    return Vector3Add(system->obstacleCenter, system->importedObstacleOffset);
}

static Quaternion ImportedObstacleOrientationQuaternion(const ParticleSystem3D *system)
{
    Quaternion q = system->importedObstacleOrientation;
    const float lengthSquared = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (lengthSquared <= 1e-8f) {
        q = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
    }
    return QuaternionNormalize(q);
}

static void ResetImportedObstacleRotationState(ParticleSystem3D *system)
{
    system->importedObstacleOrientation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
    system->importedObstacleRotationDegrees = (Vector3){0.0f, 0.0f, 0.0f};
}

static void ApplyImportedObstacleRotationDeltaWorld(ParticleSystem3D *system, Vector3 axis, float deltaDegrees)
{
    if (fabsf(deltaDegrees) <= 1e-4f) {
        return;
    }

    const Quaternion current = ImportedObstacleOrientationQuaternion(system);
    const Quaternion delta = QuaternionFromAxisAngle(axis, deltaDegrees * DEG2RAD);
    system->importedObstacleOrientation = QuaternionNormalize(QuaternionMultiply(delta, current));
}


static Vector3 TransformDirection(Matrix transform, Vector3 v)
{
        return (Vector3){
            transform.m0 * v.x + transform.m4 * v.y + transform.m8 * v.z,
            transform.m1 * v.x + transform.m5 * v.y + transform.m9 * v.z,
            transform.m2 * v.x + transform.m6 * v.y + transform.m10 * v.z,
        };
    }

static Vector3 RotateImportedObstacleWorldDirectionToLocal(const ParticleSystem3D *system, Vector3 v)
{
    const Quaternion inverse = QuaternionInvert(ImportedObstacleOrientationQuaternion(system));
    return Vector3RotateByQuaternion(v, inverse);
}

static Matrix ImportedObstacleRotationMatrix(const ParticleSystem3D *system)
{
    return QuaternionToMatrix(ImportedObstacleOrientationQuaternion(system));
}

    static Matrix ImportedObstacleDrawTransformMatrix(const ParticleSystem3D *system)
    {
        const float userScale = fmaxf(system->importedObstacleUserScale, 0.05f);
        const Vector3 center = ImportedObstacleWorldCenter(system);
        Matrix transform = ImportedObstacleRotationMatrix(system);
        transform.m0 *= userScale;
        transform.m1 *= userScale;
        transform.m2 *= userScale;
        transform.m4 *= userScale;
        transform.m5 *= userScale;
        transform.m6 *= userScale;
        transform.m8 *= userScale;
        transform.m9 *= userScale;
        transform.m10 *= userScale;
        transform.m12 = center.x;
        transform.m13 = center.y;
        transform.m14 = center.z;
        return transform;
    }

    static float DistanceSquaredPointTriangle3D(Vector3 p, Vector3 a, Vector3 b, Vector3 c)
    {
        const Vector3 ab = Vector3Subtract(b, a);
        const Vector3 ac = Vector3Subtract(c, a);
        const Vector3 ap = Vector3Subtract(p, a);
        const float d1 = Vector3DotProduct(ab, ap);
        const float d2 = Vector3DotProduct(ac, ap);
        if (d1 <= 0.0f && d2 <= 0.0f) {
            return Vector3LengthSqr(ap);
        }

        const Vector3 bp = Vector3Subtract(p, b);
        const float d3 = Vector3DotProduct(ab, bp);
        const float d4 = Vector3DotProduct(ac, bp);
        if (d3 >= 0.0f && d4 <= d3) {
            return Vector3LengthSqr(bp);
        }

        const float vc = d1 * d4 - d3 * d2;
        if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
            const float v = d1 / (d1 - d3);
            const Vector3 projection = Vector3Add(a, Vector3Scale(ab, v));
            return Vector3LengthSqr(Vector3Subtract(p, projection));
        }

        const Vector3 cp = Vector3Subtract(p, c);
        const float d5 = Vector3DotProduct(ab, cp);
        const float d6 = Vector3DotProduct(ac, cp);
        if (d6 >= 0.0f && d5 <= d6) {
            return Vector3LengthSqr(cp);
        }

        const float vb = d5 * d2 - d1 * d6;
        if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
            const float w = d2 / (d2 - d6);
            const Vector3 projection = Vector3Add(a, Vector3Scale(ac, w));
            return Vector3LengthSqr(Vector3Subtract(p, projection));
        }

        const float va = d3 * d6 - d5 * d4;
        if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
            const Vector3 bc = Vector3Subtract(c, b);
            const float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
            const Vector3 projection = Vector3Add(b, Vector3Scale(bc, w));
            return Vector3LengthSqr(Vector3Subtract(p, projection));
        }

        const float denom = 1.0f / (va + vb + vc);
        const float v = vb * denom;
        const float w = vc * denom;
        const Vector3 projection = Vector3Add(a, Vector3Add(Vector3Scale(ab, v), Vector3Scale(ac, w)));
        return Vector3LengthSqr(Vector3Subtract(p, projection));
    }

    static bool RayIntersectsTriangle3D(Vector3 origin, Vector3 direction, Vector3 a, Vector3 b, Vector3 c, float *tOut)
    {
        const Vector3 edge1 = Vector3Subtract(b, a);
        const Vector3 edge2 = Vector3Subtract(c, a);
        const Vector3 pvec = Vector3CrossProduct(direction, edge2);
        const float det = Vector3DotProduct(edge1, pvec);
        if (fabsf(det) < 1e-7f) {
            return false;
        }

        const float invDet = 1.0f / det;
        const Vector3 tvec = Vector3Subtract(origin, a);
        const float u = Vector3DotProduct(tvec, pvec) * invDet;
        if (u < 0.0f || u > 1.0f) {
            return false;
        }

        const Vector3 qvec = Vector3CrossProduct(tvec, edge1);
        const float v = Vector3DotProduct(direction, qvec) * invDet;
        if (v < 0.0f || (u + v) > 1.0f) {
            return false;
        }

        const float t = Vector3DotProduct(edge2, qvec) * invDet;
        if (t <= 1e-5f) {
            return false;
        }

        if (tOut != NULL) {
            *tOut = t;
        }
        return true;
    }

    static float SampleImportedObstacleSdf(const ParticleSystem3D *system, Vector3 p)
    {
        if (!system->importedObstacleLoaded || system->importedSdfValues == NULL ||
            system->importedSdfWidth < 2 || system->importedSdfHeight < 2 || system->importedSdfDepth < 2) {
            return 1e6f;
        }

        const Vector3 clamped = ClampVector3ToBox(p, system->importedObstacleLocalMin, system->importedObstacleLocalMax);
        const Vector3 outside = Vector3Subtract(p, clamped);
        const float outsideDistance = Vector3Length(outside);
        const Vector3 size = Vector3Subtract(system->importedObstacleLocalMax, system->importedObstacleLocalMin);
        if (size.x <= 1e-6f || size.y <= 1e-6f || size.z <= 1e-6f) {
            return 1e6f;
        }

        const float gx = ((clamped.x - system->importedObstacleLocalMin.x) / size.x) * (float)system->importedSdfWidth - 0.5f;
        const float gy = ((clamped.y - system->importedObstacleLocalMin.y) / size.y) * (float)system->importedSdfHeight - 0.5f;
        const float gz = ((clamped.z - system->importedObstacleLocalMin.z) / size.z) * (float)system->importedSdfDepth - 0.5f;

        const int x0 = ClampInt((int)floorf(gx), 0, system->importedSdfWidth - 1);
        const int y0 = ClampInt((int)floorf(gy), 0, system->importedSdfHeight - 1);
        const int z0 = ClampInt((int)floorf(gz), 0, system->importedSdfDepth - 1);
        const int x1 = ClampInt(x0 + 1, 0, system->importedSdfWidth - 1);
        const int y1 = ClampInt(y0 + 1, 0, system->importedSdfHeight - 1);
        const int z1 = ClampInt(z0 + 1, 0, system->importedSdfDepth - 1);
        const float tx = ClampFloat(gx - (float)x0, 0.0f, 1.0f);
        const float ty = ClampFloat(gy - (float)y0, 0.0f, 1.0f);
        const float tz = ClampFloat(gz - (float)z0, 0.0f, 1.0f);

        const int strideY = system->importedSdfWidth;
        const int strideZ = system->importedSdfWidth * system->importedSdfHeight;
        #define SDF_AT(ix, iy, iz) system->importedSdfValues[(iz) * strideZ + (iy) * strideY + (ix)]
        const float c000 = SDF_AT(x0, y0, z0);
        const float c100 = SDF_AT(x1, y0, z0);
        const float c010 = SDF_AT(x0, y1, z0);
        const float c110 = SDF_AT(x1, y1, z0);
        const float c001 = SDF_AT(x0, y0, z1);
        const float c101 = SDF_AT(x1, y0, z1);
        const float c011 = SDF_AT(x0, y1, z1);
        const float c111 = SDF_AT(x1, y1, z1);
        #undef SDF_AT

        const float c00 = LerpFloat(c000, c100, tx);
        const float c10 = LerpFloat(c010, c110, tx);
        const float c01 = LerpFloat(c001, c101, tx);
        const float c11 = LerpFloat(c011, c111, tx);
        const float c0 = LerpFloat(c00, c10, ty);
        const float c1 = LerpFloat(c01, c11, ty);
        return LerpFloat(c0, c1, tz) + outsideDistance;
    }

    static float SampleImportedObstacleSdfWorld(const ParticleSystem3D *system, Vector3 worldPoint)
    {
        const Vector3 center = ImportedObstacleWorldCenter(system);
        Vector3 localPoint = Vector3Subtract(worldPoint, center);
        localPoint = RotateImportedObstacleWorldDirectionToLocal(system, localPoint);
        const float userScale = fmaxf(system->importedObstacleUserScale, 0.05f);
        localPoint = Vector3Scale(localPoint, 1.0f / userScale);
        return SampleImportedObstacleSdf(system, localPoint) * userScale;
    }

    static float ExtrudeSignedDistance(float baseDistance, float localZ, float halfDepth)
    {
        const float dz = fabsf(localZ) - halfDepth;
        const float outsideX = fmaxf(baseDistance, 0.0f);
        const float outsideY = fmaxf(dz, 0.0f);
        return sqrtf(outsideX * outsideX + outsideY * outsideY) + fminf(fmaxf(baseDistance, dz), 0.0f);
    }

    static float WaterCylinderSignedDistance(const ParticleSystem3D *system, float x, float y, float z)
    {
        if (!WaterCylindersActive(system)) {
            return 1e6f;
        }

        float minDistance = 1e6f;
        const int count = ClampInt(system->waterCylinderCount, 0, WATER_CYLINDER_MAX);
        for (int i = 0; i < count; ++i) {
            const float dx = x - system->waterCylinderX[i];
            const float dy = y - system->waterCylinderY[i];
            const float radialDistance = sqrtf(dx * dx + dy * dy) - system->waterCylinderRadius[i];
            const float distance = ExtrudeSignedDistance(radialDistance,
                z - system->waterCylinderZ[i], system->waterCylinderDepth[i] * 0.5f);
            minDistance = fminf(minDistance, distance);
        }
        return minDistance;
    }

    static float WaterRectangleSignedDistance(const ParticleSystem3D *system, float x, float y, float z)
    {
        if (!WaterRectanglesActive(system)) {
            return 1e6f;
        }

        float minDistance = 1e6f;
        const int count = ClampInt(system->waterCylinderCount, 0, WATER_CYLINDER_MAX);
        for (int i = 0; i < count; ++i) {
            const Vector2 local = RotateVector((Vector2){
                x - system->waterCylinderX[i],
                y - system->waterCylinderY[i],
            }, -system->waterRectangleAngleDegrees[i] * DEG2RAD);
            const float baseDistance = RectangleSignedDistance(local, (Vector2){
                system->waterRectangleWidth[i] * 0.5f,
                system->waterRectangleHeight[i] * 0.5f,
            });
            const float distance = ExtrudeSignedDistance(baseDistance,
                z - system->waterCylinderZ[i], system->waterCylinderDepth[i] * 0.5f);
            minDistance = fminf(minDistance, distance);
        }
        return minDistance;
    }

    static float ObstacleSignedDistanceLocal(const ParticleSystem3D *system, Vector3 p)
    {
        float baseDistance = 0.0f;
        switch (system->obstacleModel) {
            case OBSTACLE_AIRFOIL:
                baseDistance = PolygonSignedDistanceScaled(AIRFOIL_OUTLINE_POINTS, AIRFOIL_OUTLINE_POINT_COUNT,
                    (Vector2){p.x, p.y}, system->obstacleRadius);
                break;
            case OBSTACLE_CAR:
                baseDistance = PolygonSignedDistanceScaledXY(CAR_OUTLINE_POINTS, CAR_OUTLINE_POINT_COUNT,
                    (Vector2){p.x, -p.y},
                    (Vector2){system->obstacleRadius * CAR_SHAPE_SCALE_X, system->obstacleRadius * CAR_SHAPE_SCALE_Y});
                break;
            case OBSTACLE_RECTANGLE:
                baseDistance = RectangleSignedDistance((Vector2){p.x, p.y},
                    (Vector2){system->obstacleRectWidth * 0.5f, system->obstacleRectHeight * 0.5f});
                break;
            case OBSTACLE_IMPORTED:
                return SampleImportedObstacleSdf(system, p);
            case OBSTACLE_CIRCLE:
            default:
                baseDistance = sqrtf(p.x * p.x + p.y * p.y) - system->obstacleRadius;
                break;
        }

        return ExtrudeSignedDistance(baseDistance, p.z, system->obstacleDepth * 0.5f);
    }

    static float ObstacleSignedDistance(const ParticleSystem3D *system, float x, float y, float z)
    {
        if (system->obstacleModel == OBSTACLE_IMPORTED) {
            return SampleImportedObstacleSdfWorld(system, (Vector3){x, y, z});
        }
        return ObstacleSignedDistanceLocal(system, ObstacleWorldToLocal(system, x, y, z));
    }

    static Vector3 ObstacleNormal(const ParticleSystem3D *system, float x, float y, float z)
    {
        const float epsilon = fmaxf(system->params.particleRadius * 0.65f, 0.75f);
        const float dx = ObstacleSignedDistance(system, x + epsilon, y, z) - ObstacleSignedDistance(system, x - epsilon, y, z);
        const float dy = ObstacleSignedDistance(system, x, y + epsilon, z) - ObstacleSignedDistance(system, x, y - epsilon, z);
        const float dz = ObstacleSignedDistance(system, x, y, z + epsilon) - ObstacleSignedDistance(system, x, y, z - epsilon);
        const Vector3 normal = {dx, dy, dz};
        const float length = Vector3Length(normal);
        if (length > 1e-6f) {
            return Vector3Scale(normal, 1.0f / length);
        }

        const Vector3 fallback = Vector3Subtract((Vector3){x, y, z}, system->obstacleCenter);
        const float fallbackLength = Vector3Length(fallback);
        if (fallbackLength > 1e-6f) {
            return Vector3Scale(fallback, 1.0f / fallbackLength);
        }
        return (Vector3){1.0f, 0.0f, 0.0f};
    }

    static float SolidObstacleSignedDistance(const ParticleSystem3D *system, float x, float y, float z)
    {
        if (SceneIsWindTunnel(system)) {
            return ObstacleSignedDistance(system, x, y, z);
        }
        if (WaterCylindersActive(system)) {
            return WaterCylinderSignedDistance(system, x, y, z);
        }
        if (WaterRectanglesActive(system)) {
            return WaterRectangleSignedDistance(system, x, y, z);
        }
        return 1e6f;
    }

    static Vector3 SolidObstacleNormal(const ParticleSystem3D *system, float x, float y, float z)
    {
        if (SceneIsWindTunnel(system)) {
            return ObstacleNormal(system, x, y, z);
        }

        const float epsilon = fmaxf(system->params.particleRadius * 0.65f, 0.75f);
        const float dx = SolidObstacleSignedDistance(system, x + epsilon, y, z) -
            SolidObstacleSignedDistance(system, x - epsilon, y, z);
        const float dy = SolidObstacleSignedDistance(system, x, y + epsilon, z) -
            SolidObstacleSignedDistance(system, x, y - epsilon, z);
        const float dz = SolidObstacleSignedDistance(system, x, y, z + epsilon) -
            SolidObstacleSignedDistance(system, x, y, z - epsilon);
        const Vector3 normal = {dx, dy, dz};
        const float length = Vector3Length(normal);
        if (length > 1e-6f) {
            return Vector3Scale(normal, 1.0f / length);
        }

        const int selectedMax = (system->waterCylinderCount > 0) ? system->waterCylinderCount - 1 : 0;
        const int selected = ClampInt(system->waterCylinderSelected, 0, selectedMax);
        const Vector3 fallback = {
            x - system->waterCylinderX[selected],
            y - system->waterCylinderY[selected],
            z - system->waterCylinderZ[selected],
        };
        const float fallbackLength = Vector3Length(fallback);
        if (fallbackLength > 1e-6f) {
            return Vector3Scale(fallback, 1.0f / fallbackLength);
        }
        return (Vector3){1.0f, 0.0f, 0.0f};
    }

    static float WindTunnelAxisProfile(float value, float minValue, float maxValue)
    {
        const float t = ClampFloat(RangeLerp(minValue, maxValue, value), 0.0f, 1.0f);
        const float centered = 2.0f * t - 1.0f;
        return fmaxf(0.12f, 1.0f - centered * centered);
    }

    static float WindTunnelProfile(const ParticleSystem3D *system, float y, float z)
    {
        const float radius = system->params.particleRadius;
        const float yProfile = WindTunnelAxisProfile(y, system->boundsMin.y + radius, system->boundsMax.y - radius);
        const float zProfile = WindTunnelAxisProfile(z, system->boundsMin.z + radius, system->boundsMax.z - radius);
        return yProfile * zProfile;
    }

    static void SetBackendNotice(ParticleSystem3D *system, const char *message)
    {
        system->backendNotice = message;
        system->backendNoticeUntil = GetTime() + 2.0;
    }

    static float EffectiveAcousticSoundSpeed(const ParticleSystem3D *system)
    {
        const float baseSoundSpeed = (system->baseSoundSpeed > 0.0f) ? system->baseSoundSpeed : system->params.soundSpeed;
        if (!AcousticsActive(system)) {
            return baseSoundSpeed;
        }
        return fmaxf(baseSoundSpeed, system->acousticSoundSpeed);
    }

    static float AcousticAudioSlowdownFactor(const ParticleSystem3D *system);
    static bool AcousticSlowMotionAudioActive(const ParticleSystem3D *system);
    static float AcousticDriverTimeFromSimulationTime(const ParticleSystem3D *system, float simulationTime);

    static float EffectiveSpeakerAmplitude(const ParticleSystem3D *system)
    {
        if (!AcousticsActive(system)) {
            return system->speakerAmplitude;
        }

        const float frequency = fmaxf(system->speakerFrequency, 1e-4f);
        const float maxSurfaceSpeed = ClampFloat(system->acousticMachLimit, 0.05f, 1.25f) *
            fmaxf(EffectiveAcousticSoundSpeed(system), 1e-4f);
        const float cappedAmplitude = maxSurfaceSpeed / (2.0f * PI_F * frequency);
        return fminf(system->speakerAmplitude, cappedAmplitude);
    }

    static float AcousticEnvelopeMultiplier(const ParticleSystem3D *system)
    {
        if (!system->acousticAudioLoaded || system->acousticEnvelope == NULL ||
            system->acousticEnvelopeCount <= 1 || system->acousticEnvelopeSampleRate <= 0.0f) {
            return 1.0f;
        }

        const float driverTime = AcousticDriverTimeFromSimulationTime(system, system->simulationTime);
        const float samplePosition = (driverTime - ACOUSTIC_AUDIO_PREROLL_SECONDS) *
            system->acousticEnvelopeSampleRate;
        if (samplePosition < 0.0f || samplePosition >= (float)(system->acousticEnvelopeCount - 1)) {
            return 0.0f;
        }

        const int i0 = (int)floorf(samplePosition);
        const int i1 = i0 + 1;
        const float t = samplePosition - (float)i0;
        const float a = system->acousticEnvelope[i0];
        const float b = system->acousticEnvelope[i1];
        return ClampFloat(a + (b - a) * t, 0.0f, 1.0f);
    }

    static float AcousticWaveformSample(const ParticleSystem3D *system, float time)
    {
        if (!system->acousticAudioLoaded || system->acousticWaveform == NULL ||
            system->acousticWaveformCount <= 1 || system->acousticWaveformSampleRate <= 0.0f) {
            return 0.0f;
        }

        const float samplePosition = (time - ACOUSTIC_AUDIO_PREROLL_SECONDS) * system->acousticWaveformSampleRate;
        if (samplePosition < 0.0f || samplePosition >= (float)(system->acousticWaveformCount - 1)) {
            return 0.0f;
        }

        const int i0 = (int)floorf(samplePosition);
        const int i1 = i0 + 1;
        const float t = samplePosition - (float)i0;
        const float a = system->acousticWaveform[i0];
        const float b = system->acousticWaveform[i1];
        return ClampFloat(a + (b - a) * t, -1.0f, 1.0f);
    }

    static float AcousticVisualWaveformSample(const ParticleSystem3D *system, float time)
    {
        if (!system->acousticAudioLoaded || system->acousticWaveformSampleRate <= 0.0f) {
            return 0.0f;
        }

        const float smoothingWindow = fmaxf(1.0f / system->acousticWaveformSampleRate, 1.0f / 240.0f);
        const float a = AcousticWaveformSample(system, time - smoothingWindow);
        const float b = AcousticWaveformSample(system, time - smoothingWindow * 0.5f);
        const float c = AcousticWaveformSample(system, time);
        const float d = AcousticWaveformSample(system, time + smoothingWindow * 0.5f);
        const float e = AcousticWaveformSample(system, time + smoothingWindow);
        return ClampFloat((a + b * 2.0f + c * 3.0f + d * 2.0f + e) / 9.0f, -1.0f, 1.0f);
    }

    static float AcousticResolvedFrequencyEstimate(const ParticleSystem3D *system)
    {
        const float h = fmaxf(system->params.supportRadius, 1e-4f);
        return EffectiveAcousticSoundSpeed(system) / (6.0f * h);
    }

    static float EstimateAcousticBakeSupportRadius(const ParticleSystem3D *system, int targetParticles)
    {
        const MaterialParams gas = MATERIAL_PRESETS[MATERIAL_GAS];
        const Vector3 boundsSize = {
            (system->boundsSize.x > 0.0f) ? system->boundsSize.x : 150.0f,
            (system->boundsSize.y > 0.0f) ? system->boundsSize.y : 82.0f,
            (system->boundsSize.z > 0.0f) ? system->boundsSize.z : 88.0f,
        };
        const Vector3 fillSize = {
            boundsSize.x * 0.98f,
            boundsSize.y * 0.96f,
            boundsSize.z * 0.96f,
        };
        const float volume = fmaxf(fillSize.x * fillSize.y * fillSize.z, 1.0f);
        const float targetSpacing = cbrtf(volume / (float)ClampInt(targetParticles, 1, BAKE_MAX_PARTICLE_COUNT));
        const float resolutionScale = ClampFloat((targetSpacing * 0.96f) / gas.spacing, 0.06f, 1.75f);
        return fmaxf(gas.supportRadius * resolutionScale, 1e-4f);
    }

    static float AcousticResolvedFrequencyEstimateForBakeTarget(const ParticleSystem3D *system, int targetParticles)
    {
        const float h = EstimateAcousticBakeSupportRadius(system, targetParticles);
        return EffectiveAcousticSoundSpeed(system) / (6.0f * h);
    }

    static float AcousticAudioTargetBandwidthHz(const ParticleSystem3D *system)
    {
        if (system != NULL && system->acousticAudioLoaded && system->acousticAudioBandwidthHz > 0.0f) {
            return ClampFloat(system->acousticAudioBandwidthHz,
                ACOUSTIC_AUDIO_MIN_TARGET_BANDWIDTH_HZ,
                ACOUSTIC_AUDIO_TARGET_BANDWIDTH_HZ);
        }
        return ACOUSTIC_AUDIO_TARGET_BANDWIDTH_HZ;
    }

    static float AcousticRequiredSlowdownForBakeTarget(const ParticleSystem3D *system, int targetParticles)
    {
        const float resolvedHz = AcousticResolvedFrequencyEstimateForBakeTarget(system, targetParticles);
        const float required = AcousticAudioTargetBandwidthHz(system) / fmaxf(resolvedHz, 1e-4f);
        return ClampFloat(ceilf(required), ACOUSTIC_AUDIO_MIN_SLOWDOWN_FACTOR, ACOUSTIC_AUDIO_MAX_SLOWDOWN_FACTOR);
    }

    static float AcousticAudioSlowdownFactor(const ParticleSystem3D *system)
    {
        if (system == NULL || system->acousticAudioSlowdownFactor <= 1.0f) {
            return ACOUSTIC_AUDIO_SLOWDOWN_FACTOR;
        }
        return system->acousticAudioSlowdownFactor;
    }

    static float AcousticBakeSlowdownForTarget(const ParticleSystem3D *system, int targetParticles)
    {
        if (!AcousticSlowMotionAudioActive(system)) {
            return AcousticAudioSlowdownFactor(system);
        }
        return fmaxf(AcousticAudioSlowdownFactor(system),
            AcousticRequiredSlowdownForBakeTarget(system, targetParticles));
    }

    static bool AcousticSlowMotionAudioActive(const ParticleSystem3D *system)
    {
        return system != NULL && system->acousticAudioLoaded &&
            system->acousticWaveform != NULL &&
            system->acousticWaveformCount > 1 &&
            system->acousticWaveformSampleRate > 0.0f;
    }

    static float AcousticDriverTimeFromSimulationTime(const ParticleSystem3D *system, float simulationTime)
    {
        if (!AcousticSlowMotionAudioActive(system)) {
            return simulationTime;
        }
        return ACOUSTIC_AUDIO_PREROLL_SECONDS +
            (simulationTime - ACOUSTIC_AUDIO_PREROLL_SECONDS) / AcousticAudioSlowdownFactor(system);
    }

    static float AcousticSpeakerMicDelaySeconds(const ParticleSystem3D *system)
    {
        const float dx = system->micPosition.x - system->speakerBaseCenter.x;
        const float dy = system->micPosition.y - system->speakerBaseCenter.y;
        const float dz = system->micPosition.z - system->speakerBaseCenter.z;
        const float distance = sqrtf(dx * dx + dy * dy + dz * dz);
        return distance / fmaxf(EffectiveAcousticSoundSpeed(system), 1e-4f);
    }

    static void ApplyAudioAcousticPreset(ParticleSystem3D *system)
    {
        system->scene = SCENE_TANK;
        system->tankPreset = MATERIAL_GAS;
        system->preset = MATERIAL_GAS;
        system->acousticsEnabled = true;
        system->acousticSoundSpeed = 420.0f;
        system->acousticMachLimit = 0.60f;
        system->acousticViscosityScale = 0.05f;
        system->acousticDragScale = 0.0f;
        system->acousticAudioSlowdownFactor = ACOUSTIC_AUDIO_MIN_SLOWDOWN_FACTOR;
        system->speakerAmplitude = 1.4f;
        system->speakerWidth = fmaxf(system->speakerWidth, 22.0f);
        system->speakerHeight = fmaxf(system->speakerHeight, 72.0f);
        system->speakerDepth = fmaxf(system->speakerDepth, 34.0f);
    }

    static void ClampAcousticAnchors(ParticleSystem3D *system)
    {
        const float padding = system->params.particleRadius * 2.0f;
        const float minX = system->boundsMin.x + padding;
        const float maxX = system->boundsMax.x - padding;
        const float minY = system->boundsMin.y + padding;
        const float maxY = system->boundsMax.y - padding;
        const float minZ = system->boundsMin.z + padding;
        const float maxZ = system->boundsMax.z - padding;
        const float halfWidth = system->speakerWidth * 0.5f;
        const float halfHeight = system->speakerHeight * 0.5f;
        const float halfDepth = system->speakerDepth * 0.5f;
        const float amplitude = EffectiveSpeakerAmplitude(system);

        system->speakerBaseCenter.x = ClampFloat(system->speakerBaseCenter.x,
            minX + halfWidth + amplitude,
            maxX - halfWidth - amplitude);
        system->speakerBaseCenter.y = ClampFloat(system->speakerBaseCenter.y,
            minY + halfHeight,
            maxY - halfHeight);
        system->speakerBaseCenter.z = ClampFloat(system->speakerBaseCenter.z,
            minZ + halfDepth,
            maxZ - halfDepth);
        system->micPosition.x = ClampFloat(system->micPosition.x, minX, maxX);
        system->micPosition.y = ClampFloat(system->micPosition.y, minY, maxY);
        system->micPosition.z = ClampFloat(system->micPosition.z, minZ, maxZ);
    }

    static void GetSpeakerState(const ParticleSystem3D *system, Vector3 *center, Vector3 *velocity, Vector3 *halfSize)
    {
        const bool visualOnly = (velocity == NULL);
        const float phase = system->simulationTime * system->speakerFrequency * 2.0f * PI_F;
        const float amplitude = EffectiveSpeakerAmplitude(system);
        const float angularFrequency = system->speakerFrequency * 2.0f * PI_F;
        float displacement = amplitude * AcousticEnvelopeMultiplier(system) * sinf(phase);
        float speed = amplitude * angularFrequency * cosf(phase);
        if (system->acousticAudioLoaded && system->acousticWaveform != NULL && system->acousticWaveformCount > 1) {
            const float driverDt = 1.0f / fmaxf(system->acousticWaveformSampleRate, 1.0f);
            const float driverTime = AcousticDriverTimeFromSimulationTime(system, system->simulationTime);
            const float slowdown = AcousticAudioSlowdownFactor(system);
            const float previous = visualOnly
                ? AcousticVisualWaveformSample(system, driverTime - driverDt)
                : AcousticWaveformSample(system, driverTime - driverDt);
            const float current = visualOnly
                ? AcousticVisualWaveformSample(system, driverTime)
                : AcousticWaveformSample(system, driverTime);
            const float next = visualOnly
                ? AcousticVisualWaveformSample(system, driverTime + driverDt)
                : AcousticWaveformSample(system, driverTime + driverDt);
            displacement = amplitude * current;
            speed = amplitude * (next - previous) / (2.0f * driverDt * slowdown);
            const float maxSurfaceSpeed = ClampFloat(system->acousticMachLimit, 0.05f, 1.25f) *
                fmaxf(EffectiveAcousticSoundSpeed(system), 1e-4f);
            speed = ClampFloat(speed, -maxSurfaceSpeed, maxSurfaceSpeed);
        }
        if (center != NULL) {
            *center = (Vector3){
                system->speakerBaseCenter.x + displacement,
                system->speakerBaseCenter.y,
                system->speakerBaseCenter.z,
            };
        }
        if (velocity != NULL) {
            *velocity = (Vector3){speed, 0.0f, 0.0f};
        }
        if (halfSize != NULL) {
            *halfSize = (Vector3){
                system->speakerWidth * 0.5f,
                system->speakerHeight * 0.5f,
                system->speakerDepth * 0.5f,
            };
        }
    }

    static float SpeakerPeakSurfaceSpeed(const ParticleSystem3D *system)
    {
        if (AcousticSlowMotionAudioActive(system)) {
            return ClampFloat(system->acousticMachLimit, 0.05f, 1.25f) *
                fmaxf(EffectiveAcousticSoundSpeed(system), 1e-4f);
        }
        const float angularFrequency = system->speakerFrequency * 2.0f * PI_F;
        return fabsf(EffectiveSpeakerAmplitude(system) * angularFrequency);
    }

    static float SpeakerSignedDistance(const ParticleSystem3D *system, float x, float y, float z)
    {
        Vector3 center;
        Vector3 halfSize;
        GetSpeakerState(system, &center, NULL, &halfSize);
        return BoxSignedDistance((Vector3){x - center.x, y - center.y, z - center.z}, halfSize);
    }

    static Vector3 SpeakerNormal(const ParticleSystem3D *system, float x, float y, float z)
    {
        const float epsilon = fmaxf(system->params.particleRadius * 0.65f, 0.75f);
        const float dx = SpeakerSignedDistance(system, x + epsilon, y, z) - SpeakerSignedDistance(system, x - epsilon, y, z);
        const float dy = SpeakerSignedDistance(system, x, y + epsilon, z) - SpeakerSignedDistance(system, x, y - epsilon, z);
        const float dz = SpeakerSignedDistance(system, x, y, z + epsilon) - SpeakerSignedDistance(system, x, y, z - epsilon);
        const Vector3 normal = {dx, dy, dz};
        const float length = Vector3Length(normal);
        if (length > 1e-6f) {
            return Vector3Scale(normal, 1.0f / length);
        }
        return (Vector3){1.0f, 0.0f, 0.0f};
    }

    static void ResetMicrophoneHistory(ParticleSystem3D *system)
    {
        memset(system->micWaveform, 0, sizeof(system->micWaveform));
        memset(system->micWaveformDisplay, 0, sizeof(system->micWaveformDisplay));
        system->micWaveformHead = 0;
        system->micWaveformCount = 0;
        system->micSignal = 0.0f;
        system->micBaseline = 0.0f;
        system->audioOutputSignal = 0.0f;
        system->audioOutputState = 0.0f;
        system->audioMonitorPhase = 0.0f;
    }

    static void PushMicrophoneSample(ParticleSystem3D *system, float signal)
    {
        system->micSignal = signal;
        system->audioOutputSignal = signal;
        system->micWaveform[system->micWaveformHead] = signal;
        system->micWaveformHead = (system->micWaveformHead + 1) % MIC_WAVEFORM_SAMPLES;
        if (system->micWaveformCount < MIC_WAVEFORM_SAMPLES) {
            system->micWaveformCount += 1;
        }

        const int start = (system->micWaveformHead - system->micWaveformCount + MIC_WAVEFORM_SAMPLES) % MIC_WAVEFORM_SAMPLES;
        for (int i = 0; i < system->micWaveformCount; ++i) {
            const int index = (start + i) % MIC_WAVEFORM_SAMPLES;
            system->micWaveformDisplay[i] = system->micWaveform[index];
        }
    }

    static void SampleMicrophone(ParticleSystem3D *system)
    {
        if (!AcousticsActive(system)) {
            return;
        }

        float weightedPressure = 0.0f;
        float totalWeight = 0.0f;
        const float radius2 = system->micRadius * system->micRadius;
        if (!system->sortedStateDirty && system->cellStarts != NULL && system->sortedPressure != NULL &&
            system->gridWidth > 0 && system->gridHeight > 0 && system->gridDepth > 0) {
            const float searchRadius = system->micRadius + system->params.supportRadius;
            const int minCellX = ClampInt((int)((system->micPosition.x - searchRadius - system->boundsMin.x) * system->invCellSize),
                0, system->gridWidth - 1);
            const int maxCellX = ClampInt((int)((system->micPosition.x + searchRadius - system->boundsMin.x) * system->invCellSize),
                0, system->gridWidth - 1);
            const int minCellY = ClampInt((int)((system->micPosition.y - searchRadius - system->boundsMin.y) * system->invCellSize),
                0, system->gridHeight - 1);
            const int maxCellY = ClampInt((int)((system->micPosition.y + searchRadius - system->boundsMin.y) * system->invCellSize),
                0, system->gridHeight - 1);
            const int minCellZ = ClampInt((int)((system->micPosition.z - searchRadius - system->boundsMin.z) * system->invCellSize),
                0, system->gridDepth - 1);
            const int maxCellZ = ClampInt((int)((system->micPosition.z + searchRadius - system->boundsMin.z) * system->invCellSize),
                0, system->gridDepth - 1);

            for (int cellZ = minCellZ; cellZ <= maxCellZ; ++cellZ) {
                for (int cellY = minCellY; cellY <= maxCellY; ++cellY) {
                    for (int cellX = minCellX; cellX <= maxCellX; ++cellX) {
                        const int cellIndex = (cellZ * system->gridHeight + cellY) * system->gridWidth + cellX;
                        const int start = system->cellStarts[cellIndex];
                        const int end = system->cellStarts[cellIndex + 1];
                        for (int slot = start; slot < end; ++slot) {
                            const float dx = system->sortedX[slot] - system->micPosition.x;
                            const float dy = system->sortedY[slot] - system->micPosition.y;
                            const float dz = system->sortedZ[slot] - system->micPosition.z;
                            const float r2 = dx * dx + dy * dy + dz * dz;
                            if (r2 < radius2) {
                                const float weight = 1.0f - sqrtf(r2 / fmaxf(radius2, 1e-6f));
                                weightedPressure += system->sortedPressure[slot] * weight;
                                totalWeight += weight;
                            }
                        }
                    }
                }
            }
        } else {
            for (int i = 0; i < system->particleCount; ++i) {
                const float dx = system->x[i] - system->micPosition.x;
                const float dy = system->y[i] - system->micPosition.y;
                const float dz = system->z[i] - system->micPosition.z;
                const float r2 = dx * dx + dy * dy + dz * dz;
                if (r2 < radius2) {
                    const float weight = 1.0f - sqrtf(r2 / fmaxf(radius2, 1e-6f));
                    weightedPressure += system->pressure[i] * weight;
                    totalWeight += weight;
                }
            }
        }

        const float sample = (totalWeight > 1e-5f) ? (weightedPressure / totalWeight) : 0.0f;
        if (system->micWaveformCount == 0) {
            system->micBaseline = sample;
        } else {
            float baselineAlpha = 0.015f;
            if (AcousticSlowMotionAudioActive(system)) {
                const float tau = fmaxf(0.25f * AcousticAudioSlowdownFactor(system), 1.0f);
                baselineAlpha = ClampFloat(system->lastStepDt / tau, 0.000001f, 0.015f);
            }
            system->micBaseline = (1.0f - baselineAlpha) * system->micBaseline + baselineAlpha * sample;
        }

        const float scale = fmaxf(system->params.soundSpeed * system->params.soundSpeed * system->params.restDensity * 0.65f, 1e-4f);
        PushMicrophoneSample(system, ClampFloat((sample - system->micBaseline) / scale, -1.0f, 1.0f));
    }

    static void MicAudioStreamCallback3D(void *bufferData, unsigned int frames)
    {
        float *samples = (float *)bufferData;
        if (samples == NULL) {
            return;
        }

        ParticleSystem3D *system = gAudioOutputSystem3D;
        if (system == NULL || !system->audioOutputReady || !system->audioOutputEnabled ||
            system->paused || !AcousticsActive(system)) {
            memset(samples, 0, (size_t)frames * sizeof(float));
            return;
        }

        float state = system->audioOutputState;
        float phase = system->audioMonitorPhase;
        const float carrierHz = system->audioMonitorPitchHz;
        for (unsigned int i = 0; i < frames; ++i) {
            const float target = system->audioOutputSignal;
            state += (target - state) * 0.035f;
            float output = state;
            if (carrierHz > 1.0f) {
                phase += (2.0f * PI_F * carrierHz) / (float)AUDIO_OUTPUT_SAMPLE_RATE;
                if (phase > 2.0f * PI_F) {
                    phase = fmodf(phase, 2.0f * PI_F);
                }
                output = state * sinf(phase);
            }
            samples[i] = ClampFloat(output * AUDIO_OUTPUT_GAIN, -1.0f, 1.0f);
        }
        system->audioOutputState = state;
        system->audioMonitorPhase = phase;
    }

    static bool InitializeAudioOutput(ParticleSystem3D *system)
    {
        if (system->audioOutputReady) {
            if (!IsAudioStreamPlaying(system->micAudioStream)) {
                PlayAudioStream(system->micAudioStream);
            }
            gAudioOutputSystem3D = system;
            return true;
        }

        if (!IsAudioDeviceReady()) {
            SetAudioStreamBufferSizeDefault(AUDIO_OUTPUT_BUFFER_FRAMES);
            InitAudioDevice();
        }
        if (!IsAudioDeviceReady()) {
            return false;
        }

        system->micAudioStream = LoadAudioStream(AUDIO_OUTPUT_SAMPLE_RATE, 32, 1);
        if (!IsAudioStreamValid(system->micAudioStream)) {
            memset(&system->micAudioStream, 0, sizeof(system->micAudioStream));
            return false;
        }

        SetAudioStreamVolume(system->micAudioStream, 1.0f);
        SetAudioStreamCallback(system->micAudioStream, MicAudioStreamCallback3D);
        PlayAudioStream(system->micAudioStream);
        system->audioOutputReady = true;
        gAudioOutputSystem3D = system;
        return true;
    }

    static void ShutdownAudioOutput(ParticleSystem3D *system)
    {
        if (gAudioOutputSystem3D == system) {
            gAudioOutputSystem3D = NULL;
        }

        if (system->audioOutputReady) {
            StopAudioStream(system->micAudioStream);
            UnloadAudioStream(system->micAudioStream);
            memset(&system->micAudioStream, 0, sizeof(system->micAudioStream));
            system->audioOutputReady = false;
        }

        if (IsAudioDeviceReady()) {
            CloseAudioDevice();
        }
    }

    static Vector3 ImportedObstacleTargetSize(const ParticleSystem3D *system)
    {
        return (Vector3){
            system->boundsSize.x * 0.18f,
            system->boundsSize.y * 0.26f,
            system->boundsSize.z * 0.30f,
        };
    }

    static void UnloadImportedObstacle(ParticleSystem3D *system)
    {
        if (system->importedObstacleLoaded) {
            UnloadModel(system->importedObstacleModel);
        }
        system->importedObstacleLoaded = false;
        memset(&system->importedObstacleModel, 0, sizeof(system->importedObstacleModel));
        system->importedObstaclePath[0] = '\0';
    system->importedObstacleScale = 1.0f;
    system->importedObstacleUserScale = 1.0f;
    system->importedObstacleOffset = (Vector3){0.0f, 0.0f, 0.0f};
    ResetImportedObstacleRotationState(system);
    system->importedObstacleRawCenter = (Vector3){0.0f, 0.0f, 0.0f};
        system->importedObstacleLocalMin = (Vector3){0.0f, 0.0f, 0.0f};
        system->importedObstacleLocalMax = (Vector3){0.0f, 0.0f, 0.0f};
        system->importedSdfWidth = 0;
        system->importedSdfHeight = 0;
        system->importedSdfDepth = 0;
        system->importedSdfVoxelCount = 0;
        system->importedSdfMaxResolution = 0;
        system->importedSdfBakeQuality = false;
        if (system->importedSdfValues != NULL) {
            MemFree(system->importedSdfValues);
            system->importedSdfValues = NULL;
        }
    }

    typedef struct ObjVertexBuffer {
        Vector3 *items;
        int count;
        int capacity;
    } ObjVertexBuffer;

    typedef struct ObjIndexBuffer {
        int *items;
        int count;
        int capacity;
    } ObjIndexBuffer;

    static bool EnsureObjBufferCapacity(void **items, int *capacity, int minCapacity, size_t itemSize)
    {
        if (minCapacity <= *capacity) {
            return true;
        }

        int nextCapacity = (*capacity > 0) ? *capacity : 64;
        while (nextCapacity < minCapacity) {
            if (nextCapacity > INT_MAX / 2) {
                nextCapacity = minCapacity;
                break;
            }
            nextCapacity *= 2;
        }

        void *resized = MemRealloc(*items, (unsigned int)((size_t)nextCapacity * itemSize));
        if (resized == NULL) {
            return false;
        }

        *items = resized;
        *capacity = nextCapacity;
        return true;
    }

    static bool PushObjVertex(ObjVertexBuffer *buffer, Vector3 value)
    {
        if (!EnsureObjBufferCapacity((void **)&buffer->items, &buffer->capacity, buffer->count + 1, sizeof(Vector3))) {
            return false;
        }

        buffer->items[buffer->count++] = value;
        return true;
    }

    static bool PushObjIndex(ObjIndexBuffer *buffer, int value)
    {
        if (!EnsureObjBufferCapacity((void **)&buffer->items, &buffer->capacity, buffer->count + 1, sizeof(int))) {
            return false;
        }

        buffer->items[buffer->count++] = value;
        return true;
    }

    static void FreeObjVertexBuffer(ObjVertexBuffer *buffer)
    {
        if (buffer->items != NULL) {
            MemFree(buffer->items);
        }
        buffer->items = NULL;
        buffer->count = 0;
        buffer->capacity = 0;
    }

    static void FreeObjIndexBuffer(ObjIndexBuffer *buffer)
    {
        if (buffer->items != NULL) {
            MemFree(buffer->items);
        }
        buffer->items = NULL;
        buffer->count = 0;
        buffer->capacity = 0;
    }

    static void UnloadMeshCpuData(Mesh *mesh)
    {
        if (mesh == NULL) {
            return;
        }

        if (mesh->vertices != NULL) {
            MemFree(mesh->vertices);
        }
        if (mesh->normals != NULL) {
            MemFree(mesh->normals);
        }
        if (mesh->texcoords != NULL) {
            MemFree(mesh->texcoords);
        }
        if (mesh->indices != NULL) {
            MemFree(mesh->indices);
        }
        memset(mesh, 0, sizeof(*mesh));
    }

    static bool ParseObjFaceVertexIndex(const char *token, int vertexCount, int *outIndex)
    {
        if (token == NULL || token[0] == '\0') {
            return false;
        }

        char *end = NULL;
        const long parsedIndex = strtol(token, &end, 10);
        if (end == token || (*end != '\0' && *end != '/')) {
            return false;
        }

        int resolvedIndex = 0;
        if (parsedIndex > 0) {
            resolvedIndex = (int)parsedIndex - 1;
        } else if (parsedIndex < 0) {
            resolvedIndex = vertexCount + (int)parsedIndex;
        } else {
            return false;
        }

        if (resolvedIndex < 0 || resolvedIndex >= vertexCount) {
            return false;
        }

        *outIndex = resolvedIndex;
        return true;
    }

    static bool LoadObjMeshFromPath(const char *path, Mesh *outMesh)
    {
        bool success = false;
        FILE *file = fopen(path, "r");
        if (file == NULL) {
            return false;
        }

        ObjVertexBuffer positions = {0};
        ObjIndexBuffer triangleIndices = {0};
        ObjIndexBuffer faceIndices = {0};
        Mesh mesh = {0};
        char line[8192];

        while (fgets(line, sizeof(line), file) != NULL) {
            char *cursor = line;
            while (*cursor != '\0' && isspace((unsigned char)*cursor)) {
                cursor += 1;
            }

            if (cursor[0] == '\0' || cursor[0] == '#') {
                continue;
            }

            if (cursor[0] == 'v' && isspace((unsigned char)cursor[1])) {
                char *parse = cursor + 1;
                const float x = strtof(parse, &parse);
                const float y = strtof(parse, &parse);
                const float z = strtof(parse, &parse);
                if (!PushObjVertex(&positions, (Vector3){x, y, z})) {
                    goto cleanup;
                }
                continue;
            }

            if (cursor[0] == 'f' && isspace((unsigned char)cursor[1])) {
                faceIndices.count = 0;
                char *token = cursor + 1;
                while (*token != '\0') {
                    while (*token != '\0' && isspace((unsigned char)*token)) {
                        token += 1;
                    }
                    if (*token == '\0' || *token == '\n' || *token == '\r') {
                        break;
                    }

                    char *tokenEnd = token;
                    while (*tokenEnd != '\0' && !isspace((unsigned char)*tokenEnd)) {
                        tokenEnd += 1;
                    }

                    const char saved = *tokenEnd;
                    *tokenEnd = '\0';

                    int vertexIndex = -1;
                    const bool validIndex = ParseObjFaceVertexIndex(token, positions.count, &vertexIndex);

                    *tokenEnd = saved;
                    if (!validIndex || !PushObjIndex(&faceIndices, vertexIndex)) {
                        goto cleanup;
                    }

                    token = tokenEnd;
                }

                for (int i = 1; i + 1 < faceIndices.count; ++i) {
                    const int i0 = faceIndices.items[0];
                    const int i1 = faceIndices.items[i];
                    const int i2 = faceIndices.items[i + 1];
                    if (i0 == i1 || i1 == i2 || i0 == i2) {
                        continue;
                    }

                    const Vector3 a = positions.items[i0];
                    const Vector3 b = positions.items[i1];
                    const Vector3 c = positions.items[i2];
                    const Vector3 normal = Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a));
                    if (Vector3LengthSqr(normal) <= 1e-10f) {
                        continue;
                    }

                    if (!PushObjIndex(&triangleIndices, i0) ||
                        !PushObjIndex(&triangleIndices, i1) ||
                        !PushObjIndex(&triangleIndices, i2)) {
                        goto cleanup;
                    }
                }
            }
        }

        if (triangleIndices.count < 3) {
            goto cleanup;
        }

        mesh.vertexCount = triangleIndices.count;
        mesh.triangleCount = triangleIndices.count / 3;
        mesh.vertices = (float *)MemAlloc((unsigned int)((size_t)mesh.vertexCount * 3u * sizeof(float)));
        mesh.normals = (float *)MemAlloc((unsigned int)((size_t)mesh.vertexCount * 3u * sizeof(float)));
        if (mesh.vertices == NULL || mesh.normals == NULL) {
            goto cleanup;
        }

        for (int triangleIndex = 0; triangleIndex < mesh.triangleCount; ++triangleIndex) {
            const int sourceIndex = triangleIndex * 3;
            const Vector3 a = positions.items[triangleIndices.items[sourceIndex + 0]];
            const Vector3 b = positions.items[triangleIndices.items[sourceIndex + 1]];
            const Vector3 c = positions.items[triangleIndices.items[sourceIndex + 2]];
            const Vector3 faceNormal = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a)));
            const Vector3 vertices[3] = {a, b, c};

            for (int vertexOffset = 0; vertexOffset < 3; ++vertexOffset) {
                const int vertexCursor = sourceIndex + vertexOffset;
                mesh.vertices[vertexCursor * 3 + 0] = vertices[vertexOffset].x;
                mesh.vertices[vertexCursor * 3 + 1] = vertices[vertexOffset].y;
                mesh.vertices[vertexCursor * 3 + 2] = vertices[vertexOffset].z;
                mesh.normals[vertexCursor * 3 + 0] = faceNormal.x;
                mesh.normals[vertexCursor * 3 + 1] = faceNormal.y;
                mesh.normals[vertexCursor * 3 + 2] = faceNormal.z;
            }
        }

        *outMesh = mesh;
        memset(&mesh, 0, sizeof(mesh));
        success = true;

    cleanup:
        fclose(file);
        FreeObjVertexBuffer(&positions);
        FreeObjIndexBuffer(&triangleIndices);
        FreeObjIndexBuffer(&faceIndices);
        if (!success) {
            UnloadMeshCpuData(&mesh);
        }
        return success;
    }

    static int ModelTriangleCount(const Model *model)
    {
        int triangleCount = 0;
        for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
            triangleCount += model->meshes[meshIndex].triangleCount;
        }
        return triangleCount;
    }

    static Vector3 ModelVertexAt(const Mesh *mesh, int vertexIndex)
    {
        return (Vector3){
            mesh->vertices[vertexIndex * 3 + 0],
            mesh->vertices[vertexIndex * 3 + 1],
            mesh->vertices[vertexIndex * 3 + 2],
        };
    }

    static void SyncImportedMeshGpuBuffers(Mesh *mesh, bool updateNormals)
    {
        if (mesh == NULL) {
            return;
        }

        const bool meshUploaded = (mesh->vaoId != 0) || (mesh->vboId != NULL && mesh->vboId[0] != 0);
        if (!meshUploaded) {
            UploadMesh(mesh, false);
            return;
        }

        if (mesh->vertices != NULL) {
            UpdateMeshBuffer(*mesh, 0, mesh->vertices, mesh->vertexCount * 3 * (int)sizeof(float), 0);
        }
        if (updateNormals && mesh->normals != NULL) {
            UpdateMeshBuffer(*mesh, 2, mesh->normals, mesh->vertexCount * 3 * (int)sizeof(float), 0);
        }
    }

    static void BakeImportedModelTransform(Model *model)
    {
        const Matrix normalTransform = MatrixTranspose(MatrixInvert(model->transform));
        for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
            Mesh *mesh = &model->meshes[meshIndex];
            if (mesh->vertices != NULL) {
                for (int vertexIndex = 0; vertexIndex < mesh->vertexCount; ++vertexIndex) {
                    const Vector3 transformed = Vector3Transform(ModelVertexAt(mesh, vertexIndex), model->transform);
                    mesh->vertices[vertexIndex * 3 + 0] = transformed.x;
                    mesh->vertices[vertexIndex * 3 + 1] = transformed.y;
                    mesh->vertices[vertexIndex * 3 + 2] = transformed.z;
                }
            }
            if (mesh->normals != NULL) {
                for (int vertexIndex = 0; vertexIndex < mesh->vertexCount; ++vertexIndex) {
                    const Vector3 normal = {
                        mesh->normals[vertexIndex * 3 + 0],
                        mesh->normals[vertexIndex * 3 + 1],
                        mesh->normals[vertexIndex * 3 + 2],
                    };
                    const Vector3 transformedNormal = Vector3Normalize(TransformDirection(normalTransform, normal));
                    mesh->normals[vertexIndex * 3 + 0] = transformedNormal.x;
                    mesh->normals[vertexIndex * 3 + 1] = transformedNormal.y;
                    mesh->normals[vertexIndex * 3 + 2] = transformedNormal.z;
                }
            }
            SyncImportedMeshGpuBuffers(mesh, true);
        }
        model->transform = MatrixIdentity();
    }

    static void NormalizeImportedModelGeometry(Model *model, Vector3 rawCenter, float scale)
    {
        if (model == NULL) {
            return;
        }

        for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
            Mesh *mesh = &model->meshes[meshIndex];
            if (mesh->vertices == NULL) {
                continue;
            }

            for (int vertexIndex = 0; vertexIndex < mesh->vertexCount; ++vertexIndex) {
                const Vector3 raw = ModelVertexAt(mesh, vertexIndex);
                const Vector3 local = Vector3Scale(Vector3Subtract(raw, rawCenter), scale);
                mesh->vertices[vertexIndex * 3 + 0] = local.x;
                mesh->vertices[vertexIndex * 3 + 1] = local.y;
                mesh->vertices[vertexIndex * 3 + 2] = local.z;
            }

            SyncImportedMeshGpuBuffers(mesh, false);
        }
    }

    static bool BuildImportedObstacleSdf(ParticleSystem3D *system, Model *model, int maxResolution, bool normalizeGeometry)
    {
        const int triangleCount = ModelTriangleCount(model);
        if (triangleCount <= 0) {
            return false;
        }

        ImportedTriangle *triangles = (ImportedTriangle *)MemAlloc((size_t)triangleCount * sizeof(ImportedTriangle));
        if (triangles == NULL) {
            return false;
        }

        Vector3 rawMin = {FLT_MAX, FLT_MAX, FLT_MAX};
        Vector3 rawMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
        int triangleCursor = 0;
        for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
            const Mesh *mesh = &model->meshes[meshIndex];
            if (mesh->vertices == NULL || mesh->triangleCount <= 0) {
                continue;
            }

            for (int triangleIndex = 0; triangleIndex < mesh->triangleCount; ++triangleIndex) {
                const int i0 = (mesh->indices != NULL) ? mesh->indices[triangleIndex * 3 + 0] : triangleIndex * 3 + 0;
                const int i1 = (mesh->indices != NULL) ? mesh->indices[triangleIndex * 3 + 1] : triangleIndex * 3 + 1;
                const int i2 = (mesh->indices != NULL) ? mesh->indices[triangleIndex * 3 + 2] : triangleIndex * 3 + 2;
                if (i0 < 0 || i1 < 0 || i2 < 0 ||
                    i0 >= mesh->vertexCount || i1 >= mesh->vertexCount || i2 >= mesh->vertexCount) {
                    continue;
                }
                Vector3 a = Vector3Transform(ModelVertexAt(mesh, i0), model->transform);
                Vector3 b = Vector3Transform(ModelVertexAt(mesh, i1), model->transform);
                Vector3 c = Vector3Transform(ModelVertexAt(mesh, i2), model->transform);
                rawMin.x = fminf(rawMin.x, fminf(a.x, fminf(b.x, c.x)));
                rawMin.y = fminf(rawMin.y, fminf(a.y, fminf(b.y, c.y)));
                rawMin.z = fminf(rawMin.z, fminf(a.z, fminf(b.z, c.z)));
                rawMax.x = fmaxf(rawMax.x, fmaxf(a.x, fmaxf(b.x, c.x)));
                rawMax.y = fmaxf(rawMax.y, fmaxf(a.y, fmaxf(b.y, c.y)));
                rawMax.z = fmaxf(rawMax.z, fmaxf(a.z, fmaxf(b.z, c.z)));
                triangles[triangleCursor++] = (ImportedTriangle){a, b, c};
            }
        }

        if (triangleCursor <= 0) {
            MemFree(triangles);
            return false;
        }

        Vector3 localMin = {FLT_MAX, FLT_MAX, FLT_MAX};
        Vector3 localMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
        Vector3 rawCenter = {0.0f, 0.0f, 0.0f};
        float scale = 1.0f;
        if (normalizeGeometry) {
            rawCenter = Vector3Scale(Vector3Add(rawMin, rawMax), 0.5f);
            const Vector3 rawSize = Vector3Subtract(rawMax, rawMin);
            const Vector3 targetSize = ImportedObstacleTargetSize(system);
            const float safeSizeX = fmaxf(rawSize.x, 1e-4f);
            const float safeSizeY = fmaxf(rawSize.y, 1e-4f);
            const float safeSizeZ = fmaxf(rawSize.z, 1e-4f);
            scale = fminf(targetSize.x / safeSizeX, fminf(targetSize.y / safeSizeY, targetSize.z / safeSizeZ));
        }

        for (int i = 0; i < triangleCursor; ++i) {
            if (normalizeGeometry) {
                triangles[i].a = Vector3Scale(Vector3Subtract(triangles[i].a, rawCenter), scale);
                triangles[i].b = Vector3Scale(Vector3Subtract(triangles[i].b, rawCenter), scale);
                triangles[i].c = Vector3Scale(Vector3Subtract(triangles[i].c, rawCenter), scale);
            }
            localMin.x = fminf(localMin.x, fminf(triangles[i].a.x, fminf(triangles[i].b.x, triangles[i].c.x)));
            localMin.y = fminf(localMin.y, fminf(triangles[i].a.y, fminf(triangles[i].b.y, triangles[i].c.y)));
            localMin.z = fminf(localMin.z, fminf(triangles[i].a.z, fminf(triangles[i].b.z, triangles[i].c.z)));
            localMax.x = fmaxf(localMax.x, fmaxf(triangles[i].a.x, fmaxf(triangles[i].b.x, triangles[i].c.x)));
            localMax.y = fmaxf(localMax.y, fmaxf(triangles[i].a.y, fmaxf(triangles[i].b.y, triangles[i].c.y)));
            localMax.z = fmaxf(localMax.z, fmaxf(triangles[i].a.z, fmaxf(triangles[i].b.z, triangles[i].c.z)));
        }

        const float padding = fmaxf(system->params.supportRadius * 1.4f, 2.0f);
        localMin = Vector3Subtract(localMin, (Vector3){padding, padding, padding});
        localMax = Vector3Add(localMax, (Vector3){padding, padding, padding});
        const Vector3 localSize = Vector3Subtract(localMax, localMin);
        const float maxExtent = fmaxf(localSize.x, fmaxf(localSize.y, localSize.z));
        const int sdfMaxResolution = ClampInt(maxResolution, IMPORTED_SDF_MIN_RES, IMPORTED_SDF_BAKE_MAX_RES);
        const int width = ClampInt((int)lroundf((localSize.x / fmaxf(maxExtent, 1e-4f)) * (float)sdfMaxResolution),
            IMPORTED_SDF_MIN_RES, sdfMaxResolution);
        const int height = ClampInt((int)lroundf((localSize.y / fmaxf(maxExtent, 1e-4f)) * (float)sdfMaxResolution),
            IMPORTED_SDF_MIN_RES, sdfMaxResolution);
        const int depth = ClampInt((int)lroundf((localSize.z / fmaxf(maxExtent, 1e-4f)) * (float)sdfMaxResolution),
            IMPORTED_SDF_MIN_RES, sdfMaxResolution);
        const int voxelCount = width * height * depth;

        float *sdfValues = (float *)MemAlloc((size_t)voxelCount * sizeof(float));
        if (sdfValues == NULL) {
            MemFree(triangles);
            return false;
        }

        const Vector3 rayDir = Vector3Normalize((Vector3){1.0f, 0.231f, 0.497f});
        for (int z = 0; z < depth; ++z) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    const Vector3 p = {
                        localMin.x + ((float)x + 0.5f) / (float)width * localSize.x,
                        localMin.y + ((float)y + 0.5f) / (float)height * localSize.y,
                        localMin.z + ((float)z + 0.5f) / (float)depth * localSize.z,
                    };

                    float minDistanceSquared = FLT_MAX;
                    int hitCount = 0;
                    for (int i = 0; i < triangleCursor; ++i) {
                        minDistanceSquared = fminf(minDistanceSquared,
                            DistanceSquaredPointTriangle3D(p, triangles[i].a, triangles[i].b, triangles[i].c));
                        if (RayIntersectsTriangle3D(p, rayDir, triangles[i].a, triangles[i].b, triangles[i].c, NULL)) {
                            hitCount += 1;
                        }
                    }

                    const float distance = sqrtf(fmaxf(minDistanceSquared, 0.0f));
                    const float signedDistance = ((hitCount & 1) != 0) ? -distance : distance;
                    sdfValues[(z * height + y) * width + x] = signedDistance;
                }
            }
        }

        MemFree(triangles);
        if (system->importedSdfValues != NULL) {
            MemFree(system->importedSdfValues);
        }
        if (normalizeGeometry) {
            NormalizeImportedModelGeometry(model, rawCenter, scale);
            system->importedObstacleRawCenter = (Vector3){0.0f, 0.0f, 0.0f};
            system->importedObstacleScale = 1.0f;
        }
        system->importedObstacleLocalMin = localMin;
        system->importedObstacleLocalMax = localMax;
        system->importedSdfWidth = width;
        system->importedSdfHeight = height;
        system->importedSdfDepth = depth;
        system->importedSdfVoxelCount = voxelCount;
        system->importedSdfMaxResolution = sdfMaxResolution;
        system->importedSdfBakeQuality = sdfMaxResolution > IMPORTED_SDF_MAX_RES;
        system->importedSdfValues = sdfValues;
        InvalidateGpuBackendState(system);
        return true;
    }

    static Model LoadImportedModelFromPath(const char *path)
    {
        Model model = {0};
        if (path != NULL && IsFileExtension(path, ".obj")) {
            Mesh mesh = {0};
            if (LoadObjMeshFromPath(path, &mesh)) {
                model = LoadModelFromMesh(mesh);
            }
            return model;
        }

    #if defined(__APPLE__) || defined(__unix__) || defined(__linux__)
        char previousDirectory[PATH_MAX];
        const bool havePreviousDirectory = (getcwd(previousDirectory, sizeof(previousDirectory)) != NULL);
        const char *assetDirectory = GetDirectoryPath(path);
        const bool changedDirectory = havePreviousDirectory &&
            assetDirectory != NULL &&
            assetDirectory[0] != '\0' &&
            chdir(assetDirectory) == 0;

        model = LoadModel(path);

        if (changedDirectory) {
            (void)chdir(previousDirectory);
        }
    #else
        model = LoadModel(path);
    #endif
        return model;
    }

    static bool LoadImportedObstacle(ParticleSystem3D *system, const char *path)
    {
        Model model = LoadImportedModelFromPath(path);
        if (model.meshCount <= 0 || model.meshes == NULL) {
            return false;
        }

        BakeImportedModelTransform(&model);
        UnloadImportedObstacle(system);
        if (!BuildImportedObstacleSdf(system, &model, IMPORTED_SDF_MAX_RES, true)) {
            UnloadModel(model);
            return false;
        }

    system->importedObstacleModel = model;
    system->importedObstacleLoaded = true;
    snprintf(system->importedObstaclePath, sizeof(system->importedObstaclePath), "%s", path);
    system->importedObstacleUserScale = 1.0f;
    system->importedObstacleOffset = (Vector3){0.0f, 0.0f, 0.0f};
    ResetImportedObstacleRotationState(system);
    system->obstacleModel = OBSTACLE_IMPORTED;
        if (system->scene != SCENE_WIND_TUNNEL) {
            SetSimulationScene(system, SCENE_WIND_TUNNEL);
        } else {
            ResetSimulation(system, MATERIAL_GAS);
        }
        InvalidateGpuBackendState(system);
        return true;
    }

    static bool UpgradeImportedObstacleSdfForBake(ParticleSystem3D *system)
    {
        if (!system->importedObstacleLoaded || system->obstacleModel != OBSTACLE_IMPORTED) {
            return true;
        }
        if (system->importedSdfBakeQuality && system->importedSdfMaxResolution >= IMPORTED_SDF_BAKE_MAX_RES) {
            return true;
        }
        return BuildImportedObstacleSdf(system, &system->importedObstacleModel, IMPORTED_SDF_BAKE_MAX_RES, false);
    }

    static Material ImportedObstacleMaterial(const Model *model, int meshIndex)
    {
        if (model != NULL && model->materials != NULL && model->materialCount > 0) {
            int materialIndex = 0;
            if (model->meshMaterial != NULL && meshIndex >= 0 && meshIndex < model->meshCount) {
                materialIndex = model->meshMaterial[meshIndex];
            }
            if (materialIndex < 0 || materialIndex >= model->materialCount) {
                materialIndex = 0;
            }

            if (IsMaterialValid(model->materials[materialIndex])) {
                Material material = model->materials[materialIndex];
                if (material.maps != NULL) {
                    const Texture2D fallbackTexture = GetShapesTexture();
                    if (!IsTextureValid(material.maps[MATERIAL_MAP_ALBEDO].texture) && IsTextureValid(fallbackTexture)) {
                        material.maps[MATERIAL_MAP_ALBEDO].texture = fallbackTexture;
                    }
                }
                return material;
            }
        }

        static Material fallbackMaterial = {0};
        static bool fallbackMaterialReady = false;
        if (!fallbackMaterialReady || !IsMaterialValid(fallbackMaterial)) {
            fallbackMaterial = LoadMaterialDefault();
            fallbackMaterialReady = true;
        }
        return fallbackMaterial;
    }

    static bool OpenImportedObstacleDialog(ParticleSystem3D *system)
    {
        char path[IMPORTED_OBSTACLE_PATH_MAX];
        if (!OpenObjFileDialog(path, sizeof(path))) {
            return false;
        }

        if (LoadImportedObstacle(system, path)) {
            SetBackendNotice(system, "Imported OBJ as a static SDF obstacle.");
            return true;
        }

        SetBackendNotice(system, "OBJ import failed.");
        return false;
    }

    static void AcousticFft(float *real, float *imag, int n)
    {
        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            while ((j & bit) != 0) {
                j ^= bit;
                bit >>= 1;
            }
            j ^= bit;
            if (i < j) {
                const float realTemp = real[i];
                const float imagTemp = imag[i];
                real[i] = real[j];
                imag[i] = imag[j];
                real[j] = realTemp;
                imag[j] = imagTemp;
            }
        }

        for (int length = 2; length <= n; length <<= 1) {
            const float angle = -2.0f * PI_F / (float)length;
            const float stepReal = cosf(angle);
            const float stepImag = sinf(angle);
            const int halfLength = length >> 1;
            for (int base = 0; base < n; base += length) {
                float wReal = 1.0f;
                float wImag = 0.0f;
                for (int offset = 0; offset < halfLength; ++offset) {
                    const int evenIndex = base + offset;
                    const int oddIndex = evenIndex + halfLength;
                    const float oddReal = real[oddIndex] * wReal - imag[oddIndex] * wImag;
                    const float oddImag = real[oddIndex] * wImag + imag[oddIndex] * wReal;
                    const float evenReal = real[evenIndex];
                    const float evenImag = imag[evenIndex];
                    real[evenIndex] = evenReal + oddReal;
                    imag[evenIndex] = evenImag + oddImag;
                    real[oddIndex] = evenReal - oddReal;
                    imag[oddIndex] = evenImag - oddImag;

                    const float nextReal = wReal * stepReal - wImag * stepImag;
                    wImag = wReal * stepImag + wImag * stepReal;
                    wReal = nextReal;
                }
            }
        }
    }

    static float AnalyzeAudioUsefulBandwidthHz(const float *samples, int sampleCount, float sampleRate)
    {
        const float nyquistHz = fmaxf(sampleRate * 0.5f, 0.0f);
        const float maxTargetHz = fminf(ACOUSTIC_AUDIO_TARGET_BANDWIDTH_HZ, nyquistHz * 0.98f);
        if (samples == NULL || sampleCount < 512 || sampleRate <= 0.0f || maxTargetHz <= 0.0f) {
            return ACOUSTIC_AUDIO_MIN_TARGET_BANDWIDTH_HZ;
        }

        int fftSize = ACOUSTIC_AUDIO_ANALYSIS_FFT_SIZE;
        while (fftSize > sampleCount && fftSize > 512) {
            fftSize >>= 1;
        }
        if (fftSize > sampleCount || fftSize < 512) {
            return ACOUSTIC_AUDIO_MIN_TARGET_BANDWIDTH_HZ;
        }

        const int binCount = fftSize / 2 + 1;
        const int maxBin = ClampInt((int)floorf(maxTargetHz * (float)fftSize / sampleRate), 1, binCount - 1);
        float *binPower = (float *)MemAlloc((size_t)binCount * sizeof(float));
        float *real = (float *)MemAlloc((size_t)fftSize * sizeof(float));
        float *imag = (float *)MemAlloc((size_t)fftSize * sizeof(float));
        if (binPower == NULL || real == NULL || imag == NULL) {
            if (binPower != NULL) {
                MemFree(binPower);
            }
            if (real != NULL) {
                MemFree(real);
            }
            if (imag != NULL) {
                MemFree(imag);
            }
            return ACOUSTIC_AUDIO_TARGET_BANDWIDTH_HZ;
        }

        memset(binPower, 0, (size_t)binCount * sizeof(float));
        int windowCount = ClampInt(sampleCount / ClampInt(fftSize / 2, 1, fftSize), 1,
            ACOUSTIC_AUDIO_ANALYSIS_MAX_WINDOWS);
        if (windowCount <= 1 && sampleCount > fftSize) {
            windowCount = 2;
        }
        const int maxStart = sampleCount - fftSize;
        for (int windowIndex = 0; windowIndex < windowCount; ++windowIndex) {
            const int start = (windowCount <= 1 || maxStart <= 0)
                ? ClampInt(maxStart / 2, 0, (maxStart > 0) ? maxStart : 0)
                : (int)llround((double)maxStart * (double)windowIndex / (double)(windowCount - 1));
            for (int i = 0; i < fftSize; ++i) {
                const float window = 0.5f - 0.5f * cosf(2.0f * PI_F * (float)i / (float)(fftSize - 1));
                real[i] = samples[start + i] * window;
                imag[i] = 0.0f;
            }
            AcousticFft(real, imag, fftSize);
            for (int bin = 1; bin <= maxBin; ++bin) {
                binPower[bin] += real[bin] * real[bin] + imag[bin] * imag[bin];
            }
        }

        float totalPower = 0.0f;
        float peakPower = 0.0f;
        for (int bin = 1; bin <= maxBin; ++bin) {
            totalPower += binPower[bin];
            peakPower = fmaxf(peakPower, binPower[bin]);
        }

        float targetHz = ACOUSTIC_AUDIO_MIN_TARGET_BANDWIDTH_HZ;
        if (totalPower > 1e-12f && peakPower > 1e-12f) {
            const float rolloffTarget = totalPower * ACOUSTIC_AUDIO_ANALYSIS_ROLLOFF;
            float cumulativePower = 0.0f;
            int rolloffBin = 1;
            for (int bin = 1; bin <= maxBin; ++bin) {
                cumulativePower += binPower[bin];
                rolloffBin = bin;
                if (cumulativePower >= rolloffTarget) {
                    break;
                }
            }

            const float gatedPower = peakPower * ACOUSTIC_AUDIO_ANALYSIS_GATE_RATIO;
            int gatedBin = 1;
            for (int bin = 1; bin <= maxBin; ++bin) {
                const float localPower =
                    binPower[ClampInt(bin - 1, 1, maxBin)] +
                    binPower[bin] +
                    binPower[ClampInt(bin + 1, 1, maxBin)];
                if (localPower >= gatedPower * 3.0f) {
                    gatedBin = bin;
                }
            }

            const float binHz = sampleRate / (float)fftSize;
            const float rolloffHz = (float)rolloffBin * binHz;
            const float gatedHz = (float)gatedBin * binHz;
            targetHz = fmaxf(rolloffHz, gatedHz) * ACOUSTIC_AUDIO_BANDWIDTH_HEADROOM;
        }

        MemFree(binPower);
        MemFree(real);
        MemFree(imag);
        return ClampFloat(targetHz, ACOUSTIC_AUDIO_MIN_TARGET_BANDWIDTH_HZ, maxTargetHz);
    }

    static void ClearAcousticAudio(ParticleSystem3D *system)
    {
        if (system->acousticEnvelope != NULL) {
            MemFree(system->acousticEnvelope);
            system->acousticEnvelope = NULL;
        }
        if (system->acousticWaveform != NULL) {
            MemFree(system->acousticWaveform);
            system->acousticWaveform = NULL;
        }
        system->acousticEnvelopeCount = 0;
        system->acousticEnvelopeSampleRate = 0.0f;
        system->acousticEnvelopeDuration = 0.0f;
        system->acousticWaveformCount = 0;
        system->acousticWaveformSampleRate = 0.0f;
        system->acousticAudioBandwidthHz = 0.0f;
        system->acousticAudioLoaded = false;
        system->acousticAudioPath[0] = '\0';
        snprintf(system->acousticAudioLabel, sizeof(system->acousticAudioLabel), "%s", "procedural");
    }

    static bool LoadAcousticAudioPath(ParticleSystem3D *system, const char *path)
    {
        if (path == NULL || path[0] == '\0') {
            return false;
        }

        Wave wave = LoadWave(path);
        if (!IsWaveValid(wave) || wave.frameCount <= 0) {
            if (IsWaveValid(wave)) {
                UnloadWave(wave);
            }
            SetBackendNotice(system, "Audio import failed.");
            return false;
        }

        WaveFormat(&wave, BAKE_MIC_EXPORT_SAMPLE_RATE, 32, 1);
        float *samples = LoadWaveSamples(wave);
        if (samples == NULL) {
            UnloadWave(wave);
            SetBackendNotice(system, "Audio sample decode failed.");
            return false;
        }
        const float sourceBandwidthHz = AnalyzeAudioUsefulBandwidthHz(samples, (int)wave.frameCount, (float)wave.sampleRate);

        const float envelopeRate = 1000.0f;
        const int envelopeCount = ClampInt((int)ceilf((float)wave.frameCount / (float)wave.sampleRate * envelopeRate) + 2, 2, 600000);
        float *envelope = (float *)MemAlloc((size_t)envelopeCount * sizeof(float));
        if (envelope == NULL) {
            UnloadWaveSamples(samples);
            UnloadWave(wave);
            SetBackendNotice(system, "Audio envelope allocation failed.");
            return false;
        }

        const float waveformRate = ACOUSTIC_AUDIO_DRIVER_SAMPLE_RATE;
        const int waveformCount = ClampInt((int)ceilf((float)wave.frameCount / (float)wave.sampleRate * waveformRate) + 2, 2, 5000000);
        float *waveform = (float *)MemAlloc((size_t)waveformCount * sizeof(float));
        if (waveform == NULL) {
            MemFree(envelope);
            UnloadWaveSamples(samples);
            UnloadWave(wave);
            SetBackendNotice(system, "Audio waveform allocation failed.");
            return false;
        }

        const int framesPerEnvelopeSample = ClampInt((int)floorf((float)wave.sampleRate / envelopeRate), 1, wave.sampleRate);
        float peak = 1e-6f;
        for (int i = 0; i < envelopeCount; ++i) {
            const int frameStart = i * framesPerEnvelopeSample;
            const int frameEnd = ClampInt(frameStart + framesPerEnvelopeSample, frameStart, (int)wave.frameCount);
            float sum = 0.0f;
            int count = 0;
            for (int frame = frameStart; frame < frameEnd; ++frame) {
                sum += fabsf(samples[frame]);
                ++count;
            }
            const float value = (count > 0) ? (sum / (float)count) : 0.0f;
            envelope[i] = value;
            peak = fmaxf(peak, value);
        }

        for (int i = 0; i < envelopeCount; ++i) {
            envelope[i] = ClampFloat(envelope[i] / peak, 0.0f, 1.0f);
        }

        float waveformPeak = 1e-6f;
        for (int i = 0; i < waveformCount; ++i) {
            const float sourcePosition = (float)i / waveformRate * (float)wave.sampleRate;
            const int i0 = ClampInt((int)floorf(sourcePosition), 0, (int)wave.frameCount - 1);
            const int i1 = ClampInt(i0 + 1, 0, (int)wave.frameCount - 1);
            const float t = sourcePosition - (float)i0;
            const float value = samples[i0] + (samples[i1] - samples[i0]) * t;
            waveform[i] = value;
            waveformPeak = fmaxf(waveformPeak, fabsf(value));
        }

        for (int i = 0; i < waveformCount; ++i) {
            waveform[i] = ClampFloat(waveform[i] / waveformPeak, -1.0f, 1.0f);
        }

        ClearAcousticAudio(system);
        system->acousticEnvelope = envelope;
        system->acousticEnvelopeCount = envelopeCount;
        system->acousticEnvelopeSampleRate = envelopeRate;
        system->acousticEnvelopeDuration = (float)wave.frameCount / (float)wave.sampleRate;
        system->acousticWaveform = waveform;
        system->acousticWaveformCount = waveformCount;
        system->acousticWaveformSampleRate = waveformRate;
        system->acousticAudioBandwidthHz = sourceBandwidthHz;
        system->acousticAudioLoaded = true;
        system->acousticsEnabled = true;
        snprintf(system->acousticAudioPath, sizeof(system->acousticAudioPath), "%s", path);
        snprintf(system->acousticAudioLabel, sizeof(system->acousticAudioLabel), "%s", GetFileName(path));
        UnloadWaveSamples(samples);
        UnloadWave(wave);
        ApplyAudioAcousticPreset(system);
        ResetSimulation(system, MATERIAL_GAS);
        ClampAcousticAnchors(system);
        SetBackendNotice(system, "Loaded audio and enabled high-resolution gas acoustic bake setup.");
        return true;
    }

    static bool LoadAcousticAudioFile(ParticleSystem3D *system)
    {
        char path[BAKE_PATH_MAX];
        if (!OpenAudioFileDialog(path, sizeof(path))) {
            return false;
        }

        return LoadAcousticAudioPath(system, path);
    }

    static void ConfigureSceneParameters(ParticleSystem3D *system)
    {
        system->obstacleCenter = system->boundsCenter;
        system->obstacleRadius = 0.0f;
        system->obstacleDepth = 0.0f;
        system->obstacleShell = 0.0f;
        system->obstacleStrength = 0.0f;
        system->obstacleDamping = 0.0f;
        system->flowTargetSpeed = 0.0f;
        system->flowDrive = 0.0f;

        if (WaterObstaclesActive(system)) {
            system->obstacleShell = system->params.supportRadius * 1.55f;
            system->obstacleStrength = system->params.soundSpeed * system->params.soundSpeed * 1.32f;
            system->obstacleDamping = system->params.soundSpeed * 0.74f / fmaxf(system->params.supportRadius, 1.0f);
            return;
        }

        if (!SceneIsWindTunnel(system)) {
            return;
        }

        system->obstacleCenter = (Vector3){
            system->boundsMin.x + system->boundsSize.x * 0.43f,
            system->boundsMin.y + system->boundsSize.y * 0.50f,
            system->boundsCenter.z,
        };
        system->obstacleDepth = system->boundsSize.z * 0.78f;

        switch (system->obstacleModel) {
            case OBSTACLE_AIRFOIL:
                system->obstacleRadius = fminf(system->boundsSize.x, system->boundsSize.y) * 0.060f;
                system->obstacleShell = system->params.supportRadius * 1.08f;
                system->obstacleStrength = system->params.soundSpeed * system->params.soundSpeed * 0.42f;
                system->obstacleDamping = system->params.soundSpeed * 0.55f / fmaxf(system->params.supportRadius, 1.0f);
                break;
            case OBSTACLE_CAR:
                system->obstacleRadius = fminf(system->boundsSize.x, system->boundsSize.y) * 0.062f;
                system->obstacleDepth = system->boundsSize.z * 0.34f;
                system->obstacleShell = system->params.supportRadius * 1.42f;
                system->obstacleStrength = system->params.soundSpeed * system->params.soundSpeed * 0.66f;
                system->obstacleDamping = system->params.soundSpeed * 0.78f / fmaxf(system->params.supportRadius, 1.0f);
                break;
            case OBSTACLE_RECTANGLE:
                system->obstacleRadius = 0.5f * fmaxf(system->obstacleRectWidth, system->obstacleRectHeight);
                system->obstacleShell = system->params.supportRadius * 1.12f;
                system->obstacleStrength = system->params.soundSpeed * system->params.soundSpeed * 0.44f;
                system->obstacleDamping = system->params.soundSpeed * 0.58f / fmaxf(system->params.supportRadius, 1.0f);
                break;
            case OBSTACLE_IMPORTED:
                system->obstacleCenter = system->boundsCenter;
                system->obstacleRadius = fminf(system->boundsSize.x, system->boundsSize.y) * 0.075f;
                system->obstacleShell = system->params.supportRadius * 1.22f;
                system->obstacleStrength = system->params.soundSpeed * system->params.soundSpeed * 0.54f;
                system->obstacleDamping = system->params.soundSpeed * 0.68f / fmaxf(system->params.supportRadius, 1.0f);
                break;
            case OBSTACLE_CIRCLE:
            default:
                system->obstacleRadius = fminf(system->boundsSize.x, system->boundsSize.y) * 0.085f;
                system->obstacleShell = system->params.supportRadius * 1.55f;
                system->obstacleStrength = system->params.soundSpeed * system->params.soundSpeed * 0.55f;
                system->obstacleDamping = system->params.soundSpeed * 0.70f / fmaxf(system->params.supportRadius, 1.0f);
                break;
        }

        if (system->bakeCapacityMode) {
            system->obstacleStrength *= 1.28f;
            system->obstacleDamping *= 1.18f;
            if (system->obstacleModel == OBSTACLE_IMPORTED) {
                system->obstacleShell *= 0.88f;
            }
        }

        const float flowScale = fmaxf(system->flowSpeedScale, 0.35f);
        system->flowTargetSpeed = system->params.soundSpeed * 1.10f * flowScale;
        system->flowDrive = system->params.soundSpeed * system->params.soundSpeed * 0.18f * flowScale /
            fmaxf(system->params.supportRadius, 1.0f);
    }

    static float TracerDyeValueForPoint(const ParticleSystem3D *system, float x, float y, float z, int seed)
    {
        const float xNorm = Saturate(RangeLerp(system->boundsMin.x, system->boundsMax.x, x));
        const float yNorm = Saturate(RangeLerp(system->boundsMin.y, system->boundsMax.y, y));
        const float zNorm = Saturate(RangeLerp(system->boundsMin.z, system->boundsMax.z, z));

        if (SceneIsWindTunnel(system)) {
            const float stripes = 0.5f + 0.5f * sinf(yNorm * 10.5f + zNorm * 7.2f + (float)seed * 0.031f);
            const float edgeBias = 0.45f + 0.55f * (0.65f * zNorm + 0.35f * (1.0f - yNorm));
            return Saturate(0.15f + 0.75f * (0.72f * stripes + 0.28f * edgeBias));
        }

        const float swirls = 0.5f + 0.5f * sinf(xNorm * 6.4f + zNorm * 8.2f + (float)seed * 0.017f);
        const float buoyantMix = 0.55f * (1.0f - yNorm) + 0.45f * swirls;
        return Saturate(0.12f + 0.78f * buoyantMix);
    }

    static Vector3 SlicePanelPosition(const ParticleSystem3D *system, int axis)
    {
        switch (axis) {
            case 0:
                return (Vector3){LerpFloat(system->boundsMin.x, system->boundsMax.x, system->sliceXNormalized), 0.0f, 0.0f};
            case 1:
                return (Vector3){0.0f, LerpFloat(system->boundsMin.y, system->boundsMax.y, system->sliceYNormalized), 0.0f};
            case 2:
            default:
                return (Vector3){0.0f, 0.0f, LerpFloat(system->boundsMin.z, system->boundsMax.z, system->sliceZNormalized)};
        }
    }

    static bool SliceAxisEnabled(const ParticleSystem3D *system, int axis)
    {
        switch (axis) {
            case 0: return system->sliceXEnabled;
            case 1: return system->sliceYEnabled;
            case 2:
            default:
                return system->sliceZEnabled;
        }
    }

    static int ActivePathlineCount(const ParticleSystem3D *system)
    {
        return system->pathlineDensity * system->pathlineDensity;
    }

    static Vector3 SeedFlowLinePosition(const ParticleSystem3D *system, int index, int density)
    {
        const int gridY = ClampInt(density, FLOW_LINE_DENSITY_MIN, FLOW_LINE_DENSITY_MAX);
        const int gridZ = gridY;
        const int yIndex = index / gridZ;
        const int zIndex = index % gridZ;
        const float yT = ((float)yIndex + 0.5f) / (float)gridY;
        const float zT = ((float)zIndex + 0.5f) / (float)gridZ;
        return (Vector3){
            system->boundsMin.x + system->params.supportRadius * 1.8f,
            LerpFloat(system->boundsMin.y + system->boundsSize.y * 0.16f, system->boundsMax.y - system->boundsSize.y * 0.16f, yT),
            LerpFloat(system->boundsMin.z + system->boundsSize.z * 0.10f, system->boundsMax.z - system->boundsSize.z * 0.10f, zT),
        };
    }

    static void ResetPathlines(ParticleSystem3D *system)
    {
        system->pathlineAccumulator = 0.0f;
        const int activeCount = ActivePathlineCount(system);
        for (int i = 0; i < activeCount; ++i) {
            const Vector3 seed = SeedFlowLinePosition(system, i, system->pathlineDensity);
            system->pathlines[i].current = seed;
            system->pathlines[i].count = 1;
            system->pathlines[i].trail[0] = seed;
        }
        for (int i = activeCount; i < FLOW_PATHLINE_COUNT_MAX; ++i) {
            system->pathlines[i].count = 0;
        }
    }

    static void UpdateMassAndEquationOfState(ParticleSystem3D *system)
    {
        if (AcousticsActive(system)) {
            system->params.soundSpeed = EffectiveAcousticSoundSpeed(system);
            system->params.kinematicViscosity = fmaxf(0.01f,
                system->baseKinematicViscosity * ClampFloat(system->acousticViscosityScale, 0.01f, 4.0f));
            system->params.globalDrag = fmaxf(0.0f,
                system->baseGlobalDrag * ClampFloat(system->acousticDragScale, 0.0f, 4.0f));
        } else {
            if (system->baseSoundSpeed > 0.0f) {
                system->params.soundSpeed = system->baseSoundSpeed;
            }
            if (system->baseKinematicViscosity > 0.0f) {
                system->params.kinematicViscosity = system->baseKinematicViscosity;
            }
            if (system->baseGlobalDrag >= 0.0f) {
                system->params.globalDrag = system->baseGlobalDrag;
            }
        }

        const float h = system->params.supportRadius;
        system->supportRadiusSquared = h * h;
        system->mass = system->params.restDensity *
            system->params.spacing * system->params.spacing * system->params.spacing;
        system->pressureStiffness = system->params.restDensity *
            system->params.soundSpeed * system->params.soundSpeed / 7.0f;
        system->densityKernel = 315.0f / (64.0f * PI_F * powf(h, 9.0f));
        system->pressureKernelGrad = -45.0f / (PI_F * powf(h, 6.0f));
        system->viscosityKernelLap = 45.0f / (PI_F * powf(h, 6.0f));
        system->interactionRadius = h * 6.0f;
        system->interactionRadiusSquared = system->interactionRadius * system->interactionRadius;
        system->interactionStrength = system->params.soundSpeed * system->params.soundSpeed * 2.4f;
        system->speakerShell = h * 1.25f;
        system->speakerStrength = system->params.soundSpeed * system->params.soundSpeed * 1.25f;
        system->speakerDamping = system->params.soundSpeed * 1.45f / fmaxf(h, 1.0f);
    }

    static void RefreshDerivedSimulationParameters(ParticleSystem3D *system)
    {
        UpdateMassAndEquationOfState(system);
        ConfigureSceneParameters(system);
        ClampAcousticAnchors(system);
        system->scalarFieldsDirty = true;
    }

    static void AllocateSystem(ParticleSystem3D *system, int maxParticles)
    {
        memset(system, 0, sizeof(*system));
        system->maxParticles = maxParticles;
        system->viewMode = VIEW_PARTICLES;
        system->colorMode = COLOR_MATERIAL;
        system->activeBackend = SIM_BACKEND_CPU;
        system->scene = SCENE_TANK;
        system->obstacleModel = OBSTACLE_CIRCLE;
        system->waterObstacleEnabled = false;
        system->waterObstacleShape = UI_3D_WATER_OBSTACLE_CYLINDERS;
        system->waterCylinderCount = 4;
        system->waterCylinderSelected = 0;
        system->obstacleAngleDegrees = 0.0f;
        system->obstacleRectWidth = RECT_WIDTH_MIN_3D;
        system->obstacleRectHeight = RECT_HEIGHT_MIN_3D;
        system->tankPreset = MATERIAL_WATER;
        system->gpuBackendAvailable = false;
        system->timeScale = 1.0f;
        system->flowSpeedScale = 1.0f;
        system->showParticles = true;
        system->sliceXNormalized = 0.50f;
        system->sliceYNormalized = 0.50f;
        system->sliceZNormalized = 0.50f;
        system->sliceXEnabled = true;
        system->sliceYEnabled = true;
        system->sliceZEnabled = true;
        system->streamlineDensity = 4;
        system->streamlineStepCount = 20;
        system->pathlineDensity = 4;
        system->pathlineTrailLength = 18;
        system->importedObstacleUserScale = 1.0f;
        system->importedObstacleOrientation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
        system->speakerWidth = 18.0f;
        system->speakerHeight = 52.0f;
        system->speakerDepth = 26.0f;
        system->speakerFrequency = 2.4f;
        system->speakerAmplitude = 10.0f;
        system->micRadius = 18.0f;
        system->acousticSoundSpeed = 180.0f;
        system->acousticMachLimit = 0.30f;
        system->acousticViscosityScale = 0.40f;
        system->acousticDragScale = 0.35f;
        system->acousticAudioSlowdownFactor = ACOUSTIC_AUDIO_SLOWDOWN_FACTOR;
        system->audioOutputEnabled = true;
        system->audioMonitorPitchHz = 180.0f;

        const size_t particleBytes = (size_t)maxParticles * sizeof(float);
        const size_t particleIntBytes = (size_t)maxParticles * sizeof(int);
        const size_t drawBytes = (size_t)maxParticles * sizeof(ParticleDrawItem);

        system->x = (float *)MemAlloc(particleBytes);
        system->y = (float *)MemAlloc(particleBytes);
        system->z = (float *)MemAlloc(particleBytes);
        system->vx = (float *)MemAlloc(particleBytes);
        system->vy = (float *)MemAlloc(particleBytes);
        system->vz = (float *)MemAlloc(particleBytes);
        system->ax = (float *)MemAlloc(particleBytes);
        system->ay = (float *)MemAlloc(particleBytes);
        system->az = (float *)MemAlloc(particleBytes);
        system->density = (float *)MemAlloc(particleBytes);
        system->pressure = (float *)MemAlloc(particleBytes);
        system->temperature = (float *)MemAlloc(particleBytes);
        system->temperatureRate = (float *)MemAlloc(particleBytes);
        system->dye = (float *)MemAlloc(particleBytes);
        system->vorticity = (float *)MemAlloc(particleBytes);
        system->xsphVX = (float *)MemAlloc(particleBytes);
        system->xsphVY = (float *)MemAlloc(particleBytes);
        system->xsphVZ = (float *)MemAlloc(particleBytes);
        system->sortedX = (float *)MemAlloc(particleBytes);
        system->sortedY = (float *)MemAlloc(particleBytes);
        system->sortedZ = (float *)MemAlloc(particleBytes);
        system->sortedVX = (float *)MemAlloc(particleBytes);
        system->sortedVY = (float *)MemAlloc(particleBytes);
        system->sortedVZ = (float *)MemAlloc(particleBytes);
        system->sortedTemperature = (float *)MemAlloc(particleBytes);
        system->sortedDensity = (float *)MemAlloc(particleBytes);
        system->sortedPressure = (float *)MemAlloc(particleBytes);
        system->particleCells = (int *)MemAlloc(particleIntBytes);
        system->sortedCellIndices = (int *)MemAlloc(particleIntBytes);
        system->sortedIndices = (int *)MemAlloc(particleIntBytes);
        system->drawItems = (ParticleDrawItem *)MemAlloc(drawBytes);

        if (system->x == NULL || system->y == NULL || system->z == NULL ||
            system->vx == NULL || system->vy == NULL || system->vz == NULL ||
            system->ax == NULL || system->ay == NULL || system->az == NULL ||
            system->density == NULL || system->pressure == NULL ||
            system->temperature == NULL || system->temperatureRate == NULL ||
            system->dye == NULL || system->vorticity == NULL ||
            system->xsphVX == NULL || system->xsphVY == NULL || system->xsphVZ == NULL ||
            system->sortedX == NULL || system->sortedY == NULL || system->sortedZ == NULL ||
            system->sortedVX == NULL || system->sortedVY == NULL || system->sortedVZ == NULL ||
            system->sortedTemperature == NULL || system->sortedDensity == NULL || system->sortedPressure == NULL ||
            system->particleCells == NULL || system->sortedCellIndices == NULL ||
            system->sortedIndices == NULL || system->drawItems == NULL) {
            fprintf(stderr, "Failed to allocate 3D simulation buffers\n");
            exit(EXIT_FAILURE);
        }
    }

    static bool ResizeSystemCapacity(ParticleSystem3D *system, int maxParticles)
    {
        if (maxParticles <= system->maxParticles) {
            return true;
        }

        ShutdownGpuBackend(system);
        const size_t particleBytes = (size_t)maxParticles * sizeof(float);
        const size_t particleIntBytes = (size_t)maxParticles * sizeof(int);
        const size_t drawBytes = (size_t)maxParticles * sizeof(ParticleDrawItem);

    #define RESIZE_FLOAT_BUFFER(name) \
        do { \
            float *nextBuffer = (float *)MemRealloc(system->name, particleBytes); \
            if (nextBuffer == NULL) { \
                return false; \
            } \
            system->name = nextBuffer; \
        } while (0)

    #define RESIZE_INT_BUFFER(name) \
        do { \
            int *nextBuffer = (int *)MemRealloc(system->name, particleIntBytes); \
            if (nextBuffer == NULL) { \
                return false; \
            } \
            system->name = nextBuffer; \
        } while (0)

        RESIZE_FLOAT_BUFFER(x);
        RESIZE_FLOAT_BUFFER(y);
        RESIZE_FLOAT_BUFFER(z);
        RESIZE_FLOAT_BUFFER(vx);
        RESIZE_FLOAT_BUFFER(vy);
        RESIZE_FLOAT_BUFFER(vz);
        RESIZE_FLOAT_BUFFER(ax);
        RESIZE_FLOAT_BUFFER(ay);
        RESIZE_FLOAT_BUFFER(az);
        RESIZE_FLOAT_BUFFER(density);
        RESIZE_FLOAT_BUFFER(pressure);
        RESIZE_FLOAT_BUFFER(temperature);
        RESIZE_FLOAT_BUFFER(temperatureRate);
        RESIZE_FLOAT_BUFFER(dye);
        RESIZE_FLOAT_BUFFER(vorticity);
        RESIZE_FLOAT_BUFFER(xsphVX);
        RESIZE_FLOAT_BUFFER(xsphVY);
        RESIZE_FLOAT_BUFFER(xsphVZ);
        RESIZE_FLOAT_BUFFER(sortedX);
        RESIZE_FLOAT_BUFFER(sortedY);
        RESIZE_FLOAT_BUFFER(sortedZ);
        RESIZE_FLOAT_BUFFER(sortedVX);
        RESIZE_FLOAT_BUFFER(sortedVY);
        RESIZE_FLOAT_BUFFER(sortedVZ);
        RESIZE_FLOAT_BUFFER(sortedTemperature);
        RESIZE_FLOAT_BUFFER(sortedDensity);
        RESIZE_FLOAT_BUFFER(sortedPressure);
        RESIZE_INT_BUFFER(particleCells);
        RESIZE_INT_BUFFER(sortedCellIndices);
        RESIZE_INT_BUFFER(sortedIndices);

        ParticleDrawItem *nextDrawItems = (ParticleDrawItem *)MemRealloc(system->drawItems, drawBytes);
        if (nextDrawItems == NULL) {
            return false;
        }
        system->drawItems = nextDrawItems;
        system->maxParticles = maxParticles;

    #undef RESIZE_FLOAT_BUFFER
    #undef RESIZE_INT_BUFFER

        return true;
    }

    static void RebuildGrid(ParticleSystem3D *system)
    {
        const int newGridWidth = (int)ceilf(system->boundsSize.x / system->params.supportRadius) + 2;
        const int newGridHeight = (int)ceilf(system->boundsSize.y / system->params.supportRadius) + 2;
        const int newGridDepth = (int)ceilf(system->boundsSize.z / system->params.supportRadius) + 2;
        const int newCellCount = newGridWidth * newGridHeight * newGridDepth;
        const bool cellCountChanged = (newCellCount != system->cellCount);
        const bool gridShapeChanged = (newGridWidth != system->gridWidth) ||
            (newGridHeight != system->gridHeight) || (newGridDepth != system->gridDepth);

        if (cellCountChanged) {
            if (system->cellCounts != NULL) {
                MemFree(system->cellCounts);
            }
            if (system->cellStarts != NULL) {
                MemFree(system->cellStarts);
            }
            if (system->cellOffsets != NULL) {
                MemFree(system->cellOffsets);
            }
            if (system->cellNeighborCounts != NULL) {
                MemFree(system->cellNeighborCounts);
            }
            if (system->cellNeighbors != NULL) {
                MemFree(system->cellNeighbors);
            }

            system->cellCounts = (int *)MemAlloc((size_t)newCellCount * sizeof(int));
            system->cellStarts = (int *)MemAlloc((size_t)(newCellCount + 1) * sizeof(int));
            system->cellOffsets = (int *)MemAlloc((size_t)newCellCount * sizeof(int));
            system->cellNeighborCounts = (int *)MemAlloc((size_t)newCellCount * sizeof(int));
            system->cellNeighbors = (int *)MemAlloc((size_t)newCellCount * 27u * sizeof(int));

            if (system->cellCounts == NULL || system->cellStarts == NULL || system->cellOffsets == NULL ||
                system->cellNeighborCounts == NULL || system->cellNeighbors == NULL) {
                fprintf(stderr, "Failed to allocate 3D grid cells\n");
                exit(EXIT_FAILURE);
            }
            system->cellCount = newCellCount;
        }

        system->gridWidth = newGridWidth;
        system->gridHeight = newGridHeight;
        system->gridDepth = newGridDepth;
        system->cellSize = system->params.supportRadius;
        system->invCellSize = 1.0f / system->cellSize;

        if (cellCountChanged || gridShapeChanged) {
            for (int cellZ = 0; cellZ < system->gridDepth; ++cellZ) {
                for (int cellY = 0; cellY < system->gridHeight; ++cellY) {
                    for (int cellX = 0; cellX < system->gridWidth; ++cellX) {
                        const int cellIndex = (cellZ * system->gridHeight + cellY) * system->gridWidth + cellX;
                        const int neighborBase = cellIndex * 27;
                        int neighborCount = 0;

                        for (int offsetZ = -1; offsetZ <= 1; ++offsetZ) {
                            const int neighborZ = cellZ + offsetZ;
                            if (neighborZ < 0 || neighborZ >= system->gridDepth) {
                                continue;
                            }
                            for (int offsetY = -1; offsetY <= 1; ++offsetY) {
                                const int neighborY = cellY + offsetY;
                                if (neighborY < 0 || neighborY >= system->gridHeight) {
                                    continue;
                                }
                                for (int offsetX = -1; offsetX <= 1; ++offsetX) {
                                    const int neighborX = cellX + offsetX;
                                    if (neighborX < 0 || neighborX >= system->gridWidth) {
                                        continue;
                                    }
                                    system->cellNeighbors[neighborBase + neighborCount] =
                                        (neighborZ * system->gridHeight + neighborY) * system->gridWidth + neighborX;
                                    ++neighborCount;
                                }
                            }
                        }

                        system->cellNeighborCounts[cellIndex] = neighborCount;
                    }
                }
            }
        }
    }

    static void AddBoundaryDensityContribution(const ParticleSystem3D *system, float x, float y, float z, float *density)
    {
        if (system->preset == MATERIAL_GAS) {
            return;
        }

        const float radius = system->params.particleRadius;
        const float boundaryMass = system->mass * BoundaryMassScale(system);
        const float h2 = system->supportRadiusSquared;
        const float minX = system->boundsMin.x + radius;
        const float maxX = system->boundsMax.x - radius;
        const float minY = system->boundsMin.y + radius;
        const float maxY = system->boundsMax.y - radius;
        const float minZ = system->boundsMin.z + radius;
        const float maxZ = system->boundsMax.z - radius;
        const float distances[6] = {
            x - minX, maxX - x, y - minY, maxY - y, z - minZ, maxZ - z,
        };

        for (int wall = 0; wall < 6; ++wall) {
            if (SceneIsWindTunnel(system) && wall < 2) {
                continue;
            }
            const float mirroredDistance = 2.0f * distances[wall];
            const float r2 = mirroredDistance * mirroredDistance;
            if (r2 < h2) {
                const float diff = h2 - r2;
                *density += boundaryMass * system->densityKernel * diff * diff * diff;
            }
        }
    }

    static void AddBoundaryForceContribution(const ParticleSystem3D *system,
        float x, float y, float z,
        float vx, float vy, float vz,
        float rhoi, float pressureI, float temperature,
        float *ax, float *ay, float *az)
    {
        const float radius = system->params.particleRadius;
        const float h = system->params.supportRadius;
        const float h2 = system->supportRadiusSquared;
        const float minX = system->boundsMin.x + radius;
        const float maxX = system->boundsMax.x - radius;
        const float minY = system->boundsMin.y + radius;
        const float maxY = system->boundsMax.y - radius;
        const float minZ = system->boundsMin.z + radius;
        const float maxZ = system->boundsMax.z - radius;
        const float distances[6] = {
            x - minX, maxX - x, y - minY, maxY - y, z - minZ, maxZ - z,
        };
        const Vector3 normals[6] = {
            {1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f},
        };

        if (system->preset == MATERIAL_GAS) {
            const float wallK = 2.2f * system->params.soundSpeed * system->params.soundSpeed / h;
            const float wallDamp = 1.2f * system->params.soundSpeed / h;
            for (int wall = 0; wall < 6; ++wall) {
                if (SceneIsWindTunnel(system) && wall < 2) {
                    continue;
                }
                const float d = distances[wall];
                if (d < h) {
                    const float falloff = 1.0f - d / h;
                    const float vn = vx * normals[wall].x + vy * normals[wall].y + vz * normals[wall].z;
                    const float repulse = wallK * falloff * falloff;
                    const float damp = -wallDamp * fminf(vn, 0.0f);
                    *ax += normals[wall].x * (repulse + damp);
                    *ay += normals[wall].y * (repulse + damp);
                    *az += normals[wall].z * (repulse + damp);
                }
            }
            return;
        }

        const float boundaryMass = system->mass * BoundaryMassScale(system);
        const float boundaryDensity = LocalRestDensity(system, temperature);
        const float boundaryPressure = fmaxf(pressureI, 0.10f * system->pressureStiffness);

        for (int wall = 0; wall < 6; ++wall) {
            if (SceneIsWindTunnel(system) && wall < 2) {
                continue;
            }
            const float mirroredDistance = 2.0f * distances[wall];
            const float r2 = mirroredDistance * mirroredDistance;
            if (r2 > 1e-8f && r2 < h2) {
                const float r = sqrtf(r2);
                const float influence = h - r;
                const float pressureTerm = -boundaryMass *
                    ((pressureI / (rhoi * rhoi)) + (boundaryPressure / (boundaryDensity * boundaryDensity))) *
                    system->pressureKernelGrad * influence * influence;
                const float viscosityTerm = system->params.kinematicViscosity * boundaryMass *
                    system->viscosityKernelLap * influence / boundaryDensity;
                *ax += pressureTerm * normals[wall].x;
                *ay += pressureTerm * normals[wall].y;
                *az += pressureTerm * normals[wall].z;
                *ax -= viscosityTerm * vx * fabsf(normals[wall].x);
                *ay -= viscosityTerm * vy * fabsf(normals[wall].y);
                *az -= viscosityTerm * vz * fabsf(normals[wall].z);
            }
        }
    }

    static float PressureFromState(const ParticleSystem3D *system, float density, float temperature)
    {
        if (system->preset == MATERIAL_GAS) {
            const float normalizedTemperature = fmaxf(0.30f, temperature);
            const float gasStiffness = 0.65f * system->params.soundSpeed * system->params.soundSpeed;
            return gasStiffness * density * normalizedTemperature;
        }

        const float ratio = density / LocalRestDensity(system, temperature);
        const float pressure = system->pressureStiffness * (Pow7(ratio) - 1.0f);
        return fmaxf(0.0f, pressure);
    }

    static void RespawnWindTunnelParticle(ParticleSystem3D *system, int index)
    {
        const float radius = system->params.particleRadius;
        const float left = system->boundsMin.x + radius;
        const float top = system->boundsMin.y + radius;
        const float bottom = system->boundsMax.y - radius;
        const float front = system->boundsMin.z + radius;
        const float back = system->boundsMax.z - radius;
        const float inletDepth = fmaxf(system->params.supportRadius * 1.6f, radius * 4.0f);
        const float tunnelHeight = fmaxf(bottom - top, radius * 2.0f);
        const float tunnelDepth = fmaxf(back - front, radius * 2.0f);
        const int seedBase = (int)floorf(GetTime() * 1536.0f) + index * 97;
        const float particleY = ClampFloat(top + HashNoise(seedBase + 17) * tunnelHeight +
            (HashNoise(seedBase + 41) - 0.5f) * system->params.spacing * 0.22f, top, bottom);
        const float particleZ = ClampFloat(front + HashNoise(seedBase + 59) * tunnelDepth +
            (HashNoise(seedBase + 73) - 0.5f) * system->params.spacing * 0.18f, front, back);
        const float profile = WindTunnelProfile(system, particleY, particleZ);

        system->x[index] = left + HashNoise(seedBase + 109) * inletDepth;
        system->y[index] = particleY;
        system->z[index] = particleZ;
        system->vx[index] = system->flowTargetSpeed * profile +
            (HashNoise(seedBase + 149) - 0.5f) * system->params.soundSpeed * 0.020f;
        system->vy[index] = (HashNoise(seedBase + 191) - 0.5f) * system->params.soundSpeed * 0.014f;
        system->vz[index] = (HashNoise(seedBase + 211) - 0.5f) * system->params.soundSpeed * 0.014f;
        system->ax[index] = system->flowDrive * profile;
        system->ay[index] = 0.0f;
        system->az[index] = 0.0f;
        system->temperature[index] = ClampFloat(
            system->params.ambientTemperature + (HashNoise(seedBase + 233) - 0.5f) * 0.02f,
            TEMP_MIN, TEMP_MAX);
        system->temperatureRate[index] = 0.0f;
        system->xsphVX[index] = 0.0f;
        system->xsphVY[index] = 0.0f;
        system->xsphVZ[index] = 0.0f;
        system->density[index] = system->params.restDensity;
        system->pressure[index] = PressureFromState(system, system->density[index], system->temperature[index]);
    }

    static void AddSceneForceContribution(const ParticleSystem3D *system,
        float x, float y, float z,
        float vx, float vy, float vz,
        float *ax, float *ay, float *az)
    {
        if (SceneIsWindTunnel(system)) {
            const float profile = WindTunnelProfile(system, y, z);
            const float xNorm = ClampFloat(RangeLerp(system->boundsMin.x, system->boundsMax.x, x), 0.0f, 1.0f);
            const float driveBias = 1.18f - 0.22f * xNorm;
            *ax += system->flowDrive * profile * driveBias *
                (1.0f - vx / fmaxf(system->flowTargetSpeed, 1e-4f));
            *ay -= system->flowDrive * 0.030f * vy;
            *az -= system->flowDrive * 0.030f * vz;
        }

        if (!SolidObstacleActive(system)) {
            return;
        }

        const float signedDistance = SolidObstacleSignedDistance(system, x, y, z);
        if (signedDistance < system->obstacleShell) {
            const Vector3 normal = SolidObstacleNormal(system, x, y, z);
            const float falloff = ClampFloat(1.0f - signedDistance / system->obstacleShell, 0.0f, 1.8f);
            const float repulse = system->obstacleStrength * falloff * falloff;
            const float vn = vx * normal.x + vy * normal.y + vz * normal.z;
            const float damp = -system->obstacleDamping * fminf(vn, 0.0f);
            *ax += normal.x * (repulse + damp);
            *ay += normal.y * (repulse + damp);
            *az += normal.z * (repulse + damp);
        }
    }

    static void AddSpeakerForceContribution(const ParticleSystem3D *system,
        float x, float y, float z,
        float vx, float vy, float vz,
        float *ax, float *ay, float *az)
    {
        if (!AcousticsActive(system)) {
            return;
        }

        const float signedDistance = SpeakerSignedDistance(system, x, y, z);
        if (signedDistance >= system->speakerShell) {
            return;
        }

        Vector3 speakerVelocity;
        GetSpeakerState(system, NULL, &speakerVelocity, NULL);
        const Vector3 normal = SpeakerNormal(system, x, y, z);
        const float falloff = ClampFloat(1.0f - signedDistance / system->speakerShell, 0.0f, 1.8f);
        const float repulse = system->speakerStrength * falloff * falloff;
        const float relativeVn =
            (vx - speakerVelocity.x) * normal.x +
            (vy - speakerVelocity.y) * normal.y +
            (vz - speakerVelocity.z) * normal.z;
        const float damp = -system->speakerDamping * fminf(relativeVn, 0.0f);
        *ax += normal.x * (repulse + damp);
        *ay += normal.y * (repulse + damp);
        *az += normal.z * (repulse + damp);
    }

    static void ResolveObstacle(ParticleSystem3D *system, int index)
    {
        if (!SolidObstacleActive(system)) {
            return;
        }

        float surfaceOffsetScale = (SceneIsWindTunnel(system) && system->obstacleModel == OBSTACLE_CAR) ? 1.20f : 0.70f;
        if (system->bakeCapacityMode && SceneIsWindTunnel(system)) {
            surfaceOffsetScale = (system->obstacleModel == OBSTACLE_IMPORTED) ? 0.52f : fminf(surfaceOffsetScale, 0.62f);
        }
        const float surfaceOffset = system->params.particleRadius * surfaceOffsetScale;
        Vector3 normal = {1.0f, 0.0f, 0.0f};
        bool resolved = false;

        int maxIterations = (SceneIsWindTunnel(system) && system->obstacleModel == OBSTACLE_CAR) ? 4 : 2;
        if (system->bakeCapacityMode && SceneIsWindTunnel(system)) {
            maxIterations = (system->obstacleModel == OBSTACLE_IMPORTED) ? 5 : ((maxIterations > 3) ? maxIterations : 3);
        }
        for (int iteration = 0; iteration < maxIterations; ++iteration) {
            const float signedDistance = SolidObstacleSignedDistance(system, system->x[index], system->y[index], system->z[index]);
            if (signedDistance >= surfaceOffset) {
                break;
            }

            normal = SolidObstacleNormal(system, system->x[index], system->y[index], system->z[index]);
            const float pushOut = (surfaceOffset - signedDistance) +
                system->params.particleRadius * ((system->bakeCapacityMode && SceneIsWindTunnel(system)) ? 0.14f :
                    ((SceneIsWindTunnel(system) && system->obstacleModel == OBSTACLE_CAR) ? 0.18f : 0.08f));
            system->x[index] += normal.x * pushOut;
            system->y[index] += normal.y * pushOut;
            system->z[index] += normal.z * pushOut;
            resolved = true;
        }

        if (!resolved) {
            return;
        }

        const float vn = system->vx[index] * normal.x + system->vy[index] * normal.y + system->vz[index] * normal.z;
        if (vn < 0.0f) {
            system->vx[index] -= vn * normal.x;
            system->vy[index] -= vn * normal.y;
            system->vz[index] -= vn * normal.z;
        }
        system->vx[index] *= 0.985f;
        system->vy[index] *= 0.985f;
        system->vz[index] *= 0.985f;
    }

    static void ResolveSpeaker(ParticleSystem3D *system, int index)
    {
        if (!AcousticsActive(system)) {
            return;
        }

        const float surfaceOffset = system->params.particleRadius * 0.70f;
        const float signedDistance = SpeakerSignedDistance(system, system->x[index], system->y[index], system->z[index]);
        if (signedDistance >= surfaceOffset) {
            return;
        }

        Vector3 speakerVelocity;
        GetSpeakerState(system, NULL, &speakerVelocity, NULL);
        const Vector3 normal = SpeakerNormal(system, system->x[index], system->y[index], system->z[index]);
        const float pushOut = surfaceOffset - signedDistance;
        system->x[index] += normal.x * pushOut;
        system->y[index] += normal.y * pushOut;
        system->z[index] += normal.z * pushOut;

        const float relativeVn =
            (system->vx[index] - speakerVelocity.x) * normal.x +
            (system->vy[index] - speakerVelocity.y) * normal.y +
            (system->vz[index] - speakerVelocity.z) * normal.z;
        if (relativeVn < 0.0f) {
            system->vx[index] -= relativeVn * normal.x;
            system->vy[index] -= relativeVn * normal.y;
            system->vz[index] -= relativeVn * normal.z;
        }
        system->vx[index] = 0.992f * system->vx[index] + 0.008f * speakerVelocity.x;
        system->vy[index] = 0.992f * system->vy[index] + 0.008f * speakerVelocity.y;
        system->vz[index] = 0.992f * system->vz[index] + 0.008f * speakerVelocity.z;
    }

    static void SetSimulationScene(ParticleSystem3D *system, SimulationScene scene);
    static void ResetSimulation(ParticleSystem3D *system, MaterialPreset preset);

    static void SetObstacleModel(ParticleSystem3D *system, ObstacleModel obstacleModel)
    {
        if (!SceneIsWindTunnel(system) && obstacleModel == OBSTACLE_IMPORTED) {
            obstacleModel = OBSTACLE_CIRCLE;
        }
        if (system->obstacleModel == obstacleModel) {
            return;
        }

        system->obstacleModel = obstacleModel;
        if (SceneIsWindTunnel(system)) {
            ResetSimulation(system, MATERIAL_GAS);
        } else if (SceneIsWaterTank(system) && system->waterObstacleEnabled) {
            ResetSimulation(system, MATERIAL_WATER);
        }
    }

    static void SetWaterObstacleEnabled(ParticleSystem3D *system, bool enabled)
    {
        if (system->waterObstacleEnabled == enabled) {
            return;
        }
        system->waterObstacleEnabled = enabled;
        InitializeDefaultWaterCylinders(system);
        ClampWaterCylinders(system);
        if (SceneIsWaterTank(system)) {
            ResetSimulation(system, MATERIAL_WATER);
        } else {
            ConfigureSceneParameters(system);
            InvalidateGpuBackendState(system);
        }
    }

    static void SetWaterObstacleShape(ParticleSystem3D *system, int shape)
    {
        InitializeDefaultWaterCylinders(system);
        const int clampedShape = ClampInt(shape, 0, UI_3D_WATER_OBSTACLE_SHAPE_COUNT - 1);
        if (system->waterObstacleShape == clampedShape) {
            return;
        }
        system->waterObstacleShape = clampedShape;
        ClampWaterCylinders(system);
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetWaterCylinderCount(ParticleSystem3D *system, int count)
    {
        InitializeDefaultWaterCylinders(system);
        const int oldCount = system->waterCylinderCount;
        system->waterCylinderCount = ClampInt(count, 1, WATER_CYLINDER_MAX);
        for (int i = oldCount; i < system->waterCylinderCount; ++i) {
            if (system->waterObstacleShape == UI_3D_WATER_OBSTACLE_RECTANGLES) {
                SetDefaultWaterRectangleSlot(system, i, system->waterCylinderCount);
            } else {
                SetDefaultWaterCylinderSlot(system, i, system->waterCylinderCount);
            }
        }
        ClampWaterCylinders(system);
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetWaterCylinderSelected(ParticleSystem3D *system, int selected)
    {
        InitializeDefaultWaterCylinders(system);
        system->waterCylinderSelected = ClampInt(selected, 0, system->waterCylinderCount - 1);
    }

    static void SetSelectedWaterCylinderPosition(ParticleSystem3D *system, int axis, float value)
    {
        InitializeDefaultWaterCylinders(system);
        ClampWaterCylinders(system);
        const int index = ClampInt(system->waterCylinderSelected, 0, system->waterCylinderCount - 1);
        if (axis == 0) {
            system->waterCylinderX[index] = value;
        } else if (axis == 1) {
            system->waterCylinderY[index] = value;
        } else {
            system->waterCylinderZ[index] = value;
        }
        ClampWaterCylinders(system);
        InvalidateGpuBackendState(system);
    }

    static void SetSelectedWaterCylinderRadius(ParticleSystem3D *system, float radius)
    {
        InitializeDefaultWaterCylinders(system);
        ClampWaterCylinders(system);
        const int index = ClampInt(system->waterCylinderSelected, 0, system->waterCylinderCount - 1);
        system->waterCylinderRadius[index] = radius;
        ClampWaterCylinders(system);
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetSelectedWaterCylinderDepth(ParticleSystem3D *system, float depth)
    {
        InitializeDefaultWaterCylinders(system);
        ClampWaterCylinders(system);
        const int index = ClampInt(system->waterCylinderSelected, 0, system->waterCylinderCount - 1);
        system->waterCylinderDepth[index] = depth;
        ClampWaterCylinders(system);
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetSelectedWaterRectangleWidth(ParticleSystem3D *system, float width)
    {
        InitializeDefaultWaterCylinders(system);
        ClampWaterCylinders(system);
        const int index = ClampInt(system->waterCylinderSelected, 0, system->waterCylinderCount - 1);
        system->waterRectangleWidth[index] = width;
        ClampWaterCylinders(system);
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetSelectedWaterRectangleHeight(ParticleSystem3D *system, float height)
    {
        InitializeDefaultWaterCylinders(system);
        ClampWaterCylinders(system);
        const int index = ClampInt(system->waterCylinderSelected, 0, system->waterCylinderCount - 1);
        system->waterRectangleHeight[index] = height;
        ClampWaterCylinders(system);
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetSelectedWaterRectangleAngle(ParticleSystem3D *system, float angleDegrees)
    {
        InitializeDefaultWaterCylinders(system);
        ClampWaterCylinders(system);
        const int index = ClampInt(system->waterCylinderSelected, 0, system->waterCylinderCount - 1);
        system->waterRectangleAngleDegrees[index] = ClampFloat(angleDegrees, -180.0f, 180.0f);
        ClampWaterCylinders(system);
        InvalidateGpuBackendState(system);
    }

    static void CycleObstacleModel(ParticleSystem3D *system)
    {
        SetObstacleModel(system, (ObstacleModel)((system->obstacleModel + 1) % OBSTACLE_MODEL_COUNT));
    }

    static void SetSimulationBackend(ParticleSystem3D *system, SimulationBackend backend)
    {
        if (backend == SIM_BACKEND_GPU && !system->gpuBackendAvailable) {
            SetBackendNotice(system, "GPU solver is not available in this build. Still using CPU.");
            system->activeBackend = SIM_BACKEND_CPU;
            return;
        }

        if (system->activeBackend != backend) {
            system->activeBackend = backend;
            const int clampedTarget = ClampInt(system->targetParticleCount,
                BackendMinTargetParticleCount(backend),
                BackendMaxTargetParticleCount(system, backend));
            const bool targetChanged = (clampedTarget != system->targetParticleCount);
            system->targetParticleCount = clampedTarget;
            if (targetChanged) {
                ResetSimulation(system, system->preset);
            }
        }
    }

    static void ToggleSimulationBackend(ParticleSystem3D *system)
    {
        const SimulationBackend nextBackend = (system->activeBackend == SIM_BACKEND_CPU)
            ? SIM_BACKEND_GPU
            : SIM_BACKEND_CPU;
        if (nextBackend == SIM_BACKEND_GPU) {
            (void)InitializeGpuBackend(system);
        }
        SetSimulationBackend(system, nextBackend);
    }

    static void SetTargetParticleCount(ParticleSystem3D *system, int targetParticleCount)
    {
        const int clampedTarget = ClampInt(targetParticleCount,
            BackendMinTargetParticleCount(system->activeBackend),
            BackendMaxTargetParticleCount(system, system->activeBackend));
        if (system->targetParticleCount == clampedTarget) {
            return;
        }

        system->targetParticleCount = clampedTarget;
        ResetSimulation(system, system->preset);
    }

    static void SetWindSpeedScale(ParticleSystem3D *system, float flowSpeedScale)
    {
        const float clamped = ClampFloat(flowSpeedScale, 0.35f, 2.50f);
        if (fabsf(system->flowSpeedScale - clamped) < 1e-4f) {
            return;
        }

        system->flowSpeedScale = clamped;
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetObstacleAngleDegrees(ParticleSystem3D *system, float obstacleAngleDegrees)
    {
        const float clamped = ClampFloat(obstacleAngleDegrees, -180.0f, 180.0f);
        if (fabsf(system->obstacleAngleDegrees - clamped) < 1e-4f) {
            return;
        }

        system->obstacleAngleDegrees = clamped;
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetObstacleRectangleWidth(ParticleSystem3D *system, float obstacleRectWidth)
    {
        const float clamped = ClampFloat(obstacleRectWidth, RECT_WIDTH_MIN_3D, RECT_WIDTH_MAX_3D);
        if (fabsf(system->obstacleRectWidth - clamped) < 1e-4f) {
            return;
        }

        system->obstacleRectWidth = clamped;
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetObstacleRectangleHeight(ParticleSystem3D *system, float obstacleRectHeight)
    {
        const float clamped = ClampFloat(obstacleRectHeight, RECT_HEIGHT_MIN_3D, RECT_HEIGHT_MAX_3D);
        if (fabsf(system->obstacleRectHeight - clamped) < 1e-4f) {
            return;
        }

        system->obstacleRectHeight = clamped;
        ConfigureSceneParameters(system);
        InvalidateGpuBackendState(system);
    }

    static void SetSimulationScene(ParticleSystem3D *system, SimulationScene scene)
    {
        if (system->scene == scene) {
            return;
        }

        system->scene = scene;
        if (scene == SCENE_WIND_TUNNEL) {
            ResetSimulation(system, MATERIAL_GAS);
        } else {
            ResetSimulation(system, system->tankPreset);
        }
    }

    static void ToggleSimulationScene(ParticleSystem3D *system)
    {
        SetSimulationScene(system, (system->scene == SCENE_TANK) ? SCENE_WIND_TUNNEL : SCENE_TANK);
    }

    static float SampleDensityAtSortedSlot(const ParticleSystem3D *system, int slot, float sampleMass)
    {
        const float xi = system->sortedX[slot];
        const float yi = system->sortedY[slot];
        const float zi = system->sortedZ[slot];
        const int baseCell = system->sortedCellIndices[slot];
        const int neighborBase = baseCell * 27;
        const int neighborCount = system->cellNeighborCounts[baseCell];
        float density = 0.0f;

        for (int neighbor = 0; neighbor < neighborCount; ++neighbor) {
            const int cellIndex = system->cellNeighbors[neighborBase + neighbor];
            const int start = system->cellStarts[cellIndex];
            const int end = system->cellStarts[cellIndex + 1];

            for (int cursor = start; cursor < end; ++cursor) {
                const float dx = xi - system->sortedX[cursor];
                const float dy = yi - system->sortedY[cursor];
                const float dz = zi - system->sortedZ[cursor];
                const float r2 = dx * dx + dy * dy + dz * dz;

                if (r2 < system->supportRadiusSquared) {
                    const float diff = system->supportRadiusSquared - r2;
                    density += sampleMass * system->densityKernel * diff * diff * diff;
                }
            }
        }

        AddBoundaryDensityContribution(system, xi, yi, zi, &density);
        return density;
    }

    static void CalibrateParticleMass(ParticleSystem3D *system)
    {
        if (system->preset == MATERIAL_GAS || system->particleCount <= 0) {
            return;
        }
        if (system->bakeCapacityMode && system->particleCount > 200000) {
            return;
        }

        double densitySum = 0.0;
        for (int slot = 0; slot < system->particleCount; ++slot) {
            densitySum += (double)SampleDensityAtSortedSlot(system, slot, 1.0f);
        }

        const float averageDensity = (float)(densitySum / (double)system->particleCount);
        if (averageDensity > 1e-6f) {
            system->mass = system->params.restDensity / averageDensity;
        }
    }

    static void ResetSimulation(ParticleSystem3D *system, MaterialPreset preset)
    {
        InvalidateGpuBackendState(system);
        if (preset != MATERIAL_GAS || !SceneIsWindTunnel(system)) {
            system->tankPreset = preset;
        }
        if (SceneIsWindTunnel(system)) {
            preset = MATERIAL_GAS;
        }

        system->preset = preset;
        system->params = MATERIAL_PRESETS[preset];
        system->worldWidth = 150.0f;
        system->worldHeight = 82.0f;
        system->worldDepth = 88.0f;
        system->boundsMin = (Vector3){-75.0f, 0.0f, -44.0f};
        system->boundsMax = (Vector3){75.0f, 82.0f, 44.0f};
        system->boundsSize = Vector3Subtract(system->boundsMax, system->boundsMin);
        system->boundsCenter = Vector3Scale(Vector3Add(system->boundsMin, system->boundsMax), 0.5f);
        InitializeDefaultWaterCylinders(system);
        system->paused = false;
        system->accumulator = 0.0f;
        system->interactionActive = false;
        system->simulationTime = 0.0f;
        system->lastStepDt = system->params.timeStep;
        system->framesUntilDiagnostics = 0;
        system->framesUntilVorticity = 0;
        system->scalarFieldsDirty = true;
        system->sortedStateDirty = true;
        system->vorticityDirty = true;
        system->playbackPreviewMode = false;
        system->playbackPreviewSourceCount = 0;

        float fillWidthRatio = 0.48f;
        float fillHeightRatio = 0.68f;
        float fillDepthRatio = 0.52f;
        Vector3 origin = {
            system->boundsMin.x + system->boundsSize.x * 0.07f,
            system->boundsMin.y + system->boundsSize.y * 0.06f,
            system->boundsMin.z + system->boundsSize.z * 0.13f,
        };

        if (SceneIsWindTunnel(system)) {
            fillWidthRatio = 0.995f;
            fillHeightRatio = 0.82f;
            fillDepthRatio = 0.96f;
            origin = (Vector3){
                system->boundsMin.x + system->boundsSize.x * 0.002f,
                system->boundsMin.y + system->boundsSize.y * 0.09f,
                system->boundsMin.z + system->boundsSize.z * 0.02f,
            };
        } else if (preset == MATERIAL_GAS) {
            fillWidthRatio = 0.98f;
            fillHeightRatio = 0.96f;
            fillDepthRatio = 0.96f;
            origin = (Vector3){
                system->boundsMin.x + system->boundsSize.x * 0.01f,
                system->boundsMin.y + system->boundsSize.y * 0.02f,
                system->boundsMin.z + system->boundsSize.z * 0.02f,
            };
        }

        const Vector3 fillSize = {
            system->boundsSize.x * fillWidthRatio,
            system->boundsSize.y * fillHeightRatio,
            system->boundsSize.z * fillDepthRatio,
        };
        const int targetParticles = EffectiveTargetParticleCount(system);
        const float targetSpacing = cbrtf((fillSize.x * fillSize.y * fillSize.z) / (float)targetParticles);
        const float minResolutionScale = system->bakeCapacityMode ? 0.06f :
            ((system->activeBackend == SIM_BACKEND_GPU) ? 0.12f : 0.30f);
        const float resolutionScale = ClampFloat((targetSpacing * 0.96f) / system->params.spacing, minResolutionScale, 1.75f);

        system->params.spacing *= resolutionScale;
        system->params.supportRadius *= resolutionScale;
        const float minParticleRadius = system->bakeCapacityMode ? 0.18f :
            ((system->activeBackend == SIM_BACKEND_GPU) ? 0.28f : 0.60f);
        system->params.particleRadius = fmaxf(minParticleRadius,
            system->params.particleRadius * resolutionScale);
        const float minTimeStep = system->bakeCapacityMode ? (1.0f / 1440.0f) :
            ((system->activeBackend == SIM_BACKEND_GPU) ? (1.0f / 960.0f) : (1.0f / 720.0f));
        const float minTimeScale = system->bakeCapacityMode ? 0.18f :
            ((system->activeBackend == SIM_BACKEND_GPU) ? 0.22f : 0.45f);
        system->params.timeStep = fmaxf(minTimeStep, system->params.timeStep * fmaxf(resolutionScale, minTimeScale));
        system->baseSoundSpeed = system->params.soundSpeed;
        system->baseKinematicViscosity = system->params.kinematicViscosity;
        system->baseGlobalDrag = system->params.globalDrag;

        ClampWaterCylinders(system);
        RefreshDerivedSimulationParameters(system);
        if (!system->acousticHandlesInitialized) {
            system->speakerBaseCenter = (Vector3){
                system->boundsMin.x + system->boundsSize.x * 0.18f,
                system->boundsMin.y + system->boundsSize.y * 0.50f,
                system->boundsCenter.z,
            };
            system->micPosition = (Vector3){
                system->boundsMin.x + system->boundsSize.x * 0.78f,
                system->boundsMin.y + system->boundsSize.y * 0.50f,
                system->boundsCenter.z,
            };
            system->acousticHandlesInitialized = true;
        }
        ClampAcousticAnchors(system);
        ResetMicrophoneHistory(system);
        RebuildGrid(system);

        const float spacing = system->params.spacing;
        const int countX = (int)fmaxf(1.0f, floorf(fillSize.x / spacing));
        const int countY = (int)fmaxf(1.0f, floorf(fillSize.y / spacing));
        const int countZ = (int)fmaxf(1.0f, floorf(fillSize.z / spacing));
        const float blockHeight = fmaxf(1.0f, (float)(countY - 1) * spacing);

        system->particleCount = 0;
        for (int y = 0; y < countY && system->particleCount < targetParticles; ++y) {
            for (int z = 0; z < countZ && system->particleCount < targetParticles; ++z) {
                for (int x = 0; x < countX && system->particleCount < targetParticles; ++x) {
                    const int seedIndex = (y * countZ + z) * countX + x;
                    const float jitterScale = (preset == MATERIAL_GAS) ? 0.16f : 0.08f;
                    const float particleX = origin.x + (float)x * spacing +
                        (HashNoise(seedIndex * 3 + 11) - 0.5f) * spacing * jitterScale;
                    const float particleY = origin.y + (float)y * spacing +
                        (HashNoise(seedIndex * 3 + 23) - 0.5f) * spacing * jitterScale;
                    const float particleZ = origin.z + (float)z * spacing +
                        (HashNoise(seedIndex * 3 + 47) - 0.5f) * spacing * jitterScale;

                    if (SolidObstacleActive(system) &&
                        SolidObstacleSignedDistance(system, particleX, particleY, particleZ) < system->params.particleRadius * 1.2f) {
                        continue;
                    }
                    if (AcousticsActive(system) &&
                        SpeakerSignedDistance(system, particleX, particleY, particleZ) < system->params.particleRadius * 1.2f) {
                        continue;
                    }

                    const int index = system->particleCount++;
                    const float verticalMix = RangeLerp(origin.y, origin.y + blockHeight, particleY);
                    const float randomVelX = HashNoise(index * 5 + 101) - 0.5f;
                    const float randomVelY = HashNoise(index * 5 + 211) - 0.5f;
                    const float randomVelZ = HashNoise(index * 5 + 307) - 0.5f;

                    system->x[index] = particleX;
                    system->y[index] = particleY;
                    system->z[index] = particleZ;
                    if (SceneIsWindTunnel(system)) {
                        const float baseFlow = system->flowTargetSpeed * WindTunnelProfile(system, particleY, particleZ);
                        system->vx[index] = baseFlow + randomVelX * system->params.soundSpeed * 0.035f;
                        system->vy[index] = randomVelY * system->params.soundSpeed * 0.025f;
                        system->vz[index] = randomVelZ * system->params.soundSpeed * 0.025f;
                    } else {
                        system->vx[index] = (preset == MATERIAL_GAS) ? randomVelX * system->params.soundSpeed * 0.10f : 0.0f;
                        system->vy[index] = (preset == MATERIAL_GAS) ? randomVelY * system->params.soundSpeed * 0.10f : 0.0f;
                        system->vz[index] = (preset == MATERIAL_GAS) ? randomVelZ * system->params.soundSpeed * 0.10f : 0.0f;
                    }
                    system->ax[index] = 0.0f;
                    system->ay[index] = 0.0f;
                    system->az[index] = 0.0f;
                    system->density[index] = system->params.restDensity;
                    system->pressure[index] = 0.0f;
                    system->temperature[index] = ClampFloat(
                        system->params.ambientTemperature +
                            system->params.initialTemperatureGradient * (1.0f - verticalMix) +
                            (HashNoise(index * 7 + 17) - 0.5f) * 0.03f,
                        TEMP_MIN, TEMP_MAX);
                    system->temperatureRate[index] = 0.0f;
                    system->dye[index] = TracerDyeValueForPoint(system, particleX, particleY, particleZ, index);
                    system->vorticity[index] = 0.0f;
                    system->particleCells[index] = 0;
                }
            }
        }

        BuildGrid(system);
        CalibrateParticleMass(system);
        BuildGrid(system);
        ResetPathlines(system);
        system->sortedStateDirty = false;
        system->vorticityDirty = true;
    }

    static void BuildGrid(ParticleSystem3D *system)
    {
        memset(system->cellCounts, 0, (size_t)system->cellCount * sizeof(int));

        for (int i = 0; i < system->particleCount; ++i) {
            const int cellX = ClampInt((int)((system->x[i] - system->boundsMin.x) * system->invCellSize), 0, system->gridWidth - 1);
            const int cellY = ClampInt((int)((system->y[i] - system->boundsMin.y) * system->invCellSize), 0, system->gridHeight - 1);
            const int cellZ = ClampInt((int)((system->z[i] - system->boundsMin.z) * system->invCellSize), 0, system->gridDepth - 1);
            const int cellIndex = (cellZ * system->gridHeight + cellY) * system->gridWidth + cellX;
            system->particleCells[i] = cellIndex;
            system->cellCounts[cellIndex] += 1;
        }

        system->cellStarts[0] = 0;
        for (int cell = 0; cell < system->cellCount; ++cell) {
            system->cellStarts[cell + 1] = system->cellStarts[cell] + system->cellCounts[cell];
            system->cellOffsets[cell] = system->cellStarts[cell];
        }

        for (int i = 0; i < system->particleCount; ++i) {
            const int cellIndex = system->particleCells[i];
            system->sortedIndices[system->cellOffsets[cellIndex]++] = i;
        }

        for (int slot = 0; slot < system->particleCount; ++slot) {
            const int particleIndex = system->sortedIndices[slot];
            system->sortedX[slot] = system->x[particleIndex];
            system->sortedY[slot] = system->y[particleIndex];
            system->sortedZ[slot] = system->z[particleIndex];
            system->sortedVX[slot] = system->vx[particleIndex];
            system->sortedVY[slot] = system->vy[particleIndex];
            system->sortedVZ[slot] = system->vz[particleIndex];
            system->sortedTemperature[slot] = system->temperature[particleIndex];
            system->sortedCellIndices[slot] = system->particleCells[particleIndex];
        }

        system->sortedStateDirty = false;
        system->vorticityDirty = true;
    }

    static void ComputeDensityAndPressureParticle(ParticleSystem3D *system, int slot)
    {
        const int particleIndex = system->sortedIndices[slot];
        const float density = SampleDensityAtSortedSlot(system, slot, system->mass);
        const float clampedDensity = fmaxf(DensityFloor(system), density);

        system->density[particleIndex] = clampedDensity;
        system->pressure[particleIndex] = PressureFromState(system, clampedDensity, system->sortedTemperature[slot]);
        system->sortedDensity[slot] = clampedDensity;
        system->sortedPressure[slot] = system->pressure[particleIndex];
    }

    static void ComputeDensityAndPressure(ParticleSystem3D *system)
    {
        if (ShouldParallelize(system->particleCount)) {
            dispatch_apply((size_t)system->particleCount,
                dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
                ^(size_t index) {
                    ComputeDensityAndPressureParticle(system, (int)index);
                });
        } else {
            for (int i = 0; i < system->particleCount; ++i) {
                ComputeDensityAndPressureParticle(system, i);
            }
        }
    }

    static void ComputeVorticityParticle(ParticleSystem3D *system, int slot)
    {
        const float xi = system->sortedX[slot];
        const float yi = system->sortedY[slot];
        const float zi = system->sortedZ[slot];
        const float vxi = system->sortedVX[slot];
        const float vyi = system->sortedVY[slot];
        const float vzi = system->sortedVZ[slot];
        const int particleIndex = system->sortedIndices[slot];
        const int baseCell = system->sortedCellIndices[slot];
        const int neighborBase = baseCell * 27;
        const int neighborCount = system->cellNeighborCounts[baseCell];
        const float h = system->params.supportRadius;
        const float h2 = system->supportRadiusSquared;
        Vector3 omega = {0.0f, 0.0f, 0.0f};

        for (int neighbor = 0; neighbor < neighborCount; ++neighbor) {
            const int cellIndex = system->cellNeighbors[neighborBase + neighbor];
            const int start = system->cellStarts[cellIndex];
            const int end = system->cellStarts[cellIndex + 1];

            for (int cursor = start; cursor < end; ++cursor) {
                if (cursor == slot) {
                    continue;
                }

                const float dx = xi - system->sortedX[cursor];
                const float dy = yi - system->sortedY[cursor];
                const float dz = zi - system->sortedZ[cursor];
                const float r2 = dx * dx + dy * dy + dz * dz;
                if (r2 <= 1e-8f || r2 >= h2) {
                    continue;
                }

                const float r = sqrtf(r2);
                const float influence = h - r;
                const float invR = 1.0f / r;
                const float neighborDensity = fmaxf(system->density[system->sortedIndices[cursor]], 1e-4f);
                const float gradScale = -system->pressureKernelGrad * influence * influence * invR;
                const Vector3 gradW = {
                    gradScale * dx,
                    gradScale * dy,
                    gradScale * dz,
                };
                const Vector3 deltaV = {
                    system->sortedVX[cursor] - vxi,
                    system->sortedVY[cursor] - vyi,
                    system->sortedVZ[cursor] - vzi,
                };
                const Vector3 curl = Vector3CrossProduct(deltaV, gradW);
                omega.x += system->mass * curl.x / neighborDensity;
                omega.y += system->mass * curl.y / neighborDensity;
                omega.z += system->mass * curl.z / neighborDensity;
            }
        }

        system->vorticity[particleIndex] = Vector3Length(omega);
    }

    static void ComputeVorticityField(ParticleSystem3D *system)
    {
        if (system->particleCount <= 0) {
            system->stats.minVorticity = 0.0f;
            system->stats.maxVorticity = 0.0f;
            system->stats.avgVorticity = 0.0f;
            system->vorticityDirty = false;
            system->framesUntilVorticity = VorticityRefreshIntervalForBackend(system->activeBackend);
            return;
        }

        if (ShouldParallelize(system->particleCount)) {
            dispatch_apply((size_t)system->particleCount,
                dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
                ^(size_t index) {
                    ComputeVorticityParticle(system, (int)index);
                });
        } else {
            for (int i = 0; i < system->particleCount; ++i) {
                ComputeVorticityParticle(system, i);
            }
        }

        float minVorticity = FLT_MAX;
        float maxVorticity = 0.0f;
        float sumVorticity = 0.0f;
        int scaleSampleCount = 0;
        for (int i = 0; i < system->particleCount; ++i) {
            const float value = system->vorticity[i];
            if (UseParticleForWindVorticityScale(system, i)) {
                minVorticity = fminf(minVorticity, value);
                maxVorticity = fmaxf(maxVorticity, value);
                scaleSampleCount += 1;
            }
            sumVorticity += value;
        }
        if (scaleSampleCount == 0) {
            for (int i = 0; i < system->particleCount; ++i) {
                const float value = system->vorticity[i];
                minVorticity = fminf(minVorticity, value);
                maxVorticity = fmaxf(maxVorticity, value);
            }
        }
        system->stats.minVorticity = (minVorticity == FLT_MAX) ? 0.0f : minVorticity;
        system->stats.maxVorticity = maxVorticity;
        system->stats.avgVorticity = sumVorticity / (float)system->particleCount;
        system->vorticityDirty = false;
        system->framesUntilVorticity = VorticityRefreshIntervalForBackend(system->activeBackend);
    }

    static Vector3 SampleVelocityAtPoint(const ParticleSystem3D *system, Vector3 point)
    {
        if (system->sortedStateDirty || system->particleCount <= 0) {
            return (Vector3){0.0f, 0.0f, 0.0f};
        }

        const int cellX = ClampInt((int)((point.x - system->boundsMin.x) * system->invCellSize), 0, system->gridWidth - 1);
        const int cellY = ClampInt((int)((point.y - system->boundsMin.y) * system->invCellSize), 0, system->gridHeight - 1);
        const int cellZ = ClampInt((int)((point.z - system->boundsMin.z) * system->invCellSize), 0, system->gridDepth - 1);
        const int baseCell = (cellZ * system->gridHeight + cellY) * system->gridWidth + cellX;
        const int neighborBase = baseCell * 27;
        const int neighborCount = system->cellNeighborCounts[baseCell];
        const float h2 = system->supportRadiusSquared;
        Vector3 velocity = {0.0f, 0.0f, 0.0f};
        float totalWeight = 0.0f;

        for (int neighbor = 0; neighbor < neighborCount; ++neighbor) {
            const int cellIndex = system->cellNeighbors[neighborBase + neighbor];
            const int start = system->cellStarts[cellIndex];
            const int end = system->cellStarts[cellIndex + 1];
            for (int cursor = start; cursor < end; ++cursor) {
                const float dx = point.x - system->sortedX[cursor];
                const float dy = point.y - system->sortedY[cursor];
                const float dz = point.z - system->sortedZ[cursor];
                const float r2 = dx * dx + dy * dy + dz * dz;
                if (r2 >= h2) {
                    continue;
                }

                const float q = 1.0f - r2 / h2;
                const float weight = q * q * q;
                velocity.x += system->sortedVX[cursor] * weight;
                velocity.y += system->sortedVY[cursor] * weight;
                velocity.z += system->sortedVZ[cursor] * weight;
                totalWeight += weight;
            }
        }

        if (totalWeight > 1e-5f) {
            return Vector3Scale(velocity, 1.0f / totalWeight);
        }

        if (SceneIsWindTunnel(system)) {
            const float profile = WindTunnelProfile(system, point.y, point.z);
            return (Vector3){system->flowTargetSpeed * profile, 0.0f, 0.0f};
        }
        return (Vector3){0.0f, 0.0f, 0.0f};
    }

    static void RefreshTracerDye(ParticleSystem3D *system)
    {
        if (system->colorMode != COLOR_DYE) {
            return;
        }

        if (SceneIsWindTunnel(system)) {
            const float reseedX = system->boundsMin.x + system->params.supportRadius * 2.6f;
            for (int i = 0; i < system->particleCount; ++i) {
                if (system->x[i] <= reseedX) {
                    system->dye[i] = TracerDyeValueForPoint(system, system->x[i], system->y[i], system->z[i], i);
                }
            }
            return;
        }

        for (int i = 0; i < system->particleCount; ++i) {
            system->dye[i] = ClampFloat(system->dye[i], 0.0f, 1.0f);
        }
    }

    static void RespawnPathline(ParticleSystem3D *system, int index)
    {
        const Vector3 seed = SeedFlowLinePosition(system, index, system->pathlineDensity);
        system->pathlines[index].current = seed;
        system->pathlines[index].count = 1;
        system->pathlines[index].trail[0] = seed;
    }

    static void AppendPathlinePoint(FlowPathline *pathline, Vector3 point, int trailLength)
    {
        const int clampedLength = ClampInt(trailLength, FLOW_PATHLINE_TRAIL_MIN, FLOW_PATHLINE_TRAIL_MAX);
        if (pathline->count < clampedLength) {
            pathline->trail[pathline->count++] = point;
            return;
        }

        memmove(&pathline->trail[0], &pathline->trail[1], (size_t)(clampedLength - 1) * sizeof(Vector3));
        pathline->trail[clampedLength - 1] = point;
    }

    static void UpdatePathlines(ParticleSystem3D *system, float frameDelta)
    {
        if (!system->showPathlines || !SceneIsWindTunnel(system)) {
            return;
        }

        if (system->sortedStateDirty) {
            BuildGrid(system);
        }

        system->pathlineAccumulator += frameDelta;
        const float captureStep = 1.0f / 36.0f;
        const float advectDt = ClampFloat(frameDelta, 1.0f / 240.0f, 1.0f / 30.0f);

        const int activeCount = ActivePathlineCount(system);
        for (int i = 0; i < activeCount; ++i) {
            FlowPathline *pathline = &system->pathlines[i];
            Vector3 velocity = SampleVelocityAtPoint(system, pathline->current);
            const float speed = Vector3Length(velocity);
            if (speed < 1e-4f) {
                RespawnPathline(system, i);
                continue;
            }

            pathline->current = Vector3Add(pathline->current, Vector3Scale(velocity, advectDt));
            const bool outsideBounds =
                pathline->current.x > system->boundsMax.x - system->params.particleRadius ||
                pathline->current.y < system->boundsMin.y || pathline->current.y > system->boundsMax.y ||
                pathline->current.z < system->boundsMin.z || pathline->current.z > system->boundsMax.z;
            if (outsideBounds || (SolidObstacleActive(system) &&
                    SolidObstacleSignedDistance(system, pathline->current.x, pathline->current.y, pathline->current.z) < 0.0f)) {
                RespawnPathline(system, i);
                continue;
            }

            if (system->pathlineAccumulator >= captureStep) {
                AppendPathlinePoint(pathline, pathline->current, system->pathlineTrailLength);
            }
        }

        if (system->pathlineAccumulator >= captureStep) {
            system->pathlineAccumulator = fmodf(system->pathlineAccumulator, captureStep);
        }
    }

    static void RefreshVisualizationState(ParticleSystem3D *system, float frameDelta)
    {
        if (VisualizationNeedsGrid(system) && system->sortedStateDirty) {
            BuildGrid(system);
        }

        if (ColorModeNeedsVorticity(system->colorMode)) {
            const bool shouldRefreshVorticity = system->vorticityDirty &&
                (system->framesUntilVorticity <= 0 || system->paused);
            if (shouldRefreshVorticity && system->sortedStateDirty) {
                BuildGrid(system);
            }
            if (shouldRefreshVorticity) {
                ComputeVorticityField(system);
            } else if (system->framesUntilVorticity > 0) {
                system->framesUntilVorticity -= 1;
            }
        }

        RefreshTracerDye(system);
        if (!system->paused) {
            UpdatePathlines(system, frameDelta);
        }
    }

    static void ComputeForceParticle(ParticleSystem3D *system, int slot)
    {
        const float h = system->params.supportRadius;
        const float h2 = system->supportRadiusSquared;
        const float xi = system->sortedX[slot];
        const float yi = system->sortedY[slot];
        const float zi = system->sortedZ[slot];
        const float vxi = system->sortedVX[slot];
        const float vyi = system->sortedVY[slot];
        const float vzi = system->sortedVZ[slot];
        const float rhoi = system->sortedDensity[slot];
        const float pressureI = system->sortedPressure[slot];
        const float temperatureI = system->sortedTemperature[slot];
        const int particleIndex = system->sortedIndices[slot];
        const int baseCell = system->sortedCellIndices[slot];
        const int neighborBase = baseCell * 27;
        const int neighborCount = system->cellNeighborCounts[baseCell];

        float ax = -system->params.globalDrag * vxi;
        float ay = -system->params.gravity +
            system->params.buoyancy * (temperatureI - system->params.ambientTemperature) -
            system->params.globalDrag * vyi;
        float az = -system->params.globalDrag * vzi;
        float temperatureRate = 0.0f;
        float xsphVX = 0.0f;
        float xsphVY = 0.0f;
        float xsphVZ = 0.0f;

        for (int neighbor = 0; neighbor < neighborCount; ++neighbor) {
            const int cellIndex = system->cellNeighbors[neighborBase + neighbor];
            const int start = system->cellStarts[cellIndex];
            const int end = system->cellStarts[cellIndex + 1];

            for (int cursor = start; cursor < end; ++cursor) {
                if (cursor == slot) {
                    continue;
                }

                const float dx = xi - system->sortedX[cursor];
                const float dy = yi - system->sortedY[cursor];
                const float dz = zi - system->sortedZ[cursor];
                const float r2 = dx * dx + dy * dy + dz * dz;
                if (r2 <= 1e-8f || r2 >= h2) {
                    continue;
                }

                const float r = sqrtf(r2);
                const float influence = h - r;
                const float invR = 1.0f / r;
                const float neighborDensity = system->sortedDensity[cursor];
                const float pressureTerm = -system->mass *
                    ((pressureI / (rhoi * rhoi)) +
                    (system->sortedPressure[cursor] / (neighborDensity * neighborDensity))) *
                    system->pressureKernelGrad * influence * influence;
                const float viscosityTerm = system->params.kinematicViscosity * system->mass *
                    system->viscosityKernelLap * influence / neighborDensity;
                const float densityKernelValue = system->densityKernel * (h2 - r2) * (h2 - r2) * (h2 - r2);

                ax += pressureTerm * dx * invR;
                ay += pressureTerm * dy * invR;
                az += pressureTerm * dz * invR;
                ax += viscosityTerm * (system->sortedVX[cursor] - vxi);
                ay += viscosityTerm * (system->sortedVY[cursor] - vyi);
                az += viscosityTerm * (system->sortedVZ[cursor] - vzi);

                temperatureRate += system->params.temperatureDiffusion * system->mass *
                    (system->sortedTemperature[cursor] - temperatureI) *
                    system->viscosityKernelLap * influence / (rhoi * neighborDensity);

                xsphVX += system->mass * (system->sortedVX[cursor] - vxi) * densityKernelValue / neighborDensity;
                xsphVY += system->mass * (system->sortedVY[cursor] - vyi) * densityKernelValue / neighborDensity;
                xsphVZ += system->mass * (system->sortedVZ[cursor] - vzi) * densityKernelValue / neighborDensity;
            }
        }

        AddBoundaryForceContribution(system, xi, yi, zi, vxi, vyi, vzi, rhoi, pressureI, temperatureI, &ax, &ay, &az);
        AddSceneForceContribution(system, xi, yi, zi, vxi, vyi, vzi, &ax, &ay, &az);
        AddSpeakerForceContribution(system, xi, yi, zi, vxi, vyi, vzi, &ax, &ay, &az);

        if (system->interactionActive) {
            const float dx = xi - system->interactionPoint.x;
            const float dy = yi - system->interactionPoint.y;
            const float dz = zi - system->interactionPoint.z;
            const float r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > 1e-8f && r2 < system->interactionRadiusSquared) {
                const float r = sqrtf(r2);
                const float falloff = 1.0f - (r / system->interactionRadius);
                const float impulse = system->interactionStrength * falloff * falloff;
                const float invR = 1.0f / r;
                ax += dx * invR * impulse;
                ay += dy * invR * impulse;
                az += dz * invR * impulse;
            }
        }

        system->ax[particleIndex] = ax;
        system->ay[particleIndex] = ay;
        system->az[particleIndex] = az;
        system->temperatureRate[particleIndex] = temperatureRate;
        system->xsphVX[particleIndex] = xsphVX;
        system->xsphVY[particleIndex] = xsphVY;
        system->xsphVZ[particleIndex] = xsphVZ;
    }

    static void ComputeForces(ParticleSystem3D *system)
    {
        if (ShouldParallelize(system->particleCount)) {
            dispatch_apply((size_t)system->particleCount,
                dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
                ^(size_t index) {
                    ComputeForceParticle(system, (int)index);
                });
        } else {
            for (int i = 0; i < system->particleCount; ++i) {
                ComputeForceParticle(system, i);
            }
        }
    }

    static void ResolveBounds(ParticleSystem3D *system, int index)
    {
        const float radius = system->params.particleRadius;
        const float left = system->boundsMin.x + radius;
        const float right = system->boundsMax.x - radius;
        const float bottom = system->boundsMin.y + radius;
        const float top = system->boundsMax.y - radius;
        const float front = system->boundsMin.z + radius;
        const float back = system->boundsMax.z - radius;

        if (SceneIsWindTunnel(system)) {
            if (system->x[index] < left || system->x[index] > right) {
                RespawnWindTunnelParticle(system, index);
                return;
            }
        } else if (system->x[index] < left) {
            system->x[index] = left;
            system->vx[index] = fabsf(system->vx[index]) * system->params.wallBounce;
            system->vy[index] *= system->params.wallFriction;
            system->vz[index] *= system->params.wallFriction;
        } else if (system->x[index] > right) {
            system->x[index] = right;
            system->vx[index] = -fabsf(system->vx[index]) * system->params.wallBounce;
            system->vy[index] *= system->params.wallFriction;
            system->vz[index] *= system->params.wallFriction;
        }

        if (system->y[index] < bottom) {
            system->y[index] = bottom;
            system->vy[index] = fabsf(system->vy[index]) * system->params.wallBounce;
            system->vx[index] *= system->params.wallFriction;
            system->vz[index] *= system->params.wallFriction;
        } else if (system->y[index] > top) {
            system->y[index] = top;
            system->vy[index] = -fabsf(system->vy[index]) * system->params.wallBounce;
            system->vx[index] *= system->params.wallFriction;
            system->vz[index] *= system->params.wallFriction;
        }

        if (system->z[index] < front) {
            system->z[index] = front;
            system->vz[index] = fabsf(system->vz[index]) * system->params.wallBounce;
            system->vx[index] *= system->params.wallFriction;
            system->vy[index] *= system->params.wallFriction;
        } else if (system->z[index] > back) {
            system->z[index] = back;
            system->vz[index] = -fabsf(system->vz[index]) * system->params.wallBounce;
            system->vx[index] *= system->params.wallFriction;
            system->vy[index] *= system->params.wallFriction;
        }
    }

    static void IntegrateParticle(ParticleSystem3D *system, int i, float dt)
    {
        system->vx[i] += system->ax[i] * dt;
        system->vy[i] += system->ay[i] * dt;
        system->vz[i] += system->az[i] * dt;

        const float maxSpeed = (system->preset == MATERIAL_GAS)
            ? system->params.soundSpeed * 5.0f
            : system->params.soundSpeed * 2.5f;
        const float speed2 = system->vx[i] * system->vx[i] +
            system->vy[i] * system->vy[i] +
            system->vz[i] * system->vz[i];
        if (speed2 > maxSpeed * maxSpeed) {
            const float scale = maxSpeed / sqrtf(speed2);
            system->vx[i] *= scale;
            system->vy[i] *= scale;
            system->vz[i] *= scale;
        }

        const float advVx = system->vx[i] + XsphBlend(system) * system->xsphVX[i];
        const float advVy = system->vy[i] + XsphBlend(system) * system->xsphVY[i];
        const float advVz = system->vz[i] + XsphBlend(system) * system->xsphVZ[i];
        system->x[i] += advVx * dt;
        system->y[i] += advVy * dt;
        system->z[i] += advVz * dt;
        system->temperature[i] = ClampFloat(
            system->temperature[i] + system->temperatureRate[i] * dt,
            TEMP_MIN, TEMP_MAX);

        if (system->preset != MATERIAL_GAS) {
            system->vx[i] *= 0.9992f;
            system->vy[i] *= 0.9992f;
            system->vz[i] *= 0.9992f;
        }

        ResolveBounds(system, i);
        ResolveObstacle(system, i);
        ResolveSpeaker(system, i);
    }

    static float ComputeAdaptiveTimeStep(const ParticleSystem3D *system, float maxStep)
    {
        float maxSpeed = 0.0f;
        float maxAcceleration = 0.0f;

        for (int i = 0; i < system->particleCount; ++i) {
            const float speed = sqrtf(system->vx[i] * system->vx[i] + system->vy[i] * system->vy[i] + system->vz[i] * system->vz[i]);
            const float acceleration = sqrtf(system->ax[i] * system->ax[i] + system->ay[i] * system->ay[i] + system->az[i] * system->az[i]);
            maxSpeed = fmaxf(maxSpeed, speed);
            maxAcceleration = fmaxf(maxAcceleration, acceleration);
        }

        const float h = system->params.supportRadius;
        const float cflDt = 0.30f * h / fmaxf(system->params.soundSpeed + maxSpeed, 1e-4f);
        const float forceDt = 0.25f * sqrtf(h / fmaxf(maxAcceleration, 1e-4f));
        const float viscDt = 0.125f * h * h / fmaxf(system->params.kinematicViscosity, 1e-4f);
        float stepLimit = MinFloat3(maxStep, cflDt, fminf(forceDt, viscDt));
        float minDt = system->params.timeStep * 0.20f;

        if (AcousticsActive(system)) {
            const float speakerSpeed = SpeakerPeakSurfaceSpeed(system);
            const float speakerWaveDt = 0.20f * h / fmaxf(system->params.soundSpeed + speakerSpeed, 1e-4f);
            const float speakerPhaseDt = 1.0f / fmaxf(system->speakerFrequency * 24.0f, 1e-4f);
            stepLimit = fminf(stepLimit, fminf(speakerWaveDt, speakerPhaseDt));
            minDt = system->params.timeStep * 0.05f;
        }

        return ClampFloat(stepLimit, minDt, maxStep);
    }

    static void Integrate(ParticleSystem3D *system, float dt)
    {
        if (ShouldParallelize(system->particleCount)) {
            dispatch_apply((size_t)system->particleCount,
                dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
                ^(size_t index) {
                    IntegrateParticle(system, (int)index, dt);
                });
        } else {
            for (int i = 0; i < system->particleCount; ++i) {
                IntegrateParticle(system, i, dt);
            }
        }
    }

    static void UpdateDiagnostics(ParticleSystem3D *system)
    {
        Diagnostics diagnostics = {
            .minDensity = FLT_MAX,
            .maxDensity = -FLT_MAX,
            .minPressure = FLT_MAX,
            .maxPressure = -FLT_MAX,
            .minSpeed = FLT_MAX,
            .maxSpeed = -FLT_MAX,
            .minTemperature = FLT_MAX,
            .maxTemperature = -FLT_MAX,
            .avgDensity = 0.0f,
            .avgPressure = 0.0f,
            .avgSpeed = 0.0f,
            .avgTemperature = 0.0f,
            .minVorticity = FLT_MAX,
            .maxVorticity = -FLT_MAX,
            .avgVorticity = 0.0f,
        };

        int vorticityScaleSampleCount = 0;
        for (int i = 0; i < system->particleCount; ++i) {
            const float speed = sqrtf(system->vx[i] * system->vx[i] + system->vy[i] * system->vy[i] + system->vz[i] * system->vz[i]);
            const float vorticity = system->vorticity[i];
            diagnostics.minDensity = fminf(diagnostics.minDensity, system->density[i]);
            diagnostics.maxDensity = fmaxf(diagnostics.maxDensity, system->density[i]);
            diagnostics.minPressure = fminf(diagnostics.minPressure, system->pressure[i]);
            diagnostics.maxPressure = fmaxf(diagnostics.maxPressure, system->pressure[i]);
            diagnostics.minSpeed = fminf(diagnostics.minSpeed, speed);
            diagnostics.maxSpeed = fmaxf(diagnostics.maxSpeed, speed);
            diagnostics.minTemperature = fminf(diagnostics.minTemperature, system->temperature[i]);
            diagnostics.maxTemperature = fmaxf(diagnostics.maxTemperature, system->temperature[i]);
            if (UseParticleForWindVorticityScale(system, i)) {
                diagnostics.minVorticity = fminf(diagnostics.minVorticity, vorticity);
                diagnostics.maxVorticity = fmaxf(diagnostics.maxVorticity, vorticity);
                vorticityScaleSampleCount += 1;
            }
            diagnostics.avgDensity += system->density[i];
            diagnostics.avgPressure += system->pressure[i];
            diagnostics.avgSpeed += speed;
            diagnostics.avgTemperature += system->temperature[i];
            diagnostics.avgVorticity += vorticity;
        }

        if (vorticityScaleSampleCount == 0) {
            for (int i = 0; i < system->particleCount; ++i) {
                const float vorticity = system->vorticity[i];
                diagnostics.minVorticity = fminf(diagnostics.minVorticity, vorticity);
                diagnostics.maxVorticity = fmaxf(diagnostics.maxVorticity, vorticity);
            }
        }

        const float invCount = (system->particleCount > 0) ? 1.0f / (float)system->particleCount : 0.0f;
        diagnostics.avgDensity *= invCount;
        diagnostics.avgPressure *= invCount;
        diagnostics.avgSpeed *= invCount;
        diagnostics.avgTemperature *= invCount;
        diagnostics.avgVorticity *= invCount;
        diagnostics.minVorticity = (diagnostics.minVorticity == FLT_MAX) ? 0.0f : diagnostics.minVorticity;
        diagnostics.maxVorticity = fmaxf(diagnostics.maxVorticity, 0.0f);
        system->stats = diagnostics;
    }

    static void StepSimulationCPU(ParticleSystem3D *system, float frameDelta)
    {
        system->accumulator = fminf(system->accumulator + frameDelta * system->timeScale,
            system->params.timeStep * (float)MAX_SIM_STEPS_PER_FRAME);
        const float maxStep = system->params.timeStep;
        const float minStep = maxStep * 0.20f;
        int steps = 0;

        const double startTime = GetTime();
        while (system->accumulator >= minStep && steps < MAX_SIM_STEPS_PER_FRAME) {
            BuildGrid(system);
            ComputeDensityAndPressure(system);
            ComputeForces(system);
            SampleMicrophone(system);
            const float dt = ComputeAdaptiveTimeStep(system, fminf(maxStep, system->accumulator));
            system->lastStepDt = dt;
            Integrate(system, dt);
            system->simulationTime += dt;
            system->accumulator -= dt;
            ++steps;
            system->scalarFieldsDirty = true;
            system->sortedStateDirty = true;
            system->vorticityDirty = true;
        }

        const bool needsFreshDensity = ColorModeNeedsFreshDensity(system->colorMode);
        const bool refreshDiagnostics = needsFreshDensity || system->framesUntilDiagnostics <= 0;

        if (system->scalarFieldsDirty && (needsFreshDensity || refreshDiagnostics)) {
            BuildGrid(system);
            ComputeDensityAndPressure(system);
            system->scalarFieldsDirty = false;
        }

        if (refreshDiagnostics) {
            UpdateDiagnostics(system);
            system->framesUntilDiagnostics = DiagnosticRefreshIntervalForBackend(system->activeBackend);
        } else {
            system->framesUntilDiagnostics -= 1;
        }

        system->lastSimStepMs = (GetTime() - startTime) * 1000.0;
    }

    #if defined(__APPLE__)
    static NSString *LoadMetalSource3D(void)
    {
        NSString *workingPath = @"src/gpu_backend_3d.metal";
        NSError *error = nil;
        NSString *source = [NSString stringWithContentsOfFile:workingPath encoding:NSUTF8StringEncoding error:&error];
        if (source != nil) {
            return source;
        }

        NSString *executablePath = [[NSBundle mainBundle] executablePath];
        if (executablePath != nil) {
            NSString *bundleRelativePath = [[[[executablePath stringByDeletingLastPathComponent]
                stringByAppendingPathComponent:@"../src"] stringByStandardizingPath]
                stringByAppendingPathComponent:@"gpu_backend_3d.metal"];
            source = [NSString stringWithContentsOfFile:bundleRelativePath encoding:NSUTF8StringEncoding error:&error];
            if (source != nil) {
                return source;
            }
        }

        return nil;
    }

    static GpuSimParams3D MakeGpuSimParams3D(const ParticleSystem3D *system, float dt)
    {
        const float obstacleAngle = ObstacleAngleRadians(system);
        const Vector3 importedCenter = ImportedObstacleWorldCenter(system);
        const Matrix importedInverseRotation = MatrixTranspose(ImportedObstacleRotationMatrix(system));
        Vector3 speakerCenter = {0.0f, 0.0f, 0.0f};
        Vector3 speakerVelocity = {0.0f, 0.0f, 0.0f};
        Vector3 speakerHalfSize = {0.0f, 0.0f, 0.0f};
        if (AcousticsActive(system)) {
            GetSpeakerState(system, &speakerCenter, &speakerVelocity, &speakerHalfSize);
        }
        GpuSimParams3D params = (GpuSimParams3D){
            .particleCount = (uint32_t)system->particleCount,
            .gridWidth = (uint32_t)system->gridWidth,
            .gridHeight = (uint32_t)system->gridHeight,
            .gridDepth = (uint32_t)system->gridDepth,
            .cellCount = (uint32_t)system->cellCount,
            .preset = (uint32_t)system->preset,
            .scene = (uint32_t)system->scene,
            .obstacleModel = (uint32_t)system->obstacleModel,
            .obstacleEnabled = SolidObstacleActive(system) ? 1u : 0u,
            .waterCylinderCount = WaterObstaclesActive(system) ? (uint32_t)system->waterCylinderCount : 0u,
            .waterObstacleShape = (uint32_t)system->waterObstacleShape,
            .interactionActive = system->interactionActive ? 1u : 0u,
            .acousticsEnabled = AcousticsActive(system) ? 1u : 0u,
            .importedSdfWidth = (uint32_t)system->importedSdfWidth,
            .importedSdfHeight = (uint32_t)system->importedSdfHeight,
            .importedSdfDepth = (uint32_t)system->importedSdfDepth,
            .boundsMinX = system->boundsMin.x,
            .boundsMinY = system->boundsMin.y,
            .boundsMinZ = system->boundsMin.z,
            .boundsSizeX = system->boundsSize.x,
            .boundsSizeY = system->boundsSize.y,
            .boundsSizeZ = system->boundsSize.z,
            .supportRadius = system->params.supportRadius,
            .supportRadiusSquared = system->supportRadiusSquared,
            .particleRadius = system->params.particleRadius,
            .restDensity = system->params.restDensity,
            .soundSpeed = system->params.soundSpeed,
            .kinematicViscosity = system->params.kinematicViscosity,
            .gravity = system->params.gravity,
            .buoyancy = system->params.buoyancy,
            .temperatureDiffusion = system->params.temperatureDiffusion,
            .thermalExpansion = system->params.thermalExpansion,
            .ambientTemperature = system->params.ambientTemperature,
            .globalDrag = system->params.globalDrag,
            .wallBounce = system->params.wallBounce,
            .wallFriction = system->params.wallFriction,
            .mass = system->mass,
            .pressureStiffness = system->pressureStiffness,
            .densityKernel = system->densityKernel,
            .pressureKernelGrad = system->pressureKernelGrad,
            .viscosityKernelLap = system->viscosityKernelLap,
            .interactionRadius = system->interactionRadius,
            .interactionRadiusSquared = system->interactionRadiusSquared,
            .interactionStrength = system->interactionStrength,
            .interactionX = system->interactionPoint.x,
            .interactionY = system->interactionPoint.y,
            .interactionZ = system->interactionPoint.z,
            .obstacleCenterX = system->obstacleCenter.x,
            .obstacleCenterY = system->obstacleCenter.y,
            .obstacleCenterZ = system->obstacleCenter.z,
            .obstacleRadius = system->obstacleRadius,
            .obstacleAngleCos = cosf(obstacleAngle),
            .obstacleAngleSin = sinf(obstacleAngle),
            .obstacleRectHalfWidth = system->obstacleRectWidth * 0.5f,
            .obstacleRectHalfHeight = system->obstacleRectHeight * 0.5f,
            .obstacleHalfDepth = system->obstacleDepth * 0.5f,
            .importedSdfMinX = system->importedObstacleLocalMin.x,
            .importedSdfMinY = system->importedObstacleLocalMin.y,
            .importedSdfMinZ = system->importedObstacleLocalMin.z,
            .importedSdfSizeX = system->importedObstacleLocalMax.x - system->importedObstacleLocalMin.x,
            .importedSdfSizeY = system->importedObstacleLocalMax.y - system->importedObstacleLocalMin.y,
            .importedSdfSizeZ = system->importedObstacleLocalMax.z - system->importedObstacleLocalMin.z,
            .importedCenterX = importedCenter.x,
            .importedCenterY = importedCenter.y,
            .importedCenterZ = importedCenter.z,
            .importedScale = fmaxf(system->importedObstacleUserScale, 0.05f),
            .importedInvRotXX = importedInverseRotation.m0,
            .importedInvRotXY = importedInverseRotation.m4,
            .importedInvRotXZ = importedInverseRotation.m8,
            .importedInvRotYX = importedInverseRotation.m1,
            .importedInvRotYY = importedInverseRotation.m5,
            .importedInvRotYZ = importedInverseRotation.m9,
            .importedInvRotZX = importedInverseRotation.m2,
            .importedInvRotZY = importedInverseRotation.m6,
            .importedInvRotZZ = importedInverseRotation.m10,
            .obstacleShell = system->obstacleShell,
            .obstacleStrength = system->obstacleStrength,
            .obstacleDamping = system->obstacleDamping,
            .speakerCenterX = speakerCenter.x,
            .speakerCenterY = speakerCenter.y,
            .speakerCenterZ = speakerCenter.z,
            .speakerVelocityX = speakerVelocity.x,
            .speakerVelocityY = speakerVelocity.y,
            .speakerVelocityZ = speakerVelocity.z,
            .speakerHalfWidth = speakerHalfSize.x,
            .speakerHalfHeight = speakerHalfSize.y,
            .speakerHalfDepth = speakerHalfSize.z,
            .speakerShell = system->speakerShell,
            .speakerStrength = system->speakerStrength,
            .speakerDamping = system->speakerDamping,
            .flowTargetSpeed = system->flowTargetSpeed,
            .flowDrive = system->flowDrive,
            .simulationTime = system->simulationTime,
            .substepDt = dt,
        };

        const int waterCount = WaterObstaclesActive(system) ? ClampInt(system->waterCylinderCount, 0, WATER_CYLINDER_MAX) : 0;
        for (int i = 0; i < WATER_CYLINDER_MAX; ++i) {
            params.waterCylinderX[i] = system->waterCylinderX[i];
            params.waterCylinderY[i] = system->waterCylinderY[i];
            params.waterCylinderZ[i] = system->waterCylinderZ[i];
            params.waterCylinderRadius[i] = system->waterCylinderRadius[i];
            params.waterCylinderHalfDepth[i] = system->waterCylinderDepth[i] * 0.5f;
            params.waterRectangleHalfWidth[i] = system->waterRectangleWidth[i] * 0.5f;
            params.waterRectangleHalfHeight[i] = system->waterRectangleHeight[i] * 0.5f;
            params.waterRectangleAngleCos[i] = cosf(system->waterRectangleAngleDegrees[i] * DEG2RAD);
            params.waterRectangleAngleSin[i] = sinf(system->waterRectangleAngleDegrees[i] * DEG2RAD);
        }
        params.waterCylinderCount = (uint32_t)waterCount;
        return params;
    }

    @implementation MetalGpuBackend3D {
        id<MTLDevice> _device;
        id<MTLCommandQueue> _commandQueue;
        id<MTLLibrary> _library;
        id<MTLComputePipelineState> _clearCountsPSO;
        id<MTLComputePipelineState> _computeCellsPSO;
        id<MTLComputePipelineState> _scatterIndicesPSO;
        id<MTLComputePipelineState> _gatherSortedPSO;
        id<MTLComputePipelineState> _densityPressurePSO;
        id<MTLComputePipelineState> _forcePSO;
        id<MTLComputePipelineState> _integratePSO;
        id<MTLBuffer> _paramsBuffer;

        id<MTLBuffer> _xBuffer;
        id<MTLBuffer> _yBuffer;
        id<MTLBuffer> _zBuffer;
        id<MTLBuffer> _vxBuffer;
        id<MTLBuffer> _vyBuffer;
        id<MTLBuffer> _vzBuffer;
        id<MTLBuffer> _axBuffer;
        id<MTLBuffer> _ayBuffer;
        id<MTLBuffer> _azBuffer;
        id<MTLBuffer> _densityBuffer;
        id<MTLBuffer> _pressureBuffer;
        id<MTLBuffer> _temperatureBuffer;
        id<MTLBuffer> _temperatureRateBuffer;
        id<MTLBuffer> _xsphVXBuffer;
        id<MTLBuffer> _xsphVYBuffer;
        id<MTLBuffer> _xsphVZBuffer;
        id<MTLBuffer> _sortedXBuffer;
        id<MTLBuffer> _sortedYBuffer;
        id<MTLBuffer> _sortedZBuffer;
        id<MTLBuffer> _sortedVXBuffer;
        id<MTLBuffer> _sortedVYBuffer;
        id<MTLBuffer> _sortedVZBuffer;
        id<MTLBuffer> _sortedTemperatureBuffer;
        id<MTLBuffer> _sortedDensityBuffer;
        id<MTLBuffer> _sortedPressureBuffer;
        id<MTLBuffer> _particleCellsBuffer;
        id<MTLBuffer> _sortedCellIndicesBuffer;
        id<MTLBuffer> _sortedIndicesBuffer;
        id<MTLBuffer> _cellCountsBuffer;
        id<MTLBuffer> _cellStartsBuffer;
        id<MTLBuffer> _cellOffsetsBuffer;
        id<MTLBuffer> _cellNeighborCountsBuffer;
        id<MTLBuffer> _cellNeighborsBuffer;
        id<MTLBuffer> _importedSdfBuffer;
        id<MTLBuffer> _fallbackScalarBuffer;

        const void *_cellCountsPtr;
        const void *_cellStartsPtr;
        const void *_cellOffsetsPtr;
        const void *_cellNeighborCountsPtr;
        const void *_cellNeighborsPtr;
        NSUInteger _cellCountsLength;
        NSUInteger _cellStartsLength;
        NSUInteger _cellOffsetsLength;
        NSUInteger _cellNeighborCountsLength;
        NSUInteger _cellNeighborsLength;
        const void *_importedSdfPtr;
        NSUInteger _importedSdfLength;
    }

    - (id<MTLBuffer>)wrapBufferForPointer:(void *)pointer
                                length:(NSUInteger)length
                                    label:(NSString *)label
                                    error:(NSString * _Nullable * _Nullable)error
    {
        if (pointer == NULL || length == 0) {
            if (error != NULL) {
                *error = [NSString stringWithFormat:@"Missing storage for %@", label];
            }
            return nil;
        }

        id<MTLBuffer> buffer = [_device newBufferWithBytesNoCopy:pointer
                                                        length:length
                                                        options:MTLResourceStorageModeShared
                                                    deallocator:nil];
        if (buffer == nil) {
            if (error != NULL) {
                *error = [NSString stringWithFormat:@"Failed to wrap %@", label];
            }
            return nil;
        }

        buffer.label = label;
        return buffer;
    }

    - (BOOL)createFixedBuffersForSystem:(ParticleSystem3D *)system error:(NSString * _Nullable * _Nullable)error
    {
        const NSUInteger particleBytes = (NSUInteger)system->maxParticles * sizeof(float);
        const NSUInteger particleIntBytes = (NSUInteger)system->maxParticles * sizeof(int);

        _xBuffer = [self wrapBufferForPointer:system->x length:particleBytes label:@"x3d" error:error];
        _yBuffer = [self wrapBufferForPointer:system->y length:particleBytes label:@"y3d" error:error];
        _zBuffer = [self wrapBufferForPointer:system->z length:particleBytes label:@"z3d" error:error];
        _vxBuffer = [self wrapBufferForPointer:system->vx length:particleBytes label:@"vx3d" error:error];
        _vyBuffer = [self wrapBufferForPointer:system->vy length:particleBytes label:@"vy3d" error:error];
        _vzBuffer = [self wrapBufferForPointer:system->vz length:particleBytes label:@"vz3d" error:error];
        _axBuffer = [self wrapBufferForPointer:system->ax length:particleBytes label:@"ax3d" error:error];
        _ayBuffer = [self wrapBufferForPointer:system->ay length:particleBytes label:@"ay3d" error:error];
        _azBuffer = [self wrapBufferForPointer:system->az length:particleBytes label:@"az3d" error:error];
        _densityBuffer = [self wrapBufferForPointer:system->density length:particleBytes label:@"density3d" error:error];
        _pressureBuffer = [self wrapBufferForPointer:system->pressure length:particleBytes label:@"pressure3d" error:error];
        _temperatureBuffer = [self wrapBufferForPointer:system->temperature length:particleBytes label:@"temperature3d" error:error];
        _temperatureRateBuffer = [self wrapBufferForPointer:system->temperatureRate length:particleBytes label:@"temperatureRate3d" error:error];
        _xsphVXBuffer = [self wrapBufferForPointer:system->xsphVX length:particleBytes label:@"xsphVX3d" error:error];
        _xsphVYBuffer = [self wrapBufferForPointer:system->xsphVY length:particleBytes label:@"xsphVY3d" error:error];
        _xsphVZBuffer = [self wrapBufferForPointer:system->xsphVZ length:particleBytes label:@"xsphVZ3d" error:error];
        _sortedXBuffer = [self wrapBufferForPointer:system->sortedX length:particleBytes label:@"sortedX3d" error:error];
        _sortedYBuffer = [self wrapBufferForPointer:system->sortedY length:particleBytes label:@"sortedY3d" error:error];
        _sortedZBuffer = [self wrapBufferForPointer:system->sortedZ length:particleBytes label:@"sortedZ3d" error:error];
        _sortedVXBuffer = [self wrapBufferForPointer:system->sortedVX length:particleBytes label:@"sortedVX3d" error:error];
        _sortedVYBuffer = [self wrapBufferForPointer:system->sortedVY length:particleBytes label:@"sortedVY3d" error:error];
        _sortedVZBuffer = [self wrapBufferForPointer:system->sortedVZ length:particleBytes label:@"sortedVZ3d" error:error];
        _sortedTemperatureBuffer = [self wrapBufferForPointer:system->sortedTemperature length:particleBytes label:@"sortedTemp3d" error:error];
        _sortedDensityBuffer = [self wrapBufferForPointer:system->sortedDensity length:particleBytes label:@"sortedDensity3d" error:error];
        _sortedPressureBuffer = [self wrapBufferForPointer:system->sortedPressure length:particleBytes label:@"sortedPressure3d" error:error];
        _particleCellsBuffer = [self wrapBufferForPointer:system->particleCells length:particleIntBytes label:@"particleCells3d" error:error];
        _sortedCellIndicesBuffer = [self wrapBufferForPointer:system->sortedCellIndices length:particleIntBytes label:@"sortedCellIndices3d" error:error];
        _sortedIndicesBuffer = [self wrapBufferForPointer:system->sortedIndices length:particleIntBytes label:@"sortedIndices3d" error:error];

        return _xBuffer != nil && _yBuffer != nil && _zBuffer != nil &&
            _vxBuffer != nil && _vyBuffer != nil && _vzBuffer != nil &&
            _axBuffer != nil && _ayBuffer != nil && _azBuffer != nil &&
            _densityBuffer != nil && _pressureBuffer != nil &&
            _temperatureBuffer != nil && _temperatureRateBuffer != nil &&
            _xsphVXBuffer != nil && _xsphVYBuffer != nil && _xsphVZBuffer != nil &&
            _sortedXBuffer != nil && _sortedYBuffer != nil && _sortedZBuffer != nil &&
            _sortedVXBuffer != nil && _sortedVYBuffer != nil && _sortedVZBuffer != nil &&
            _sortedTemperatureBuffer != nil && _sortedDensityBuffer != nil && _sortedPressureBuffer != nil &&
            _particleCellsBuffer != nil && _sortedCellIndicesBuffer != nil && _sortedIndicesBuffer != nil;
    }

    - (BOOL)rebindGridBuffer:(id<MTLBuffer> __strong *)buffer
                    track:(const void **)trackedPointer
                    bytes:(NSUInteger *)trackedLength
                    pointer:(void *)pointer
                    length:(NSUInteger)length
                    label:(NSString *)label
                    error:(NSString * _Nullable * _Nullable)error
    {
        if (*buffer != nil && *trackedPointer == pointer && *trackedLength == length) {
            return YES;
        }

        *buffer = [self wrapBufferForPointer:pointer length:length label:label error:error];
        if (*buffer == nil) {
            return NO;
        }

        *trackedPointer = pointer;
        *trackedLength = length;
        return YES;
    }

    - (id<MTLComputePipelineState>)pipelineNamed:(NSString *)name error:(NSString * _Nullable * _Nullable)error
    {
        id<MTLFunction> function = [_library newFunctionWithName:name];
        if (function == nil) {
            if (error != NULL) {
                *error = [NSString stringWithFormat:@"Missing Metal kernel %@", name];
            }
            return nil;
        }

        NSError *pipelineError = nil;
        id<MTLComputePipelineState> pipeline = [_device newComputePipelineStateWithFunction:function error:&pipelineError];
        if (pipeline == nil && error != NULL) {
            *error = [NSString stringWithFormat:@"Failed to create pipeline %@: %@", name, pipelineError.localizedDescription];
        }
        return pipeline;
    }

    - (instancetype)initWithSystem:(ParticleSystem3D *)system error:(NSString * _Nullable * _Nullable)error
    {
        self = [super init];
        if (self == nil) {
            if (error != NULL) {
                *error = @"Failed to initialize 3D Metal backend object.";
            }
            return nil;
        }

        _device = MTLCreateSystemDefaultDevice();
        if (_device == nil) {
            if (error != NULL) {
                *error = @"No Metal device is available.";
            }
            return nil;
        }

        _commandQueue = [_device newCommandQueue];
        if (_commandQueue == nil) {
            if (error != NULL) {
                *error = @"Failed to create a Metal command queue.";
            }
            return nil;
        }

        NSString *source = LoadMetalSource3D();
        if (source == nil) {
            if (error != NULL) {
                *error = @"Could not load src/gpu_backend_3d.metal.";
            }
            return nil;
        }

        MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
        options.mathMode = MTLMathModeFast;

        NSError *libraryError = nil;
        _library = [_device newLibraryWithSource:source options:options error:&libraryError];
        if (_library == nil) {
            if (error != NULL) {
                *error = [NSString stringWithFormat:@"Failed to compile 3D Metal shaders: %@", libraryError.localizedDescription];
            }
            return nil;
        }

        _clearCountsPSO = [self pipelineNamed:@"clearCounts" error:error];
        _computeCellsPSO = [self pipelineNamed:@"computeParticleCells" error:error];
        _scatterIndicesPSO = [self pipelineNamed:@"scatterSortedIndices" error:error];
        _gatherSortedPSO = [self pipelineNamed:@"gatherSortedData" error:error];
        _densityPressurePSO = [self pipelineNamed:@"computeDensityPressure" error:error];
        _forcePSO = [self pipelineNamed:@"computeForces" error:error];
        _integratePSO = [self pipelineNamed:@"integrateParticles" error:error];
        if (_clearCountsPSO == nil || _computeCellsPSO == nil || _scatterIndicesPSO == nil ||
            _gatherSortedPSO == nil || _densityPressurePSO == nil || _forcePSO == nil || _integratePSO == nil) {
            return nil;
        }

        _paramsBuffer = [_device newBufferWithLength:sizeof(GpuSimParams3D) options:MTLResourceStorageModeShared];
        if (_paramsBuffer == nil) {
            if (error != NULL) {
                *error = @"Failed to allocate 3D GPU parameter buffer.";
            }
            return nil;
        }

        _fallbackScalarBuffer = [_device newBufferWithLength:sizeof(float) options:MTLResourceStorageModeShared];
        if (_fallbackScalarBuffer == nil) {
            if (error != NULL) {
                *error = @"Failed to allocate 3D GPU fallback scalar buffer.";
            }
            return nil;
        }

        if (![self createFixedBuffersForSystem:system error:error]) {
            return nil;
        }

        if (![self prepareForSystem:system error:error]) {
            return nil;
        }

        return self;
    }

    - (void)invalidateGridBindings
    {
        _cellCountsBuffer = nil;
        _cellStartsBuffer = nil;
        _cellOffsetsBuffer = nil;
        _cellNeighborCountsBuffer = nil;
        _cellNeighborsBuffer = nil;
        _cellCountsPtr = NULL;
        _cellStartsPtr = NULL;
        _cellOffsetsPtr = NULL;
        _cellNeighborCountsPtr = NULL;
        _cellNeighborsPtr = NULL;
        _cellCountsLength = 0;
        _cellStartsLength = 0;
        _cellOffsetsLength = 0;
        _cellNeighborCountsLength = 0;
        _cellNeighborsLength = 0;
    }

    - (BOOL)prepareForSystem:(ParticleSystem3D *)system error:(NSString * _Nullable * _Nullable)error
    {
        if (![self rebindGridBuffer:&_cellCountsBuffer track:&_cellCountsPtr bytes:&_cellCountsLength
                            pointer:system->cellCounts length:(NSUInteger)system->cellCount * sizeof(int)
                            label:@"cellCounts3d" error:error]) {
            return NO;
        }
        if (![self rebindGridBuffer:&_cellStartsBuffer track:&_cellStartsPtr bytes:&_cellStartsLength
                            pointer:system->cellStarts length:(NSUInteger)(system->cellCount + 1) * sizeof(int)
                            label:@"cellStarts3d" error:error]) {
            return NO;
        }
        if (![self rebindGridBuffer:&_cellOffsetsBuffer track:&_cellOffsetsPtr bytes:&_cellOffsetsLength
                            pointer:system->cellOffsets length:(NSUInteger)system->cellCount * sizeof(int)
                            label:@"cellOffsets3d" error:error]) {
            return NO;
        }
        if (![self rebindGridBuffer:&_cellNeighborCountsBuffer track:&_cellNeighborCountsPtr bytes:&_cellNeighborCountsLength
                            pointer:system->cellNeighborCounts length:(NSUInteger)system->cellCount * sizeof(int)
                            label:@"cellNeighborCounts3d" error:error]) {
            return NO;
        }
        if (![self rebindGridBuffer:&_cellNeighborsBuffer track:&_cellNeighborsPtr bytes:&_cellNeighborsLength
                            pointer:system->cellNeighbors length:(NSUInteger)system->cellCount * 27u * sizeof(int)
                            label:@"cellNeighbors3d" error:error]) {
            return NO;
        }

        if (system->importedSdfValues != NULL && system->importedSdfVoxelCount > 0) {
            if (![self rebindGridBuffer:&_importedSdfBuffer track:&_importedSdfPtr bytes:&_importedSdfLength
                                pointer:system->importedSdfValues length:(NSUInteger)system->importedSdfVoxelCount * sizeof(float)
                                label:@"importedSdf3d" error:error]) {
                return NO;
            }
        } else {
            _importedSdfBuffer = _fallbackScalarBuffer;
            _importedSdfPtr = NULL;
            _importedSdfLength = sizeof(float);
        }
        return YES;
    }

    - (void)writeParams:(const GpuSimParams3D *)params
    {
        memcpy(_paramsBuffer.contents, params, sizeof(*params));
    }

    - (void)dispatchCount:(NSUInteger)count pipeline:(id<MTLComputePipelineState>)pipeline encoder:(id<MTLComputeCommandEncoder>)encoder
    {
        if (count == 0) {
            return;
        }

        const NSUInteger threadCount = MIN((NSUInteger)256, pipeline.maxTotalThreadsPerThreadgroup);
        [encoder setComputePipelineState:pipeline];
        [encoder dispatchThreads:MTLSizeMake(count, 1, 1)
        threadsPerThreadgroup:MTLSizeMake(threadCount, 1, 1)];
    }

    - (BOOL)finishCommandBuffer:(id<MTLCommandBuffer>)commandBuffer error:(NSString * _Nullable * _Nullable)error
    {
        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
        if (commandBuffer.status != MTLCommandBufferStatusCompleted) {
            if (error != NULL) {
                NSString *errorText = (commandBuffer.error.localizedDescription != nil)
                    ? commandBuffer.error.localizedDescription
                    : @"Unknown Metal command failure.";
                *error = [NSString stringWithFormat:@"Metal command buffer failed: %@", errorText];
            }
            return NO;
        }
        return YES;
    }

    - (BOOL)runBuildDensityForceForSystem:(ParticleSystem3D *)system includeForce:(BOOL)includeForce error:(NSString * _Nullable * _Nullable)error
    {
        if (![self prepareForSystem:system error:error]) {
            return NO;
        }

        GpuSimParams3D params = MakeGpuSimParams3D(system, 0.0f);
        [self writeParams:&params];

        id<MTLCommandBuffer> countBuffer = [_commandQueue commandBuffer];
        id<MTLComputeCommandEncoder> countEncoder = [countBuffer computeCommandEncoder];
        [countEncoder setBuffer:_cellCountsBuffer offset:0 atIndex:0];
        [countEncoder setBuffer:_paramsBuffer offset:0 atIndex:1];
        [self dispatchCount:(NSUInteger)system->cellCount pipeline:_clearCountsPSO encoder:countEncoder];

        [countEncoder setBuffer:_xBuffer offset:0 atIndex:0];
        [countEncoder setBuffer:_yBuffer offset:0 atIndex:1];
        [countEncoder setBuffer:_zBuffer offset:0 atIndex:2];
        [countEncoder setBuffer:_particleCellsBuffer offset:0 atIndex:3];
        [countEncoder setBuffer:_cellCountsBuffer offset:0 atIndex:4];
        [countEncoder setBuffer:_paramsBuffer offset:0 atIndex:5];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_computeCellsPSO encoder:countEncoder];
        [countEncoder endEncoding];

        if (![self finishCommandBuffer:countBuffer error:error]) {
            return NO;
        }

        system->cellStarts[0] = 0;
        for (int cell = 0; cell < system->cellCount; ++cell) {
            system->cellStarts[cell + 1] = system->cellStarts[cell] + system->cellCounts[cell];
            system->cellOffsets[cell] = system->cellStarts[cell];
        }

        id<MTLCommandBuffer> simBuffer = [_commandQueue commandBuffer];
        id<MTLComputeCommandEncoder> encoder = [simBuffer computeCommandEncoder];

        [encoder setBuffer:_particleCellsBuffer offset:0 atIndex:0];
        [encoder setBuffer:_cellOffsetsBuffer offset:0 atIndex:1];
        [encoder setBuffer:_sortedIndicesBuffer offset:0 atIndex:2];
        [encoder setBuffer:_paramsBuffer offset:0 atIndex:3];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_scatterIndicesPSO encoder:encoder];

        [encoder setBuffer:_sortedIndicesBuffer offset:0 atIndex:0];
        [encoder setBuffer:_particleCellsBuffer offset:0 atIndex:1];
        [encoder setBuffer:_xBuffer offset:0 atIndex:2];
        [encoder setBuffer:_yBuffer offset:0 atIndex:3];
        [encoder setBuffer:_zBuffer offset:0 atIndex:4];
        [encoder setBuffer:_vxBuffer offset:0 atIndex:5];
        [encoder setBuffer:_vyBuffer offset:0 atIndex:6];
        [encoder setBuffer:_vzBuffer offset:0 atIndex:7];
        [encoder setBuffer:_temperatureBuffer offset:0 atIndex:8];
        [encoder setBuffer:_sortedXBuffer offset:0 atIndex:9];
        [encoder setBuffer:_sortedYBuffer offset:0 atIndex:10];
        [encoder setBuffer:_sortedZBuffer offset:0 atIndex:11];
        [encoder setBuffer:_sortedVXBuffer offset:0 atIndex:12];
        [encoder setBuffer:_sortedVYBuffer offset:0 atIndex:13];
        [encoder setBuffer:_sortedVZBuffer offset:0 atIndex:14];
        [encoder setBuffer:_sortedTemperatureBuffer offset:0 atIndex:15];
        [encoder setBuffer:_sortedCellIndicesBuffer offset:0 atIndex:16];
        [encoder setBuffer:_paramsBuffer offset:0 atIndex:17];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_gatherSortedPSO encoder:encoder];

        [encoder setBuffer:_sortedXBuffer offset:0 atIndex:0];
        [encoder setBuffer:_sortedYBuffer offset:0 atIndex:1];
        [encoder setBuffer:_sortedZBuffer offset:0 atIndex:2];
        [encoder setBuffer:_sortedTemperatureBuffer offset:0 atIndex:3];
        [encoder setBuffer:_sortedCellIndicesBuffer offset:0 atIndex:4];
        [encoder setBuffer:_cellStartsBuffer offset:0 atIndex:5];
        [encoder setBuffer:_cellNeighborCountsBuffer offset:0 atIndex:6];
        [encoder setBuffer:_cellNeighborsBuffer offset:0 atIndex:7];
        [encoder setBuffer:_sortedIndicesBuffer offset:0 atIndex:8];
        [encoder setBuffer:_densityBuffer offset:0 atIndex:9];
        [encoder setBuffer:_pressureBuffer offset:0 atIndex:10];
        [encoder setBuffer:_sortedDensityBuffer offset:0 atIndex:11];
        [encoder setBuffer:_sortedPressureBuffer offset:0 atIndex:12];
        [encoder setBuffer:_paramsBuffer offset:0 atIndex:13];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_densityPressurePSO encoder:encoder];

        if (includeForce) {
            [encoder setBuffer:_sortedIndicesBuffer offset:0 atIndex:0];
            [encoder setBuffer:_sortedXBuffer offset:0 atIndex:1];
            [encoder setBuffer:_sortedYBuffer offset:0 atIndex:2];
            [encoder setBuffer:_sortedZBuffer offset:0 atIndex:3];
            [encoder setBuffer:_sortedVXBuffer offset:0 atIndex:4];
            [encoder setBuffer:_sortedVYBuffer offset:0 atIndex:5];
            [encoder setBuffer:_sortedVZBuffer offset:0 atIndex:6];
            [encoder setBuffer:_sortedTemperatureBuffer offset:0 atIndex:7];
            [encoder setBuffer:_sortedDensityBuffer offset:0 atIndex:8];
            [encoder setBuffer:_sortedPressureBuffer offset:0 atIndex:9];
            [encoder setBuffer:_sortedCellIndicesBuffer offset:0 atIndex:10];
            [encoder setBuffer:_cellStartsBuffer offset:0 atIndex:11];
            [encoder setBuffer:_cellNeighborCountsBuffer offset:0 atIndex:12];
            [encoder setBuffer:_cellNeighborsBuffer offset:0 atIndex:13];
            [encoder setBuffer:_axBuffer offset:0 atIndex:14];
            [encoder setBuffer:_ayBuffer offset:0 atIndex:15];
            [encoder setBuffer:_azBuffer offset:0 atIndex:16];
            [encoder setBuffer:_temperatureRateBuffer offset:0 atIndex:17];
            [encoder setBuffer:_xsphVXBuffer offset:0 atIndex:18];
            [encoder setBuffer:_xsphVYBuffer offset:0 atIndex:19];
            [encoder setBuffer:_xsphVZBuffer offset:0 atIndex:20];
            [encoder setBuffer:_importedSdfBuffer offset:0 atIndex:21];
            [encoder setBuffer:_paramsBuffer offset:0 atIndex:22];
            [self dispatchCount:(NSUInteger)system->particleCount pipeline:_forcePSO encoder:encoder];
        }

        [encoder endEncoding];
        return [self finishCommandBuffer:simBuffer error:error];
    }

    - (BOOL)runStepForSystem:(ParticleSystem3D *)system dt:(float)dt error:(NSString * _Nullable * _Nullable)error
    {
        if (![self prepareForSystem:system error:error]) {
            return NO;
        }

        GpuSimParams3D params = MakeGpuSimParams3D(system, dt);
        [self writeParams:&params];

        id<MTLCommandBuffer> countBuffer = [_commandQueue commandBuffer];
        id<MTLComputeCommandEncoder> countEncoder = [countBuffer computeCommandEncoder];
        [countEncoder setBuffer:_cellCountsBuffer offset:0 atIndex:0];
        [countEncoder setBuffer:_paramsBuffer offset:0 atIndex:1];
        [self dispatchCount:(NSUInteger)system->cellCount pipeline:_clearCountsPSO encoder:countEncoder];

        [countEncoder setBuffer:_xBuffer offset:0 atIndex:0];
        [countEncoder setBuffer:_yBuffer offset:0 atIndex:1];
        [countEncoder setBuffer:_zBuffer offset:0 atIndex:2];
        [countEncoder setBuffer:_particleCellsBuffer offset:0 atIndex:3];
        [countEncoder setBuffer:_cellCountsBuffer offset:0 atIndex:4];
        [countEncoder setBuffer:_paramsBuffer offset:0 atIndex:5];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_computeCellsPSO encoder:countEncoder];
        [countEncoder endEncoding];

        if (![self finishCommandBuffer:countBuffer error:error]) {
            return NO;
        }

        system->cellStarts[0] = 0;
        for (int cell = 0; cell < system->cellCount; ++cell) {
            system->cellStarts[cell + 1] = system->cellStarts[cell] + system->cellCounts[cell];
            system->cellOffsets[cell] = system->cellStarts[cell];
        }

        id<MTLCommandBuffer> simBuffer = [_commandQueue commandBuffer];
        id<MTLComputeCommandEncoder> encoder = [simBuffer computeCommandEncoder];

        [encoder setBuffer:_particleCellsBuffer offset:0 atIndex:0];
        [encoder setBuffer:_cellOffsetsBuffer offset:0 atIndex:1];
        [encoder setBuffer:_sortedIndicesBuffer offset:0 atIndex:2];
        [encoder setBuffer:_paramsBuffer offset:0 atIndex:3];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_scatterIndicesPSO encoder:encoder];

        [encoder setBuffer:_sortedIndicesBuffer offset:0 atIndex:0];
        [encoder setBuffer:_particleCellsBuffer offset:0 atIndex:1];
        [encoder setBuffer:_xBuffer offset:0 atIndex:2];
        [encoder setBuffer:_yBuffer offset:0 atIndex:3];
        [encoder setBuffer:_zBuffer offset:0 atIndex:4];
        [encoder setBuffer:_vxBuffer offset:0 atIndex:5];
        [encoder setBuffer:_vyBuffer offset:0 atIndex:6];
        [encoder setBuffer:_vzBuffer offset:0 atIndex:7];
        [encoder setBuffer:_temperatureBuffer offset:0 atIndex:8];
        [encoder setBuffer:_sortedXBuffer offset:0 atIndex:9];
        [encoder setBuffer:_sortedYBuffer offset:0 atIndex:10];
        [encoder setBuffer:_sortedZBuffer offset:0 atIndex:11];
        [encoder setBuffer:_sortedVXBuffer offset:0 atIndex:12];
        [encoder setBuffer:_sortedVYBuffer offset:0 atIndex:13];
        [encoder setBuffer:_sortedVZBuffer offset:0 atIndex:14];
        [encoder setBuffer:_sortedTemperatureBuffer offset:0 atIndex:15];
        [encoder setBuffer:_sortedCellIndicesBuffer offset:0 atIndex:16];
        [encoder setBuffer:_paramsBuffer offset:0 atIndex:17];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_gatherSortedPSO encoder:encoder];

        [encoder setBuffer:_sortedXBuffer offset:0 atIndex:0];
        [encoder setBuffer:_sortedYBuffer offset:0 atIndex:1];
        [encoder setBuffer:_sortedZBuffer offset:0 atIndex:2];
        [encoder setBuffer:_sortedTemperatureBuffer offset:0 atIndex:3];
        [encoder setBuffer:_sortedCellIndicesBuffer offset:0 atIndex:4];
        [encoder setBuffer:_cellStartsBuffer offset:0 atIndex:5];
        [encoder setBuffer:_cellNeighborCountsBuffer offset:0 atIndex:6];
        [encoder setBuffer:_cellNeighborsBuffer offset:0 atIndex:7];
        [encoder setBuffer:_sortedIndicesBuffer offset:0 atIndex:8];
        [encoder setBuffer:_densityBuffer offset:0 atIndex:9];
        [encoder setBuffer:_pressureBuffer offset:0 atIndex:10];
        [encoder setBuffer:_sortedDensityBuffer offset:0 atIndex:11];
        [encoder setBuffer:_sortedPressureBuffer offset:0 atIndex:12];
        [encoder setBuffer:_paramsBuffer offset:0 atIndex:13];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_densityPressurePSO encoder:encoder];

        [encoder setBuffer:_sortedIndicesBuffer offset:0 atIndex:0];
        [encoder setBuffer:_sortedXBuffer offset:0 atIndex:1];
        [encoder setBuffer:_sortedYBuffer offset:0 atIndex:2];
        [encoder setBuffer:_sortedZBuffer offset:0 atIndex:3];
        [encoder setBuffer:_sortedVXBuffer offset:0 atIndex:4];
        [encoder setBuffer:_sortedVYBuffer offset:0 atIndex:5];
        [encoder setBuffer:_sortedVZBuffer offset:0 atIndex:6];
        [encoder setBuffer:_sortedTemperatureBuffer offset:0 atIndex:7];
        [encoder setBuffer:_sortedDensityBuffer offset:0 atIndex:8];
        [encoder setBuffer:_sortedPressureBuffer offset:0 atIndex:9];
        [encoder setBuffer:_sortedCellIndicesBuffer offset:0 atIndex:10];
        [encoder setBuffer:_cellStartsBuffer offset:0 atIndex:11];
        [encoder setBuffer:_cellNeighborCountsBuffer offset:0 atIndex:12];
        [encoder setBuffer:_cellNeighborsBuffer offset:0 atIndex:13];
        [encoder setBuffer:_axBuffer offset:0 atIndex:14];
        [encoder setBuffer:_ayBuffer offset:0 atIndex:15];
        [encoder setBuffer:_azBuffer offset:0 atIndex:16];
        [encoder setBuffer:_temperatureRateBuffer offset:0 atIndex:17];
        [encoder setBuffer:_xsphVXBuffer offset:0 atIndex:18];
        [encoder setBuffer:_xsphVYBuffer offset:0 atIndex:19];
        [encoder setBuffer:_xsphVZBuffer offset:0 atIndex:20];
        [encoder setBuffer:_importedSdfBuffer offset:0 atIndex:21];
        [encoder setBuffer:_paramsBuffer offset:0 atIndex:22];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_forcePSO encoder:encoder];

        [encoder setBuffer:_xBuffer offset:0 atIndex:0];
        [encoder setBuffer:_yBuffer offset:0 atIndex:1];
        [encoder setBuffer:_zBuffer offset:0 atIndex:2];
        [encoder setBuffer:_vxBuffer offset:0 atIndex:3];
        [encoder setBuffer:_vyBuffer offset:0 atIndex:4];
        [encoder setBuffer:_vzBuffer offset:0 atIndex:5];
        [encoder setBuffer:_axBuffer offset:0 atIndex:6];
        [encoder setBuffer:_ayBuffer offset:0 atIndex:7];
        [encoder setBuffer:_azBuffer offset:0 atIndex:8];
        [encoder setBuffer:_temperatureBuffer offset:0 atIndex:9];
        [encoder setBuffer:_temperatureRateBuffer offset:0 atIndex:10];
        [encoder setBuffer:_xsphVXBuffer offset:0 atIndex:11];
        [encoder setBuffer:_xsphVYBuffer offset:0 atIndex:12];
        [encoder setBuffer:_xsphVZBuffer offset:0 atIndex:13];
        [encoder setBuffer:_importedSdfBuffer offset:0 atIndex:14];
        [encoder setBuffer:_paramsBuffer offset:0 atIndex:15];
        [self dispatchCount:(NSUInteger)system->particleCount pipeline:_integratePSO encoder:encoder];

        [encoder endEncoding];
        return [self finishCommandBuffer:simBuffer error:error];
    }
    @end

    static bool InitializeGpuBackend(ParticleSystem3D *system)
    {
        if (system->gpuBackend != NULL) {
            return true;
        }

        NSString *error = nil;
        MetalGpuBackend3D *backend = [[MetalGpuBackend3D alloc] initWithSystem:system error:&error];
        if (backend == nil) {
            if (error != nil) {
                fprintf(stderr, "3D GPU backend init failed: %s\n", [error UTF8String]);
            }
            system->gpuBackendAvailable = false;
            return false;
        }

        system->gpuBackend = (__bridge_retained void *)backend;
        system->gpuBackendAvailable = true;
        return true;
    }

    static void ShutdownGpuBackend(ParticleSystem3D *system)
    {
        if (system->gpuBackend != NULL) {
            CFBridgingRelease(system->gpuBackend);
            system->gpuBackend = NULL;
        }
    }

    static void InvalidateGpuBackendState(ParticleSystem3D *system)
    {
        if (system->gpuBackend != NULL) {
            MetalGpuBackend3D *backend = (__bridge MetalGpuBackend3D *)system->gpuBackend;
            [backend invalidateGridBindings];
        }
    }

    static bool RunGpuDensityForce(ParticleSystem3D *system, bool includeForce)
    {
        if (!InitializeGpuBackend(system)) {
            return false;
        }

        MetalGpuBackend3D *backend = (__bridge MetalGpuBackend3D *)system->gpuBackend;
        NSString *error = nil;
        if (![backend runBuildDensityForceForSystem:system includeForce:includeForce error:&error]) {
            if (error != nil) {
                fprintf(stderr, "3D GPU refresh failed: %s\n", [error UTF8String]);
            }
            return false;
        }
        return true;
    }

    static bool RunGpuStep(ParticleSystem3D *system, float dt)
    {
        if (!InitializeGpuBackend(system)) {
            return false;
        }

        MetalGpuBackend3D *backend = (__bridge MetalGpuBackend3D *)system->gpuBackend;
        NSString *error = nil;
        if (![backend runStepForSystem:system dt:dt error:&error]) {
            if (error != nil) {
                fprintf(stderr, "3D GPU step failed: %s\n", [error UTF8String]);
            }
            return false;
        }
        return true;
    }
    #else
    static bool InitializeGpuBackend(ParticleSystem3D *system) { (void)system; return false; }
    static void ShutdownGpuBackend(ParticleSystem3D *system) { (void)system; }
    static void InvalidateGpuBackendState(ParticleSystem3D *system) { (void)system; }
    static bool RunGpuDensityForce(ParticleSystem3D *system, bool includeForce) { (void)system; (void)includeForce; return false; }
    static bool RunGpuStep(ParticleSystem3D *system, float dt) { (void)system; (void)dt; return false; }
    #endif

    static bool StepSimulationGPU(ParticleSystem3D *system, float frameDelta)
    {
        system->accumulator = fminf(system->accumulator + frameDelta * system->timeScale,
            system->params.timeStep * (float)MAX_SIM_STEPS_PER_FRAME);
        const float maxStep = system->params.timeStep;
        const float minStep = maxStep * 0.20f;
        int steps = 0;

        const double startTime = GetTime();
        while (system->accumulator >= minStep && steps < MAX_SIM_STEPS_PER_FRAME) {
            float dt = fminf(maxStep, system->accumulator);
            if (AcousticsActive(system)) {
                const float h = system->params.supportRadius;
                const float speakerSpeed = SpeakerPeakSurfaceSpeed(system);
                const float speakerWaveDt = 0.20f * h / fmaxf(system->params.soundSpeed + speakerSpeed, 1e-4f);
                const float speakerPhaseDt = 1.0f / fmaxf(system->speakerFrequency * 24.0f, 1e-4f);
                const float minDt = system->params.timeStep * 0.05f;
                const float acousticLimit = fminf(dt, fminf(speakerWaveDt, speakerPhaseDt));
                dt = (minDt < acousticLimit) ? ClampFloat(acousticLimit, minDt, maxStep) : acousticLimit;
            }
            system->lastStepDt = dt;
            if (!RunGpuStep(system, dt)) {
                SetBackendNotice(system, "3D GPU step failed. Falling back to CPU.");
                system->activeBackend = SIM_BACKEND_CPU;
                StepSimulationCPU(system, frameDelta);
                return false;
            }

            system->sortedStateDirty = false;
            SampleMicrophone(system);
            system->simulationTime += dt;
            system->accumulator -= dt;
            ++steps;
            system->scalarFieldsDirty = true;
            system->sortedStateDirty = true;
            system->vorticityDirty = true;
        }

        const bool needsFreshDensity = ColorModeNeedsFreshDensity(system->colorMode);
        const bool refreshDiagnostics = needsFreshDensity || system->framesUntilDiagnostics <= 0;

        if (system->scalarFieldsDirty && (needsFreshDensity || refreshDiagnostics)) {
            if (!RunGpuDensityForce(system, false)) {
                SetBackendNotice(system, "3D GPU refresh failed. Falling back to CPU.");
                system->activeBackend = SIM_BACKEND_CPU;
                StepSimulationCPU(system, 0.0f);
                return false;
            }
            system->scalarFieldsDirty = false;
        }

        if (refreshDiagnostics) {
            UpdateDiagnostics(system);
            system->framesUntilDiagnostics = DiagnosticRefreshIntervalForBackend(system->activeBackend);
        } else {
            system->framesUntilDiagnostics -= 1;
        }

        system->lastSimStepMs = (GetTime() - startTime) * 1000.0;
        return true;
    }

    static void StepSimulation(ParticleSystem3D *system, float frameDelta)
    {
        switch (system->activeBackend) {
            case SIM_BACKEND_GPU:
                (void)StepSimulationGPU(system, frameDelta);
                break;
            case SIM_BACKEND_CPU:
            default:
                StepSimulationCPU(system, frameDelta);
                break;
        }
    }

    static void BakeSetNotice(BakeSession3D *bake, const char *message)
    {
        if (message == NULL) {
            bake->notice[0] = '\0';
            return;
        }
        snprintf(bake->notice, sizeof(bake->notice), "%s", message);
    }

    static bool EnsureDirectoryExists(const char *path)
    {
        if (path == NULL || path[0] == '\0') {
            return false;
        }
        if (mkdir(path, 0755) == 0) {
            return true;
        }
        return errno == EEXIST;
    }

    static char gProjectRootPath[BAKE_PATH_MAX] = {0};

    static void StripTrailingSlash(char *path)
    {
        if (path == NULL) {
            return;
        }
        size_t length = strlen(path);
        while (length > 1 && path[length - 1] == '/') {
            path[length - 1] = '\0';
            --length;
        }
    }

    static void ParentDirectoryInPlace(char *path)
    {
        if (path == NULL || path[0] == '\0') {
            return;
        }
        StripTrailingSlash(path);
        char *slash = strrchr(path, '/');
        if (slash == NULL) {
            snprintf(path, BAKE_PATH_MAX, "%s", ".");
        } else if (slash == path) {
            slash[1] = '\0';
        } else {
            *slash = '\0';
        }
    }

    static bool DirectoryLooksLikeProjectRoot(const char *path)
    {
        char marker[BAKE_PATH_MAX];
        snprintf(marker, sizeof(marker), "%s/src/main_3d.c", path);
        if (FileExists(marker)) {
            return true;
        }
        snprintf(marker, sizeof(marker), "%s/Makefile", path);
        return FileExists(marker);
    }

    static bool FindProjectRootFromDirectory(const char *startPath, char *outPath, size_t outPathSize)
    {
        if (startPath == NULL || startPath[0] == '\0') {
            return false;
        }

        char candidate[BAKE_PATH_MAX];
        snprintf(candidate, sizeof(candidate), "%s", startPath);
        StripTrailingSlash(candidate);

        for (int depth = 0; depth < 8; ++depth) {
            if (DirectoryLooksLikeProjectRoot(candidate)) {
                snprintf(outPath, outPathSize, "%s", candidate);
                return true;
            }
            char previous[BAKE_PATH_MAX];
            snprintf(previous, sizeof(previous), "%s", candidate);
            ParentDirectoryInPlace(candidate);
            if (strcmp(previous, candidate) == 0) {
                break;
            }
        }
        return false;
    }

    static const char *ProjectRootPath(void)
    {
        if (gProjectRootPath[0] != '\0') {
            return gProjectRootPath;
        }

    #if defined(__APPLE__)
        char executablePath[BAKE_PATH_MAX];
        uint32_t executablePathSize = (uint32_t)sizeof(executablePath);
        if (_NSGetExecutablePath(executablePath, &executablePathSize) == 0) {
            char resolvedPath[BAKE_PATH_MAX];
            const char *pathForDirectory = executablePath;
            if (realpath(executablePath, resolvedPath) != NULL) {
                pathForDirectory = resolvedPath;
            }
            char executableDirectory[BAKE_PATH_MAX];
            snprintf(executableDirectory, sizeof(executableDirectory), "%s", pathForDirectory);
            ParentDirectoryInPlace(executableDirectory);
            if (FindProjectRootFromDirectory(executableDirectory, gProjectRootPath, sizeof(gProjectRootPath))) {
                return gProjectRootPath;
            }
        }
    #endif

        const char *appDirectory = GetApplicationDirectory();
        if (appDirectory != NULL && appDirectory[0] != '\0' &&
            FindProjectRootFromDirectory(appDirectory, gProjectRootPath, sizeof(gProjectRootPath))) {
            return gProjectRootPath;
        }

        char currentDirectory[BAKE_PATH_MAX];
        if (getcwd(currentDirectory, sizeof(currentDirectory)) != NULL &&
            FindProjectRootFromDirectory(currentDirectory, gProjectRootPath, sizeof(gProjectRootPath))) {
            return gProjectRootPath;
        }

        snprintf(gProjectRootPath, sizeof(gProjectRootPath), "%s", ".");
        return gProjectRootPath;
    }

    static void MakeProjectPath(const char *relativePath, char *outPath, size_t outPathSize)
    {
        if (relativePath == NULL || relativePath[0] == '\0') {
            snprintf(outPath, outPathSize, "%s", ProjectRootPath());
            return;
        }
        if (relativePath[0] == '/') {
            snprintf(outPath, outPathSize, "%s", relativePath);
            return;
        }
        snprintf(outPath, outPathSize, "%s/%s", ProjectRootPath(), relativePath);
    }

    static const char *BakeSceneName(const ParticleSystem3D *system)
    {
        if (SceneIsWindTunnel(system)) {
            return "wind";
        }
        return (system->preset == MATERIAL_GAS) ? "gas" : "water";
    }

    static void BakeMakeCachePath(BakeSession3D *bake, const ParticleSystem3D *system)
    {
        char bakesDirectory[BAKE_PATH_MAX];
        MakeProjectPath("bakes", bakesDirectory, sizeof(bakesDirectory));
        EnsureDirectoryExists(bakesDirectory);

        time_t rawTime = time(NULL);
        struct tm localTime;
        localtime_r(&rawTime, &localTime);
        char stamp[64];
        strftime(stamp, sizeof(stamp), "%Y-%m-%d_%H%M%S", &localTime);

        snprintf(bake->cacheDir, sizeof(bake->cacheDir), "%s/%s_%s", bakesDirectory, BakeSceneName(system), stamp);
        for (int suffix = 1; suffix < 100; ++suffix) {
            if (EnsureDirectoryExists(bake->cacheDir)) {
                break;
            }
            snprintf(bake->cacheDir, sizeof(bake->cacheDir), "%s/%s_%s_%02d",
                bakesDirectory, BakeSceneName(system), stamp, suffix);
        }
        snprintf(bake->latestPath, sizeof(bake->latestPath), "%s/latest.txt", bakesDirectory);
    }

    static void BakeSessionInit(BakeSession3D *bake)
    {
        memset(bake, 0, sizeof(*bake));
        bake->status = BAKE_STATUS_IDLE;
        bake->duration = BAKE_DEFAULT_DURATION_SECONDS;
        bake->particleTarget = BAKE_DEFAULT_PARTICLE_COUNT;
        bake->audioSlowMotion = false;
        bake->audioSlowdownFactor = 1.0f;
        bake->loadedPreviewFrame = -1;
        bake->etaSeconds = 0.0f;
        bake->lastBakedTimeForEta = 0.0f;
        bake->lastEtaWallTime = 0.0;
        BakeSetNotice(bake, "Bake mode is idle.");
    }

    static void BakeSessionFree(BakeSession3D *bake)
    {
        if (bake->previewParticles != NULL) {
            MemFree(bake->previewParticles);
            bake->previewParticles = NULL;
        }
        if (bake->micSamples != NULL) {
            MemFree(bake->micSamples);
            bake->micSamples = NULL;
        }
        bake->previewParticleCapacity = 0;
        bake->micSampleCapacity = 0;
    }

    static bool BakeEnsurePreviewCapacity(BakeSession3D *bake, int count)
    {
        if (count <= bake->previewParticleCapacity) {
            return true;
        }

        BakePreviewParticle3D *next =
            (BakePreviewParticle3D *)MemRealloc(bake->previewParticles, (size_t)count * sizeof(BakePreviewParticle3D));
        if (next == NULL) {
            return false;
        }
        bake->previewParticles = next;
        bake->previewParticleCapacity = count;
        return true;
    }

    static bool BakeAppendMicSample(BakeSession3D *bake, float time, float value)
    {
        if (bake->micSampleCount >= bake->micSampleCapacity) {
            const int nextCapacity = (bake->micSampleCapacity > 0) ? bake->micSampleCapacity * 2 : 4096;
            BakeMicSample3D *next =
                (BakeMicSample3D *)MemRealloc(bake->micSamples, (size_t)nextCapacity * sizeof(BakeMicSample3D));
            if (next == NULL) {
                return false;
            }
            bake->micSamples = next;
            bake->micSampleCapacity = nextCapacity;
        }

        bake->micSamples[bake->micSampleCount++] = (BakeMicSample3D){
            .time = time,
            .value = value,
        };
        bake->canExportMic = true;
        return true;
    }

    static bool BakeWriteManifest(const ParticleSystem3D *system, const BakeSession3D *bake)
    {
        char path[BAKE_PATH_MAX];
        snprintf(path, sizeof(path), "%s/manifest.txt", bake->cacheDir);
        FILE *file = fopen(path, "wb");
        if (file == NULL) {
            return false;
        }

        fprintf(file, "version=%d\n", BAKE_CACHE_VERSION);
        fprintf(file, "scene=%s\n", BakeSceneName(system));
        fprintf(file, "duration=%.6f\n", bake->duration);
        fprintf(file, "baked_time=%.6f\n", bake->bakedTime);
        fprintf(file, "particle_target=%d\n", bake->particleTarget);
        fprintf(file, "preview_fps=%d\n", BAKE_PREVIEW_FPS);
        fprintf(file, "preview_cap=%d\n", BAKE_PREVIEW_PARTICLE_CAP);
        fprintf(file, "preview_frames=%d\n", bake->previewFrameCount);
        fprintf(file, "keyframes=%d\n", bake->keyframeCount);
        fprintf(file, "keyframe_interval=%.6f\n", BAKE_KEYFRAME_INTERVAL_SECONDS);
        fprintf(file, "color_mode=%d\n", system->colorMode);
        fprintf(file, "view_mode=%d\n", system->viewMode);
        fprintf(file, "acoustics_enabled=%d\n", system->acousticsEnabled ? 1 : 0);
        fprintf(file, "audio_path=%s\n", system->acousticAudioLoaded ? system->acousticAudioPath : "");
        fprintf(file, "audio_duration=%.6f\n", system->acousticAudioLoaded ? system->acousticEnvelopeDuration : 0.0f);
        fprintf(file, "audio_driver_rate=%.6f\n", system->acousticAudioLoaded ? system->acousticWaveformSampleRate : 0.0f);
        fprintf(file, "audio_bandwidth_target=%.6f\n", system->acousticAudioLoaded ? AcousticAudioTargetBandwidthHz(system) : 0.0f);
        fprintf(file, "audio_slow_motion=%d\n", bake->audioSlowMotion ? 1 : 0);
        fprintf(file, "audio_slowdown=%.6f\n", bake->audioSlowdownFactor);
        fprintf(file, "audio_driver_substeps=%.6f\n", ACOUSTIC_AUDIO_BAKE_SUBSTEPS_PER_SAMPLE);
        fprintf(file, "audio_preroll=%.6f\n", ACOUSTIC_AUDIO_PREROLL_SECONDS);
        fprintf(file, "audio_postroll=%.6f\n", ACOUSTIC_AUDIO_POSTROLL_SECONDS);
        fprintf(file, "imported_obj=%s\n", system->importedObstacleLoaded ? system->importedObstaclePath : "");
        fprintf(file, "imported_sdf_resolution=%d %d %d\n",
            system->importedSdfWidth, system->importedSdfHeight, system->importedSdfDepth);
        fprintf(file, "imported_sdf_bake_quality=%d\n", system->importedSdfBakeQuality ? 1 : 0);
        fclose(file);

        FILE *latest = fopen(bake->latestPath, "wb");
        if (latest != NULL) {
            fprintf(latest, "%s\n", bake->cacheDir);
            fclose(latest);
        }
        return true;
    }

    static bool BakeWritePreviewFrame(const ParticleSystem3D *system, BakeSession3D *bake, int frameIndex)
    {
        const int particleCount = system->particleCount;
        const int previewCount = ClampInt(particleCount, 0, BAKE_PREVIEW_PARTICLE_CAP);
        if (previewCount <= 0 || !BakeEnsurePreviewCapacity(bake, previewCount)) {
            return false;
        }

        const int stride = fmaxf(1.0f, ceilf((float)particleCount / (float)previewCount));
        for (int i = 0; i < previewCount; ++i) {
            const int source = ClampInt(i * stride, 0, particleCount - 1);
            bake->previewParticles[i] = (BakePreviewParticle3D){
                .x = system->x[source],
                .y = system->y[source],
                .z = system->z[source],
                .vx = system->vx[source],
                .vy = system->vy[source],
                .vz = system->vz[source],
                .temperature = system->temperature[source],
                .density = system->density[source],
                .pressure = system->pressure[source],
                .dye = system->dye[source],
                .vorticity = system->vorticity[source],
            };
        }

        char path[BAKE_PATH_MAX];
        snprintf(path, sizeof(path), "%s/preview_%05d.bin", bake->cacheDir, frameIndex);
        FILE *file = fopen(path, "wb");
        if (file == NULL) {
            return false;
        }

        const uint32_t magic = 0x56505346u;
        const uint32_t version = BAKE_CACHE_VERSION;
        const float time = (float)frameIndex / (float)BAKE_PREVIEW_FPS;
        fwrite(&magic, sizeof(magic), 1, file);
        fwrite(&version, sizeof(version), 1, file);
        fwrite(&previewCount, sizeof(previewCount), 1, file);
        fwrite(&time, sizeof(time), 1, file);
        fwrite(bake->previewParticles, sizeof(BakePreviewParticle3D), (size_t)previewCount, file);
        fclose(file);

        bake->previewParticleCount = previewCount;
        bake->previewFrameCount = fmaxf(bake->previewFrameCount, frameIndex + 1);
        bake->loadedPreviewFrame = frameIndex;
        bake->hasCache = true;
        return true;
    }

    static bool BakeWriteKeyframe(const ParticleSystem3D *system, BakeSession3D *bake, int keyframeIndex)
    {
        char path[BAKE_PATH_MAX];
        snprintf(path, sizeof(path), "%s/key_%04d.bin", bake->cacheDir, keyframeIndex);
        FILE *file = fopen(path, "wb");
        if (file == NULL) {
            return false;
        }

        const uint32_t magic = 0x59454b46u;
        const uint32_t version = BAKE_CACHE_VERSION;
        const int count = system->particleCount;
        fwrite(&magic, sizeof(magic), 1, file);
        fwrite(&version, sizeof(version), 1, file);
        fwrite(&count, sizeof(count), 1, file);
        fwrite(&system->simulationTime, sizeof(system->simulationTime), 1, file);
        fwrite(system->x, sizeof(float), (size_t)count, file);
        fwrite(system->y, sizeof(float), (size_t)count, file);
        fwrite(system->z, sizeof(float), (size_t)count, file);
        fwrite(system->vx, sizeof(float), (size_t)count, file);
        fwrite(system->vy, sizeof(float), (size_t)count, file);
        fwrite(system->vz, sizeof(float), (size_t)count, file);
        fwrite(system->temperature, sizeof(float), (size_t)count, file);
        fwrite(system->dye, sizeof(float), (size_t)count, file);
        fclose(file);

        bake->keyframeCount = fmaxf(bake->keyframeCount, keyframeIndex + 1);
        return true;
    }

    static bool BakeLoadPreviewFrame(ParticleSystem3D *system, BakeSession3D *bake, int frameIndex)
    {
        if (frameIndex < 0 || frameIndex >= bake->previewFrameCount) {
            return false;
        }
        if (frameIndex == bake->loadedPreviewFrame && bake->previewParticleCount > 0) {
            return true;
        }

        char path[BAKE_PATH_MAX];
        snprintf(path, sizeof(path), "%s/preview_%05d.bin", bake->cacheDir, frameIndex);
        FILE *file = fopen(path, "rb");
        if (file == NULL) {
            BakeSetNotice(bake, "Preview frame missing from cache.");
            return false;
        }

        uint32_t magic = 0;
        uint32_t version = 0;
        int count = 0;
        float time = 0.0f;
        fread(&magic, sizeof(magic), 1, file);
        fread(&version, sizeof(version), 1, file);
        fread(&count, sizeof(count), 1, file);
        fread(&time, sizeof(time), 1, file);
        if (magic != 0x56505346u || version != BAKE_CACHE_VERSION || count <= 0) {
            fclose(file);
            BakeSetNotice(bake, "Preview cache version mismatch.");
            return false;
        }
        if (count > system->maxParticles && !ResizeSystemCapacity(system, count)) {
            fclose(file);
            BakeSetNotice(bake, "Preview particle buffer allocation failed.");
            return false;
        }

        if (!BakeEnsurePreviewCapacity(bake, count)) {
            fclose(file);
            BakeSetNotice(bake, "Preview allocation failed.");
            return false;
        }

        const size_t readCount = fread(bake->previewParticles, sizeof(BakePreviewParticle3D), (size_t)count, file);
        fclose(file);
        if (readCount != (size_t)count) {
            BakeSetNotice(bake, "Preview frame read failed.");
            return false;
        }

        for (int i = 0; i < count; ++i) {
            const BakePreviewParticle3D p = bake->previewParticles[i];
            system->x[i] = p.x;
            system->y[i] = p.y;
            system->z[i] = p.z;
            system->vx[i] = p.vx;
            system->vy[i] = p.vy;
            system->vz[i] = p.vz;
            system->density[i] = p.density;
            system->pressure[i] = p.pressure;
            system->temperature[i] = p.temperature;
            system->dye[i] = p.dye;
            system->vorticity[i] = p.vorticity;
        }
        system->particleCount = count;
        system->scalarFieldsDirty = true;
        system->sortedStateDirty = true;
        system->vorticityDirty = true;
        system->playbackPreviewMode = true;
        system->playbackPreviewSourceCount = (bake->particleTarget > count) ? bake->particleTarget : count;
        bake->previewParticleCount = count;
        bake->loadedPreviewFrame = frameIndex;
        return true;
    }

    static float BakeSampleMicTimelineCursor(const BakeSession3D *bake, float time, int *cursor)
    {
        if (bake->micSampleCount <= 0 || bake->micSamples == NULL) {
            return 0.0f;
        }
        if (time <= bake->micSamples[0].time) {
            if (cursor != NULL) {
                *cursor = 1;
            }
            return bake->micSamples[0].value;
        }

        int hi = (cursor != NULL) ? ClampInt(*cursor, 1, bake->micSampleCount - 1) : 1;
        while (hi < bake->micSampleCount && bake->micSamples[hi].time < time) {
            ++hi;
        }
        if (cursor != NULL) {
            *cursor = hi;
        }
        if (hi >= bake->micSampleCount) {
            return bake->micSamples[bake->micSampleCount - 1].value;
        }

        const BakeMicSample3D a = bake->micSamples[hi - 1];
        const BakeMicSample3D b = bake->micSamples[hi];
        const float span = fmaxf(b.time - a.time, 1e-6f);
        const float t = ClampFloat((time - a.time) / span, 0.0f, 1.0f);
        return a.value + (b.value - a.value) * t;
    }

    static bool BakeWritePcm16MonoWav(const char *path, const short *pcm, int frameCount)
    {
        if (path == NULL || pcm == NULL || frameCount <= 0) {
            return false;
        }
        Wave wave = {
            .frameCount = (unsigned int)frameCount,
            .sampleRate = BAKE_MIC_EXPORT_SAMPLE_RATE,
            .sampleSize = 16,
            .channels = 1,
            .data = (void *)pcm,
        };
        return ExportWave(wave, path);
    }

    static bool BakeExportMicWav(const ParticleSystem3D *system, BakeSession3D *bake)
    {
        if (!bake->canExportMic || bake->micSampleCount <= 1) {
            BakeSetNotice(bake, "No bake mic signal to export yet.");
            return false;
        }

        char path[BAKE_PATH_MAX];
        char exportDirectory[BAKE_PATH_MAX];
        MakeProjectPath("export_sound", exportDirectory, sizeof(exportDirectory));
        if (!EnsureDirectoryExists(exportDirectory)) {
            BakeSetNotice(bake, "Could not create export_sound folder.");
            return false;
        }
        time_t rawTime = time(NULL);
        struct tm localTime;
        localtime_r(&rawTime, &localTime);
        char stamp[64];
        strftime(stamp, sizeof(stamp), "%Y-%m-%d_%H%M%S", &localTime);
        const bool slowAudio = system != NULL && AcousticSlowMotionAudioActive(system) && bake->audioSlowMotion;
        const float exportDuration = slowAudio
            ? fmaxf(system->acousticEnvelopeDuration, 0.05f)
            : fmaxf(bake->bakedTime, 0.05f);
        const int frameCount = ClampInt((int)ceilf(exportDuration * (float)BAKE_MIC_EXPORT_SAMPLE_RATE),
            1, INT_MAX / (int)sizeof(short));
        short *pcm = (short *)MemAlloc((size_t)frameCount * sizeof(short));
        float *samples = (float *)MemAlloc((size_t)frameCount * sizeof(float));
        if (pcm == NULL || samples == NULL) {
            if (pcm != NULL) {
                MemFree(pcm);
            }
            if (samples != NULL) {
                MemFree(samples);
            }
            BakeSetNotice(bake, "Mic WAV allocation failed.");
            return false;
        }

        const float slowdown = slowAudio ? fmaxf(bake->audioSlowdownFactor, 1.0f) : 1.0f;
        const float propagationDelay = slowAudio ? AcousticSpeakerMicDelaySeconds(system) : 0.0f;
        float peak = 1e-6f;
        int micCursor = 1;
        for (int i = 0; i < frameCount; ++i) {
            const float audioTime = (float)i / (float)BAKE_MIC_EXPORT_SAMPLE_RATE;
            const float sampleTime = slowAudio
                ? (ACOUSTIC_AUDIO_PREROLL_SECONDS + audioTime * slowdown + propagationDelay)
                : audioTime;
            const float value = ClampFloat(BakeSampleMicTimelineCursor(bake, sampleTime, &micCursor), -1.0f, 1.0f);
            samples[i] = value;
            peak = fmaxf(peak, fabsf(value));
        }

        const float normalize = (peak > 1e-5f) ? fminf(0.95f / peak, 18.0f) : 1.0f;
        for (int i = 0; i < frameCount; ++i) {
            pcm[i] = (short)lrintf(ClampFloat(samples[i] * normalize, -1.0f, 1.0f) * 32767.0f);
        }

        snprintf(path, sizeof(path), slowAudio ? "%s/simulated_audio_%s.wav" : "%s/mic_export_%s.wav",
            exportDirectory, stamp);
        const bool ok = BakeWritePcm16MonoWav(path, pcm, frameCount);
        if (ok) {
            if (slowAudio) {
                snprintf(bake->notice, sizeof(bake->notice),
                    "Exported simulated audio %s. Time-compressed %.0fx from the SPH mic signal.",
                    path, slowdown);
            } else {
                snprintf(bake->notice, sizeof(bake->notice), "Exported %s", path);
            }
        } else {
            BakeSetNotice(bake, "Mic WAV export failed.");
        }
        MemFree(samples);
        MemFree(pcm);
        return ok;
    }

    static bool BakeGpuStepOnce(ParticleSystem3D *system)
    {
        float dt = system->params.timeStep;
        if (AcousticsActive(system)) {
            const float h = system->params.supportRadius;
            const float speakerSpeed = SpeakerPeakSurfaceSpeed(system);
            const float speakerWaveDt = 0.20f * h / fmaxf(system->params.soundSpeed + speakerSpeed, 1e-4f);
            const float speakerPhaseDt = 1.0f / fmaxf(system->speakerFrequency * 24.0f, 1e-4f);
            float acousticDt = fminf(speakerWaveDt, speakerPhaseDt);
            const bool audioDriverActive = system->acousticAudioLoaded &&
                system->acousticWaveformSampleRate > 0.0f &&
                system->simulationTime >= ACOUSTIC_AUDIO_PREROLL_SECONDS;
            if (system->acousticAudioLoaded && !audioDriverActive) {
                acousticDt = fminf(acousticDt,
                    fmaxf(ACOUSTIC_AUDIO_PREROLL_SECONDS - system->simulationTime, 1e-6f));
            }
            if (audioDriverActive) {
                const float slowdown = AcousticAudioSlowdownFactor(system);
                acousticDt = fminf(acousticDt,
                    slowdown / (system->acousticWaveformSampleRate * ACOUSTIC_AUDIO_BAKE_SUBSTEPS_PER_SAMPLE));
            }
            const float minDt = audioDriverActive
                ? (AcousticAudioSlowdownFactor(system) /
                    (system->acousticWaveformSampleRate * ACOUSTIC_AUDIO_BAKE_MIN_SUBSTEPS_PER_SAMPLE))
                : (system->params.timeStep * 0.05f);
            const float acousticLimit = fminf(dt, acousticDt);
            dt = (minDt < acousticLimit) ? ClampFloat(acousticLimit, minDt, system->params.timeStep) : acousticLimit;
        }

        system->lastStepDt = dt;
        if (!RunGpuStep(system, dt)) {
            return false;
        }

        system->sortedStateDirty = false;
        SampleMicrophone(system);
        system->simulationTime += dt;
        system->scalarFieldsDirty = true;
        system->sortedStateDirty = true;
        system->vorticityDirty = true;
        return true;
    }

    static void BakeStart(ParticleSystem3D *system, BakeSession3D *bake)
    {
        if (system->acousticAudioLoaded) {
            if (!AcousticsAvailable(system)) {
                system->scene = SCENE_TANK;
                system->tankPreset = MATERIAL_GAS;
                system->preset = MATERIAL_GAS;
            }
            system->acousticsEnabled = true;
        }
        const bool slowAudioBake = AcousticsAvailable(system) &&
            AcousticSlowMotionAudioActive(system) &&
            system->acousticEnvelopeDuration > 0.0f;
        const int requestedTarget = ClampInt(bake->particleTarget, 1, BAKE_MAX_PARTICLE_COUNT);
        const float audioSlowdown = slowAudioBake
            ? AcousticBakeSlowdownForTarget(system, requestedTarget)
            : AcousticAudioSlowdownFactor(system);
        if (slowAudioBake) {
            system->acousticAudioSlowdownFactor = audioSlowdown;
        }
        float requestedDuration = ClampFloat(bake->duration, BAKE_MIN_DURATION_SECONDS, BAKE_MAX_DURATION_SECONDS);
        if (slowAudioBake) {
            requestedDuration = fmaxf(BAKE_MIN_DURATION_SECONDS,
                system->acousticEnvelopeDuration * audioSlowdown +
                ACOUSTIC_AUDIO_PREROLL_SECONDS + ACOUSTIC_AUDIO_POSTROLL_SECONDS);
        }
        BakeSessionFree(bake);
        BakeSessionInit(bake);
        bake->duration = requestedDuration;
        bake->particleTarget = requestedTarget;
        bake->audioSlowMotion = slowAudioBake;
        bake->audioSlowdownFactor = slowAudioBake ? audioSlowdown : 1.0f;

        if (!ResizeSystemCapacity(system, requestedTarget)) {
            bake->status = BAKE_STATUS_STOPPED;
            BakeSetNotice(bake, "Bake particle buffer allocation failed.");
            return;
        }

        if (!InitializeGpuBackend(system)) {
            bake->status = BAKE_STATUS_STOPPED;
            BakeSetNotice(bake, "Bake requires Metal GPU initialization; bake not started.");
            return;
        }

        BakeMakeCachePath(bake, system);
        if (!EnsureDirectoryExists(bake->cacheDir)) {
            bake->status = BAKE_STATUS_STOPPED;
            BakeSetNotice(bake, "Could not create bake cache directory.");
            return;
        }

        system->activeBackend = SIM_BACKEND_GPU;
        system->bakeCapacityMode = true;
        system->targetParticleCount = requestedTarget;
        system->paused = false;
        system->accumulator = 0.0f;
        if (!UpgradeImportedObstacleSdfForBake(system)) {
            bake->status = BAKE_STATUS_STOPPED;
            system->bakeCapacityMode = false;
            BakeSetNotice(bake, "Bake OBJ SDF rebuild failed; bake not started.");
            return;
        }
        ResetSimulation(system, system->preset);

        bake->status = BAKE_STATUS_BAKING;
        bake->startSimulationTime = system->simulationTime;
        bake->bakedTime = 0.0f;
        bake->progress = 0.0f;
        bake->playbackTime = 0.0f;
        bake->playbackPlaying = false;
        bake->wallStartTime = GetTime();
        bake->etaSeconds = 0.0f;
        bake->lastBakedTimeForEta = 0.0f;
        bake->lastEtaWallTime = bake->wallStartTime;
        bake->nextPreviewFrame = 1;
        bake->nextKeyframeIndex = 1;
        BakeWritePreviewFrame(system, bake, 0);
        BakeWriteKeyframe(system, bake, 0);
        BakeWriteManifest(system, bake);
        if (system->acousticAudioLoaded && AcousticsAvailable(system)) {
            snprintf(bake->notice, sizeof(bake->notice),
                "Slow-motion SPH audio bake: %.0fx slowdown, %.1fs audio -> %.1fs sim. Target %.0f Hz; restored estimate %.0f Hz.",
                bake->audioSlowdownFactor,
                system->acousticEnvelopeDuration,
                bake->duration,
                AcousticAudioTargetBandwidthHz(system),
                AcousticResolvedFrequencyEstimateForBakeTarget(system, bake->particleTarget) * bake->audioSlowdownFactor);
        } else {
            if (system->importedObstacleLoaded && system->importedSdfBakeQuality) {
                snprintf(bake->notice, sizeof(bake->notice),
                    "Baking %d / %d particles with bake OBJ SDF %dx%dx%d. Preview is capped at %d.",
                    system->particleCount, bake->particleTarget,
                    system->importedSdfWidth, system->importedSdfHeight, system->importedSdfDepth,
                    BAKE_PREVIEW_PARTICLE_CAP);
            } else {
                snprintf(bake->notice, sizeof(bake->notice),
                    "Baking %d / %d simulated particles on Metal GPU. Playback preview is capped at %d.",
                    system->particleCount, bake->particleTarget, BAKE_PREVIEW_PARTICLE_CAP);
            }
        }
    }

    static void BakeStop(BakeSession3D *bake)
    {
        if (bake->status == BAKE_STATUS_BAKING) {
            bake->status = BAKE_STATUS_STOPPED;
            bake->playbackPlaying = false;
            BakeSetNotice(bake, "Bake stopped. Cached frames remain playable.");
        }
    }

    static void BakeUpdate(ParticleSystem3D *system, BakeSession3D *bake)
    {
        if (bake->status != BAKE_STATUS_BAKING) {
            return;
        }

        const double start = GetTime();
        int steps = 0;
        while (bake->bakedTime < bake->duration && (GetTime() - start) < BAKE_CHUNK_BUDGET_SECONDS) {
            if (!BakeGpuStepOnce(system)) {
                bake->status = BAKE_STATUS_STOPPED;
                bake->playbackPlaying = false;
                system->bakeCapacityMode = false;
                BakeSetNotice(bake, "Metal bake step failed; bake stopped without CPU fallback.");
                break;
            }

            bake->bakedTime = fmaxf(0.0f, system->simulationTime - bake->startSimulationTime);
            bake->progress = ClampFloat(bake->bakedTime / fmaxf(bake->duration, 1e-4f), 0.0f, 1.0f);
            if (AcousticsActive(system)) {
                BakeAppendMicSample(bake, bake->bakedTime, system->micSignal);
            }

            const float cacheTimeline = bake->audioSlowMotion
                ? (bake->bakedTime / fmaxf(bake->audioSlowdownFactor, 1.0f))
                : bake->bakedTime;
            while (bake->nextPreviewFrame <= (int)floorf(cacheTimeline * (float)BAKE_PREVIEW_FPS)) {
                if (!BakeWritePreviewFrame(system, bake, bake->nextPreviewFrame)) {
                    bake->status = BAKE_STATUS_STOPPED;
                    system->bakeCapacityMode = false;
                    BakeSetNotice(bake, "Preview cache write failed; bake stopped.");
                    break;
                }
                ++bake->nextPreviewFrame;
            }

            while (bake->nextKeyframeIndex <= (int)floorf(cacheTimeline / BAKE_KEYFRAME_INTERVAL_SECONDS)) {
                if (!BakeWriteKeyframe(system, bake, bake->nextKeyframeIndex)) {
                    bake->status = BAKE_STATUS_STOPPED;
                    system->bakeCapacityMode = false;
                    BakeSetNotice(bake, "Keyframe cache write failed; bake stopped.");
                    break;
                }
                ++bake->nextKeyframeIndex;
            }

            if (bake->status != BAKE_STATUS_BAKING) {
                break;
            }
            ++steps;
        }

        bake->lastChunkMs = (GetTime() - start) * 1000.0;
        if (bake->status == BAKE_STATUS_BAKING && bake->progress > 0.001f) {
            const double now = GetTime();
            const double wallDelta = fmax(now - bake->lastEtaWallTime, 1e-4);
            const float simDelta = bake->bakedTime - bake->lastBakedTimeForEta;
            if (simDelta > 1e-6f) {
                const float localRate = simDelta / (float)wallDelta;
                const float localEta = (bake->duration - bake->bakedTime) / fmaxf(localRate, 1e-6f);
                const float smoothing = (bake->etaSeconds > 0.0f) ? 0.10f : 1.0f;
                bake->etaSeconds = bake->etaSeconds + (localEta - bake->etaSeconds) * smoothing;
                bake->lastBakedTimeForEta = bake->bakedTime;
                bake->lastEtaWallTime = now;
            }
        }
        if (bake->status == BAKE_STATUS_BAKING && bake->bakedTime >= bake->duration) {
            bake->status = BAKE_STATUS_COMPLETE;
            bake->bakedTime = bake->duration;
            bake->progress = 1.0f;
            bake->etaSeconds = 0.0f;
            bake->playbackPlaying = false;
            system->bakeCapacityMode = false;
            BakeSetNotice(bake, "Bake complete. Use the bottom timeline to play or scrub.");
        }
        BakeWriteManifest(system, bake);
        (void)steps;
    }

    static bool BakeSetPlaybackTime(ParticleSystem3D *system, BakeSession3D *bake, float playbackTime)
    {
        if (!bake->hasCache || bake->previewFrameCount <= 0) {
            return false;
        }

        bake->playbackTime = ClampFloat(playbackTime, 0.0f, fmaxf(bake->bakedTime, 0.0f));
        const float cacheTimeline = bake->audioSlowMotion
            ? (bake->playbackTime / fmaxf(bake->audioSlowdownFactor, 1.0f))
            : bake->playbackTime;
        const int frameIndex = ClampInt((int)lrintf(cacheTimeline * (float)BAKE_PREVIEW_FPS), 0, bake->previewFrameCount - 1);
        if (BakeLoadPreviewFrame(system, bake, frameIndex)) {
            if (bake->status != BAKE_STATUS_BAKING) {
                bake->status = BAKE_STATUS_PLAYBACK;
            }
            return true;
        }
        return false;
    }

    static void BakePlaybackUpdate(ParticleSystem3D *system, BakeSession3D *bake, float frameDelta)
    {
        if (!bake->playbackPlaying || !bake->hasCache || bake->status == BAKE_STATUS_BAKING) {
            return;
        }

        const float nextTime = bake->playbackTime + frameDelta;
        if (nextTime >= bake->bakedTime) {
            bake->playbackPlaying = false;
            BakeSetPlaybackTime(system, bake, bake->bakedTime);
            return;
        }
        BakeSetPlaybackTime(system, bake, nextTime);
    }

    static bool BakeLoadLatest(ParticleSystem3D *system, BakeSession3D *bake)
    {
        char latestPath[BAKE_PATH_MAX];
        MakeProjectPath("bakes/latest.txt", latestPath, sizeof(latestPath));
        FILE *latest = fopen(latestPath, "rb");
        if (latest == NULL) {
            BakeSetNotice(bake, "No latest bake cache found.");
            return false;
        }

        char cacheDir[BAKE_PATH_MAX];
        if (fgets(cacheDir, sizeof(cacheDir), latest) == NULL) {
            fclose(latest);
            BakeSetNotice(bake, "Latest bake cache path is empty.");
            return false;
        }
        fclose(latest);
        cacheDir[strcspn(cacheDir, "\r\n")] = '\0';

        char manifestPath[BAKE_PATH_MAX];
        snprintf(manifestPath, sizeof(manifestPath), "%s/manifest.txt", cacheDir);
        FILE *manifest = fopen(manifestPath, "rb");
        if (manifest == NULL) {
            BakeSetNotice(bake, "Latest bake manifest missing.");
            return false;
        }

        float duration = BAKE_DEFAULT_DURATION_SECONDS;
        float bakedTime = 0.0f;
        int target = BAKE_DEFAULT_PARTICLE_COUNT;
        int previewFrames = 0;
        int keyframes = 0;
        char line[512];
        while (fgets(line, sizeof(line), manifest) != NULL) {
            if (sscanf(line, "duration=%f", &duration) == 1) continue;
            if (sscanf(line, "baked_time=%f", &bakedTime) == 1) continue;
            if (sscanf(line, "particle_target=%d", &target) == 1) continue;
            if (sscanf(line, "preview_frames=%d", &previewFrames) == 1) continue;
            if (sscanf(line, "keyframes=%d", &keyframes) == 1) continue;
        }
        fclose(manifest);

        BakeSessionFree(bake);
        BakeSessionInit(bake);
        snprintf(bake->cacheDir, sizeof(bake->cacheDir), "%s", cacheDir);
        snprintf(bake->latestPath, sizeof(bake->latestPath), "%s", latestPath);
        bake->duration = fmaxf(duration, BAKE_MIN_DURATION_SECONDS);
        bake->bakedTime = ClampFloat(bakedTime, 0.0f, bake->duration);
        bake->particleTarget = ClampInt(target, 1, BAKE_MAX_PARTICLE_COUNT);
        bake->previewFrameCount = previewFrames;
        bake->keyframeCount = keyframes;
        bake->progress = (bake->duration > 0.0f) ? ClampFloat(bake->bakedTime / bake->duration, 0.0f, 1.0f) : 0.0f;
        bake->status = BAKE_STATUS_PLAYBACK;
        bake->hasCache = previewFrames > 0;
        bake->canExportMic = false;
        BakeSetNotice(bake, "Loaded latest bake preview cache.");
        return BakeSetPlaybackTime(system, bake, 0.0f);
    }

    static void ResetCamera(OrbitCameraState *orbit, Camera3D *camera, const ParticleSystem3D *system)
    {
        orbit->target = system->boundsCenter;
        orbit->yaw = -0.78f;
        orbit->pitch = 0.38f;
        orbit->distance = fmaxf(system->boundsSize.x, system->boundsSize.z) * 1.20f;
        orbit->flyMode = false;
        camera->target = orbit->target;
        camera->position = (Vector3){
            orbit->target.x + cosf(orbit->yaw) * cosf(orbit->pitch) * orbit->distance,
            orbit->target.y + sinf(orbit->pitch) * orbit->distance,
            orbit->target.z + sinf(orbit->yaw) * cosf(orbit->pitch) * orbit->distance,
        };
        camera->up = (Vector3){0.0f, 1.0f, 0.0f};
        camera->fovy = 45.0f;
        camera->projection = CAMERA_PERSPECTIVE;
        orbit->flyPosition = camera->position;
        orbit->flyYaw = orbit->yaw + PI_F;
        orbit->flyPitch = -orbit->pitch;
    }

    static Vector3 FlyCameraForward(const OrbitCameraState *orbit)
    {
        return Vector3Normalize((Vector3){
            cosf(orbit->flyYaw) * cosf(orbit->flyPitch),
            sinf(orbit->flyPitch),
            sinf(orbit->flyYaw) * cosf(orbit->flyPitch),
        });
    }

    static void SetFlyCameraMode(OrbitCameraState *orbit, Camera3D *camera, const ParticleSystem3D *system, bool flyMode)
    {
        if (orbit->flyMode == flyMode) {
            return;
        }

        orbit->flyMode = flyMode;
        if (flyMode) {
            const Vector3 forward = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
            orbit->flyPosition = camera->position;
            orbit->flyPitch = asinf(ClampFloat(forward.y, -0.98f, 0.98f));
            orbit->flyYaw = atan2f(forward.z, forward.x);
        } else {
            orbit->target = system->boundsCenter;
            const Vector3 offset = Vector3Subtract(camera->position, orbit->target);
            orbit->distance = ClampFloat(Vector3Length(offset), 10.0f, fmaxf(system->boundsSize.x, system->boundsSize.z) * 3.10f);
            if (orbit->distance > 1e-4f) {
                orbit->pitch = asinf(ClampFloat(offset.y / orbit->distance, -0.98f, 0.98f));
                orbit->yaw = atan2f(offset.z, offset.x);
            }
        }
    }

    static void UpdateOrbitCamera(OrbitCameraState *orbit, Camera3D *camera, const ParticleSystem3D *system,
        float frameDelta, bool allowMouseInput, bool allowKeyboardInput)
    {
        if (orbit->flyMode) {
            if (allowMouseInput && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                const Vector2 delta = GetMouseDelta();
                orbit->flyYaw -= delta.x * 0.0045f;
                orbit->flyPitch += delta.y * 0.0040f;
            }
            orbit->flyPitch = ClampFloat(orbit->flyPitch, -1.35f, 1.35f);

            const Vector3 forward = FlyCameraForward(orbit);
            const Vector3 worldUp = {0.0f, 1.0f, 0.0f};
            Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, worldUp));
            if (Vector3LengthSqr(right) < 1e-6f) {
                right = (Vector3){1.0f, 0.0f, 0.0f};
            }
            Vector3 move = {0.0f, 0.0f, 0.0f};
            if (allowKeyboardInput) {
                if (IsKeyDown(KEY_W)) move = Vector3Add(move, forward);
                if (IsKeyDown(KEY_S)) move = Vector3Subtract(move, forward);
                if (IsKeyDown(KEY_D)) move = Vector3Add(move, right);
                if (IsKeyDown(KEY_A)) move = Vector3Subtract(move, right);
                if (IsKeyDown(KEY_E)) move = Vector3Add(move, worldUp);
                if (IsKeyDown(KEY_Q)) move = Vector3Subtract(move, worldUp);
            }
            if (Vector3LengthSqr(move) > 1e-6f) {
                float speed = fmaxf(system->boundsSize.x, system->boundsSize.z) * 0.55f;
                if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                    speed *= 2.6f;
                }
                if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) {
                    speed *= 0.30f;
                }
                orbit->flyPosition = Vector3Add(orbit->flyPosition,
                    Vector3Scale(Vector3Normalize(move), speed * fmaxf(frameDelta, 0.0f)));
            }

            camera->position = orbit->flyPosition;
            camera->target = Vector3Add(orbit->flyPosition, forward);
            camera->up = worldUp;
            return;
        }

        if (allowMouseInput && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            const Vector2 delta = GetMouseDelta();
            orbit->yaw -= delta.x * 0.0045f;
            orbit->pitch += delta.y * 0.0040f;
        }

        orbit->pitch = ClampFloat(orbit->pitch, 0.10f, 1.24f);
        if (allowMouseInput) {
            const float wheel = GetMouseWheelMove();
            if (fabsf(wheel) > 1e-4f) {
                orbit->distance *= (wheel > 0.0f) ? 0.90f : 1.10f;
            }
        }

        const float minDistance = fmaxf(system->boundsSize.y, 32.0f) * 0.62f;
        const float maxDistance = fmaxf(system->boundsSize.x, system->boundsSize.z) * 3.10f;
        orbit->distance = ClampFloat(orbit->distance, minDistance, maxDistance);

        camera->target = orbit->target;
        camera->position = (Vector3){
            orbit->target.x + cosf(orbit->yaw) * cosf(orbit->pitch) * orbit->distance,
            orbit->target.y + sinf(orbit->pitch) * orbit->distance,
            orbit->target.z + sinf(orbit->yaw) * cosf(orbit->pitch) * orbit->distance,
        };
    }

    static void UpdateInteraction(ParticleSystem3D *system, Camera3D camera, bool allowMouseInput)
    {
        system->interactionActive = false;
        if (!allowMouseInput || !IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            return;
        }

        const float inset = system->params.particleRadius * 1.5f;
        BoundingBox innerBox = {
            .min = Vector3Add(system->boundsMin, (Vector3){inset, inset, inset}),
            .max = Vector3Subtract(system->boundsMax, (Vector3){inset, inset, inset}),
        };
        const Ray ray = GetMouseRay(GetMousePosition(), camera);
        const RayCollision hit = GetRayCollisionBox(ray, innerBox);
        if (!hit.hit) {
            return;
        }

        const Vector3 point = Vector3Add(hit.point, Vector3Scale(ray.direction, system->params.supportRadius * 2.3f));
        system->interactionPoint = (Vector3){
            ClampFloat(point.x, innerBox.min.x, innerBox.max.x),
            ClampFloat(point.y, innerBox.min.y, innerBox.max.y),
            ClampFloat(point.z, innerBox.min.z, innerBox.max.z),
        };
        system->interactionActive = true;
    }

    static Color ParticleColor(const ParticleSystem3D *system, int index)
    {
        switch (system->colorMode) {
            case COLOR_TEMPERATURE: {
                const float t = RangeLerp(system->stats.minTemperature, system->stats.maxTemperature, system->temperature[index]);
                return ColorRamp(t);
            }
            case COLOR_PRESSURE: {
                const float t = RangeLerp(system->stats.minPressure, system->stats.maxPressure, system->pressure[index]);
                return ColorRamp(t);
            }
            case COLOR_SPEED: {
                const float speed = sqrtf(system->vx[index] * system->vx[index] +
                    system->vy[index] * system->vy[index] + system->vz[index] * system->vz[index]);
                const float t = RangeLerp(system->stats.minSpeed, system->stats.maxSpeed, speed);
                return ColorRamp(t);
            }
            case COLOR_DENSITY: {
                const float t = RangeLerp(system->stats.minDensity, system->stats.maxDensity, system->density[index]);
                return ColorRamp(t);
            }
            case COLOR_DYE: {
                const float t = Saturate(system->dye[index]);
                return ColorFromHSV(210.0f - 210.0f * t, 0.92f, 1.0f);
            }
            case COLOR_VORTICITY: {
                const float t = RangeLerp(system->stats.minVorticity, system->stats.maxVorticity, system->vorticity[index]);
                return ColorFromHSV(250.0f - 250.0f * Saturate(t), 0.95f, 1.0f);
            }
            case COLOR_MATERIAL:
            default:
                return (system->preset == MATERIAL_WATER)
                    ? (Color){64, 168, 255, 230}
                    : (Color){210, 235, 255, 185};
        }
    }

    static int CompareDrawItems(const void *lhs, const void *rhs)
    {
        const ParticleDrawItem *a = (const ParticleDrawItem *)lhs;
        const ParticleDrawItem *b = (const ParticleDrawItem *)rhs;
        if (a->depth < b->depth) {
            return 1;
        }
        if (a->depth > b->depth) {
            return -1;
        }
        return 0;
    }

    static void BuildDrawOrder(ParticleSystem3D *system, Camera3D camera, int drawStride)
    {
        const Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        const Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));
        const float aspect = (float)GetScreenWidth() / (float)fmaxf(GetScreenHeight(), 1);
        const float tanHalfFovY = tanf(camera.fovy * DEG2RAD * 0.5f);
        const float tanHalfFovX = tanHalfFovY * aspect;
        const float radiusMargin = (system->viewMode == VIEW_SMOOTHING_RADIUS)
            ? system->params.supportRadius * 1.35f
            : system->params.particleRadius * 1.8f;
        system->drawItemCount = 0;
        for (int i = 0; i < system->particleCount; ++i) {
            if (!ShouldDrawParticleSample(i, drawStride)) {
                continue;
            }
            const Vector3 delta = Vector3Subtract((Vector3){system->x[i], system->y[i], system->z[i]}, camera.position);
            const float depth = Vector3DotProduct(delta, forward);
            if (depth <= -radiusMargin) {
                continue;
            }

            const float horizontal = fabsf(Vector3DotProduct(delta, right));
            const float vertical = fabsf(Vector3DotProduct(delta, up));
            const float depthMargin = fmaxf(depth, 0.0f) + radiusMargin;
            if (horizontal > depthMargin * tanHalfFovX + radiusMargin ||
                vertical > depthMargin * tanHalfFovY + radiusMargin) {
                continue;
            }
            system->drawItems[system->drawItemCount++] = (ParticleDrawItem){
                .index = i,
                .depth = depth,
            };
        }

        if (system->viewMode == VIEW_PARTICLES && drawStride <= 1 &&
            system->drawItemCount <= PARTICLE_SORT_DRAW_LIMIT) {
            qsort(system->drawItems, (size_t)system->drawItemCount, sizeof(system->drawItems[0]), CompareDrawItems);
        }
    }

    static void CameraBillboardBasis(Camera3D camera, Vector3 *rightOut, Vector3 *upOut)
    {
        const Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        if (Vector3LengthSqr(right) < 1e-6f) {
            right = (Vector3){1.0f, 0.0f, 0.0f};
        }
        Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));
        if (Vector3LengthSqr(up) < 1e-6f) {
            up = camera.up;
        }
        *rightOut = right;
        *upOut = up;
    }

    static void DrawParticleBillboardQuad(Vector3 center, Vector3 right, Vector3 up, float diameter, Color color)
    {
        const Vector3 rightHalf = Vector3Scale(right, diameter * 0.5f);
        const Vector3 upHalf = Vector3Scale(up, diameter * 0.5f);
        const Vector3 bottomLeft = Vector3Subtract(Vector3Subtract(center, rightHalf), upHalf);
        const Vector3 bottomRight = Vector3Subtract(Vector3Add(center, rightHalf), upHalf);
        const Vector3 topRight = Vector3Add(Vector3Add(center, rightHalf), upHalf);
        const Vector3 topLeft = Vector3Add(Vector3Subtract(center, rightHalf), upHalf);

        rlCheckRenderBatchLimit(4);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(0.0f, 1.0f);
        rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
        rlTexCoord2f(1.0f, 1.0f);
        rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
        rlTexCoord2f(1.0f, 0.0f);
        rlVertex3f(topRight.x, topRight.y, topRight.z);
        rlTexCoord2f(0.0f, 0.0f);
        rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
    }

    static Texture2D CreateWhiteTexture(void)
    {
        Image image = GenImageColor(1, 1, WHITE);
        Texture2D texture = LoadTextureFromImage(image);
        UnloadImage(image);
        return texture;
    }

    static Shader LoadParticleShader(int *softPowerLoc)
    {
        static const char *const fragmentShader =
            "#version 330\n"
            "in vec2 fragTexCoord;\n"
            "in vec4 fragColor;\n"
            "uniform vec4 colDiffuse;\n"
            "uniform float uSoftPower;\n"
            "out vec4 finalColor;\n"
            "void main() {\n"
            "    vec2 uv = fragTexCoord * 2.0 - 1.0;\n"
            "    float r2 = dot(uv, uv);\n"
            "    if (r2 >= 1.0) discard;\n"
            "    float radial = max(0.0, 1.0 - r2);\n"
            "    float alpha = pow(radial, uSoftPower);\n"
            "    float shading = 0.82 + 0.18 * sqrt(radial);\n"
            "    vec4 base = fragColor * colDiffuse;\n"
            "    float finalAlpha = base.a * alpha;\n"
            "    finalColor = vec4(base.rgb * shading * finalAlpha, finalAlpha);\n"
            "}\n";

        Shader shader = LoadShaderFromMemory(NULL, fragmentShader);
        *softPowerLoc = GetShaderLocation(shader, "uSoftPower");
        return shader;
    }

    static Shader LoadParticleInstanceShader(ParticleSystem3D *system)
    {
        static const char *const vertexShader =
            "#version 330\n"
            "layout(location = 0) in vec2 vertexPosition;\n"
            "layout(location = 1) in vec2 vertexTexCoord;\n"
            "layout(location = 2) in vec3 instancePosition;\n"
            "layout(location = 3) in vec4 instanceColor;\n"
            "layout(location = 4) in float instanceDiameter;\n"
            "uniform mat4 uMvp;\n"
            "uniform vec3 uCameraRight;\n"
            "uniform vec3 uCameraUp;\n"
            "out vec2 fragTexCoord;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    vec3 world = instancePosition + uCameraRight * vertexPosition.x * instanceDiameter + uCameraUp * vertexPosition.y * instanceDiameter;\n"
            "    gl_Position = uMvp * vec4(world, 1.0);\n"
            "    fragTexCoord = vertexTexCoord;\n"
            "    fragColor = instanceColor;\n"
            "}\n";

        static const char *const fragmentShader =
            "#version 330\n"
            "in vec2 fragTexCoord;\n"
            "in vec4 fragColor;\n"
            "uniform float uSoftPower;\n"
            "out vec4 finalColor;\n"
            "void main() {\n"
            "    vec2 uv = fragTexCoord * 2.0 - 1.0;\n"
            "    float r2 = dot(uv, uv);\n"
            "    if (r2 >= 1.0) discard;\n"
            "    float radial = max(0.0, 1.0 - r2);\n"
            "    float alpha = pow(radial, uSoftPower);\n"
            "    float shading = 0.82 + 0.18 * sqrt(radial);\n"
            "    float finalAlpha = fragColor.a * alpha;\n"
            "    finalColor = vec4(fragColor.rgb * shading * finalAlpha, finalAlpha);\n"
            "}\n";

        Shader shader = LoadShaderFromMemory(vertexShader, fragmentShader);
        system->particleInstanceMvpLoc = GetShaderLocation(shader, "uMvp");
        system->particleInstanceRightLoc = GetShaderLocation(shader, "uCameraRight");
        system->particleInstanceUpLoc = GetShaderLocation(shader, "uCameraUp");
        system->particleInstanceSoftPowerLoc = GetShaderLocation(shader, "uSoftPower");
        return shader;
    }

    static void UnloadParticleInstanceRenderer(ParticleSystem3D *system)
    {
        if (system->particleInstances != NULL) {
            MemFree(system->particleInstances);
            system->particleInstances = NULL;
        }
        system->particleInstanceCapacity = 0;
        if (system->particleInstanceVbo != 0) {
            glDeleteBuffers(1, &system->particleInstanceVbo);
            system->particleInstanceVbo = 0;
        }
        if (system->particleQuadVbo != 0) {
            glDeleteBuffers(1, &system->particleQuadVbo);
            system->particleQuadVbo = 0;
        }
        if (system->particleQuadVao != 0) {
            glDeleteVertexArrays(1, &system->particleQuadVao);
            system->particleQuadVao = 0;
        }
        if (IsShaderValid(system->particleInstanceShader)) {
            UnloadShader(system->particleInstanceShader);
            system->particleInstanceShader = (Shader){0};
        }
    }

    static bool EnsureParticleInstanceRenderer(ParticleSystem3D *system, int capacity)
    {
        if (capacity <= 0) {
            return true;
        }

        if (!IsShaderValid(system->particleInstanceShader)) {
            system->particleInstanceShader = LoadParticleInstanceShader(system);
            if (!IsShaderValid(system->particleInstanceShader)) {
                return false;
            }
        }

        const int previousCapacity = system->particleInstanceCapacity;
        if (system->particleQuadVao == 0) {
            static const float quadVertices[] = {
                -0.5f, -0.5f, 0.0f, 1.0f,
                 0.5f, -0.5f, 1.0f, 1.0f,
                 0.5f,  0.5f, 1.0f, 0.0f,
                -0.5f, -0.5f, 0.0f, 1.0f,
                 0.5f,  0.5f, 1.0f, 0.0f,
                -0.5f,  0.5f, 0.0f, 0.0f,
            };

            glGenVertexArrays(1, &system->particleQuadVao);
            glBindVertexArray(system->particleQuadVao);

            glGenBuffers(1, &system->particleQuadVbo);
            glBindBuffer(GL_ARRAY_BUFFER, system->particleQuadVbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * (int)sizeof(float), (void *)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * (int)sizeof(float), (void *)(2 * sizeof(float)));

            glGenBuffers(1, &system->particleInstanceVbo);
            glBindBuffer(GL_ARRAY_BUFFER, system->particleInstanceVbo);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((size_t)capacity * sizeof(ParticleRenderInstance)), NULL, GL_STREAM_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleRenderInstance),
                (void *)offsetof(ParticleRenderInstance, x));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ParticleRenderInstance),
                (void *)offsetof(ParticleRenderInstance, r));
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleRenderInstance),
                (void *)offsetof(ParticleRenderInstance, diameter));
            glVertexAttribDivisor(2, 1);
            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);
            glBindVertexArray(0);
            system->particleInstanceCapacity = capacity;
        }

        if (capacity > system->particleInstanceCapacity) {
            glBindBuffer(GL_ARRAY_BUFFER, system->particleInstanceVbo);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((size_t)capacity * sizeof(ParticleRenderInstance)), NULL, GL_STREAM_DRAW);
            system->particleInstanceCapacity = capacity;
        }

        if (capacity > 0 && system->particleInstances == NULL) {
            system->particleInstances = (ParticleRenderInstance *)MemAlloc((size_t)capacity * sizeof(ParticleRenderInstance));
            if (system->particleInstances == NULL) {
                return false;
            }
        } else if (capacity > previousCapacity) {
            ParticleRenderInstance *next = (ParticleRenderInstance *)MemRealloc(system->particleInstances,
                (size_t)capacity * sizeof(ParticleRenderInstance));
            if (next == NULL) {
                return false;
            }
            system->particleInstances = next;
        }

        return system->particleInstances != NULL;
    }

    static void DrawParticleInstanceBatch(const ParticleSystem3D *system, Camera3D camera, int instanceCount, float softPower)
    {
        if (instanceCount <= 0 || system->particleInstanceVbo == 0 || system->particleQuadVao == 0) {
            return;
        }

        Vector3 billboardRight;
        Vector3 billboardUp;
        CameraBillboardBasis(camera, &billboardRight, &billboardUp);
        const Matrix mvp = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());

        rlDrawRenderBatchActive();
        BeginShaderMode(system->particleInstanceShader);
        SetShaderValueMatrix(system->particleInstanceShader, system->particleInstanceMvpLoc, mvp);
        SetShaderValue(system->particleInstanceShader, system->particleInstanceRightLoc, &billboardRight, SHADER_UNIFORM_VEC3);
        SetShaderValue(system->particleInstanceShader, system->particleInstanceUpLoc, &billboardUp, SHADER_UNIFORM_VEC3);
        SetShaderValue(system->particleInstanceShader, system->particleInstanceSoftPowerLoc, &softPower, SHADER_UNIFORM_FLOAT);

        glBindVertexArray(system->particleQuadVao);
        glBindBuffer(GL_ARRAY_BUFFER, system->particleInstanceVbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)((size_t)instanceCount * sizeof(ParticleRenderInstance)),
            system->particleInstances);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCount);
        glBindVertexArray(0);
        EndShaderMode();
        rlDrawRenderBatchActive();
    }

    static void DrawParticles(const ParticleSystem3D *system, Camera3D camera)
    {
        const int smoothDrawLimit = SmoothViewDrawLimit(system);
        const int particleDrawLimit = ParticleViewDrawLimit(system);
        const int drawStride = (system->viewMode == VIEW_SMOOTHING_RADIUS)
            ? ((system->particleCount > smoothDrawLimit) ? (system->particleCount / smoothDrawLimit) + 1 : 1)
            : ((system->particleCount > particleDrawLimit) ? (system->particleCount / particleDrawLimit) + 1 : 1);
        const float smoothCoverageScale = SmoothViewCoverageScale(system);
        const float particleCoverageScale = ParticleViewCoverageScale(system);
        const float diameter = (system->viewMode == VIEW_PARTICLES)
            ? system->params.particleRadius * 2.0f * particleCoverageScale *
                ClampFloat(1.0f + 0.075f * (float)(drawStride - 1), 1.0f, 1.26f)
            : system->params.supportRadius * 1.72f * smoothCoverageScale *
                ClampFloat(1.0f + 0.10f * (float)(drawStride - 1), 1.0f, 1.45f);
        const float softPower = (system->viewMode == VIEW_PARTICLES)
            ? ((drawStride > 1) ? 1.58f : 1.70f)
            : 1.12f;

        BuildDrawOrder((ParticleSystem3D *)system, camera, drawStride);

        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
        if (system->viewMode == VIEW_PARTICLES) {
            rlEnableDepthMask();
        } else {
            rlDisableDepthMask();
        }

        if (EnsureParticleInstanceRenderer((ParticleSystem3D *)system, system->drawItemCount)) {
            for (int i = 0; i < system->drawItemCount; ++i) {
                const int index = system->drawItems[i].index;
                Color color = ParticleColor(system, index);
                if (system->viewMode == VIEW_PARTICLES && drawStride > 1) {
                    const float alphaBoost = ClampFloat(1.0f + 0.10f * log2f((float)drawStride + 1.0f), 1.0f, 1.24f);
                    color.a = (unsigned char)ClampInt((int)lroundf((float)color.a * alphaBoost), 24, 255);
                }
                if (system->viewMode == VIEW_SMOOTHING_RADIUS) {
                    const float countScale = sqrtf(fmaxf((float)system->particleCount / (float)smoothDrawLimit, 1.0f));
                    const float alpha = (10.0f / countScale) * powf((float)drawStride, 0.30f);
                    color.a = (unsigned char)ClampInt((int)lroundf(alpha), 5, 32);
                }

                system->particleInstances[i] = (ParticleRenderInstance){
                    .x = system->x[index],
                    .y = system->y[index],
                    .z = system->z[index],
                    .r = color.r,
                    .g = color.g,
                    .b = color.b,
                    .a = color.a,
                    .diameter = diameter,
                };
            }
            DrawParticleInstanceBatch(system, camera, system->drawItemCount, softPower);
        } else {
            BeginShaderMode(system->particleShader);
            SetShaderValue(system->particleShader, system->particleShaderSoftPowerLoc, &softPower, SHADER_UNIFORM_FLOAT);
            Vector3 billboardRight;
            Vector3 billboardUp;
            CameraBillboardBasis(camera, &billboardRight, &billboardUp);
            rlSetTexture(system->particleTexture.id);
            rlBegin(RL_QUADS);
            for (int i = 0; i < system->drawItemCount; ++i) {
                const int index = system->drawItems[i].index;
                Color color = ParticleColor(system, index);
                if (system->viewMode == VIEW_PARTICLES && drawStride > 1) {
                    const float alphaBoost = ClampFloat(1.0f + 0.10f * log2f((float)drawStride + 1.0f), 1.0f, 1.24f);
                    color.a = (unsigned char)ClampInt((int)lroundf((float)color.a * alphaBoost), 24, 255);
                }
                if (system->viewMode == VIEW_SMOOTHING_RADIUS) {
                    const float countScale = sqrtf(fmaxf((float)system->particleCount / (float)smoothDrawLimit, 1.0f));
                    const float alpha = (10.0f / countScale) * powf((float)drawStride, 0.30f);
                    color.a = (unsigned char)ClampInt((int)lroundf(alpha), 5, 32);
                }

                DrawParticleBillboardQuad((Vector3){system->x[index], system->y[index], system->z[index]},
                    billboardRight, billboardUp, diameter, color);
            }
            rlEnd();
            rlSetTexture(0);
            EndShaderMode();
        }

        rlEnableDepthMask();
        EndBlendMode();
    }

    static float ProjectedWorldRadius(Camera3D camera, Vector3 center, float radius)
    {
        const int screenWidth = GetScreenWidth();
        const int screenHeight = GetScreenHeight();
        const Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        const Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));
        const Vector2 center2D = GetWorldToScreenEx(center, camera, screenWidth, screenHeight);
        const Vector2 right2D = GetWorldToScreenEx(Vector3Add(center, Vector3Scale(right, radius)), camera, screenWidth, screenHeight);
        const Vector2 up2D = GetWorldToScreenEx(Vector3Add(center, Vector3Scale(up, radius)), camera, screenWidth, screenHeight);
        return 0.5f * (Vector2Distance(center2D, right2D) + Vector2Distance(center2D, up2D));
    }

    static void DrawMicOverlay(const ParticleSystem3D *system, Camera3D camera)
    {
        if (!AcousticsActive(system)) {
            return;
        }

        const Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        const Vector3 viewDelta = Vector3Subtract(system->micPosition, camera.position);
        if (Vector3DotProduct(viewDelta, forward) <= 0.0f) {
            return;
        }

        const Vector2 center = GetWorldToScreenEx(system->micPosition, camera, GetScreenWidth(), GetScreenHeight());
        const float pixelRadius = ProjectedWorldRadius(camera, system->micPosition, system->micRadius);
        if (pixelRadius < 2.0f) {
            return;
        }

        DrawCircleV(center, pixelRadius, (Color){18, 30, 40, 180});
        DrawCircleLines((int)lroundf(center.x), (int)lroundf(center.y), pixelRadius, (Color){255, 208, 112, 255});
        DrawCircleLines((int)lroundf(center.x), (int)lroundf(center.y), fmaxf(pixelRadius * 0.22f, 5.0f), (Color){255, 208, 112, 255});
        DrawLineEx((Vector2){center.x - pixelRadius * 0.34f, center.y}, (Vector2){center.x + pixelRadius * 0.34f, center.y},
            2.0f, (Color){255, 208, 112, 220});
        DrawLineEx((Vector2){center.x, center.y - pixelRadius * 0.34f}, (Vector2){center.x, center.y + pixelRadius * 0.34f},
            2.0f, (Color){255, 208, 112, 220});
    }

    static void DrawInteractionOverlay(const ParticleSystem3D *system, Camera3D camera)
    {
        if (!system->interactionActive) {
            return;
        }

        const Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        const Vector3 viewDelta = Vector3Subtract(system->interactionPoint, camera.position);
        if (Vector3DotProduct(viewDelta, forward) <= 0.0f) {
            return;
        }

        const Vector2 center = GetWorldToScreenEx(system->interactionPoint, camera, GetScreenWidth(), GetScreenHeight());
        const float pixelRadius = ProjectedWorldRadius(camera, system->interactionPoint, system->interactionRadius);
        if (pixelRadius < 2.0f) {
            return;
        }

        const float ringWidth = ClampFloat(pixelRadius * 0.09f, 2.0f, 7.0f);
        const int segments = ClampInt((int)lroundf(pixelRadius * 0.65f), 48, 160);
        DrawRing(center, fmaxf(0.0f, pixelRadius - ringWidth), pixelRadius, 0.0f, 360.0f, segments,
            (Color){255, 244, 176, 44});
        DrawRingLines(center, fmaxf(0.0f, pixelRadius - ringWidth * 0.55f), pixelRadius, 0.0f, 360.0f, segments,
            (Color){255, 244, 176, 220});
    }

    static void DrawSlicePlaneQuad(Vector3 a, Vector3 b, Vector3 c, Vector3 d, Color fill, Color outline)
    {
        DrawTriangle3D(a, b, c, fill);
        DrawTriangle3D(a, c, d, fill);
        DrawLine3D(a, b, outline);
        DrawLine3D(b, c, outline);
        DrawLine3D(c, d, outline);
        DrawLine3D(d, a, outline);
    }

    static void DrawSlicePanels(const ParticleSystem3D *system)
    {
        if (!system->showSlicePanels ||
            (CurrentUiMode(system) != UI_3D_MODE_WATER_TANK && CurrentUiMode(system) != UI_3D_MODE_GAS_TANK &&
                CurrentUiMode(system) != UI_3D_MODE_WIND_TUNNEL)) {
            return;
        }

        const float x = SlicePanelPosition(system, 0).x;
        const float y = SlicePanelPosition(system, 1).y;
        const float z = SlicePanelPosition(system, 2).z;
        const Color xFill = (Color){52, 130, 210, 54};
        const Color yFill = (Color){255, 184, 92, 42};
        const Color zFill = (Color){118, 220, 186, 46};
        const Color xOutline = (Color){108, 176, 238, 185};
        const Color yOutline = (Color){255, 208, 132, 176};
        const Color zOutline = (Color){164, 236, 202, 180};

        BeginBlendMode(BLEND_ALPHA);
        rlDisableDepthMask();

        if (SliceAxisEnabled(system, 0)) {
            const Vector3 x0 = {x, system->boundsMin.y, system->boundsMin.z};
            const Vector3 x1 = {x, system->boundsMax.y, system->boundsMin.z};
            const Vector3 x2 = {x, system->boundsMax.y, system->boundsMax.z};
            const Vector3 x3 = {x, system->boundsMin.y, system->boundsMax.z};
            DrawSlicePlaneQuad(x0, x1, x2, x3, xFill, xOutline);
        }

        if (SliceAxisEnabled(system, 1)) {
            const Vector3 y0 = {system->boundsMin.x, y, system->boundsMin.z};
            const Vector3 y1 = {system->boundsMax.x, y, system->boundsMin.z};
            const Vector3 y2 = {system->boundsMax.x, y, system->boundsMax.z};
            const Vector3 y3 = {system->boundsMin.x, y, system->boundsMax.z};
            DrawSlicePlaneQuad(y0, y1, y2, y3, yFill, yOutline);
        }

        if (SliceAxisEnabled(system, 2)) {
            const Vector3 z0 = {system->boundsMin.x, system->boundsMin.y, z};
            const Vector3 z1 = {system->boundsMax.x, system->boundsMin.y, z};
            const Vector3 z2 = {system->boundsMax.x, system->boundsMax.y, z};
            const Vector3 z3 = {system->boundsMin.x, system->boundsMax.y, z};
            DrawSlicePlaneQuad(z0, z1, z2, z3, zFill, zOutline);
        }

        rlEnableDepthMask();
        EndBlendMode();
    }

    static float SliceHighlightStrength(const ParticleSystem3D *system, int index)
    {
        if (!system->showSlicePanels) {
            return 0.0f;
        }

        const float halfThickness = system->params.supportRadius * 0.45f;
        float nearest = FLT_MAX;
        if (SliceAxisEnabled(system, 0)) {
            nearest = fminf(nearest, fabsf(system->x[index] - SlicePanelPosition(system, 0).x));
        }
        if (SliceAxisEnabled(system, 1)) {
            nearest = fminf(nearest, fabsf(system->y[index] - SlicePanelPosition(system, 1).y));
        }
        if (SliceAxisEnabled(system, 2)) {
            nearest = fminf(nearest, fabsf(system->z[index] - SlicePanelPosition(system, 2).z));
        }
        if (nearest == FLT_MAX) {
            return 0.0f;
        }
        if (nearest >= halfThickness) {
            return 0.0f;
        }
        return 1.0f - nearest / halfThickness;
    }

    static void DrawSliceParticleHighlights(const ParticleSystem3D *system, Camera3D camera)
    {
        if (!system->showSlicePanels ||
            (CurrentUiMode(system) != UI_3D_MODE_WATER_TANK && CurrentUiMode(system) != UI_3D_MODE_GAS_TANK &&
                CurrentUiMode(system) != UI_3D_MODE_WIND_TUNNEL)) {
            return;
        }

        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
        rlDisableDepthMask();
        const bool smoothingView = (system->viewMode == VIEW_SMOOTHING_RADIUS);
        const float softPower = smoothingView ? 1.05f : 1.35f;

        if (EnsureParticleInstanceRenderer((ParticleSystem3D *)system, system->particleCount)) {
            int instanceCount = 0;
            for (int i = 0; i < system->particleCount; ++i) {
                const float strength = SliceHighlightStrength(system, i);
                if (strength <= 0.0f) {
                    continue;
                }

                Color color = ParticleColor(system, i);
                if (smoothingView) {
                    color.a = (unsigned char)ClampInt((int)lroundf(34.0f + 86.0f * strength), 28, 132);
                } else {
                    color.a = (unsigned char)ClampInt((int)lroundf(92.0f + 128.0f * strength), 72, 220);
                }
                const float diameter = smoothingView
                    ? system->params.supportRadius * (1.12f + 0.30f * strength)
                    : system->params.particleRadius * (2.90f + 0.85f * strength);
                system->particleInstances[instanceCount++] = (ParticleRenderInstance){
                    .x = system->x[i],
                    .y = system->y[i],
                    .z = system->z[i],
                    .r = color.r,
                    .g = color.g,
                    .b = color.b,
                    .a = color.a,
                    .diameter = diameter,
                };
            }
            DrawParticleInstanceBatch(system, camera, instanceCount, softPower);
        } else {
            BeginShaderMode(system->particleShader);
            SetShaderValue(system->particleShader, system->particleShaderSoftPowerLoc, &softPower, SHADER_UNIFORM_FLOAT);
            Vector3 billboardRight;
            Vector3 billboardUp;
            CameraBillboardBasis(camera, &billboardRight, &billboardUp);
            rlSetTexture(system->particleTexture.id);
            rlBegin(RL_QUADS);
            for (int i = 0; i < system->particleCount; ++i) {
                const float strength = SliceHighlightStrength(system, i);
                if (strength <= 0.0f) {
                    continue;
                }

                Color color = ParticleColor(system, i);
                if (smoothingView) {
                    color.a = (unsigned char)ClampInt((int)lroundf(34.0f + 86.0f * strength), 28, 132);
                } else {
                    color.a = (unsigned char)ClampInt((int)lroundf(92.0f + 128.0f * strength), 72, 220);
                }
                const float diameter = smoothingView
                    ? system->params.supportRadius * (1.12f + 0.30f * strength)
                    : system->params.particleRadius * (2.90f + 0.85f * strength);
                DrawParticleBillboardQuad((Vector3){system->x[i], system->y[i], system->z[i]},
                    billboardRight, billboardUp, diameter, color);
            }
            rlEnd();
            rlSetTexture(0);
            EndShaderMode();
        }

        rlEnableDepthMask();
        EndBlendMode();
    }

    static void DrawStreamlines(const ParticleSystem3D *system)
    {
        if (!system->showStreamlines || !SceneIsWindTunnel(system) || system->sortedStateDirty) {
            return;
        }

        const int density = ClampInt(system->streamlineDensity, FLOW_LINE_DENSITY_MIN, FLOW_LINE_DENSITY_MAX);
        const int stepCount = ClampInt(system->streamlineStepCount, STREAMLINE_STEPS_MIN, STREAMLINE_STEPS_MAX);
        for (int yIndex = 0; yIndex < density; ++yIndex) {
            for (int zIndex = 0; zIndex < density; ++zIndex) {
                const int seedIndex = yIndex * density + zIndex;
                Vector3 point = SeedFlowLinePosition(system, seedIndex, density);
                const Color baseColor = (Color){120, 210, 255, 110};

                for (int step = 0; step < stepCount; ++step) {
                    const Vector3 velocity = SampleVelocityAtPoint(system, point);
                    const float speed = Vector3Length(velocity);
                    if (speed < 1e-4f) {
                        break;
                    }

                    const float stepLength = system->params.supportRadius * 0.72f;
                    const Vector3 delta = Vector3Scale(Vector3Normalize(velocity), stepLength);
                    const Vector3 nextPoint = Vector3Add(point, delta);
                    if (nextPoint.x > system->boundsMax.x || nextPoint.y < system->boundsMin.y || nextPoint.y > system->boundsMax.y ||
                        nextPoint.z < system->boundsMin.z || nextPoint.z > system->boundsMax.z) {
                        break;
                    }

                    Color color = baseColor;
                    color.a = (unsigned char)ClampInt(110 - step * 4, 36, 110);
                    DrawLine3D(point, nextPoint, color);
                    point = nextPoint;
                }
            }
        }
    }

    static void DrawPathlines(const ParticleSystem3D *system)
    {
        if (!system->showPathlines || !SceneIsWindTunnel(system)) {
            return;
        }

        const int activeCount = ActivePathlineCount(system);
        for (int i = 0; i < activeCount; ++i) {
            const FlowPathline *pathline = &system->pathlines[i];
            for (int pointIndex = 1; pointIndex < pathline->count; ++pointIndex) {
                Color color = (Color){255, 202, 108, (unsigned char)ClampInt(40 + pointIndex * 10, 40, 180)};
                DrawLine3D(pathline->trail[pointIndex - 1], pathline->trail[pointIndex], color);
            }
        }
    }

    static void DrawObstaclePolygonPrism(const ParticleSystem3D *system, const Vector2 *localPoints, int pointCount,
        Color fill, Color outline)
    {
        Vector2 points2D[128];
        int indices[128];
        if (pointCount > (int)(sizeof(points2D) / sizeof(points2D[0]))) {
            pointCount = (int)(sizeof(points2D) / sizeof(points2D[0]));
        }

        for (int i = 0; i < pointCount; ++i) {
            points2D[i] = localPoints[i];
            indices[i] = i;
        }

        const float halfDepth = system->obstacleDepth * 0.5f;
        const float orientationSign = (PolygonSignedArea(points2D, pointCount) >= 0.0f) ? 1.0f : -1.0f;
        int remainingCount = pointCount;
        int guard = 0;
        while (remainingCount > 2 && guard < 512) {
            bool clippedEar = false;
            for (int i = 0; i < remainingCount; ++i) {
                if (!PolygonVertexIsEar(points2D, indices, remainingCount, i, orientationSign)) {
                    continue;
                }

                const int prevIndex = indices[(i + remainingCount - 1) % remainingCount];
                const int currIndex = indices[i];
                const int nextIndex = indices[(i + 1) % remainingCount];
                const Vector3 aFront = ObstacleLocalToWorld(system, (Vector3){points2D[prevIndex].x, points2D[prevIndex].y, -halfDepth});
                const Vector3 bFront = ObstacleLocalToWorld(system, (Vector3){points2D[currIndex].x, points2D[currIndex].y, -halfDepth});
                const Vector3 cFront = ObstacleLocalToWorld(system, (Vector3){points2D[nextIndex].x, points2D[nextIndex].y, -halfDepth});
                const Vector3 aBack = ObstacleLocalToWorld(system, (Vector3){points2D[prevIndex].x, points2D[prevIndex].y, halfDepth});
                const Vector3 bBack = ObstacleLocalToWorld(system, (Vector3){points2D[currIndex].x, points2D[currIndex].y, halfDepth});
                const Vector3 cBack = ObstacleLocalToWorld(system, (Vector3){points2D[nextIndex].x, points2D[nextIndex].y, halfDepth});
                DrawTriangle3D(aFront, bFront, cFront, fill);
                DrawTriangle3D(cBack, bBack, aBack, fill);

                for (int shift = i; shift < remainingCount - 1; ++shift) {
                    indices[shift] = indices[shift + 1];
                }
                remainingCount -= 1;
                clippedEar = true;
                break;
            }
            if (!clippedEar) {
                break;
            }
            ++guard;
        }

        for (int i = 0; i < pointCount; ++i) {
            const int next = (i + 1) % pointCount;
            const Vector3 aFront = ObstacleLocalToWorld(system, (Vector3){points2D[i].x, points2D[i].y, -halfDepth});
            const Vector3 bFront = ObstacleLocalToWorld(system, (Vector3){points2D[next].x, points2D[next].y, -halfDepth});
            const Vector3 aBack = ObstacleLocalToWorld(system, (Vector3){points2D[i].x, points2D[i].y, halfDepth});
            const Vector3 bBack = ObstacleLocalToWorld(system, (Vector3){points2D[next].x, points2D[next].y, halfDepth});
            DrawTriangle3D(aFront, bFront, bBack, fill);
            DrawTriangle3D(aFront, bBack, aBack, fill);
            DrawLine3D(aFront, bFront, outline);
            DrawLine3D(aBack, bBack, outline);
            DrawLine3D(aFront, aBack, outline);
        }
    }

    static Vector3 WaterRectangleLocalToWorld(const ParticleSystem3D *system, int index, Vector3 local)
    {
        const Vector2 rotated = RotateVector((Vector2){local.x, local.y},
            system->waterRectangleAngleDegrees[index] * DEG2RAD);
        return (Vector3){
            system->waterCylinderX[index] + rotated.x,
            system->waterCylinderY[index] + rotated.y,
            system->waterCylinderZ[index] + local.z,
        };
    }

    static void DrawWaterRectanglePrism(const ParticleSystem3D *system, int index, Color fill, Color outline)
    {
        const float halfWidth = system->waterRectangleWidth[index] * 0.5f;
        const float halfHeight = system->waterRectangleHeight[index] * 0.5f;
        const float halfDepth = system->waterCylinderDepth[index] * 0.5f;
        const Vector2 local2D[4] = {
            {-halfWidth, -halfHeight},
            {halfWidth, -halfHeight},
            {halfWidth, halfHeight},
            {-halfWidth, halfHeight},
        };

        Vector3 front[4];
        Vector3 back[4];
        for (int i = 0; i < 4; ++i) {
            front[i] = WaterRectangleLocalToWorld(system, index, (Vector3){local2D[i].x, local2D[i].y, -halfDepth});
            back[i] = WaterRectangleLocalToWorld(system, index, (Vector3){local2D[i].x, local2D[i].y, halfDepth});
        }

        DrawTriangle3D(front[0], front[1], front[2], fill);
        DrawTriangle3D(front[0], front[2], front[3], fill);
        DrawTriangle3D(back[2], back[1], back[0], fill);
        DrawTriangle3D(back[3], back[2], back[0], fill);
        for (int i = 0; i < 4; ++i) {
            const int next = (i + 1) % 4;
            DrawTriangle3D(front[i], front[next], back[next], fill);
            DrawTriangle3D(front[i], back[next], back[i], fill);
            DrawLine3D(front[i], front[next], outline);
            DrawLine3D(back[i], back[next], outline);
            DrawLine3D(front[i], back[i], outline);
        }
    }

    static void DrawObstacle(const ParticleSystem3D *system)
    {
        if (!SolidObstacleActive(system)) {
            return;
        }

        if (WaterCylindersActive(system)) {
            const Color fill = (Color){26, 42, 58, 235};
            const Color outline = (Color){116, 185, 230, 230};
            const Color selectedOutline = (Color){185, 230, 255, 255};
            const int count = ClampInt(system->waterCylinderCount, 0, WATER_CYLINDER_MAX);
            const int selectedMax = (count > 0) ? count - 1 : 0;
            const int selected = ClampInt(system->waterCylinderSelected, 0, selectedMax);
            for (int i = 0; i < count; ++i) {
                const float halfDepth = system->waterCylinderDepth[i] * 0.5f;
                const Vector3 start = {system->waterCylinderX[i], system->waterCylinderY[i], system->waterCylinderZ[i] - halfDepth};
                const Vector3 end = {system->waterCylinderX[i], system->waterCylinderY[i], system->waterCylinderZ[i] + halfDepth};
                DrawCylinderEx(start, end, system->waterCylinderRadius[i], system->waterCylinderRadius[i], 36, fill);
                DrawCylinderWiresEx(start, end, system->waterCylinderRadius[i], system->waterCylinderRadius[i],
                    36, (i == selected) ? selectedOutline : outline);
            }
            return;
        }

        if (WaterRectanglesActive(system)) {
            const Color fill = (Color){26, 42, 58, 235};
            const Color outline = (Color){116, 185, 230, 230};
            const Color selectedOutline = (Color){185, 230, 255, 255};
            const int count = ClampInt(system->waterCylinderCount, 0, WATER_CYLINDER_MAX);
            const int selectedMax = (count > 0) ? count - 1 : 0;
            const int selected = ClampInt(system->waterCylinderSelected, 0, selectedMax);
            for (int i = 0; i < count; ++i) {
                DrawWaterRectanglePrism(system, i, fill, (i == selected) ? selectedOutline : outline);
            }
            return;
        }

        const Color fill = (Color){28, 34, 44, 255};
        const Color outline = (Color){190, 214, 235, 230};

        switch (system->obstacleModel) {
            case OBSTACLE_AIRFOIL: {
                Vector2 localPoints[128];
                for (int i = 0; i < AIRFOIL_OUTLINE_POINT_COUNT; ++i) {
                    localPoints[i] = (Vector2){
                        AIRFOIL_OUTLINE_POINTS[i].x * system->obstacleRadius,
                        AIRFOIL_OUTLINE_POINTS[i].y * system->obstacleRadius,
                    };
                }
                DrawObstaclePolygonPrism(system, localPoints, AIRFOIL_OUTLINE_POINT_COUNT, fill, outline);
            } break;
            case OBSTACLE_CAR: {
                Vector2 localPoints[128];
                for (int i = 0; i < CAR_OUTLINE_POINT_COUNT; ++i) {
                    localPoints[i] = (Vector2){
                        CAR_OUTLINE_POINTS[i].x * system->obstacleRadius * CAR_SHAPE_SCALE_X,
                        -CAR_OUTLINE_POINTS[i].y * system->obstacleRadius * CAR_SHAPE_SCALE_Y,
                    };
                }
                DrawObstaclePolygonPrism(system, localPoints, CAR_OUTLINE_POINT_COUNT, fill, outline);
            } break;
            case OBSTACLE_RECTANGLE:
            {
                const Vector2 localPoints[4] = {
                    {-system->obstacleRectWidth * 0.5f, -system->obstacleRectHeight * 0.5f},
                    {system->obstacleRectWidth * 0.5f, -system->obstacleRectHeight * 0.5f},
                    {system->obstacleRectWidth * 0.5f, system->obstacleRectHeight * 0.5f},
                    {-system->obstacleRectWidth * 0.5f, system->obstacleRectHeight * 0.5f},
                };
                DrawObstaclePolygonPrism(system, localPoints, 4, fill, outline);
            }
                break;
            case OBSTACLE_IMPORTED:
                if (system->importedObstacleLoaded) {
                    const Matrix transform = ImportedObstacleDrawTransformMatrix(system);
                    const Color importedFill = (Color){52, 61, 76, 230};
                    for (int meshIndex = 0; meshIndex < system->importedObstacleModel.meshCount; ++meshIndex) {
                        Material material = ImportedObstacleMaterial(&system->importedObstacleModel, meshIndex);
                        material.maps[MATERIAL_MAP_ALBEDO].color = importedFill;
                        DrawMesh(system->importedObstacleModel.meshes[meshIndex], material, transform);
                    }
                }
                break;
            case OBSTACLE_CIRCLE:
            default: {
                const Vector3 start = {system->obstacleCenter.x, system->obstacleCenter.y, system->obstacleCenter.z - system->obstacleDepth * 0.5f};
                const Vector3 end = {system->obstacleCenter.x, system->obstacleCenter.y, system->obstacleCenter.z + system->obstacleDepth * 0.5f};
                DrawCylinderEx(start, end, system->obstacleRadius, system->obstacleRadius, 28, fill);
                DrawCylinderWiresEx(start, end, system->obstacleRadius, system->obstacleRadius, 28, outline);
            } break;
        }
    }

    static void DrawAcousticRig(const ParticleSystem3D *system)
    {
        if (!AcousticsActive(system)) {
            return;
        }

        Vector3 speakerCenter;
        Vector3 speakerVelocity;
        Vector3 speakerHalfSize;
        GetSpeakerState(system, &speakerCenter, &speakerVelocity, &speakerHalfSize);

        const Vector3 baseHalfSize = {
            system->speakerWidth * 0.5f,
            system->speakerHeight * 0.5f,
            system->speakerDepth * 0.5f,
        };
        DrawCubeV(system->speakerBaseCenter, Vector3Scale(baseHalfSize, 2.0f), (Color){22, 32, 42, 68});
        DrawCubeWiresV(system->speakerBaseCenter, Vector3Scale(baseHalfSize, 2.0f), (Color){74, 102, 136, 128});
        DrawCubeV(speakerCenter, Vector3Scale(speakerHalfSize, 2.0f), (Color){26, 34, 44, 240});
        DrawCubeWiresV(speakerCenter, Vector3Scale(speakerHalfSize, 2.0f), (Color){122, 212, 255, 255});

        const Vector3 velocityTip = Vector3Add(speakerCenter, Vector3Scale(speakerVelocity, 0.08f));
        DrawLine3D(speakerCenter, velocityTip, (Color){160, 225, 255, 210});
    }

    static void DrawTank(const ParticleSystem3D *system)
    {
        const float wirePad = fmaxf(system->params.particleRadius * 0.35f, 0.6f);
        const Vector3 wireSize = {
            system->boundsSize.x + wirePad * 2.0f,
            system->boundsSize.y + wirePad * 2.0f,
            system->boundsSize.z + wirePad * 2.0f,
        };
        rlDisableDepthMask();
        DrawCubeWiresV(system->boundsCenter, wireSize, (Color){85, 128, 170, 255});

        const Vector3 floor0 = {system->boundsMin.x - wirePad, system->boundsMin.y - wirePad, system->boundsMin.z - wirePad};
        const Vector3 floor1 = {system->boundsMax.x + wirePad, system->boundsMin.y - wirePad, system->boundsMin.z - wirePad};
        const Vector3 floor2 = {system->boundsMax.x + wirePad, system->boundsMin.y - wirePad, system->boundsMax.z + wirePad};
        const Vector3 floor3 = {system->boundsMin.x - wirePad, system->boundsMin.y - wirePad, system->boundsMax.z + wirePad};
        DrawLine3D(floor0, floor1, (Color){42, 64, 88, 200});
        DrawLine3D(floor1, floor2, (Color){42, 64, 88, 200});
        DrawLine3D(floor2, floor3, (Color){42, 64, 88, 200});
        DrawLine3D(floor3, floor0, (Color){42, 64, 88, 200});
        rlEnableDepthMask();
    }

    static void DrawHud(const ParticleSystem3D *system)
    {
        static const char *const colorModeLabels[] = {
            "Material", "Temperature", "Pressure", "Speed", "Density", "Tracer dye", "Vorticity",
        };

        const char *label = colorModeLabels[ClampInt((int)system->colorMode, 0, COLOR_MODE_COUNT - 1)];
        const int fontSize = 22;
        const int paddingX = 14;
        const int paddingY = 9;
        const int textWidth = MeasureText(label, fontSize);
        const int boxWidth = textWidth + paddingX * 2;
        const int boxHeight = fontSize + paddingY * 2;
        const int boxX = 18;
        const int boxY = 18;
        const Rectangle box = {(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight};
        DrawRectangleRec(box, (Color){9, 18, 34, 208});
        DrawRectangleLinesEx(box, 1.5f, (Color){66, 124, 184, 235});
        DrawText(label, boxX + paddingX, boxY + paddingY, fontSize, (Color){232, 242, 255, 255});

        if (system->backendNotice != NULL && GetTime() < system->backendNoticeUntil) {
            DrawText(system->backendNotice, 24, WINDOW_HEIGHT - 34, 18, (Color){196, 214, 235, 255});
        }

        if (system->paused) {
            DrawText("PAUSED", WINDOW_WIDTH / 2 - 54, 24, 28, (Color){255, 215, 90, 255});
        }
    }

    static Ui3DPanelState BuildUiPanelState(const ParticleSystem3D *system, const OrbitCameraState *orbit, const BakeSession3D *bake)
    {
        const int waterCylinderCount = ClampInt(system->waterCylinderCount, 1, WATER_CYLINDER_MAX);
        const int waterCylinderSelected = ClampInt(system->waterCylinderSelected, 0, waterCylinderCount - 1);
        const int bakeParticleTarget = ClampInt(bake->particleTarget, 1, BAKE_MAX_PARTICLE_COUNT);
        const float acousticResolvedHz = system->acousticAudioLoaded
            ? AcousticResolvedFrequencyEstimateForBakeTarget(system, bakeParticleTarget)
            : AcousticResolvedFrequencyEstimate(system);
        const float acousticSlowdown = system->acousticAudioLoaded
            ? AcousticBakeSlowdownForTarget(system, bakeParticleTarget)
            : AcousticAudioSlowdownFactor(system);
        const float acousticRestoredHz = acousticResolvedHz * acousticSlowdown;
        const float acousticAudioBandwidthHz = system->acousticAudioLoaded
            ? AcousticAudioTargetBandwidthHz(system)
            : 0.0f;
        const float acousticSlowBakeDuration = system->acousticAudioLoaded
            ? (system->acousticEnvelopeDuration * acousticSlowdown +
                ACOUSTIC_AUDIO_PREROLL_SECONDS + ACOUSTIC_AUDIO_POSTROLL_SECONDS)
            : bake->duration;
        Ui3DPanelState state = {
            .backend = (int)system->activeBackend,
            .targetParticleCount = (bake->status == BAKE_STATUS_BAKING || bake->hasCache)
                ? bake->particleTarget
                : EffectiveTargetParticleCount(system),
            .actualParticleCount = system->particleCount,
            .mode = (int)CurrentUiMode(system),
            .obstacleModel = (int)system->obstacleModel,
            .waterObstacleEnabled = system->waterObstacleEnabled,
            .waterObstacleShape = system->waterObstacleShape,
            .waterCylinderCount = waterCylinderCount,
            .waterCylinderSelected = waterCylinderSelected,
            .waterCylinderX = system->waterCylinderX[waterCylinderSelected],
            .waterCylinderY = system->waterCylinderY[waterCylinderSelected],
            .waterCylinderZ = system->waterCylinderZ[waterCylinderSelected],
            .waterCylinderRadius = system->waterCylinderRadius[waterCylinderSelected],
            .waterCylinderDepth = system->waterCylinderDepth[waterCylinderSelected],
            .waterRectangleWidth = system->waterRectangleWidth[waterCylinderSelected],
            .waterRectangleHeight = system->waterRectangleHeight[waterCylinderSelected],
            .waterRectangleAngleDegrees = system->waterRectangleAngleDegrees[waterCylinderSelected],
            .obstacleAngleDegrees = system->obstacleAngleDegrees,
            .obstacleRectWidth = system->obstacleRectWidth,
            .obstacleRectHeight = system->obstacleRectHeight,
            .importedObstacleLoaded = system->importedObstacleLoaded,
            .importedObstacleLabel = system->importedObstacleLoaded ? GetFileName(system->importedObstaclePath) : "No OBJ loaded",
            .importedScale = system->importedObstacleUserScale,
            .importedOffsetX = system->importedObstacleOffset.x,
            .importedOffsetY = system->importedObstacleOffset.y,
            .importedOffsetZ = system->importedObstacleOffset.z,
            .importedRotationX = system->importedObstacleRotationDegrees.x,
            .importedRotationY = system->importedObstacleRotationDegrees.y,
            .importedRotationZ = system->importedObstacleRotationDegrees.z,
            .viewMode = (int)system->viewMode,
            .colorMode = (int)system->colorMode,
            .flyCamera = orbit->flyMode,
            .paused = system->paused,
            .gpuBackendAvailable = system->gpuBackendAvailable,
            .windSpeedScale = system->flowSpeedScale,
            .flowMach = SceneIsWindTunnel(system)
                ? (system->flowTargetSpeed / fmaxf(system->params.soundSpeed, 1e-4f))
                : 0.0f,
            .showParticles = system->showParticles,
            .showSlicePanels = system->showSlicePanels,
            .sliceXEnabled = system->sliceXEnabled,
            .sliceXNormalized = system->sliceXNormalized,
            .sliceYEnabled = system->sliceYEnabled,
            .sliceYNormalized = system->sliceYNormalized,
            .sliceZEnabled = system->sliceZEnabled,
            .sliceZNormalized = system->sliceZNormalized,
            .showStreamlines = system->showStreamlines,
            .streamlineDensity = system->streamlineDensity,
            .streamlineStepCount = system->streamlineStepCount,
            .showPathlines = system->showPathlines,
            .pathlineDensity = system->pathlineDensity,
            .pathlineTrailLength = system->pathlineTrailLength,
            .acousticsAvailable = AcousticsAvailable(system),
            .acousticsEnabled = system->acousticsEnabled,
            .speakerFrequency = system->speakerFrequency,
            .speakerAmplitude = system->speakerAmplitude,
            .effectiveSpeakerAmplitude = EffectiveSpeakerAmplitude(system),
            .speakerWidth = system->speakerWidth,
            .speakerHeight = system->speakerHeight,
            .speakerDepth = system->speakerDepth,
            .micRadius = system->micRadius,
            .micSignal = system->micSignal,
            .acousticSoundSpeed = system->acousticSoundSpeed,
            .acousticMachLimit = system->acousticMachLimit,
            .acousticViscosityScale = system->acousticViscosityScale,
            .acousticDragScale = system->acousticDragScale,
            .audioMonitorPitchHz = system->audioMonitorPitchHz,
            .audioOutputAvailable = system->audioOutputReady,
            .audioOutputEnabled = system->audioOutputEnabled && system->audioOutputReady,
            .acousticAudioLoaded = system->acousticAudioLoaded,
            .acousticAudioLabel = system->acousticAudioLoaded ? system->acousticAudioLabel : "procedural",
            .acousticAudioDuration = system->acousticEnvelopeDuration,
            .acousticAudioBandwidthHz = acousticAudioBandwidthHz,
            .acousticResolvedHz = acousticResolvedHz,
            .acousticDriverHz = system->acousticAudioLoaded ? system->acousticWaveformSampleRate : 0.0f,
            .acousticPropagationDelay = AcousticSpeakerMicDelaySeconds(system),
            .acousticSlowdownFactor = acousticSlowdown,
            .acousticRestoredBandwidthHz = acousticRestoredHz,
            .acousticSlowBakeDuration = acousticSlowBakeDuration,
            .bakeStatus = (int)bake->status,
            .bakeSimulationLocked = bake->status == BAKE_STATUS_BAKING,
            .bakeDuration = bake->duration,
            .bakeParticleCount = bake->particleTarget,
            .bakeProgress = bake->progress,
            .bakeBakedTime = bake->bakedTime,
            .bakeEtaSeconds = bake->etaSeconds,
            .bakePlaybackTime = bake->playbackTime,
            .bakePlaybackPlaying = bake->playbackPlaying,
            .bakeHasCache = bake->hasCache,
            .bakeCanExportMic = bake->canExportMic,
            .bakeNotice = bake->notice,
            .micWaveform = system->micWaveformDisplay,
            .micWaveformCount = system->micWaveformCount,
            .fps = (float)GetFPS(),
            .lastSimStepMs = system->lastSimStepMs,
            .cameraDistance = orbit->distance,
        };
        return state;
    }

    static void ApplyUiMode(ParticleSystem3D *system, Ui3DSimMode mode)
    {
        switch (mode) {
            case UI_3D_MODE_GAS_TANK:
                system->tankPreset = MATERIAL_GAS;
                if (system->scene != SCENE_TANK) {
                    system->scene = SCENE_TANK;
                    ResetSimulation(system, MATERIAL_GAS);
                } else if (system->preset != MATERIAL_GAS) {
                    ResetSimulation(system, MATERIAL_GAS);
                }
                break;
            case UI_3D_MODE_WIND_TUNNEL:
                SetSimulationScene(system, SCENE_WIND_TUNNEL);
                break;
            case UI_3D_MODE_WATER_TANK:
            default:
                system->tankPreset = MATERIAL_WATER;
                if (system->scene != SCENE_TANK) {
                    system->scene = SCENE_TANK;
                    ResetSimulation(system, MATERIAL_WATER);
                } else if (system->preset != MATERIAL_WATER) {
                    ResetSimulation(system, MATERIAL_WATER);
                }
                break;
        }
    }

    static void ApplyUiPanelActions(ParticleSystem3D *system, const Ui3DPanelActions *actions, OrbitCameraState *orbit, Camera3D *camera)
    {
        bool refreshAcousticTuning = false;

        if (actions->requestImportObstacle) {
            (void)OpenImportedObstacleDialog(system);
        }

        if (actions->requestLoadAcousticAudio) {
            (void)LoadAcousticAudioFile(system);
        }

        if (actions->setBackend) {
            if (actions->backend == SIM_BACKEND_GPU) {
                (void)InitializeGpuBackend(system);
            }
            SetSimulationBackend(system, (SimulationBackend)actions->backend);
        }
        if (actions->setTargetParticleCount) {
            SetTargetParticleCount(system, actions->targetParticleCount);
        }
        if (actions->setMode) {
            ApplyUiMode(system, (Ui3DSimMode)actions->mode);
        }
        if (actions->setObstacleModel) {
            if (actions->obstacleModel == OBSTACLE_IMPORTED && !system->importedObstacleLoaded) {
                SetBackendNotice(system, "Open an OBJ first to use the imported obstacle.");
            } else {
                SetObstacleModel(system, (ObstacleModel)actions->obstacleModel);
            }
        }
        if (actions->setWaterObstacleEnabled) {
            SetWaterObstacleEnabled(system, actions->waterObstacleEnabled);
        }
        if (actions->setWaterObstacleShape) {
            SetWaterObstacleShape(system, actions->waterObstacleShape);
        }
        if (actions->setWaterCylinderCount) {
            SetWaterCylinderCount(system, actions->waterCylinderCount);
        }
        if (actions->setWaterCylinderSelected) {
            SetWaterCylinderSelected(system, actions->waterCylinderSelected);
        }
        if (actions->setWaterCylinderX) {
            SetSelectedWaterCylinderPosition(system, 0, actions->waterCylinderX);
        }
        if (actions->setWaterCylinderY) {
            SetSelectedWaterCylinderPosition(system, 1, actions->waterCylinderY);
        }
        if (actions->setWaterCylinderZ) {
            SetSelectedWaterCylinderPosition(system, 2, actions->waterCylinderZ);
        }
        if (actions->setWaterCylinderRadius) {
            SetSelectedWaterCylinderRadius(system, actions->waterCylinderRadius);
        }
        if (actions->setWaterCylinderDepth) {
            SetSelectedWaterCylinderDepth(system, actions->waterCylinderDepth);
        }
        if (actions->setWaterRectangleWidth) {
            SetSelectedWaterRectangleWidth(system, actions->waterRectangleWidth);
        }
        if (actions->setWaterRectangleHeight) {
            SetSelectedWaterRectangleHeight(system, actions->waterRectangleHeight);
        }
        if (actions->setWaterRectangleAngleDegrees) {
            SetSelectedWaterRectangleAngle(system, actions->waterRectangleAngleDegrees);
        }
        if (actions->setObstacleAngleDegrees) {
            SetObstacleAngleDegrees(system, actions->obstacleAngleDegrees);
        }
        if (actions->setObstacleRectWidth) {
            SetObstacleRectangleWidth(system, actions->obstacleRectWidth);
        }
        if (actions->setObstacleRectHeight) {
            SetObstacleRectangleHeight(system, actions->obstacleRectHeight);
        }
        if (actions->setImportedScale) {
            system->importedObstacleUserScale = ClampFloat(actions->importedScale, 0.35f, 3.00f);
            InvalidateGpuBackendState(system);
        }
        if (actions->setImportedOffsetX) {
            system->importedObstacleOffset.x = ClampFloat(actions->importedOffsetX, -160.0f, 160.0f);
            InvalidateGpuBackendState(system);
        }
        if (actions->setImportedOffsetY) {
            system->importedObstacleOffset.y = ClampFloat(actions->importedOffsetY, -90.0f, 90.0f);
            InvalidateGpuBackendState(system);
        }
        if (actions->setImportedOffsetZ) {
            system->importedObstacleOffset.z = ClampFloat(actions->importedOffsetZ, -90.0f, 90.0f);
            InvalidateGpuBackendState(system);
        }
        if (actions->setImportedRotationX) {
            ApplyImportedObstacleRotationDeltaWorld(system, (Vector3){1.0f, 0.0f, 0.0f}, actions->importedRotationX);
            InvalidateGpuBackendState(system);
        }
        if (actions->setImportedRotationY) {
            ApplyImportedObstacleRotationDeltaWorld(system, (Vector3){0.0f, 1.0f, 0.0f}, actions->importedRotationY);
            InvalidateGpuBackendState(system);
        }
        if (actions->setImportedRotationZ) {
            ApplyImportedObstacleRotationDeltaWorld(system, (Vector3){0.0f, 0.0f, 1.0f}, actions->importedRotationZ);
            InvalidateGpuBackendState(system);
        }
        if (actions->requestResetImportedRotation) {
            ResetImportedObstacleRotationState(system);
            InvalidateGpuBackendState(system);
        }
        if (actions->setViewMode) {
            system->viewMode = (ViewMode)actions->viewMode;
        }
        if (actions->setColorMode) {
            system->colorMode = (ColorMode)actions->colorMode;
            if (ColorModeNeedsVorticity(system->colorMode)) {
                system->framesUntilVorticity = 0;
            }
        }
        if (actions->setFlyCamera) {
            SetFlyCameraMode(orbit, camera, system, actions->flyCamera);
        }
        if (actions->setPaused) {
            system->paused = actions->paused;
            if (system->paused) {
                system->audioOutputSignal = 0.0f;
                system->audioOutputState = 0.0f;
            }
        }
        if (actions->setWindSpeedScale) {
            SetWindSpeedScale(system, actions->windSpeedScale);
        }
        if (actions->setShowParticles) {
            system->showParticles = actions->showParticles;
        }
        if (actions->setShowSlicePanels) {
            system->showSlicePanels = actions->showSlicePanels;
        }
        if (actions->setSliceXEnabled) {
            system->sliceXEnabled = actions->sliceXEnabled;
        }
        if (actions->setSliceXNormalized) {
            system->sliceXNormalized = ClampFloat(actions->sliceXNormalized, 0.05f, 0.95f);
        }
        if (actions->setSliceYEnabled) {
            system->sliceYEnabled = actions->sliceYEnabled;
        }
        if (actions->setSliceYNormalized) {
            system->sliceYNormalized = ClampFloat(actions->sliceYNormalized, 0.05f, 0.95f);
        }
        if (actions->setSliceZEnabled) {
            system->sliceZEnabled = actions->sliceZEnabled;
        }
        if (actions->setSliceZNormalized) {
            system->sliceZNormalized = ClampFloat(actions->sliceZNormalized, 0.05f, 0.95f);
        }
        if (actions->setShowStreamlines) {
            system->showStreamlines = actions->showStreamlines;
        }
        if (actions->setStreamlineDensity) {
            system->streamlineDensity = ClampInt(actions->streamlineDensity, FLOW_LINE_DENSITY_MIN, FLOW_LINE_DENSITY_MAX);
        }
        if (actions->setStreamlineStepCount) {
            system->streamlineStepCount = ClampInt(actions->streamlineStepCount, STREAMLINE_STEPS_MIN, STREAMLINE_STEPS_MAX);
        }
        if (actions->setShowPathlines) {
            system->showPathlines = actions->showPathlines;
            if (system->showPathlines) {
                ResetPathlines(system);
            }
        }
        if (actions->setPathlineDensity) {
            system->pathlineDensity = ClampInt(actions->pathlineDensity, FLOW_LINE_DENSITY_MIN, FLOW_LINE_DENSITY_MAX);
            ResetPathlines(system);
        }
        if (actions->setPathlineTrailLength) {
            system->pathlineTrailLength = ClampInt(actions->pathlineTrailLength, FLOW_PATHLINE_TRAIL_MIN, FLOW_PATHLINE_TRAIL_MAX);
        }
        if (actions->setAcousticsEnabled) {
            system->acousticsEnabled = actions->acousticsEnabled;
            ResetMicrophoneHistory(system);
            if (system->acousticsEnabled && system->audioOutputEnabled && !system->audioOutputReady) {
                (void)InitializeAudioOutput(system);
            }
            if (AcousticsAvailable(system)) {
                ResetSimulation(system, system->preset);
            }
        }
        if (actions->setSpeakerFrequency) {
            system->speakerFrequency = ClampFloat(actions->speakerFrequency, 0.1f, 40.0f);
            ClampAcousticAnchors(system);
        }
        if (actions->setSpeakerAmplitude) {
            system->speakerAmplitude = ClampFloat(actions->speakerAmplitude, 1.0f, 42.0f);
            ClampAcousticAnchors(system);
        }
        if (actions->setAcousticSoundSpeed) {
            system->acousticSoundSpeed = ClampFloat(actions->acousticSoundSpeed, 52.0f, 420.0f);
            refreshAcousticTuning = true;
        }
        if (actions->setAcousticMachLimit) {
            system->acousticMachLimit = ClampFloat(actions->acousticMachLimit, 0.10f, 0.60f);
            ClampAcousticAnchors(system);
        }
        if (actions->setAcousticViscosityScale) {
            system->acousticViscosityScale = ClampFloat(actions->acousticViscosityScale, 0.05f, 1.00f);
            refreshAcousticTuning = true;
        }
        if (actions->setAcousticDragScale) {
            system->acousticDragScale = ClampFloat(actions->acousticDragScale, 0.00f, 1.00f);
            refreshAcousticTuning = true;
        }
        if (actions->setAcousticSlowdownFactor) {
            system->acousticAudioSlowdownFactor = ClampFloat(actions->acousticSlowdownFactor,
                ACOUSTIC_AUDIO_MIN_SLOWDOWN_FACTOR, ACOUSTIC_AUDIO_MAX_SLOWDOWN_FACTOR);
        }
        if (actions->setSpeakerWidth) {
            system->speakerWidth = ClampFloat(actions->speakerWidth, 8.0f, 80.0f);
            ClampAcousticAnchors(system);
            if (AcousticsActive(system)) {
                ResetSimulation(system, system->preset);
            }
        }
        if (actions->setSpeakerHeight) {
            system->speakerHeight = ClampFloat(actions->speakerHeight, 24.0f, 220.0f);
            ClampAcousticAnchors(system);
            if (AcousticsActive(system)) {
                ResetSimulation(system, system->preset);
            }
        }
        if (actions->setSpeakerDepth) {
            system->speakerDepth = ClampFloat(actions->speakerDepth, 8.0f, 56.0f);
            ClampAcousticAnchors(system);
            if (AcousticsActive(system)) {
                ResetSimulation(system, system->preset);
            }
        }
        if (actions->setMicRadius) {
            system->micRadius = ClampFloat(actions->micRadius, 8.0f, 80.0f);
        }
        if (actions->setAudioOutputEnabled) {
            if (actions->audioOutputEnabled && !system->audioOutputReady) {
                (void)InitializeAudioOutput(system);
            }
            system->audioOutputEnabled = actions->audioOutputEnabled && system->audioOutputReady;
        }
        if (actions->setAudioMonitorPitchHz) {
            system->audioMonitorPitchHz = ClampFloat(actions->audioMonitorPitchHz, 0.0f, 1000.0f);
        }
        if (refreshAcousticTuning) {
            RefreshDerivedSimulationParameters(system);
            InvalidateGpuBackendState(system);
        }
        if (actions->requestReset) {
            ResetSimulation(system, system->preset);
        }
        if (actions->requestCameraReset) {
            ResetCamera(orbit, camera, system);
        }
    }

    static void HandlePressedKey(ParticleSystem3D *system, OrbitCameraState *orbit, Camera3D *camera, int key)
    {
        switch (key) {
            case KEY_V:
                system->viewMode = (ViewMode)((system->viewMode + 1) % VIEW_MODE_COUNT);
                break;
            case KEY_C:
                system->colorMode = (ColorMode)((system->colorMode + 1) % COLOR_MODE_COUNT);
                break;
            case KEY_O:
                CycleObstacleModel(system);
                break;
            case KEY_F:
                ToggleSimulationScene(system);
                break;
            case KEY_G:
                ToggleSimulationBackend(system);
                break;
            case KEY_ZERO:
                ResetCamera(orbit, camera, system);
                break;
            case KEY_TAB:
                SetFlyCameraMode(orbit, camera, system, !orbit->flyMode);
                break;
            case KEY_SPACE:
                system->paused = !system->paused;
                if (system->paused) {
                    system->audioOutputSignal = 0.0f;
                    system->audioOutputState = 0.0f;
                }
                break;
            case KEY_R:
                ResetSimulation(system, system->preset);
                break;
            case KEY_M: {
                const MaterialPreset nextPreset = (system->preset == MATERIAL_GAS) ? MATERIAL_WATER : MATERIAL_GAS;
                ResetSimulation(system, nextPreset);
            } break;
            case KEY_ONE:
                ResetSimulation(system, MATERIAL_WATER);
                break;
            case KEY_TWO:
                ResetSimulation(system, MATERIAL_GAS);
                break;
            case KEY_FOUR:
                SetTargetParticleCount(system, CountPresetForBackend(system->activeBackend, 0));
                break;
            case KEY_FIVE:
                SetTargetParticleCount(system, CountPresetForBackend(system->activeBackend, 1));
                break;
            case KEY_SIX:
                SetTargetParticleCount(system, CountPresetForBackend(system->activeBackend, 2));
                break;
            case KEY_SEVEN:
                SetTargetParticleCount(system, CountPresetForBackend(system->activeBackend, 3));
                break;
            case KEY_EIGHT:
                SetTargetParticleCount(system, CountPresetForBackend(system->activeBackend, 4));
                break;
            case KEY_LEFT_BRACKET:
                system->params.kinematicViscosity = fmaxf(0.05f, system->params.kinematicViscosity * 0.85f);
                RefreshDerivedSimulationParameters(system);
                break;
            case KEY_RIGHT_BRACKET:
                system->params.kinematicViscosity = fminf(80.0f, system->params.kinematicViscosity * 1.15f);
                RefreshDerivedSimulationParameters(system);
                break;
            case KEY_MINUS:
                system->params.gravity = fmaxf(0.0f, system->params.gravity - 60.0f);
                break;
            case KEY_EQUAL:
                system->params.gravity = fminf(1400.0f, system->params.gravity + 60.0f);
                break;
            default:
                break;
        }
    }

    static void ProcessInput(ParticleSystem3D *system, OrbitCameraState *orbit, Camera3D *camera,
        float frameDelta, bool allowMouseInput, bool allowKeyboardInput)
    {
        UpdateOrbitCamera(orbit, camera, system, frameDelta, allowMouseInput, allowKeyboardInput);
        UpdateInteraction(system, *camera, allowMouseInput);

        for (int key = GetKeyPressed(); key != 0; key = GetKeyPressed()) {
            if (allowKeyboardInput || ShortcutAllowedWhileUiFocused(key)) {
                HandlePressedKey(system, orbit, camera, key);
            }
        }
    }

    static void DrawWorld(const ParticleSystem3D *system, Camera3D camera)
    {
        DrawObstacle(system);
        DrawAcousticRig(system);
        if (system->showParticles) {
            DrawParticles(system, camera);
        }
        DrawSliceParticleHighlights(system, camera);
        DrawSlicePanels(system);
        DrawStreamlines(system);
        DrawPathlines(system);
        DrawTank(system);
    }

    static void Cleanup(ParticleSystem3D *system)
    {
        ShutdownAudioOutput(system);
        ShutdownGpuBackend(system);
        UnloadImportedObstacle(system);
        UnloadParticleInstanceRenderer(system);
        if (system->particleTexture.id != 0) {
            UnloadTexture(system->particleTexture);
        }
        if (system->particleShader.id != 0) {
            UnloadShader(system->particleShader);
        }
        if (system->cellCounts != NULL) MemFree(system->cellCounts);
        if (system->cellStarts != NULL) MemFree(system->cellStarts);
        if (system->cellOffsets != NULL) MemFree(system->cellOffsets);
        if (system->cellNeighborCounts != NULL) MemFree(system->cellNeighborCounts);
        if (system->cellNeighbors != NULL) MemFree(system->cellNeighbors);
        if (system->acousticEnvelope != NULL) MemFree(system->acousticEnvelope);
        if (system->acousticWaveform != NULL) MemFree(system->acousticWaveform);
        if (system->x != NULL) MemFree(system->x);
        if (system->y != NULL) MemFree(system->y);
        if (system->z != NULL) MemFree(system->z);
        if (system->vx != NULL) MemFree(system->vx);
        if (system->vy != NULL) MemFree(system->vy);
        if (system->vz != NULL) MemFree(system->vz);
        if (system->ax != NULL) MemFree(system->ax);
        if (system->ay != NULL) MemFree(system->ay);
        if (system->az != NULL) MemFree(system->az);
        if (system->density != NULL) MemFree(system->density);
        if (system->pressure != NULL) MemFree(system->pressure);
        if (system->temperature != NULL) MemFree(system->temperature);
        if (system->temperatureRate != NULL) MemFree(system->temperatureRate);
        if (system->dye != NULL) MemFree(system->dye);
        if (system->vorticity != NULL) MemFree(system->vorticity);
        if (system->xsphVX != NULL) MemFree(system->xsphVX);
        if (system->xsphVY != NULL) MemFree(system->xsphVY);
        if (system->xsphVZ != NULL) MemFree(system->xsphVZ);
        if (system->sortedX != NULL) MemFree(system->sortedX);
        if (system->sortedY != NULL) MemFree(system->sortedY);
        if (system->sortedZ != NULL) MemFree(system->sortedZ);
        if (system->sortedVX != NULL) MemFree(system->sortedVX);
        if (system->sortedVY != NULL) MemFree(system->sortedVY);
        if (system->sortedVZ != NULL) MemFree(system->sortedVZ);
        if (system->sortedTemperature != NULL) MemFree(system->sortedTemperature);
        if (system->sortedDensity != NULL) MemFree(system->sortedDensity);
        if (system->sortedPressure != NULL) MemFree(system->sortedPressure);
        if (system->particleCells != NULL) MemFree(system->particleCells);
        if (system->sortedCellIndices != NULL) MemFree(system->sortedCellIndices);
        if (system->sortedIndices != NULL) MemFree(system->sortedIndices);
        if (system->drawItems != NULL) MemFree(system->drawItems);
    }

    int main(int argc, char **argv)
    {
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
        InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "FluidSim - 3D SPH");
        SetTargetFPS(144);

        ParticleSystem3D system;
        AllocateSystem(&system, MAX_PARTICLES_3D);
        system.targetParticleCount = DEFAULT_TARGET_PARTICLES_3D;
        system.particleTexture = CreateWhiteTexture();
        system.particleShader = LoadParticleShader(&system.particleShaderSoftPowerLoc);
        ResetSimulation(&system, MATERIAL_WATER);

        BakeSession3D bake;
        BakeSessionInit(&bake);

        if (InitializeGpuBackend(&system)) {
            SetSimulationBackend(&system, SIM_BACKEND_GPU);
        }

        const char *startupImportPath = NULL;
        if (argc > 1 && argv[1] != NULL && argv[1][0] != '\0') {
            startupImportPath = argv[1];
        } else {
            startupImportPath = getenv("FLUIDSIM_IMPORT_OBJ");
        }

        if (startupImportPath != NULL && startupImportPath[0] != '\0') {
            if (LoadImportedObstacle(&system, startupImportPath)) {
                SetBackendNotice(&system, "Imported OBJ from startup path.");
            } else {
                SetBackendNotice(&system, "Failed to import OBJ from startup path.");
            }
        }

        Camera3D camera = {0};
        OrbitCameraState orbit = {0};
        ResetCamera(&orbit, &camera, &system);

        Ui3DPanelSetup();
        BuildGrid(&system);
        ComputeDensityAndPressure(&system);
        UpdateDiagnostics(&system);
        system.scalarFieldsDirty = false;
        system.framesUntilDiagnostics = DIAGNOSTIC_REFRESH_INTERVAL;

        while (!WindowShouldClose()) {
            const float frameDelta = ClampFloat(GetFrameTime(), 0.0f, 1.0f / 20.0f);

            BeginDrawing();
            ClearBackground((Color){5, 10, 18, 255});

            Ui3DPanelBegin(frameDelta);
            const Ui3DPanelState panelState = BuildUiPanelState(&system, &orbit, &bake);
            Ui3DPanelActions panelActions;
            Ui3DPanelDraw(&panelState, &panelActions);
            const Ui3DCaptureState captureState = Ui3DPanelGetCaptureState();

            ApplyUiPanelActions(&system, &panelActions, &orbit, &camera);

            const bool liveSceneChanged =
                panelActions.setBackend || panelActions.setTargetParticleCount || panelActions.setMode ||
                panelActions.setObstacleModel || panelActions.setObstacleAngleDegrees ||
                panelActions.setObstacleRectWidth || panelActions.setObstacleRectHeight ||
                panelActions.setWaterObstacleEnabled || panelActions.setWaterObstacleShape ||
                panelActions.setWaterCylinderCount ||
                panelActions.setWaterCylinderSelected || panelActions.setWaterCylinderX ||
                panelActions.setWaterCylinderY || panelActions.setWaterCylinderZ ||
                panelActions.setWaterCylinderRadius || panelActions.setWaterCylinderDepth ||
                panelActions.setWaterRectangleWidth || panelActions.setWaterRectangleHeight ||
                panelActions.setWaterRectangleAngleDegrees ||
                panelActions.requestImportObstacle || panelActions.setImportedScale ||
                panelActions.setImportedOffsetX || panelActions.setImportedOffsetY || panelActions.setImportedOffsetZ ||
                panelActions.setImportedRotationX || panelActions.setImportedRotationY || panelActions.setImportedRotationZ ||
                panelActions.requestResetImportedRotation || panelActions.setAcousticsEnabled ||
                panelActions.setSpeakerFrequency || panelActions.setSpeakerAmplitude ||
                panelActions.setAcousticSoundSpeed || panelActions.setAcousticMachLimit ||
                panelActions.setAcousticViscosityScale || panelActions.setAcousticDragScale ||
                panelActions.setAcousticSlowdownFactor ||
                panelActions.requestLoadAcousticAudio ||
                panelActions.setSpeakerWidth || panelActions.setSpeakerHeight || panelActions.setSpeakerDepth ||
                panelActions.setMicRadius || panelActions.requestReset;
            if (liveSceneChanged && bake.status != BAKE_STATUS_BAKING && !panelActions.requestBakeStart) {
                bake.status = BAKE_STATUS_IDLE;
                bake.hasCache = false;
                bake.playbackPlaying = false;
                bake.progress = 0.0f;
                bake.bakedTime = 0.0f;
                BakeSetNotice(&bake, "Bake cache detached after scene edit.");
            }

            if (panelActions.setBakeDuration) {
                bake.duration = ClampFloat(panelActions.bakeDuration, BAKE_MIN_DURATION_SECONDS, BAKE_MAX_DURATION_SECONDS);
            }
            if (panelActions.setBakeParticleCount) {
                bake.particleTarget = ClampInt(panelActions.bakeParticleCount, 1, BAKE_MAX_PARTICLE_COUNT);
            }
            if (panelActions.requestBakeStop) {
                BakeStop(&bake);
                system.bakeCapacityMode = false;
                BakeWriteManifest(&system, &bake);
            }
            if (panelActions.requestBakeLoadLatest) {
                (void)BakeLoadLatest(&system, &bake);
            }
            if (panelActions.requestBakeExportMic) {
                (void)BakeExportMicWav(&system, &bake);
            }
            if (panelActions.requestBakeStart) {
                BakeStart(&system, &bake);
            }
            if (panelActions.setBakePlaybackPlaying && bake.hasCache && bake.status != BAKE_STATUS_BAKING) {
                bake.playbackPlaying = panelActions.bakePlaybackPlaying;
                if (bake.playbackPlaying) {
                    bake.status = BAKE_STATUS_PLAYBACK;
                }
            }
            if (panelActions.setBakePlaybackTime && bake.status != BAKE_STATUS_BAKING) {
                (void)BakeSetPlaybackTime(&system, &bake, panelActions.bakePlaybackTime);
            }

            ProcessInput(&system, &orbit, &camera, frameDelta, !captureState.wantsMouse, !captureState.wantsKeyboard);

            if (bake.status == BAKE_STATUS_BAKING) {
                BakeUpdate(&system, &bake);
            } else if (bake.status == BAKE_STATUS_PLAYBACK || bake.playbackPlaying) {
                BakePlaybackUpdate(&system, &bake, frameDelta);
            } else if (!bake.hasCache && !system.paused) {
                StepSimulation(&system, frameDelta);
            } else {
                const bool needsFreshDensity = ColorModeNeedsFreshDensity(system.colorMode);
                const bool refreshDiagnostics = needsFreshDensity || system.framesUntilDiagnostics <= 0;

                if (system.scalarFieldsDirty && (needsFreshDensity || refreshDiagnostics)) {
                    BuildGrid(&system);
                    ComputeDensityAndPressure(&system);
                    system.scalarFieldsDirty = false;
                }

                if (refreshDiagnostics) {
                    UpdateDiagnostics(&system);
                    system.framesUntilDiagnostics = DiagnosticRefreshIntervalForBackend(system.activeBackend);
                } else {
                    system.framesUntilDiagnostics -= 1;
                }

                system.lastSimStepMs = 0.0;
                system.lastStepDt = 0.0f;
            }

            const bool playbackMode = (bake.status == BAKE_STATUS_PLAYBACK);
            const bool pausedBeforeVisualization = system.paused;
            if (playbackMode) {
                system.paused = true;
            }
            RefreshVisualizationState(&system, playbackMode ? 0.0f : frameDelta);
            system.paused = pausedBeforeVisualization;

            BeginMode3D(camera);
            DrawWorld(&system, camera);
            EndMode3D();

            DrawMicOverlay(&system, camera);
            DrawInteractionOverlay(&system, camera);
            DrawHud(&system);
            Ui3DPanelEnd();
            EndDrawing();
        }

        Ui3DPanelShutdown();
        BakeSessionFree(&bake);
        Cleanup(&system);
        CloseWindow();
        return 0;
    }
