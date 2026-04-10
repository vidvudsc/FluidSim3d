#include <metal_stdlib>

using namespace metal;

#define CAR_SHAPE_SCALE_X 0.82f
#define CAR_SHAPE_SCALE_Y 1.08f

struct GpuSimParams3D {
    uint particleCount;
    uint gridWidth;
    uint gridHeight;
    uint gridDepth;
    uint cellCount;
    uint preset;
    uint scene;
    uint obstacleModel;
    uint interactionActive;
    uint acousticsEnabled;
    uint importedSdfWidth;
    uint importedSdfHeight;
    uint importedSdfDepth;
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
};

constant uint MATERIAL_WATER = 0u;
constant uint MATERIAL_GAS = 1u;
constant uint SCENE_TANK = 0u;
constant uint SCENE_WIND_TUNNEL = 1u;
constant uint OBSTACLE_CIRCLE = 0u;
constant uint OBSTACLE_AIRFOIL = 1u;
constant uint OBSTACLE_CAR = 2u;
constant uint OBSTACLE_RECTANGLE = 3u;
constant uint OBSTACLE_IMPORTED = 4u;
constant float TEMP_MIN = 0.35f;
constant float TEMP_MAX = 2.50f;
constant float PI_F = 3.14159265358979323846f;

constant uint AIRFOIL_OUTLINE_POINT_COUNT = 45u;
constant float2 AIRFOIL_OUTLINE_POINTS[AIRFOIL_OUTLINE_POINT_COUNT] = {
    float2(1.9919f, 0.0067f), float2(1.9415f, 0.0171f), float2(1.8914f, 0.0272f), float2(1.7439f, 0.0560f),
    float2(1.6478f, 0.0738f), float2(1.4138f, 0.1149f), float2(1.2779f, 0.1372f), float2(0.9748f, 0.1828f),
    float2(0.8103f, 0.2051f), float2(0.4608f, 0.2467f), float2(0.2788f, 0.2653f), float2(-0.0905f, 0.2950f),
    float2(-0.2748f, 0.3054f), float2(-0.6368f, 0.3150f), float2(-0.8115f, 0.3130f), float2(-1.1373f, 0.2944f),
    float2(-1.2856f, 0.2785f), float2(-1.5474f, 0.2348f), float2(-1.6584f, 0.2079f), float2(-1.8354f, 0.1478f),
    float2(-1.9000f, 0.1157f), float2(-1.9791f, 0.0496f), float2(-1.9899f, -0.0149f), float2(-1.9699f, -0.0445f),
    float2(-1.8805f, -0.0946f), float2(-1.8122f, -0.1148f), float2(-1.6312f, -0.1454f), float2(-1.5198f, -0.1557f),
    float2(-1.2613f, -0.1671f), float2(-1.1163f, -0.1686f), float2(-0.7994f, -0.1646f), float2(-0.6299f, -0.1598f),
    float2(-0.2757f, -0.1467f), float2(-0.0941f, -0.1383f), float2(0.2717f, -0.1185f), float2(0.4525f, -0.1078f),
    float2(0.8006f, -0.0861f), float2(0.9650f, -0.0755f), float2(1.2690f, -0.0554f), float2(1.4058f, -0.0463f),
    float2(1.6419f, -0.0302f), float2(1.7392f, -0.0235f), float2(1.8889f, -0.0130f), float2(1.9399f, -0.0093f),
    float2(1.9916f, 0.0019f),
};

constant uint CAR_OUTLINE_POINT_COUNT = 35u;
constant float2 CAR_OUTLINE_POINTS[CAR_OUTLINE_POINT_COUNT] = {
    float2(-2.1600f, 0.1150f), float2(-2.0600f, -0.0550f), float2(-1.9800f, -0.1250f), float2(-1.8750f, -0.1750f),
    float2(-1.5950f, -0.2300f), float2(-1.2500f, -0.2850f), float2(-1.0700f, -0.3350f), float2(-0.8850f, -0.3950f),
    float2(-0.4900f, -0.5150f), float2(-0.0500f, -0.5600f), float2(0.1700f, -0.5600f), float2(0.3850f, -0.5450f),
    float2(0.7900f, -0.4800f), float2(1.1300f, -0.4000f), float2(1.2700f, -0.3600f), float2(1.4000f, -0.3250f),
    float2(1.6400f, -0.2850f), float2(1.8800f, -0.3250f), float2(2.0000f, -0.3750f), float2(2.1050f, -0.2550f),
    float2(2.1600f, -0.1100f), float2(2.1150f, 0.1200f), float2(2.0250f, 0.1600f), float2(1.8800f, 0.1850f),
    float2(1.4550f, 0.2000f), float2(0.9500f, 0.2000f), float2(0.6900f, 0.2000f), float2(0.4300f, 0.2000f),
    float2(-0.0900f, 0.2000f), float2(-0.6150f, 0.2000f), float2(-0.8850f, 0.2000f), float2(-1.1400f, 0.2000f),
    float2(-1.6000f, 0.1950f), float2(-1.9700f, 0.1750f), float2(-2.1100f, 0.1650f),
};

inline float clampf(float value, float minValue, float maxValue)
{
    return min(max(value, minValue), maxValue);
}

inline int clampCellCoord(int value, uint maxValue)
{
    return clamp(value, 0, (int)maxValue);
}

inline float pow7(float x)
{
    float x2 = x * x;
    float x4 = x2 * x2;
    return x4 * x2 * x;
}

inline uint hashUint32(uint x)
{
    x = x * 747796405u + 2891336453u;
    x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
    return (x >> 22u) ^ x;
}

inline float hashNoise(uint x)
{
    return float(hashUint32(x) & 0x00FFFFFFu) / float(0x00FFFFFFu);
}

inline bool sceneIsWindTunnel(constant GpuSimParams3D &params)
{
    return params.scene == SCENE_WIND_TUNNEL;
}

inline bool acousticsActive(constant GpuSimParams3D &params)
{
    return params.acousticsEnabled != 0u;
}

inline float localRestDensity(constant GpuSimParams3D &params, float temperature)
{
    float delta = temperature - params.ambientTemperature;
    float adjusted = params.restDensity * (1.0f - params.thermalExpansion * delta);
    return max(0.35f * params.restDensity, adjusted);
}

inline float densityFloor(constant GpuSimParams3D &params)
{
    return (params.preset == MATERIAL_GAS)
        ? 0.10f * params.restDensity
        : 0.50f * params.restDensity;
}

inline float xsphBlend(constant GpuSimParams3D &params)
{
    return (params.preset == MATERIAL_GAS) ? 0.0f : 0.055f;
}

inline float tunnelAxisProfile(float value, float minValue, float maxValue)
{
    float t = clampf((value - minValue) / max(maxValue - minValue, 1e-6f), 0.0f, 1.0f);
    float centered = 2.0f * t - 1.0f;
    return max(0.12f, 1.0f - centered * centered);
}

inline float windTunnelProfile(constant GpuSimParams3D &params, float y, float z)
{
    float radius = params.particleRadius;
    float minY = params.boundsMinY + radius;
    float maxY = params.boundsMinY + params.boundsSizeY - radius;
    float minZ = params.boundsMinZ + radius;
    float maxZ = params.boundsMinZ + params.boundsSizeZ - radius;
    return tunnelAxisProfile(y, minY, maxY) * tunnelAxisProfile(z, minZ, maxZ);
}

inline void respawnWindTunnelParticle(constant GpuSimParams3D &params,
    uint particleIndex,
    thread float &x,
    thread float &y,
    thread float &z,
    thread float &vx,
    thread float &vy,
    thread float &vz,
    thread float &temperature)
{
    float radius = params.particleRadius;
    float left = params.boundsMinX + radius;
    float top = params.boundsMinY + radius;
    float bottom = params.boundsMinY + params.boundsSizeY - radius;
    float front = params.boundsMinZ + radius;
    float back = params.boundsMinZ + params.boundsSizeZ - radius;
    float inletDepth = max(params.supportRadius * 1.6f, radius * 4.0f);
    float tunnelHeight = max(bottom - top, radius * 2.0f);
    float tunnelDepth = max(back - front, radius * 2.0f);
    uint timeKey = (uint)floor(params.simulationTime * 1536.0f);
    uint seedBase = timeKey + particleIndex * 97u;
    float particleY = clampf(top + hashNoise(seedBase + 17u) * tunnelHeight +
        (hashNoise(seedBase + 41u) - 0.5f) * params.supportRadius * 0.11f, top, bottom);
    float particleZ = clampf(front + hashNoise(seedBase + 59u) * tunnelDepth +
        (hashNoise(seedBase + 73u) - 0.5f) * params.supportRadius * 0.09f, front, back);
    float profile = windTunnelProfile(params, particleY, particleZ);

    x = left + hashNoise(seedBase + 109u) * inletDepth;
    y = particleY;
    z = particleZ;
    vx = params.flowTargetSpeed * profile +
        (hashNoise(seedBase + 149u) - 0.5f) * params.soundSpeed * 0.020f;
    vy = (hashNoise(seedBase + 191u) - 0.5f) * params.soundSpeed * 0.014f;
    vz = (hashNoise(seedBase + 211u) - 0.5f) * params.soundSpeed * 0.014f;
    temperature = clampf(params.ambientTemperature + (hashNoise(seedBase + 233u) - 0.5f) * 0.02f, TEMP_MIN, TEMP_MAX);
}

inline float distanceSquaredToSegment(float2 p, float2 a, float2 b)
{
    float2 ab = b - a;
    float2 ap = p - a;
    float abLengthSquared = dot(ab, ab);
    float t = (abLengthSquared > 1e-8f)
        ? clamp(dot(ap, ab) / abLengthSquared, 0.0f, 1.0f)
        : 0.0f;
    float2 closest = a + ab * t;
    float2 delta = p - closest;
    return dot(delta, delta);
}

inline float polygonSignedDistanceScaled(constant float2 *points, uint pointCount, float2 p, float scale)
{
    float minDistanceSquared = INFINITY;
    bool inside = false;

    for (uint i = 0u, j = pointCount - 1u; i < pointCount; j = i++) {
        float2 a = points[i] * scale;
        float2 b = points[j] * scale;
        minDistanceSquared = min(minDistanceSquared, distanceSquaredToSegment(p, a, b));

        float yDelta = b.y - a.y;
        bool intersects = ((a.y > p.y) != (b.y > p.y)) &&
            (p.x < (b.x - a.x) * (p.y - a.y) / (fabs(yDelta) > 1e-6f ? yDelta : 1e-6f) + a.x);
        if (intersects) {
            inside = !inside;
        }
    }

    float distance = sqrt(max(minDistanceSquared, 0.0f));
    return inside ? -distance : distance;
}

inline float polygonSignedDistanceScaledXY(constant float2 *points, uint pointCount, float2 p, float2 scale)
{
    float minDistanceSquared = INFINITY;
    bool inside = false;

    for (uint i = 0u, j = pointCount - 1u; i < pointCount; j = i++) {
        float2 a = float2(points[i].x * scale.x, points[i].y * scale.y);
        float2 b = float2(points[j].x * scale.x, points[j].y * scale.y);
        minDistanceSquared = min(minDistanceSquared, distanceSquaredToSegment(p, a, b));

        float yDelta = b.y - a.y;
        bool intersects = ((a.y > p.y) != (b.y > p.y)) &&
            (p.x < (b.x - a.x) * (p.y - a.y) / (fabs(yDelta) > 1e-6f ? yDelta : 1e-6f) + a.x);
        if (intersects) {
            inside = !inside;
        }
    }

    float distance = sqrt(max(minDistanceSquared, 0.0f));
    return inside ? -distance : distance;
}

inline float rectangleSignedDistance(float2 p, float2 halfSize)
{
    float2 d = abs(p) - halfSize;
    float ox = max(d.x, 0.0f);
    float oy = max(d.y, 0.0f);
    return sqrt(ox * ox + oy * oy) + min(max(d.x, d.y), 0.0f);
}

inline float boxSignedDistance(float3 p, float3 halfSize)
{
    float3 d = abs(p) - halfSize;
    float3 outside = max(d, float3(0.0f));
    return length(outside) + min(max(d.x, max(d.y, d.z)), 0.0f);
}

inline float3 obstacleWorldToLocal(constant GpuSimParams3D &params, float x, float y, float z)
{
    float2 p = float2(x - params.obstacleCenterX, y - params.obstacleCenterY);
    return float3(
        params.obstacleAngleCos * p.x + params.obstacleAngleSin * p.y,
        -params.obstacleAngleSin * p.x + params.obstacleAngleCos * p.y,
        z - params.obstacleCenterZ
    );
}

inline float extrudedSignedDistance(float baseDistance, float localZ, float halfDepth)
{
    float dz = fabs(localZ) - halfDepth;
    float outsideX = max(baseDistance, 0.0f);
    float outsideY = max(dz, 0.0f);
    return sqrt(outsideX * outsideX + outsideY * outsideY) + min(max(baseDistance, dz), 0.0f);
}

inline float sampleImportedObstacleSdf(constant GpuSimParams3D &params,
    const device float *importedSdf,
    float3 p);
inline float sampleImportedObstacleSdfWorld(constant GpuSimParams3D &params,
    const device float *importedSdf,
    float3 p);

inline float obstacleSignedDistanceLocal(constant GpuSimParams3D &params, const device float *importedSdf, float3 p)
{
    float baseDistance = 0.0f;
    switch (params.obstacleModel) {
        case OBSTACLE_AIRFOIL:
            baseDistance = polygonSignedDistanceScaled(AIRFOIL_OUTLINE_POINTS, AIRFOIL_OUTLINE_POINT_COUNT, float2(p.x, p.y),
                params.obstacleRadius);
            break;
        case OBSTACLE_CAR:
            baseDistance = polygonSignedDistanceScaledXY(CAR_OUTLINE_POINTS, CAR_OUTLINE_POINT_COUNT, float2(p.x, -p.y),
                float2(params.obstacleRadius * CAR_SHAPE_SCALE_X, params.obstacleRadius * CAR_SHAPE_SCALE_Y));
            break;
        case OBSTACLE_RECTANGLE:
            baseDistance = rectangleSignedDistance(float2(p.x, p.y), float2(params.obstacleRectHalfWidth, params.obstacleRectHalfHeight));
            break;
        case OBSTACLE_IMPORTED:
            return sampleImportedObstacleSdf(params, importedSdf, p);
        case OBSTACLE_CIRCLE:
        default:
            baseDistance = length(float2(p.x, p.y)) - params.obstacleRadius;
            break;
    }

    return extrudedSignedDistance(baseDistance, p.z, params.obstacleHalfDepth);
}

inline float obstacleSignedDistance(constant GpuSimParams3D &params, const device float *importedSdf, float x, float y, float z)
{
    if (params.obstacleModel == OBSTACLE_IMPORTED) {
        return sampleImportedObstacleSdfWorld(params, importedSdf, float3(x, y, z));
    }
    return obstacleSignedDistanceLocal(params, importedSdf, obstacleWorldToLocal(params, x, y, z));
}

inline float3 obstacleNormal(constant GpuSimParams3D &params, const device float *importedSdf, float x, float y, float z)
{
    float epsilon = max(params.particleRadius * 0.65f, 0.75f);
    float dx = obstacleSignedDistance(params, importedSdf, x + epsilon, y, z) - obstacleSignedDistance(params, importedSdf, x - epsilon, y, z);
    float dy = obstacleSignedDistance(params, importedSdf, x, y + epsilon, z) - obstacleSignedDistance(params, importedSdf, x, y - epsilon, z);
    float dz = obstacleSignedDistance(params, importedSdf, x, y, z + epsilon) - obstacleSignedDistance(params, importedSdf, x, y, z - epsilon);
    float3 normal = float3(dx, dy, dz);
    float len = length(normal);
    if (len > 1e-6f) {
        return normal / len;
    }

    float3 fallback = float3(x - params.obstacleCenterX, y - params.obstacleCenterY, z - params.obstacleCenterZ);
    float fallbackLen = length(fallback);
    if (fallbackLen > 1e-6f) {
        return fallback / fallbackLen;
    }
    return float3(1.0f, 0.0f, 0.0f);
}

inline float speakerSignedDistance(constant GpuSimParams3D &params, float x, float y, float z)
{
    return boxSignedDistance(float3(x - params.speakerCenterX, y - params.speakerCenterY, z - params.speakerCenterZ),
        float3(params.speakerHalfWidth, params.speakerHalfHeight, params.speakerHalfDepth));
}

inline float3 speakerNormal(constant GpuSimParams3D &params, float x, float y, float z)
{
    float epsilon = max(params.particleRadius * 0.65f, 0.75f);
    float dx = speakerSignedDistance(params, x + epsilon, y, z) - speakerSignedDistance(params, x - epsilon, y, z);
    float dy = speakerSignedDistance(params, x, y + epsilon, z) - speakerSignedDistance(params, x, y - epsilon, z);
    float dz = speakerSignedDistance(params, x, y, z + epsilon) - speakerSignedDistance(params, x, y, z - epsilon);
    float3 normal = float3(dx, dy, dz);
    float len = length(normal);
    if (len > 1e-6f) {
        return normal / len;
    }
    return float3(1.0f, 0.0f, 0.0f);
}

inline float sampleImportedObstacleSdf(constant GpuSimParams3D &params,
    const device float *importedSdf,
    float3 p)
{
    if (params.importedSdfWidth < 2u || params.importedSdfHeight < 2u || params.importedSdfDepth < 2u) {
        return 1e6f;
    }

    float3 minBox = float3(params.importedSdfMinX, params.importedSdfMinY, params.importedSdfMinZ);
    float3 maxBox = minBox + float3(params.importedSdfSizeX, params.importedSdfSizeY, params.importedSdfSizeZ);
    float3 clamped = clamp(p, minBox, maxBox);
    float outsideDistance = length(p - clamped);
    float3 size = maxBox - minBox;
    if (size.x <= 1e-6f || size.y <= 1e-6f || size.z <= 1e-6f) {
        return 1e6f;
    }

    float gx = ((clamped.x - minBox.x) / size.x) * float(params.importedSdfWidth) - 0.5f;
    float gy = ((clamped.y - minBox.y) / size.y) * float(params.importedSdfHeight) - 0.5f;
    float gz = ((clamped.z - minBox.z) / size.z) * float(params.importedSdfDepth) - 0.5f;

    uint x0 = clamp((uint)max((int)floor(gx), 0), 0u, params.importedSdfWidth - 1u);
    uint y0 = clamp((uint)max((int)floor(gy), 0), 0u, params.importedSdfHeight - 1u);
    uint z0 = clamp((uint)max((int)floor(gz), 0), 0u, params.importedSdfDepth - 1u);
    uint x1 = min(x0 + 1u, params.importedSdfWidth - 1u);
    uint y1 = min(y0 + 1u, params.importedSdfHeight - 1u);
    uint z1 = min(z0 + 1u, params.importedSdfDepth - 1u);
    float tx = clampf(gx - float(x0), 0.0f, 1.0f);
    float ty = clampf(gy - float(y0), 0.0f, 1.0f);
    float tz = clampf(gz - float(z0), 0.0f, 1.0f);

    uint strideY = params.importedSdfWidth;
    uint strideZ = params.importedSdfWidth * params.importedSdfHeight;
    #define SDF_AT(ix, iy, iz) importedSdf[(iz) * strideZ + (iy) * strideY + (ix)]
    float c000 = SDF_AT(x0, y0, z0);
    float c100 = SDF_AT(x1, y0, z0);
    float c010 = SDF_AT(x0, y1, z0);
    float c110 = SDF_AT(x1, y1, z0);
    float c001 = SDF_AT(x0, y0, z1);
    float c101 = SDF_AT(x1, y0, z1);
    float c011 = SDF_AT(x0, y1, z1);
    float c111 = SDF_AT(x1, y1, z1);
    #undef SDF_AT

    float c00 = mix(c000, c100, tx);
    float c10 = mix(c010, c110, tx);
    float c01 = mix(c001, c101, tx);
    float c11 = mix(c011, c111, tx);
    float c0 = mix(c00, c10, ty);
    float c1 = mix(c01, c11, ty);
    return mix(c0, c1, tz) + outsideDistance;
}

inline float3 rotateX3(float3 v, float radians)
{
    float c = cos(radians);
    float s = sin(radians);
    return float3(v.x, c * v.y - s * v.z, s * v.y + c * v.z);
}

inline float3 rotateY3(float3 v, float radians)
{
    float c = cos(radians);
    float s = sin(radians);
    return float3(c * v.x + s * v.z, v.y, -s * v.x + c * v.z);
}

inline float3 rotateZ3(float3 v, float radians)
{
    float c = cos(radians);
    float s = sin(radians);
    return float3(c * v.x - s * v.y, s * v.x + c * v.y, v.z);
}

inline float sampleImportedObstacleSdfWorld(constant GpuSimParams3D &params,
    const device float *importedSdf,
    float3 p)
{
    float3 local = p - float3(params.importedCenterX, params.importedCenterY, params.importedCenterZ);
    float scale = max(params.importedScale, 0.05f);
    local /= scale;
    local = float3(
        params.importedInvRotXX * local.x + params.importedInvRotXY * local.y + params.importedInvRotXZ * local.z,
        params.importedInvRotYX * local.x + params.importedInvRotYY * local.y + params.importedInvRotYZ * local.z,
        params.importedInvRotZX * local.x + params.importedInvRotZY * local.y + params.importedInvRotZZ * local.z
    );

    return sampleImportedObstacleSdf(params, importedSdf, local) * scale;
}

inline void addBoundaryDensityContribution(constant GpuSimParams3D &params, float x, float y, float z, thread float &density)
{
    if (params.preset == MATERIAL_GAS) {
        return;
    }

    float radius = params.particleRadius;
    float minX = params.boundsMinX + radius;
    float maxX = params.boundsMinX + params.boundsSizeX - radius;
    float minY = params.boundsMinY + radius;
    float maxY = params.boundsMinY + params.boundsSizeY - radius;
    float minZ = params.boundsMinZ + radius;
    float maxZ = params.boundsMinZ + params.boundsSizeZ - radius;
    float boundaryMass = params.mass * 1.12f;
    float h2 = params.supportRadiusSquared;

    float distances[6] = {
        x - minX, maxX - x, y - minY, maxY - y, z - minZ, maxZ - z,
    };

    for (uint wall = 0; wall < 6u; ++wall) {
        if (sceneIsWindTunnel(params) && wall < 2u) {
            continue;
        }
        float mirroredDistance = 2.0f * distances[wall];
        float r2 = mirroredDistance * mirroredDistance;
        if (r2 < h2) {
            float diff = h2 - r2;
            density += boundaryMass * params.densityKernel * diff * diff * diff;
        }
    }
}

inline float pressureFromState(constant GpuSimParams3D &params, float density, float temperature)
{
    if (params.preset == MATERIAL_GAS) {
        float normalizedTemperature = max(0.30f, temperature);
        float gasStiffness = 0.65f * params.soundSpeed * params.soundSpeed;
        return gasStiffness * density * normalizedTemperature;
    }

    float ratio = density / localRestDensity(params, temperature);
    float pressure = params.pressureStiffness * (pow7(ratio) - 1.0f);
    return max(0.0f, pressure);
}

inline void addBoundaryForceContribution(constant GpuSimParams3D &params,
    float x, float y, float z,
    float vx, float vy, float vz,
    float rhoi, float pressureI, float temperature,
    thread float &ax, thread float &ay, thread float &az)
{
    float radius = params.particleRadius;
    float h = params.supportRadius;
    float h2 = params.supportRadiusSquared;
    float minX = params.boundsMinX + radius;
    float maxX = params.boundsMinX + params.boundsSizeX - radius;
    float minY = params.boundsMinY + radius;
    float maxY = params.boundsMinY + params.boundsSizeY - radius;
    float minZ = params.boundsMinZ + radius;
    float maxZ = params.boundsMinZ + params.boundsSizeZ - radius;

    float distances[6] = {
        x - minX, maxX - x, y - minY, maxY - y, z - minZ, maxZ - z,
    };
    float3 normals[6] = {
        float3(1.0f, 0.0f, 0.0f), float3(-1.0f, 0.0f, 0.0f),
        float3(0.0f, 1.0f, 0.0f), float3(0.0f, -1.0f, 0.0f),
        float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, -1.0f),
    };

    if (params.preset == MATERIAL_GAS) {
        float wallK = 2.2f * params.soundSpeed * params.soundSpeed / h;
        float wallDamp = 1.2f * params.soundSpeed / h;

        for (uint wall = 0; wall < 6u; ++wall) {
            if (sceneIsWindTunnel(params) && wall < 2u) {
                continue;
            }
            float d = distances[wall];
            if (d < h) {
                float falloff = 1.0f - d / h;
                float vn = dot(float3(vx, vy, vz), normals[wall]);
                float repulse = wallK * falloff * falloff;
                float damp = -wallDamp * min(vn, 0.0f);
                ax += normals[wall].x * (repulse + damp);
                ay += normals[wall].y * (repulse + damp);
                az += normals[wall].z * (repulse + damp);
            }
        }
        return;
    }

    float boundaryMass = params.mass * 1.12f;
    float boundaryDensity = localRestDensity(params, temperature);
    float boundaryPressure = max(pressureI, 0.10f * params.pressureStiffness);

    for (uint wall = 0; wall < 6u; ++wall) {
        if (sceneIsWindTunnel(params) && wall < 2u) {
            continue;
        }
        float mirroredDistance = 2.0f * distances[wall];
        float r2 = mirroredDistance * mirroredDistance;
        if (r2 > 1e-8f && r2 < h2) {
            float r = sqrt(r2);
            float influence = h - r;
            float pressureTerm = -boundaryMass *
                ((pressureI / (rhoi * rhoi)) + (boundaryPressure / (boundaryDensity * boundaryDensity))) *
                params.pressureKernelGrad * influence * influence;
            float viscosityTerm = params.kinematicViscosity * boundaryMass *
                params.viscosityKernelLap * influence / boundaryDensity;
            ax += pressureTerm * normals[wall].x;
            ay += pressureTerm * normals[wall].y;
            az += pressureTerm * normals[wall].z;
            ax -= viscosityTerm * vx * fabs(normals[wall].x);
            ay -= viscosityTerm * vy * fabs(normals[wall].y);
            az -= viscosityTerm * vz * fabs(normals[wall].z);
        }
    }
}

inline void addSceneForceContribution(constant GpuSimParams3D &params,
    const device float *importedSdf,
    float x, float y, float z,
    float vx, float vy, float vz,
    thread float &ax, thread float &ay, thread float &az)
{
    if (!sceneIsWindTunnel(params)) {
        return;
    }

    float profile = windTunnelProfile(params, y, z);
    float xNorm = clampf((x - params.boundsMinX) / max(params.boundsSizeX, 1e-6f), 0.0f, 1.0f);
    float driveBias = 1.18f - 0.22f * xNorm;
    ax += params.flowDrive * profile * driveBias *
        (1.0f - vx / max(params.flowTargetSpeed, 1e-4f));
    ay -= params.flowDrive * 0.030f * vy;
    az -= params.flowDrive * 0.030f * vz;

    float signedDistance = obstacleSignedDistance(params, importedSdf, x, y, z);
    if (signedDistance < params.obstacleShell) {
        float3 normal = obstacleNormal(params, importedSdf, x, y, z);
        float falloff = clampf(1.0f - signedDistance / params.obstacleShell, 0.0f, 1.8f);
        float repulse = params.obstacleStrength * falloff * falloff;
        float vn = dot(float3(vx, vy, vz), normal);
        float damp = -params.obstacleDamping * min(vn, 0.0f);
        ax += normal.x * (repulse + damp);
        ay += normal.y * (repulse + damp);
        az += normal.z * (repulse + damp);
    }
}

inline void addSpeakerForceContribution(constant GpuSimParams3D &params,
    float x, float y, float z,
    float vx, float vy, float vz,
    thread float &ax, thread float &ay, thread float &az)
{
    if (!acousticsActive(params)) {
        return;
    }

    float signedDistance = speakerSignedDistance(params, x, y, z);
    if (signedDistance >= params.speakerShell) {
        return;
    }

    float3 normal = speakerNormal(params, x, y, z);
    float3 speakerVelocity = float3(params.speakerVelocityX, params.speakerVelocityY, params.speakerVelocityZ);
    float falloff = clampf(1.0f - signedDistance / params.speakerShell, 0.0f, 1.8f);
    float repulse = params.speakerStrength * falloff * falloff;
    float relativeVn = dot(float3(vx, vy, vz) - speakerVelocity, normal);
    float damp = -params.speakerDamping * min(relativeVn, 0.0f);
    ax += normal.x * (repulse + damp);
    ay += normal.y * (repulse + damp);
    az += normal.z * (repulse + damp);
}

inline void resolveObstacle(constant GpuSimParams3D &params,
    const device float *importedSdf,
    thread float &x, thread float &y, thread float &z,
    thread float &vx, thread float &vy, thread float &vz)
{
    if (!sceneIsWindTunnel(params)) {
        return;
    }

    float surfaceOffset = params.particleRadius * ((params.obstacleModel == OBSTACLE_CAR) ? 1.20f : 0.70f);
    float3 normal = float3(1.0f, 0.0f, 0.0f);
    bool resolved = false;

    uint maxIterations = (params.obstacleModel == OBSTACLE_CAR) ? 4u : 2u;
    for (uint iteration = 0u; iteration < maxIterations; ++iteration) {
        float signedDistance = obstacleSignedDistance(params, importedSdf, x, y, z);
        if (signedDistance >= surfaceOffset) {
            break;
        }

        normal = obstacleNormal(params, importedSdf, x, y, z);
        float pushOut = (surfaceOffset - signedDistance) +
            params.particleRadius * ((params.obstacleModel == OBSTACLE_CAR) ? 0.18f : 0.08f);
        x += normal.x * pushOut;
        y += normal.y * pushOut;
        z += normal.z * pushOut;
        resolved = true;
    }

    if (!resolved) {
        return;
    }

    float vn = dot(float3(vx, vy, vz), normal);
    if (vn < 0.0f) {
        vx -= vn * normal.x;
        vy -= vn * normal.y;
        vz -= vn * normal.z;
    }
    vx *= 0.985f;
    vy *= 0.985f;
    vz *= 0.985f;
}

inline void resolveSpeaker(constant GpuSimParams3D &params,
    thread float &x, thread float &y, thread float &z,
    thread float &vx, thread float &vy, thread float &vz)
{
    if (!acousticsActive(params)) {
        return;
    }

    float surfaceOffset = params.particleRadius * 0.70f;
    float signedDistance = speakerSignedDistance(params, x, y, z);
    if (signedDistance >= surfaceOffset) {
        return;
    }

    float3 normal = speakerNormal(params, x, y, z);
    float3 speakerVelocity = float3(params.speakerVelocityX, params.speakerVelocityY, params.speakerVelocityZ);
    float pushOut = surfaceOffset - signedDistance;
    x += normal.x * pushOut;
    y += normal.y * pushOut;
    z += normal.z * pushOut;

    float relativeVn = dot(float3(vx, vy, vz) - speakerVelocity, normal);
    if (relativeVn < 0.0f) {
        vx -= relativeVn * normal.x;
        vy -= relativeVn * normal.y;
        vz -= relativeVn * normal.z;
    }
    vx = 0.992f * vx + 0.008f * speakerVelocity.x;
    vy = 0.992f * vy + 0.008f * speakerVelocity.y;
    vz = 0.992f * vz + 0.008f * speakerVelocity.z;
}

inline void resolveBounds(constant GpuSimParams3D &params,
    const device float *importedSdf,
    uint particleIndex,
    thread float &x, thread float &y, thread float &z,
    thread float &vx, thread float &vy, thread float &vz,
    thread float &temperature)
{
    float radius = params.particleRadius;
    float left = params.boundsMinX + radius;
    float right = params.boundsMinX + params.boundsSizeX - radius;
    float bottom = params.boundsMinY + radius;
    float top = params.boundsMinY + params.boundsSizeY - radius;
    float front = params.boundsMinZ + radius;
    float back = params.boundsMinZ + params.boundsSizeZ - radius;

    if (sceneIsWindTunnel(params)) {
        if (x < left || x > right) {
            respawnWindTunnelParticle(params, particleIndex, x, y, z, vx, vy, vz, temperature);
            return;
        }
    } else if (x < left) {
        x = left;
        vx = fabs(vx) * params.wallBounce;
        vy *= params.wallFriction;
        vz *= params.wallFriction;
    } else if (x > right) {
        x = right;
        vx = -fabs(vx) * params.wallBounce;
        vy *= params.wallFriction;
        vz *= params.wallFriction;
    }

    if (y < bottom) {
        y = bottom;
        vy = fabs(vy) * params.wallBounce;
        vx *= params.wallFriction;
        vz *= params.wallFriction;
    } else if (y > top) {
        y = top;
        vy = -fabs(vy) * params.wallBounce;
        vx *= params.wallFriction;
        vz *= params.wallFriction;
    }

    if (z < front) {
        z = front;
        vz = fabs(vz) * params.wallBounce;
        vx *= params.wallFriction;
        vy *= params.wallFriction;
    } else if (z > back) {
        z = back;
        vz = -fabs(vz) * params.wallBounce;
        vx *= params.wallFriction;
        vy *= params.wallFriction;
    }

    resolveObstacle(params, importedSdf, x, y, z, vx, vy, vz);
}

kernel void clearCounts(device atomic_uint *cellCounts [[buffer(0)]],
    constant GpuSimParams3D &params [[buffer(1)]],
    uint id [[thread_position_in_grid]])
{
    if (id >= params.cellCount) {
        return;
    }
    atomic_store_explicit(&cellCounts[id], 0u, memory_order_relaxed);
}

kernel void computeParticleCells(const device float *x [[buffer(0)]],
    const device float *y [[buffer(1)]],
    const device float *z [[buffer(2)]],
    device int *particleCells [[buffer(3)]],
    device atomic_uint *cellCounts [[buffer(4)]],
    constant GpuSimParams3D &params [[buffer(5)]],
    uint id [[thread_position_in_grid]])
{
    if (id >= params.particleCount) {
        return;
    }

    float invCellSize = 1.0f / params.supportRadius;
    int cellX = clampCellCoord((int)((x[id] - params.boundsMinX) * invCellSize), params.gridWidth - 1u);
    int cellY = clampCellCoord((int)((y[id] - params.boundsMinY) * invCellSize), params.gridHeight - 1u);
    int cellZ = clampCellCoord((int)((z[id] - params.boundsMinZ) * invCellSize), params.gridDepth - 1u);
    int cellIndex = (cellZ * (int)params.gridHeight + cellY) * (int)params.gridWidth + cellX;
    particleCells[id] = cellIndex;
    atomic_fetch_add_explicit(&cellCounts[cellIndex], 1u, memory_order_relaxed);
}

kernel void scatterSortedIndices(const device int *particleCells [[buffer(0)]],
    device atomic_uint *cellOffsets [[buffer(1)]],
    device int *sortedIndices [[buffer(2)]],
    constant GpuSimParams3D &params [[buffer(3)]],
    uint id [[thread_position_in_grid]])
{
    if (id >= params.particleCount) {
        return;
    }
    uint slot = atomic_fetch_add_explicit(&cellOffsets[particleCells[id]], 1u, memory_order_relaxed);
    sortedIndices[slot] = (int)id;
}

kernel void gatherSortedData(const device int *sortedIndices [[buffer(0)]],
    const device int *particleCells [[buffer(1)]],
    const device float *x [[buffer(2)]],
    const device float *y [[buffer(3)]],
    const device float *z [[buffer(4)]],
    const device float *vx [[buffer(5)]],
    const device float *vy [[buffer(6)]],
    const device float *vz [[buffer(7)]],
    const device float *temperature [[buffer(8)]],
    device float *sortedX [[buffer(9)]],
    device float *sortedY [[buffer(10)]],
    device float *sortedZ [[buffer(11)]],
    device float *sortedVX [[buffer(12)]],
    device float *sortedVY [[buffer(13)]],
    device float *sortedVZ [[buffer(14)]],
    device float *sortedTemperature [[buffer(15)]],
    device int *sortedCellIndices [[buffer(16)]],
    constant GpuSimParams3D &params [[buffer(17)]],
    uint slot [[thread_position_in_grid]])
{
    if (slot >= params.particleCount) {
        return;
    }

    int particleIndex = sortedIndices[slot];
    sortedX[slot] = x[particleIndex];
    sortedY[slot] = y[particleIndex];
    sortedZ[slot] = z[particleIndex];
    sortedVX[slot] = vx[particleIndex];
    sortedVY[slot] = vy[particleIndex];
    sortedVZ[slot] = vz[particleIndex];
    sortedTemperature[slot] = temperature[particleIndex];
    sortedCellIndices[slot] = particleCells[particleIndex];
}

kernel void computeDensityPressure(const device float *sortedX [[buffer(0)]],
    const device float *sortedY [[buffer(1)]],
    const device float *sortedZ [[buffer(2)]],
    const device float *sortedTemperature [[buffer(3)]],
    const device int *sortedCellIndices [[buffer(4)]],
    const device int *cellStarts [[buffer(5)]],
    const device int *cellNeighborCounts [[buffer(6)]],
    const device int *cellNeighbors [[buffer(7)]],
    const device int *sortedIndices [[buffer(8)]],
    device float *density [[buffer(9)]],
    device float *pressure [[buffer(10)]],
    device float *sortedDensity [[buffer(11)]],
    device float *sortedPressure [[buffer(12)]],
    constant GpuSimParams3D &params [[buffer(13)]],
    uint slot [[thread_position_in_grid]])
{
    if (slot >= params.particleCount) {
        return;
    }

    float xi = sortedX[slot];
    float yi = sortedY[slot];
    float zi = sortedZ[slot];
    int baseCell = sortedCellIndices[slot];
    int neighborBase = baseCell * 27;
    int neighborCount = cellNeighborCounts[baseCell];
    float rho = 0.0f;

    for (int neighbor = 0; neighbor < neighborCount; ++neighbor) {
        int cellIndex = cellNeighbors[neighborBase + neighbor];
        int start = cellStarts[cellIndex];
        int end = cellStarts[cellIndex + 1];

        for (int cursor = start; cursor < end; ++cursor) {
            float dx = xi - sortedX[cursor];
            float dy = yi - sortedY[cursor];
            float dz = zi - sortedZ[cursor];
            float r2 = dx * dx + dy * dy + dz * dz;
            if (r2 < params.supportRadiusSquared) {
                float diff = params.supportRadiusSquared - r2;
                rho += params.mass * params.densityKernel * diff * diff * diff;
            }
        }
    }

    addBoundaryDensityContribution(params, xi, yi, zi, rho);

    float clampedDensity = max(densityFloor(params), rho);
    int particleIndex = sortedIndices[slot];
    float particlePressure = pressureFromState(params, clampedDensity, sortedTemperature[slot]);
    density[particleIndex] = clampedDensity;
    pressure[particleIndex] = particlePressure;
    sortedDensity[slot] = clampedDensity;
    sortedPressure[slot] = particlePressure;
}

kernel void computeForces(const device int *sortedIndices [[buffer(0)]],
    const device float *sortedX [[buffer(1)]],
    const device float *sortedY [[buffer(2)]],
    const device float *sortedZ [[buffer(3)]],
    const device float *sortedVX [[buffer(4)]],
    const device float *sortedVY [[buffer(5)]],
    const device float *sortedVZ [[buffer(6)]],
    const device float *sortedTemperature [[buffer(7)]],
    const device float *sortedDensity [[buffer(8)]],
    const device float *sortedPressure [[buffer(9)]],
    const device int *sortedCellIndices [[buffer(10)]],
    const device int *cellStarts [[buffer(11)]],
    const device int *cellNeighborCounts [[buffer(12)]],
    const device int *cellNeighbors [[buffer(13)]],
    device float *axBuffer [[buffer(14)]],
    device float *ayBuffer [[buffer(15)]],
    device float *azBuffer [[buffer(16)]],
    device float *temperatureRateBuffer [[buffer(17)]],
    device float *xsphVXBuffer [[buffer(18)]],
    device float *xsphVYBuffer [[buffer(19)]],
    device float *xsphVZBuffer [[buffer(20)]],
    const device float *importedSdf [[buffer(21)]],
    constant GpuSimParams3D &params [[buffer(22)]],
    uint slot [[thread_position_in_grid]])
{
    if (slot >= params.particleCount) {
        return;
    }

    float h = params.supportRadius;
    float h2 = params.supportRadiusSquared;
    float xi = sortedX[slot];
    float yi = sortedY[slot];
    float zi = sortedZ[slot];
    float vxi = sortedVX[slot];
    float vyi = sortedVY[slot];
    float vzi = sortedVZ[slot];
    float rhoi = sortedDensity[slot];
    float pressureI = sortedPressure[slot];
    float temperatureI = sortedTemperature[slot];
    int baseCell = sortedCellIndices[slot];
    int neighborBase = baseCell * 27;
    int neighborCount = cellNeighborCounts[baseCell];

    float ax = -params.globalDrag * vxi;
    float ay = -params.gravity - params.globalDrag * vyi + params.buoyancy * (temperatureI - params.ambientTemperature);
    float az = -params.globalDrag * vzi;
    float temperatureRate = 0.0f;
    float xsphVX = 0.0f;
    float xsphVY = 0.0f;
    float xsphVZ = 0.0f;

    for (int neighbor = 0; neighbor < neighborCount; ++neighbor) {
        int cellIndex = cellNeighbors[neighborBase + neighbor];
        int start = cellStarts[cellIndex];
        int end = cellStarts[cellIndex + 1];

        for (int cursor = start; cursor < end; ++cursor) {
            if ((int)slot == cursor) {
                continue;
            }

            float dx = xi - sortedX[cursor];
            float dy = yi - sortedY[cursor];
            float dz = zi - sortedZ[cursor];
            float r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > 1e-8f && r2 < h2) {
                float r = sqrt(r2);
                float influence = h - r;
                float invR = 1.0f / r;
                float neighborDensity = sortedDensity[cursor];
                float pressureTerm = -params.mass *
                    ((pressureI / (rhoi * rhoi)) +
                     (sortedPressure[cursor] / (neighborDensity * neighborDensity))) *
                    params.pressureKernelGrad * influence * influence;
                float viscosityTerm = params.kinematicViscosity * params.mass *
                    params.viscosityKernelLap * influence / neighborDensity;
                float densityKernelValue = params.densityKernel * (h2 - r2) * (h2 - r2) * (h2 - r2);

                ax += pressureTerm * dx * invR;
                ay += pressureTerm * dy * invR;
                az += pressureTerm * dz * invR;
                ax += viscosityTerm * (sortedVX[cursor] - vxi);
                ay += viscosityTerm * (sortedVY[cursor] - vyi);
                az += viscosityTerm * (sortedVZ[cursor] - vzi);

                temperatureRate += params.temperatureDiffusion * params.mass *
                    (sortedTemperature[cursor] - temperatureI) *
                    params.viscosityKernelLap * influence / (rhoi * neighborDensity);

                xsphVX += params.mass * (sortedVX[cursor] - vxi) * densityKernelValue / neighborDensity;
                xsphVY += params.mass * (sortedVY[cursor] - vyi) * densityKernelValue / neighborDensity;
                xsphVZ += params.mass * (sortedVZ[cursor] - vzi) * densityKernelValue / neighborDensity;
            }
        }
    }

    addBoundaryForceContribution(params, xi, yi, zi, vxi, vyi, vzi, rhoi, pressureI, temperatureI, ax, ay, az);
    addSceneForceContribution(params, importedSdf, xi, yi, zi, vxi, vyi, vzi, ax, ay, az);
    addSpeakerForceContribution(params, xi, yi, zi, vxi, vyi, vzi, ax, ay, az);

    if (params.interactionActive != 0u) {
        float dx = xi - params.interactionX;
        float dy = yi - params.interactionY;
        float dz = zi - params.interactionZ;
        float r2 = dx * dx + dy * dy + dz * dz;
        if (r2 > 1e-8f && r2 < params.interactionRadiusSquared) {
            float r = sqrt(r2);
            float falloff = 1.0f - (r / params.interactionRadius);
            float impulse = params.interactionStrength * falloff * falloff;
            float invR = 1.0f / r;
            ax += dx * invR * impulse;
            ay += dy * invR * impulse;
            az += dz * invR * impulse;
        }
    }

    int particleIndex = sortedIndices[slot];
    axBuffer[particleIndex] = ax;
    ayBuffer[particleIndex] = ay;
    azBuffer[particleIndex] = az;
    temperatureRateBuffer[particleIndex] = temperatureRate;
    xsphVXBuffer[particleIndex] = xsphVX;
    xsphVYBuffer[particleIndex] = xsphVY;
    xsphVZBuffer[particleIndex] = xsphVZ;
}

kernel void integrateParticles(device float *x [[buffer(0)]],
    device float *y [[buffer(1)]],
    device float *z [[buffer(2)]],
    device float *vx [[buffer(3)]],
    device float *vy [[buffer(4)]],
    device float *vz [[buffer(5)]],
    const device float *ax [[buffer(6)]],
    const device float *ay [[buffer(7)]],
    const device float *az [[buffer(8)]],
    device float *temperature [[buffer(9)]],
    const device float *temperatureRate [[buffer(10)]],
    const device float *xsphVX [[buffer(11)]],
    const device float *xsphVY [[buffer(12)]],
    const device float *xsphVZ [[buffer(13)]],
    const device float *importedSdf [[buffer(14)]],
    constant GpuSimParams3D &params [[buffer(15)]],
    uint id [[thread_position_in_grid]])
{
    if (id >= params.particleCount) {
        return;
    }

    float vxi = vx[id] + ax[id] * params.substepDt;
    float vyi = vy[id] + ay[id] * params.substepDt;
    float vzi = vz[id] + az[id] * params.substepDt;

    float maxSpeed = (params.preset == MATERIAL_GAS)
        ? params.soundSpeed * 5.0f
        : params.soundSpeed * 2.5f;
    float speed2 = vxi * vxi + vyi * vyi + vzi * vzi;
    if (speed2 > maxSpeed * maxSpeed) {
        float scale = maxSpeed / sqrt(speed2);
        vxi *= scale;
        vyi *= scale;
        vzi *= scale;
    }

    float advVx = vxi + xsphBlend(params) * xsphVX[id];
    float advVy = vyi + xsphBlend(params) * xsphVY[id];
    float advVz = vzi + xsphBlend(params) * xsphVZ[id];
    float px = x[id] + advVx * params.substepDt;
    float py = y[id] + advVy * params.substepDt;
    float pz = z[id] + advVz * params.substepDt;
    float nextTemperature = clampf(temperature[id] + temperatureRate[id] * params.substepDt, TEMP_MIN, TEMP_MAX);

    if (params.preset != MATERIAL_GAS) {
        vxi *= 0.9992f;
        vyi *= 0.9992f;
        vzi *= 0.9992f;
    }

    resolveBounds(params, importedSdf, id, px, py, pz, vxi, vyi, vzi, nextTemperature);
    resolveSpeaker(params, px, py, pz, vxi, vyi, vzi);

    x[id] = px;
    y[id] = py;
    z[id] = pz;
    vx[id] = vxi;
    vy[id] = vyi;
    vz[id] = vzi;
    temperature[id] = nextTemperature;
}
