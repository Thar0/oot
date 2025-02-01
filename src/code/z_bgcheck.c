#include "libu64/debug.h"
#include "array_count.h"
#include "attributes.h"
#include "line_numbers.h"
#include "printf.h"
#include "regs.h"
#include "segmented_address.h"
#include "sys_math3d.h"
#include "terminal.h"
#include "translation.h"
#include "z_lib.h"
#include "bgcheck.h"
#include "play_state.h"
#include "player.h"
#include "skin_matrix.h"
#include "zelda_arena.h"

#pragma increment_block_number "ntsc-1.0:136 ntsc-1.1:136 ntsc-1.2:136"

#define USE_POLY_CHK_TBL 0
#define DO_INSTRUMENTATION 1

#if DO_INSTRUMENTATION
#define INSTRUMENTATION(expr) expr
#else
#define INSTRUMENTATION(expr)
#endif

BgCheckStats gBgCheckStats;

#define ALWAYS_INLINE __attribute__((always_inline)) inline

#define complex _Complex
#define cf32 complex float
#define COS(cf) (__real__(cf))
#define SIN(cf) (__imag__(cf))

/* Construct floating-point constants inline */
static ALWAYS_INLINE f32 CF(const f32 f) {
    register fu r;
    register fu in;

    in.f = f;

    if (!__builtin_constant_p(in.i)) {
        return f;
    }

    u32 upper = (in.i >> 16);
    u32 lower = (in.i >> 0) & 0xFFFF;

    if (upper != 0) {
        __asm__("lui     %0, %1" : "=r"(r.i) : "K"(upper));
    }
    if (lower != 0) {
        __asm__("ori     %0, %1, %2" : "+r"(r.i) : "r"((upper != 0) ? r.i : 0), "K"(lower));
    }
    // mtc1
    return r.f;
}

#if 0
/**
 * Issues create_dirty_exclusive from dst to dst+size.
 *
 * This is useful for depositing large amounts of write-only data without fetching any
 * irrelevant old data.
 *
 * dst must be dcache-aligned and size must be a multiple of the dcache line size (0x10)
 */
static ALWAYS_INLINE void CACHE_CDX(void* dst, size_t size) {
    // Check constraints
    // assert((uintptr_t)dst % DCACHE_LINESIZE == 0);
    // assert(size % DCACHE_LINESIZE == 0);

    // Perform operation
    if (__builtin_constant_p(size) && size <= 4 * DCACHE_LINESIZE) {
        // For sufficiently small constant sizes this loop can unroll and
        // use the const immediate field of the cache instruction
        for (u32 i = 0; i < size; i += DCACHE_LINESIZE) {
            asm("cache %0, %1(%2)" :: "i"(CACH_PD | C_CDX), "i"(i), "r"(dst));
        }
    } else {
        // For larger sizes where the loop does not unroll or when the size
        // is not a compile-time constant, use a loop instead
        for (u32 i = 0; i < size; i += DCACHE_LINESIZE) {
            asm("cache %0, (%1)" :: "i"(CACH_PD | C_CDX), "r"(dst + i));
        }
    }
}
#endif

// Bitwise int to float
static ALWAYS_INLINE f32 i2f(u32 i) {
    fu tmp = { .i = i };
    return tmp.f;
}

// Bitwise float to int
static ALWAYS_INLINE u32 f2i(f32 f) {
    fu tmp = { .f = f };
    return tmp.i;
}

/**
 *  Get the sign of a 32-bit big-endian IEEE floating-point number.
 *  Returns 1.0f if positive, -1.0f if negative.
 */
static ALWAYS_INLINE f32 sgn(f32 f) {
    u32 sgn_bit = f2i(-0.0f);
    u32 r1 = f2i(1.0f);
    u32 rf = f2i(f);
    return i2f(r1 | (rf & sgn_bit));
}

// vec3 cast s16 -> f32
// (Vec3f)v
#define VCVT(v)      \
    ((Vec3f){        \
        .x = (v)->x, \
        .y = (v)->y, \
        .z = (v)->z, \
    })

// vec3 subtraction
// v1 - v2
#define VSUB(v1, v2)            \
    ((Vec3f){                   \
        .x = (v1)->x - (v2)->x, \
        .y = (v1)->y - (v2)->y, \
        .z = (v1)->z - (v2)->z, \
    })

// vec3 scalar-multiply-add
// s * v1 + v2
#define VSMADD(s, v1, v2)             \
    ((Vec3f){                         \
        .x = (s) * (v1)->x + (v2)->x, \
        .y = (s) * (v1)->y + (v2)->y, \
        .z = (s) * (v1)->z + (v2)->z, \
    })

// vec3 squared magnitude
#define VMAGSQ(v) (SQ((v)->x) + SQ((v)->y) + SQ((v)->z))


#define BITSET_SIZE_BYTES(n, t) (((n) + 8 * sizeof(t) - 1) / 8)

// Walk a poly linked list without copy-pasting all the boilerplate
#define POLYLIST_FOREACH(nodes, node, list)                                                          \
    for (SSNode* restrict node, *_t_ = (void*)1; _t_; _t_ = NULL)                                    \
        for (u16 _index_ = (list)->head; (_index_ != SS_NULL && ((node) = &(nodes)[_index_], true)); \
             _index_ = (node)->next)

#define NORMAL_IS_FLOOR(ny) ((ny) > 0.5f)
#define NORMAL_IS_CEILING(ny) ((ny) < -0.8f)

#define SNORMAL_IS_FLOOR(ny) ((ny) > COLPOLY_SNORMAL(0.5f))
#define SNORMAL_IS_CEILING(ny) ((ny) < COLPOLY_SNORMAL(-0.8f))

#if 0
/* The Kaze et al sincos approximation, returns cos(angle) + isin(angle) */
cf32 Math_SinCosS(s16 angle) {
#define SECOND_ORDER_COEFFICIENT 0.0000000010911122665310369f
    const f32 one = CF(1.0f);

    s32 shifter = (angle ^ (angle << 1)) & 0xC000;
    s32 x = ((angle + shifter) << 17) >> 16;
    f32 cosx = one - CF(SECOND_ORDER_COEFFICIENT) * (x * x);
    f32 sinx = sqrtf(one - cosx * cosx);

    if (shifter & 0x4000) {
        float temp = cosx;
        cosx = sinx;
        sinx = temp;
    }
    if (angle < 0) {
        sinx = -sinx;
    }
    if (shifter & 0x8000) {
        cosx = -cosx;
    }
    return __builtin_complex(cosx, sinx);
}
#endif

void BgCheck_GetStaticLookupIndicesFromPos(CollisionContext* colCtx, Vec3f* pos, Vec3i* sector);
s32 BgCheck_PosInStaticBoundingBox(CollisionContext* colCtx, Vec3f* pos);
s32 BgCheck_CheckLineImpl(CollisionContext* colCtx, u16 xpFlags1, u16 xpFlags2, Vec3f* posA, Vec3f* posB,
                          Vec3f* posResult, CollisionPoly** outPoly, s32* outBgId, Actor* actor, u32 bccFlags);
f32 BgCheck_RaycastDownDyna(DynaRaycastDown* dynaRaycastDown);
s32 BgCheck_SphVsDynaWall(CollisionContext* colCtx, u16 xpFlags, f32* outX, f32* outZ, Vec3f* pos, f32 radius,
                          CollisionPoly** outPoly, s32* outBgId, Actor* actor);
s32 BgCheck_CheckDynaCeiling(CollisionContext* colCtx, u16 xpFlags, f32* outY, Vec3f* pos, f32 chkDist,
                             CollisionPoly** outPoly, s32* outBgId, Actor* actor);
s32 BgCheck_CheckLineAgainstDyna(CollisionContext* colCtx, u16 xpFlags, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                                 CollisionPoly** outPoly, f32* distSq, s32* outBgId, Actor* actor, s32 bccFlags);
s32 BgCheck_SphVsFirstDynaPoly(CollisionContext* colCtx, u16 xpFlags, CollisionPoly** outPoly, s32* outBgId,
                               Vec3f* center, f32 radius, Actor* actor, u16 bciFlags);
void BgActor_Initialize(PlayState* play, BgActor* bgActor);

#define SS_NULL 0xFFFF

// bccFlags
#define BGCHECK_CHECK_WALL (1 << 0)
#define BGCHECK_CHECK_FLOOR (1 << 1)
#define BGCHECK_CHECK_CEILING (1 << 2)
#define BGCHECK_CHECK_ONE_FACE (1 << 3)
#define BGCHECK_CHECK_DYNA (1 << 4)
#define BGCHECK_CHECK_ALL \
    (BGCHECK_CHECK_WALL | BGCHECK_CHECK_FLOOR | BGCHECK_CHECK_CEILING | BGCHECK_CHECK_ONE_FACE | BGCHECK_CHECK_DYNA)

// bciFlags
#define BGCHECK_IGNORE_NONE 0
#define BGCHECK_IGNORE_CEILING (1 << 0)
#define BGCHECK_IGNORE_WALL (1 << 1)
#define BGCHECK_IGNORE_FLOOR (1 << 2)

// raycast down flags (downChkFlags)
#define BGCHECK_RAYCAST_DOWN_CHECK_CEILINGS (1 << 0)
#define BGCHECK_RAYCAST_DOWN_CHECK_WALLS (1 << 1)
#define BGCHECK_RAYCAST_DOWN_CHECK_FLOORS (1 << 2)
#define BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE (1 << 3) // stops checking dyna walls on finding first candidate result
#define BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY (1 << 4)  // skips walls and ceilings with normal.y < 0

// raycast down groundChk flag. When enabled, search range is limited to floors and walls with a normal.y >= 0
#define BGCHECK_GROUND_CHECK_ON (1 << 0)

s32 D_80119D90[WALL_TYPE_MAX] = {
    0,                         // WALL_TYPE_0
    WALL_FLAG_0,               // WALL_TYPE_1
    WALL_FLAG_0 | WALL_FLAG_1, // WALL_TYPE_2
    WALL_FLAG_0 | WALL_FLAG_2, // WALL_TYPE_3
    WALL_FLAG_3,               // WALL_TYPE_4
    WALL_FLAG_CRAWLSPACE_1,    // WALL_TYPE_5
    WALL_FLAG_CRAWLSPACE_2,    // WALL_TYPE_6
    WALL_FLAG_6,               // WALL_TYPE_7
};

u16 sSurfaceMaterialToSfxOffset[SURFACE_MATERIAL_MAX] = {
    SURFACE_SFX_OFFSET_DIRT,          // SURFACE_MATERIAL_DIRT
    SURFACE_SFX_OFFSET_SAND,          // SURFACE_MATERIAL_SAND
    SURFACE_SFX_OFFSET_STONE,         // SURFACE_MATERIAL_STONE
    SURFACE_SFX_OFFSET_JABU,          // SURFACE_MATERIAL_JABU
    SURFACE_SFX_OFFSET_WATER_SHALLOW, // SURFACE_MATERIAL_WATER_SHALLOW
    SURFACE_SFX_OFFSET_WATER_DEEP,    // SURFACE_MATERIAL_WATER_DEEP
    SURFACE_SFX_OFFSET_TALL_GRASS,    // SURFACE_MATERIAL_TALL_GRASS
    SURFACE_SFX_OFFSET_LAVA,          // SURFACE_MATERIAL_LAVA
    SURFACE_SFX_OFFSET_GRASS,         // SURFACE_MATERIAL_GRASS
    SURFACE_SFX_OFFSET_BRIDGE,        // SURFACE_MATERIAL_BRIDGE
    SURFACE_SFX_OFFSET_WOOD,          // SURFACE_MATERIAL_WOOD
    SURFACE_SFX_OFFSET_DIRT,          // SURFACE_MATERIAL_DIRT_SOFT
    SURFACE_SFX_OFFSET_ICE,           // SURFACE_MATERIAL_ICE
    SURFACE_SFX_OFFSET_CARPET,        // SURFACE_MATERIAL_CARPET
};

#if DEBUG_FEATURES
/**
 * original name: T_BGCheck_PosErrorCheck
 */
s32 BgCheck_PosErrorCheck(Vec3f* pos, const char* file, int line) {
    if (pos->x >= BGCHECK_XYZ_ABSMAX || pos->x <= -BGCHECK_XYZ_ABSMAX || pos->y >= BGCHECK_XYZ_ABSMAX ||
        pos->y <= -BGCHECK_XYZ_ABSMAX || pos->z >= BGCHECK_XYZ_ABSMAX || pos->z <= -BGCHECK_XYZ_ABSMAX) {
        PRINTF_COLOR_RED();
        PRINTF(T("T_BGCheck_PosErrorCheck():位置が妥当ではありません。pos (%f,%f,%f) file:%s line:%d\n",
                 "T_BGCheck_PosErrorCheck(): Position is invalid. pos (%f,%f,%f) file:%s line:%d\n"),
               pos->x, pos->y, pos->z, file, line);
        PRINTF_RST();
        return true;
    }
    return false;
}
#endif

#if 0
void CollisionPoly_TriToAABB(Vec3s* restrict vtxList, CollisionPoly* restrict poly, Vec3s* restrict min, Vec3s* restrict max) {
    u8 bbIndices = poly->bbIndices; // upper 8 bits of poly->type

    u32 minx = (bbIndices >> 6) & 0b11;
    u32 miny = 0;
    u32 minz = (bbIndices >> 4) & 0b11;
    u32 maxx = (bbIndices >> 2) & 0b11;
    u32 maxy = 2;
    u32 maxz = (bbIndices >> 0) & 0b11;

    min->x = vtxList[COLPOLY_VTX_INDEX(poly->vtxData[minx])].x;
    min->y = vtxList[COLPOLY_VTX_INDEX(poly->vtxData[miny])].y; // v1 is always min y
    min->z = vtxList[COLPOLY_VTX_INDEX(poly->vtxData[minz])].z;
    max->x = vtxList[COLPOLY_VTX_INDEX(poly->vtxData[maxx])].x;
    max->y = vtxList[COLPOLY_VTX_INDEX(poly->vtxData[maxy])].y; // v3 is always max y
    max->z = vtxList[COLPOLY_VTX_INDEX(poly->vtxData[maxz])].z;
}
#endif

static s32 SphereVsSphere16(Vec3f* restrict sphCenter, f32 sphRadius, Sphere16* restrict b) {
    Vec3f diff = VSUB(sphCenter, &b->center);
    f32 rsum = sphRadius + b->radius;
    return DOTXYZ(diff, diff) <= SQ(rsum);
}

static s32 SphereVsPoint(Vec3f* restrict sphCenter, f32 sphRadius, Vec3f* restrict p, Vec3f* restrict intersect) {
    *intersect = *p;
    // An intersection occurs if the distance from the sphere center to the point is less than or equal to the radius
    Vec3f diff = VSUB(p, sphCenter);
    return DOTXYZ(diff, diff) <= SQ(sphRadius);
}

s32 SphereVsInfiniteLine(Vec3f* restrict sphCenter, f32 sphRadius, Vec3f* restrict a, Vec3f* restrict b,
                         Vec3f* restrict intersect) {
    // d is the point on the line AB closest to c:
    // d = a + l * (b - a)
    // which occurs when CD and AB are orthogonal:
    // (d - c) . (b - a) = 0
    // Substitute d and solve for l:
    // (a - c + l * (b - a)) . (b - a) = 0
    // (a - c) . (b - a) + l * (b - a) . (b - a) = 0
    // l = ((a - c) . (b - a)) / ((b - a) . (b - a))
    // From l we compute d, then do the sphere-point intersection test on d.
    Vec3f ab = VSUB(b, a);
    Vec3f ca = VSUB(a, sphCenter);
    f32 lambda = DOTXYZ(ab, ca) / DOTXYZ(ab, ab);
    Vec3f d = VSMADD(lambda, &ab, a);
    return SphereVsPoint(sphCenter, sphRadius, &d, intersect);
}

s32 SphereVsLineSeg(Vec3f* restrict sphCenter, f32 sphRadius, Vec3f* restrict a, Vec3f* restrict b,
                    Vec3f* restrict intersect) {
    // This is like the infinite line except we force lambda into [0, 1] to not exit the line segment
    const f32 one = CF(1.0f);
    Vec3f ab = VSUB(b, a);
    Vec3f ca = VSUB(a, sphCenter);
    f32 lambda = DOTXYZ(ab, ca) / DOTXYZ(ab, ab);
    lambda = CLAMP(lambda, 0.0f, one); // Make sure we don't go oob of the line segment.
    Vec3f d = VSMADD(lambda, &ab, a);
    return SphereVsPoint(sphCenter, sphRadius, &d, intersect);
}

s32 SphereVsPlane(Vec3f* restrict sphCenter, f32 sphRadius, Vec3f* restrict triNorm, f32 triDist,
                  Vec3f* restrict intersect) {
    // The line along the plane normal intersecting the sphere center is:
    // (1) r = c + l * n
    // We want the point on this line that is also in the plane:
    // (2) r . n + d = 0
    // Substitute r from (1) into (2) and solve for l:
    // (c + l * n) . n + d = c . n + l + d = 0
    // l = - d - c . n
    f32 lambda = -triDist - DOTXYZ(*sphCenter, *triNorm);
    Vec3f closest = VSMADD(lambda, triNorm, sphCenter);
    return SphereVsPoint(sphCenter, sphRadius, &closest, intersect);
}

typedef struct {
    f32 u;
    f32 v;
    f32 w;
} BarycentricCoords;

static void Barycentric(Vec3f* restrict p, Vec3f triangle[restrict 3], Vec3f* restrict triNorm,
                        BarycentricCoords* restrict out) {
    // Compute barycentric coordinates for p with respect to triangle. Barycentric coordinates are invariant under
    // projections so we can choose to compute them in whichever projection of the triangle has the largest area.
    // The plane with the largest area is determined from the abs max component of the plane normal vector.
    // (cf. Real Time Collision Detection Section 3.4)
    f32 nnx = fabsf(triNorm->x);
    f32 nny = fabsf(triNorm->y);
    f32 nnz = fabsf(triNorm->z);

    Vec3f* restrict a = &triangle[0];
    Vec3f* restrict b = &triangle[1];
    Vec3f* restrict c = &triangle[2];

    s32 axis1, axis2;
    if (nnx >= nny && nnx >= nnz) {
        // project to yz
        axis1 = 1;
        axis2 = 2;
    } else if (nny >= nnz) {
        // project to xz
        axis1 = 2;
        axis2 = 0;
    } else {
        // project to xy
        axis1 = 0;
        axis2 = 1;
    }
    f32 u = (p->a[axis1] - b->a[axis1]) * (b->a[axis2] - c->a[axis2]) -
            (b->a[axis1] - c->a[axis1]) * (p->a[axis2] - b->a[axis2]);
    f32 v = (p->a[axis1] - c->a[axis1]) * (c->a[axis2] - a->a[axis2]) -
            (c->a[axis1] - a->a[axis1]) * (p->a[axis2] - c->a[axis2]);
    f32 nrml = (b->a[axis1] - a->a[axis1]) * (c->a[axis2] - a->a[axis2]) -
               (b->a[axis2] - a->a[axis2]) * (c->a[axis1] - a->a[axis1]);
    if (nrml < 0) {
        u = -u;
        v = -v;
    }
    out->u = u;
    out->v = v;
    out->w = fabsf(nrml) - u - v;
}

static s32 PointVsTriangle(Vec3f* restrict p, Vec3f triangle[restrict 3], Vec3f* restrict triNorm) {
    BarycentricCoords bc;
    Barycentric(p, triangle, triNorm, &bc);
    return bc.u >= 0.0f && bc.v >= 0.0f && bc.w >= 0.0f;
}

static s32 SphereVsTriangle(Vec3f* restrict sphCenter, f32 sphRadius, Vec3f triangle[restrict 3],
                            Vec3f* restrict triNorm, f32 triDist, Vec3f* restrict intersect) {
    // First check sphere vs triangle plane, if there's no overlap in the
    // plane they cannot possibly overlap at all.

    if (!SphereVsPlane(sphCenter, sphRadius, triNorm, triDist, intersect)) {
        return false;
    }

    BarycentricCoords bc;
    Barycentric(intersect, triangle, triNorm, &bc);
    Vec3f* restrict a = &triangle[0];
    Vec3f* restrict b = &triangle[1];
    Vec3f* restrict c = &triangle[2];

    // Based on the signs of the coordinates we can determine what the
    // closest feature of the triangle is to the sphere, which is either
    // - the triangle face, in which case a collision is guaranteed
    // - a line segment
    // - a vertex
    // TODO evil optimization opportunity that might not work on all platforms:
    // Currently these comparisons compile to a branch, but we can get the
    // result of an fp comparison by reading FPCSR instead, then we can &
    // those together and finally & with FPCSR_C to get the same result with
    // zero branches.
    u32 su = bc.u >= 0.0f;
    u32 sv = bc.v >= 0.0f;
    u32 sw = bc.w >= 0.0f;

    // Check codimension 0 (face)
    // The intersection here will be the plane intersection itself, since it lies inside the triangle face
    if (su & sv & sw) {
        return true;
    }

    // Check codimension 1 (edge)
    // The intersection here will be the closest point on the line segment to the sphere, which may be a vertex
    if (su & sv) {
        // ++-
        // closest is tri[0] - tri[1] line
        return SphereVsLineSeg(sphCenter, sphRadius, a, b, intersect);
    } else if (sv & sw) {
        // -++
        // closest is tri[1] - tri[2] line
        return SphereVsLineSeg(sphCenter, sphRadius, b, c, intersect);
    } else if (sw & su) {
        // +-+
        // closest is tri[2] - tri[0] line
        return SphereVsLineSeg(sphCenter, sphRadius, c, a, intersect);
    }

    // Check codimension 2 (vertex)
    if (su) {
        // +--
        // closest is tri[0]
        return SphereVsPoint(sphCenter, sphRadius, a, intersect);
    } else if (sv) {
        // -+-
        // closest is tri[1]
        return SphereVsPoint(sphCenter, sphRadius, b, intersect);
    } else if (sw) {
        // --+
        // closest is tri[2]
        return SphereVsPoint(sphCenter, sphRadius, c, intersect);
    }

    // There is no codimension 3, should never happen
    assert(false);
    return false;
}

/**
 * Get CollisionPoly's lowest y point
 */
s16 CollisionPoly_GetMinY(CollisionPoly* restrict poly, Vec3s* restrict vtxList) {
    //! @bug Due to rounding errors, some polys with a slight slope have a y normal of 1.0f/-1.0f. As such, this
    //! optimization returns the wrong minimum y for a subset of these polys.
    if (poly->normal.y == COLPOLY_SNORMAL(1.0f) || poly->normal.y == COLPOLY_SNORMAL(-1.0f)) {
        return vtxList[COLPOLY_VTX_INDEX(poly->vtxData[0])].y;
    } else {
        // TODO accelerate with bbIndices?
        s32 a = COLPOLY_VTX_INDEX(poly->vtxData[0]);
        s32 b = COLPOLY_VTX_INDEX(poly->vtxData[1]);
        s32 c = COLPOLY_VTX_INDEX(poly->vtxData[2]);
        s16 min = vtxList[a].y;

        if (min > vtxList[b].y) {
            min = vtxList[b].y;
        }
        if (min < vtxList[c].y) {
            return min;
        }
        return vtxList[c].y;
    }
}

/**
 * CollisionPoly get unit normal
 */
void CollisionPoly_GetNormalF(CollisionPoly* restrict poly, f32* restrict nx, f32* restrict ny, f32* restrict nz) {
    *nx = COLPOLY_GET_NORMAL(poly->normal.x);
    *ny = COLPOLY_GET_NORMAL(poly->normal.y);
    *nz = COLPOLY_GET_NORMAL(poly->normal.z);
}

/**
 * Compute transform matrix mapping +y (up) to the collision poly's normal
 */
void func_80038A28(CollisionPoly* restrict poly, f32 tx, f32 ty, f32 tz, MtxF* restrict dest) {
    f32 nx;
    f32 ny;
    f32 nz;
    f32 xx;
    f32 zz;
    f32 yz;
    f32 xxInv;
    f32 zzInv;

    if (poly == NULL) {
        return;
    }
    CollisionPoly_GetNormalF(poly, &nx, &ny, &nz);

    xx = sqrtf(1.0f - SQ(nx));
    if (!IS_ZERO(xx)) {
        xxInv = 1.0f / xx;
        zz = ny * xxInv;
        yz = -(nz * xxInv);
    } else {
        zz = sqrtf(1.0f - SQ(ny));
        if (!IS_ZERO(zz)) {
            zzInv = 1.0f / zz;
            yz = nx * zzInv;
            xx = -(nz * zzInv);
        } else {
            yz = 0.0f;
            xx = 0.0f;
        }
    }
    dest->xx = xx;
    dest->yx = -nx * zz;
    dest->zx = nx * yz;
    dest->xy = nx;
    dest->yy = ny;
    dest->zy = nz;
    dest->yz = yz;
    dest->zz = zz;
    dest->wx = 0.0f;
    dest->wy = 0.0f;
    dest->xz = 0.0f;
    dest->wz = 0.0f;
    dest->xw = tx;
    dest->yw = ty;
    dest->zw = tz;
    dest->ww = 1.0f;
}

/**
 * Calculate point distance from plane along normal
 */
f32 CollisionPoly_GetPointDistanceFromPlane(CollisionPoly* restrict poly, Vec3f* restrict point) {
    return DOTXYZ(poly->normal, *point) * COLPOLY_NORMAL_FRAC + poly->dist;
}

/**
 * Get Poly Vertices
 */
static void CollisionPoly_GetVertices(CollisionPoly* restrict poly, Vec3s* restrict vtxList, Vec3f* restrict dest) {
    for (s32 i = 0; i < 3; i++) {
        Vec3s* vtx = &vtxList[COLPOLY_VTX_INDEX(poly->vtxData[i])];
        dest[i].x = vtx->x;
        dest[i].y = vtx->y;
        dest[i].z = vtx->z;
    }
}

/**
 * Get vertices by bgId
 * original name: T_Polygon_GetVertex_bg_ai
 */
void CollisionPoly_GetVerticesByBgId(CollisionPoly* restrict poly, s32 bgId, CollisionContext* restrict colCtx,
                                     Vec3f* restrict dest) {
    assert(poly != NULL && bgId <= BG_ACTOR_MAX);

    Vec3s* vtxList;
    if (bgId == BGCHECK_SCENE) {
        vtxList = colCtx->colHeader->vtxList;
    } else {
        vtxList = colCtx->dyna.bgActors[bgId].vtxList;
    }
    CollisionPoly_GetVertices(poly, vtxList, dest);
}

static s32 CircleVsPoint(f32 cx, f32 cz, f32 r, f32 x, f32 z) {
    f32 dx = x - cx;
    f32 dz = z - cz;
    return SQ(dx) + SQ(dz) <= SQ(r);
}

static s32 CircleVsLineSeg(f32 cx, f32 cz, f32 r, f32 ax, f32 az, f32 bx, f32 bz) {
    // This is like the infinite line except we force lambda into [0, 1] to not exit the line segment
    const f32 one = CF(1.0f);
    f32 abx = bx - ax;
    f32 abz = bz - az;
    f32 cax = ax - cx;
    f32 caz = az - cz;
    f32 lambda = (abx * cax + abz * caz) / (SQ(abx) + SQ(abz));
    lambda = CLAMP(lambda, 0.0f, one); // Make sure we don't go oob of the line segment.
    f32 px = ax + lambda * abx;
    f32 pz = az + lambda * abz;
    return CircleVsPoint(cx, cz, r, px, pz);
}

/* Unfortunately dyna floors need to stick out by an extra unit otherwise depending on where the actor is positioned
   in the world link may start climbing and immediately become airborne because there's no floor under him.
   Hopefully when we start testing dynapoly collision in model space it won't be a problem anymore. Until then,
   we'll do what vanilla did and enlarge the triangle by 1 extra unit. */
s32 CollisionPoly_CheckYIntersectDyna(CollisionPoly* restrict poly, Vec3s* restrict vtxList, Vec3f* restrict pos,
                                      f32* restrict yIntersect) {
    Vec3s* restrict vertices[3] = {
        &vtxList[COLPOLY_VTX_INDEX(poly->vtxData[0])],
        &vtxList[COLPOLY_VTX_INDEX(poly->vtxData[1])],
        &vtxList[COLPOLY_VTX_INDEX(poly->vtxData[2])],
    };
    Vec3s* restrict a = vertices[0];
    Vec3s* restrict b = vertices[1];
    Vec3s* restrict c = vertices[2];

    f32 x = pos->x;
    f32 z = pos->z;

    // Compute barycentric coordinates (mostly)
    f32 u = (z - b->z) * (b->x - c->x) - (b->z - c->z) * (x - b->x);
    f32 v = (z - c->z) * (c->x - a->x) - (c->z - a->z) * (x - c->x);
    s32 d = (b->z - a->z) * (c->x - a->x) - (b->x - a->x) * (c->z - a->z);
    if (d < 0) {
        u = -u;
        v = -v;
        d = -d;
    }
    // Use positivity of the coordinates to determine what feature of the triangle to check
    u32 su = u >= 0.0f;
    u32 sv = v >= 0.0f;
    u32 sw = (f32)d >= u + v;

    switch (su + sv + sw) {
        case 3:
            // Codimension 0, pos is directly on the triangle face.
            break;

        case 2:
            // Codimension 1, pos is closest to a line segment.
            // Compare pos with the closest line segment, intersection if within 1.0f.
            // Note: these nested ternaries look like they'd generate a lot of asm but it's just 4 instructions.
            Vec3s* restrict start = vertices[!sw ? 0 : !su ? 1 : 2];
            Vec3s* restrict end = vertices[!sw ? 1 : !su ? 2 : 0];
            if (!CircleVsLineSeg(x, z, CF(1.0f), start->x, start->z, end->x, end->z)) {
                return false;
            }
            break;

        case 1:
            // Codimension 2, pos is closest to a vertex.
            // Compare pos with the closest vertex, intersection if within 1.0f.
            Vec3s* restrict closest = vertices[su ? 0 : sv ? 1 : 2];
            if (!CircleVsPoint(x, z, CF(1.0f), closest->x, closest->z)) {
                return false;
            }
            break;

        default:
            // There is no codimension 3 (triangles are 2D), should never happen
            assert(false);
    }
    *yIntersect = -(SHT_MAX * poly->dist + poly->normal.x * x + poly->normal.z * z) / poly->normal.y;
    return true;
}
#if 0
s32 CollisionPoly_CheckYIntersectDyna(CollisionPoly* restrict poly, Vec3s* restrict vtxList, Vec3f* restrict pos,
                                      f32* restrict yIntersect) {
    Vec3f polyVerts[3];
    CollisionPoly_GetVertices(poly, vtxList, polyVerts);

    f32 x = pos->x;
    f32 z = pos->z;
    f32 ny = COLPOLY_GET_NORMAL(poly->normal.y);
    if (Math3D_TriChkPointParaYImpl(&polyVerts[0], &polyVerts[1], &polyVerts[2], z, x, 0.0f, 1.0f, ny)) {
        *yIntersect = -(SHT_MAX * poly->dist + poly->normal.x * x + poly->normal.z * z) / poly->normal.y;
        return true;
    }
    return false;
}
#endif

/**
 * Checks if point (`x`,`z`) is within `chkDist` of `poly`, computing `yIntersect` if true
 * Determinant max 0.0f (checks if on or within poly)
 *
 * NOTE the y normal must not be too close to 0
 */
s32 CollisionPoly_CheckYIntersect(CollisionPoly* restrict poly, Vec3s* restrict vtxList, f32 x, f32 z,
                                  f32* restrict yIntersect) {
    Vec3s* vtx0 = &vtxList[COLPOLY_VTX_INDEX(poly->vtxData[0])];
    Vec3s* vtx1 = &vtxList[COLPOLY_VTX_INDEX(poly->vtxData[1])];
    Vec3s* vtx2 = &vtxList[COLPOLY_VTX_INDEX(poly->vtxData[2])];

    f32 dX = x - vtx2->x;
    f32 dZ = z - vtx2->z;
    f32 dX21 = vtx2->x - vtx1->x;
    f32 dZ21 = vtx2->z - vtx1->z;
    f32 dX02 = vtx0->x - vtx2->x;
    f32 dZ02 = vtx0->z - vtx2->z;

    f32 d = dX21 * dZ02 - dZ21 * dX02;
    f32 u = dX21 * dZ - dZ21 * dX;
    f32 v = dX02 * dZ - dZ02 * dX;
    if (d < 0.0f) {
        u = -u;
        v = -v;
    }

    if (u >= 0.0f && v >= 0.0f && u + v <= fabsf(d)) {
        // Did intersect, get y intersection
        *yIntersect = -(SHT_MAX * poly->dist + poly->normal.x * x + poly->normal.z * z) / poly->normal.y;
        return true;
    }
    return false;
}

/**
 * Test if travelling from `posA` to `posB` intersects `poly`
 * returns true if an intersection occurs, else false
 * returns `planeIntersect`, which is the point at which the line from `posA` to `posB` crosses `poly`'s plane
 * if `chkOneFace` is true, return false (no intersection) when going through the poly from A to B is done in the
 * normal's direction
 */
s32 CollisionPoly_LineVsPoly(CollisionPoly* restrict poly, Vec3s* restrict vtxList, Vec3f* restrict posA,
                             Vec3f* restrict posB, Vec3f* restrict planeIntersect, s32 chkOneFace) {
    f32 da = DOTXYZ(poly->normal, *posA) * COLPOLY_NORMAL_FRAC;
    f32 db = DOTXYZ(poly->normal, *posB) * COLPOLY_NORMAL_FRAC;
    f32 planeDistA = da + poly->dist;
    f32 planeDistB = db + poly->dist;
    f32 planeDistDelta = planeDistA - planeDistB;

    if ((planeDistA >= 0.0f && planeDistB >= 0.0f) || (planeDistA < 0.0f && planeDistB < 0.0f) ||
        (chkOneFace && planeDistA < 0.0f && planeDistB > 0.0f) || IS_ZERO(planeDistDelta)) {
        return false;
    }

    INSTRUMENTATION(gBgCheckStats.numLinePolysTested++);

    f32 frac = planeDistA / planeDistDelta;
    Vec3f normal;
    Vec3f polyVerts[3];
    CollisionPoly_GetNormalF(poly, &normal.x, &normal.y, &normal.z);
    CollisionPoly_GetVertices(poly, vtxList, polyVerts);
    planeIntersect->x = (posB->x - posA->x) * frac + posA->x;
    planeIntersect->y = (posB->y - posA->y) * frac + posA->y;
    planeIntersect->z = (posB->z - posA->z) * frac + posA->z;
    return PointVsTriangle(planeIntersect, polyVerts, &normal);
}

/**
 * Tests if sphere `center` `radius` intersects `poly`
 */
s32 CollisionPoly_SphVsPoly(CollisionPoly* poly, Vec3s* vtxList, Vec3f* center, f32 radius) {
    Vec3f polyVerts[3];
    Vec3f normal;
    Vec3f intersect;

    // TODO fold these two into SphereVsTriangle ?
    CollisionPoly_GetVertices(poly, vtxList, polyVerts);
    CollisionPoly_GetNormalF(poly, &normal.x, &normal.y, &normal.z);
    return SphereVsTriangle(center, radius, polyVerts, &normal, poly->dist, &intersect);
}

/**
 * Insert `polyId` at the start of the `ssList` list
 */
static void SSNodeList_SetSSListHead(SSNodeList* nodeList, SSList* ssList, u16 polyId) {
    // Get the current index and advance it for next time
    u16 newNodeId = nodeList->count++;
    assert(newNodeId < nodeList->max);
    // Insert the poly
    nodeList->tbl[newNodeId].polyId = polyId;
    nodeList->tbl[newNodeId].next = ssList->head;
    ssList->head = newNodeId;
}

/**
 * Add poly to StaticLookup table
 * Table is sorted by poly's smallest y vertex component
 * `ssList` is the list to append a new poly to
 * `polyList` is the CollisionPoly lookup list
 * `vtxList` is the vertex lookup list
 * `polyId` is the index of the poly in polyList to insert into the lookup table
 */
void StaticLookup_AddPolyToSSList(CollisionContext* colCtx, SSList* ssList, CollisionPoly* polyList, Vec3s* vtxList,
                                  u16 polyId) {

    // if list is null
    if (ssList->head == SS_NULL) {
        SSNodeList_SetSSListHead(&colCtx->polyNodes, ssList, polyId);
        return;
    }

    s32 polyYMin = CollisionPoly_GetMinY(&polyList[polyId], vtxList);
    SSNode* curNode = &colCtx->polyNodes.tbl[ssList->head];
    CollisionPoly* poly = &polyList[curNode->polyId];

    // if the poly being inserted has a lower y than the first poly
    // TODO this could be simplified if it is always true that v0.y < v1.y < v2.y?
    // In particular, (x < a) && (x < b) && (x < c) && (a < b < c) == (x < a) since x < a => x < b since a < b, etc.
    if (polyYMin < vtxList[COLPOLY_VTX_INDEX(poly->vtxData[0])].y &&
        polyYMin < vtxList[COLPOLY_VTX_INDEX(poly->vtxData[1])].y &&
        polyYMin < vtxList[COLPOLY_VTX_INDEX(poly->vtxData[2])].y) {
        SSNodeList_SetSSListHead(&colCtx->polyNodes, ssList, polyId);
        return;
    }

    // while not at the end of the list
    while (curNode->next != SS_NULL) {
        SSNode* nextNode = &colCtx->polyNodes.tbl[curNode->next];

        // if the poly being inserted is lower than the next poly
        poly = &polyList[nextNode->polyId];
        // TODO this could be simplified if it is always true that v0.y < v1.y < v2.y?
        // (see above)
        if (polyYMin < vtxList[COLPOLY_VTX_INDEX(poly->vtxData[0])].y &&
            polyYMin < vtxList[COLPOLY_VTX_INDEX(poly->vtxData[1])].y &&
            polyYMin < vtxList[COLPOLY_VTX_INDEX(poly->vtxData[2])].y) {
            break;
        }
        curNode = nextNode;
    }

    // either now at the end of the list, or at the position such that we're sorted by y coordinate
    u16 newNodeIndex = colCtx->polyNodes.count++;
    assert(newNodeIndex < colCtx->polyNodes.max);
    colCtx->polyNodes.tbl[newNodeIndex].polyId = polyId;
    colCtx->polyNodes.tbl[newNodeIndex].next = curNode->next;
    curNode->next = newNodeIndex;
}

/**
 * Add CollisionPoly to StaticLookup list
 */
void StaticLookup_AddPoly(SSLookup* lookup, CollisionContext* colCtx, CollisionPoly* polyList, Vec3s* vtxList,
                          u16 polyId) {
    if (SNORMAL_IS_FLOOR(polyList[polyId].normal.y)) {
        StaticLookup_AddPolyToSSList(colCtx, &lookup->floor, polyList, vtxList, polyId);
    } else if (SNORMAL_IS_CEILING(polyList[polyId].normal.y)) {
        StaticLookup_AddPolyToSSList(colCtx, &lookup->ceiling, polyList, vtxList, polyId);
    } else {
        StaticLookup_AddPolyToSSList(colCtx, &lookup->wall, polyList, vtxList, polyId);
    }
}

/**
 * Locates the closest static poly directly underneath `pos`, starting at list `ssList`
 * returns yIntersect of the closest poly, or `yIntersectMin`
 * stores the pointer of the closest poly to `outPoly`
 * if BGCHECK_GROUND_CHECK_ON is set, ignore polys with a normal.y < 0 (from vertical walls to ceilings)
 */
f32 BgCheck_RaycastDownStaticList(CollisionContext* colCtx, u16 xpFlags, SSList* ssList, CollisionPoly** outPoly,
                                  Vec3f* pos, f32 yIntersectMin, s32 groundChk) {
    CollisionPoly* restrict polyList = colCtx->colHeader->polyList;
    Vec3s* restrict vtxList = colCtx->colHeader->vtxList;
    f32 result = yIntersectMin;

    POLYLIST_FOREACH(colCtx->polyNodes.tbl, curNode, ssList) {
        CollisionPoly* curPoly = &polyList[curNode->polyId];

        INSTRUMENTATION(gBgCheckStats.numFloorPolysTraversed++);

        // Do flags check
        if (COLPOLY_VTX_CHECK_FLAGS_ANY(curPoly->flags_vIA, xpFlags) || (groundChk && curPoly->normal.y < 0)) {
            continue;
        }

        // This check comes first to avoid touching cache lines in vtxList if they are not needed
        if (ABS(curPoly->normal.y) < COLPOLY_SNORMAL(IS_ZERO_EPS)) {
            // If the y normal is very close to 0, skip
            continue;
        }

        // Do y check
        // TODO this could be simplified if it is always true that v0.y < v1.y < v2.y?
        // Since a < b < c , y < a < b < c => y < a is enough to show that y < b and y < c
        if (pos->y < vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[0])].y &&
            pos->y < vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[1])].y &&
            pos->y < vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[2])].y) {
            // Since static polys are sorted by y, we can stop searching here since any polys beyond this one will
            // also be higher up than the ray origin
            // TODO wtf? This only excludes floors in the first subdivision we check that are higher than the start
            // point, this basically never happens in practice so should we even bother with this check?
            // Ideally we'd sort polys such that future polys are lower down so if we find an intersection we can stop
            // if the next poly doesn't overlap it in y?
            break;
        }

        // Do xz aabb check
        // TODO is this worth? might be more worth if bbIndices were implemented since we wouldn't need to
        // search for min/max elements
        for (s32 i = 0; i < 3; i += 2) { /* x and z only */
            f32 min, max, temp;
            min = max = vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[0])].a[i];
            temp = vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[1])].a[i];
            if (temp < min) {
                min = temp;
            } else if (temp > max) {
                max = temp;
            }
            temp = vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[2])].a[i];
            if (temp < min) {
                min = temp;
            } else if (temp > max) {
                max = temp;
            }

            f32 aa = pos->a[i];
            if (aa < min || aa > max)
                goto cont;
        }

        // TODO bbIndices
#if 0
        for (s32 i = 0; i < 3; i += 2) { /* x and z only */
            s32 i0 = (curPoly->bbIndices >> (2 * i + 0)) & 0b11;
            s32 i1 = (curPoly->bbIndices >> (2 * i + 2)) & 0b11;
            f32 min = vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[i0])].a[i];
            f32 max = vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[i1])].a[i];
            f32 aa = pos->a[i];
            if (aa < min || aa > max)
                goto cont;
        }
#endif

        INSTRUMENTATION(gBgCheckStats.numFloorPolysTested++);

        // Check y intersect
        f32 yIntersect;
        if (CollisionPoly_CheckYIntersect(curPoly, vtxList, pos->x, pos->z, &yIntersect)) {
            INSTRUMENTATION(gBgCheckStats.numFloorPolysPassed++);
            // if poly is closer to pos without going over
            if (yIntersect < pos->y && result < yIntersect) {
                result = yIntersect;
                *outPoly = curPoly;
            }
        }
    cont:
    }
    return result;
}

/**
 * Locates the closest static poly directly underneath `pos` within `lookup`.
 * returns yIntersect of the closest poly, or `yIntersectMin`
 * stores the pointer of the closest poly to `outPoly`
 */
f32 BgCheck_RaycastDownStatic(SSLookup* lookup, CollisionContext* colCtx, u16 xpFlags, CollisionPoly** poly, Vec3f* pos,
                              u32 downChkFlags, f32 yIntersectMin) {
    f32 yIntersect = yIntersectMin;
    s32 groundChk = !!(downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);

    if (downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_FLOORS) {
        yIntersect = BgCheck_RaycastDownStaticList(colCtx, xpFlags, &lookup->floor, poly, pos, yIntersect, 0);
    }

    if ((downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_WALLS) || (downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE)) {
        yIntersect = BgCheck_RaycastDownStaticList(colCtx, xpFlags, &lookup->wall, poly, pos, yIntersect, groundChk);
    }

    if (downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_CEILINGS) {
        yIntersect = BgCheck_RaycastDownStaticList(colCtx, xpFlags, &lookup->ceiling, poly, pos, yIntersect, groundChk);
    }

    return yIntersect;
}

/**
 * Compute wall displacement on `posX` and `posZ`
 * sets `wallPolyPtr` to `poly` if `wallPolyPtr` is NULL or doesn't have `surfaceData[0] & 0x08000000` set
 * returns true if `wallPolyPtr` was changed
 * `xzLength` is sqrt( sq(poly.normal.x) + sq(poly.normal.z) )
 */
static s32 BgCheck_ComputeWallDisplacement(SurfaceType* surfaceTypes, CollisionPoly* poly, f32* posX, f32* posZ, f32 nx,
                                           f32 ny, f32 nz, f32 xzLength, f32 planeDist, f32 radius,
                                           CollisionPoly** wallPolyPtr) {
    f32 displacement = (radius - planeDist) / xzLength;

    *posX += displacement * nx;
    *posZ += displacement * nz;

    CollisionPoly* wallPoly = *wallPolyPtr;
    if (wallPoly != NULL && (surfaceTypes[wallPoly->type].data[1] & 0x08000000)) {
        return false;
    }
    *wallPolyPtr = poly;
    return true;
}

static s32 BgCheck_WallCheck(SurfaceType* surfaceTypes, Vec3f* pos, f32* outX, f32* outZ, Vec3s* vtxList,
                             CollisionPoly* polyList, SSNode* polyTbl, SSList* ssList, f32 radius, u16 xpFlags,
                             s32* outBgId, s32 bgId, CollisionPoly** outPoly) {
    s32 result = false;
    Vec3f resultPos = *pos;

    INSTRUMENTATION(gBgCheckStats.numWallTests++);

    // TODO can we swap these loops for better data locality
    // It seems like we can swap them, but it patches acute angle clips.. lmao
    // Swap the loops and do some code motion :tm: for free extra perf
    for (s32 axis = 2; axis >= 0; axis -= 2) { // 2, 0
        POLYLIST_FOREACH(polyTbl, curNode, ssList) {
            CollisionPoly* curPoly = &polyList[curNode->polyId];

            INSTRUMENTATION(gBgCheckStats.numWallPolysTraversed++);

            if (COLPOLY_VTX_CHECK_FLAGS_ANY(curPoly->flags_vIA, xpFlags)) {
                // There's exclusion flags overlap, skip this poly
                continue;
            }

            if (bgId == BGCHECK_SCENE) {
                // Dyna vertices aren't guaranteed to be sorted by min y so we can only do this for static collision
                // NOTE: Vanilla used pos here so it only really worked nicely for the z axis check and was outdated by
                // the time the x axis is checked.
                // TODO this could be simplified if it is always true that v0.y < v1.y < v2.y?
                if (resultPos.y < vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[0])].y &&
                    resultPos.y < vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[1])].y &&
                    resultPos.y < vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[2])].y) {
                    // If the result is lower than this poly we can stop checking since all future polys are higher
                    // than this one
                    break;
                }
            }

            // Get the normal
            Vec3f n;
            CollisionPoly_GetNormalF(curPoly, &n.x, &n.y, &n.z);

            // Note: assumes the normal is unit length or close enough to unit length
            f32 planeDist = DOTXYZ(n, resultPos) + curPoly->dist;
            if (fabsf(planeDist) > radius) {
                // Too far away
                continue;
            }

            // Get the extent of the normal in XZ
            f32 normalXZSQ = SQ(n.x) + SQ(n.z);
            assert(!IS_ZERO(normalXZSQ));

            // The normal should be sufficiently pronounced along xz to qualify for wall checks
            f32 normalXZ = sqrtf(normalXZSQ);
            f32 proportionXZ = fabsf(n.a[axis]) / normalXZ;
            if (proportionXZ < 0.4f) {
                continue;
            }

            // Check for axial overlap (TODO could be made faster via bbIndices?)
            f32 min, max, temp;
            max = min = vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[0])].a[axis];
            temp = vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[1])].a[axis];
            if (temp < min) {
                min = temp;
            } else if (temp > max) {
                max = temp;
            }
            temp = vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[2])].a[axis];
            if (temp < min) {
                min = temp;
            } else if (max < temp) {
                max = temp;
            }

            min -= radius;
            max += radius;

            if (resultPos.a[axis] < min || max < resultPos.a[axis]) {
                continue;
            }

            Vec3f polyVerts[3];
            CollisionPoly_GetVertices(curPoly, vtxList, polyVerts);

            s32 axis1 = 1 - (axis != 0); // z -> x, x -> y
            s32 axis2 = 2 - (axis != 0); // z -> y, x -> z

            INSTRUMENTATION(gBgCheckStats.numWallPolysTested++);

            if (!PointVsTriangle(&resultPos, polyVerts, &n)) {
                continue;
            }

            INSTRUMENTATION(gBgCheckStats.numWallPolysPassed++);

            f32 intersect =
                (n.a[axis1] * resultPos.a[axis1] + n.a[axis2] * resultPos.a[axis2] + curPoly->dist) / n.a[axis];
            f32 intersectDist = -intersect - resultPos.a[axis];

            if (fabsf(intersectDist) <= radius / proportionXZ && intersectDist * n.a[axis] <= 4.0f) {
                if (BgCheck_ComputeWallDisplacement(surfaceTypes, curPoly, &resultPos.x, &resultPos.z,
                                                    n.x, n.y, n.z, normalXZ, planeDist, radius, outPoly)) {
                    *outBgId = bgId;
                }
                result = true;
            }
        }
    }

    *outX = resultPos.x;
    *outZ = resultPos.z;
    return result;
}

/**
 * Tests for collision with a static poly ceiling
 * returns true if a collision occurs, else false
 * `outPoly` returns the poly collided with
 * `outY` returns the y coordinate needed to not collide with `outPoly`
 */
s32 BgCheck_CheckStaticCeiling(SSLookup* lookup, u16 xpFlags, CollisionContext* colCtx, f32* outY, Vec3f* pos,
                               f32 checkHeight, CollisionPoly** outPoly) {
    CollisionPoly* restrict polyList = colCtx->colHeader->polyList;
    Vec3s* restrict vtxList = colCtx->colHeader->vtxList;
    s32 result = false;

    *outY = pos->y;

    // Walk the poly linked list
    POLYLIST_FOREACH(colCtx->polyNodes.tbl, curNode, &lookup->ceiling) {
        CollisionPoly* curPoly = &polyList[curNode->polyId];

        INSTRUMENTATION(gBgCheckStats.numCeilingPolysTraversed++);

        // Check for overlap in exclusion flags, if there are any skip this poly
        if (COLPOLY_VTX_CHECK_FLAGS_ANY(curPoly->flags_vIA, xpFlags)) {
            continue;
        }

        if (ABS(curPoly->normal.y) < COLPOLY_SNORMAL(IS_ZERO_EPS)) {
            continue;
        }

        INSTRUMENTATION(gBgCheckStats.numCeilingPolysTested++);

        // Check for intersection, this is essentially a 2D intersection in xz.
        f32 ceilingY;
        if (CollisionPoly_CheckYIntersect(curPoly, vtxList, pos->x, pos->z, &ceilingY)) {
            INSTRUMENTATION(gBgCheckStats.numCeilingPolysPassed++);

            f32 intersectDist = ceilingY - *outY;
            f32 ny = COLPOLY_GET_NORMAL(curPoly->normal.y);

            if (intersectDist > 0.0f && intersectDist < checkHeight && intersectDist * ny <= 0.0f) {
                *outY = ceilingY - checkHeight;
                *outPoly = curPoly;
                result = true;
            }
        }
    }
    return result;
}

/**
 * Tests if line `posA` to `posB` intersects with a static poly in list `ssList`. Uses polyCheckTbl
 * returns true if such a poly exists, else false
 * `outPoly` returns the pointer of the poly intersected
 * `posB` and `outPos` returns the point of intersection with `outPoly`
 * `outDistSq` returns the squared distance from `posA` to the point of intersect
 */
s32 BgCheck_CheckLineAgainstSSList(SSList* ssList, CollisionContext* colCtx, u16 xpFlags1, u16 xpFlags2, Vec3f* posA,
                                   Vec3f* posB, Vec3f* outPos, CollisionPoly** outPoly, f32* outDistSq, s32 bccFlags) {
    CollisionPoly* polyList = colCtx->colHeader->polyList;
    Vec3s* vtxList = colCtx->colHeader->vtxList;
    s32 result = false;

    // Walk the poly linked list
    POLYLIST_FOREACH(colCtx->polyNodes.tbl, curNode, ssList) {
        CollisionPoly* curPoly = &polyList[curNode->polyId];

        INSTRUMENTATION(gBgCheckStats.numLinePolysTraversed++);

        // If:
        // - there is overlap with xpFlags1
        // - there is no overlap with xpFlags2 (or xpFlags2 is 0)
        if (COLPOLY_VTX_CHECK_FLAGS_ANY(curPoly->flags_vIA, xpFlags1) ||
            (xpFlags2 != 0 && !COLPOLY_VTX_CHECK_FLAGS_ANY(curPoly->flags_vIA, xpFlags2))) {
            continue;
        }

#if USE_POLY_CHK_TBL
        u32* checkedPoly = &colCtx->polyCheckTbl[curNode->polyId >> 5];
        u32 key = 1 << (curNode->polyId & 31);

        // If poly was already checked (polys that straddle multiple subdivisions may be checked more than once
        // otherwise)
        if ((*checkedPoly) & key) {
            // TODO how often is this actually triggered? If it has a very low probability of triggering, we should
            // scrap it even though the memory footprint is vastly reduced by a bitset representation.
            continue;
        }
        // Mark this poly as checked so it's skipped if checked again
        *checkedPoly |= key;
#endif

        // Check if there's overlap in y to early reject rays that can't possibly overlap
        // TODO can we sort poly indices such that vtxData[0] always points to min y?
        f32 minY = CollisionPoly_GetMinY(curPoly, vtxList);
        if (posA->y < minY && posB->y < minY) {
            break;
        }

        // Do the full line vs triangle test
        Vec3f polyIntersect;
        if (CollisionPoly_LineVsPoly(curPoly, vtxList, posA, posB, &polyIntersect, bccFlags & BGCHECK_CHECK_ONE_FACE)) {
            INSTRUMENTATION(gBgCheckStats.numLinePolysPassed++);
            // TODO switch to line parameter t to reduce the amount of calculations happening here
            Vec3f diff = VSUB(posA, &polyIntersect);
            f32 distSq = VMAGSQ(&diff);
            if (distSq < *outDistSq) {
                // Intersection is the closest one found so far, update it
                *outDistSq = distSq;
                *outPos = polyIntersect;
                *posB = polyIntersect;
                *outPoly = curPoly;
                result = true;
            }
        }
    }
    return result;
}

/**
 * Tests if line `posA` to `posB` intersects with a static poly in `lookup`. Uses polyCheckTbl
 * returns true if such a poly exists, else false
 * `outPoly` returns the pointer of the poly intersected
 * `posB` and `outPos` returns the point of intersection with `outPoly`
 * `outDistSq` returns the squared distance from `posA` to the point of intersect
 */
s32 BgCheck_CheckLineInSubdivision(SSLookup* lookup, CollisionContext* colCtx, u16 xpFlags1, u16 xpFlags2, Vec3f* posA,
                                   Vec3f* posB, Vec3f* outPos, CollisionPoly** outPoly, f32* outDistSq, u32 bccFlags) {
    s32 result = false;

    result |= (bccFlags & BGCHECK_CHECK_FLOOR) && lookup->floor.head != SS_NULL &&
              BgCheck_CheckLineAgainstSSList(&lookup->floor, colCtx, xpFlags1, xpFlags2, posA, posB, outPos, outPoly,
                                             outDistSq, bccFlags);

    result |= (bccFlags & BGCHECK_CHECK_WALL) && lookup->wall.head != SS_NULL &&
              BgCheck_CheckLineAgainstSSList(&lookup->wall, colCtx, xpFlags1, xpFlags2, posA, posB, outPos, outPoly,
                                             outDistSq, bccFlags);

    result |= (bccFlags & BGCHECK_CHECK_CEILING) && lookup->ceiling.head != SS_NULL &&
              BgCheck_CheckLineAgainstSSList(&lookup->ceiling, colCtx, xpFlags1, xpFlags2, posA, posB, outPos, outPoly,
                                             outDistSq, bccFlags);

    return result;
}

/**
 * Get first static poly intersecting sphere `center` `radius` from list `node`
 * returns true if any poly intersects the sphere, else returns false
 * `outPoly` returns the pointer of the first poly found that intersects
 */
s32 BgCheck_SphVsFirstStaticPolyList(SSList* ssList, u16 xpFlags, CollisionContext* colCtx, Vec3f* center, f32 radius,
                                     CollisionPoly** outPoly) {
    Vec3s* restrict vtxList = colCtx->colHeader->vtxList;
    CollisionPoly* restrict polyList = colCtx->colHeader->polyList;

    POLYLIST_FOREACH(colCtx->polyNodes.tbl, curNode, ssList) {
        CollisionPoly* curPoly = &polyList[curNode->polyId];

        // If there's exclusion flags overlap, skip
        if (COLPOLY_VTX_CHECK_FLAGS_ANY(curPoly->flags_vIA, xpFlags)) {
            continue;
        }

        // If the sphere top is lower than this poly, break since there can't be another poly higher
        // than this one later on.
        // TODO this could be simplified if it is always true that v0.y < v1.y < v2.y?
        if (center->y + radius < vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[0])].y &&
            center->y + radius < vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[1])].y &&
            center->y + radius < vtxList[COLPOLY_VTX_INDEX(curPoly->vtxData[2])].y) {
            break;
        }

        // Check sphere-triangle intersection
        if (CollisionPoly_SphVsPoly(curPoly, vtxList, center, radius)) {
            *outPoly = curPoly;
            return true;
        }
    }
    return false;
}

/**
 * Get first static poly intersecting sphere `center` `radius` within `lookup`
 * returns true if any poly intersects the sphere, else false
 * `outPoly` returns the first poly found that intersects
 */
s32 BgCheck_SphVsFirstStaticPoly(SSLookup* lookup, u16 xpFlags, CollisionContext* colCtx, Vec3f* center, f32 radius,
                                 CollisionPoly** outPoly, u16 bciFlags) {
    if (lookup->floor.head != SS_NULL && !(bciFlags & BGCHECK_IGNORE_FLOOR) &&
        BgCheck_SphVsFirstStaticPolyList(&lookup->floor, xpFlags, colCtx, center, radius, outPoly)) {
        return true;
    }

    if (lookup->wall.head != SS_NULL && !(bciFlags & BGCHECK_IGNORE_WALL) &&
        BgCheck_SphVsFirstStaticPolyList(&lookup->wall, xpFlags, colCtx, center, radius, outPoly)) {
        return true;
    }

    if (lookup->ceiling.head != SS_NULL && !(bciFlags & BGCHECK_IGNORE_CEILING) &&
        BgCheck_SphVsFirstStaticPolyList(&lookup->ceiling, xpFlags, colCtx, center, radius, outPoly)) {
        return true;
    }

    return false;
}

/**
 * Get StaticLookup from `pos`
 * Does not return NULL
 */
SSLookup* BgCheck_GetNearestStaticLookup(CollisionContext* colCtx, SSLookup* lookupTbl, Vec3f* pos) {
    Vec3i sector;

    BgCheck_GetStaticLookupIndicesFromPos(colCtx, pos, &sector);
    return &lookupTbl[sector.x + colCtx->subdivAmount.x * (sector.y + colCtx->subdivAmount.y * sector.z)];
}

/**
 * Get StaticLookup from `pos`
 * Returns NULL if just outside the mesh bounding box
 */
SSLookup* BgCheck_GetStaticLookup(CollisionContext* colCtx, SSLookup* lookupTbl, Vec3f* pos) {
    Vec3i sector;
    if (!BgCheck_PosInStaticBoundingBox(colCtx, pos)) {
        return NULL;
    }
    BgCheck_GetStaticLookupIndicesFromPos(colCtx, pos, &sector);
    return &lookupTbl[sector.x + colCtx->subdivAmount.x * (sector.y + colCtx->subdivAmount.y * sector.z)];
}

/**
 * Get StaticLookup subdivision indices from `pos`
 * `sector` returns the subdivision x,y,z indices containing or is nearest to `pos`
 */
void BgCheck_GetStaticLookupIndicesFromPos(CollisionContext* colCtx, Vec3f* pos, Vec3i* sector) {
    for (u32 i = 0; i < 3; i++) {
        sector->a[i] = (pos->a[i] - colCtx->minBounds.a[i]) * colCtx->subdivLengthInv.a[i];
        if (sector->a[i] < 0) {
            sector->a[i] = 0;
        } else if (sector->a[i] >= colCtx->subdivAmount.a[i]) {
            sector->a[i] = colCtx->subdivAmount.a[i] - 1;
        }
    }
}

/**
 * Get negative bias subdivision indices
 * decrements indices if `pos` is within BGCHECK_SUBDIV_OVERLAP units of the negative subdivision boundary
 * `sx`, `sy`, `sz` returns the subdivision x, y, z indices
 */
void BgCheck_GetSubdivisionMinBounds(CollisionContext* colCtx, Vec3f* pos, s32* sx, s32* sy, s32* sz) {
    f32 dx = pos->x - colCtx->minBounds.x;
    f32 dy = pos->y - colCtx->minBounds.y;
    f32 dz = pos->z - colCtx->minBounds.z;

    *sx = dx * colCtx->subdivLengthInv.x;
    *sy = dy * colCtx->subdivLengthInv.y;
    *sz = dz * colCtx->subdivLengthInv.z;

    if (((s32)dx % (s32)colCtx->subdivLength.x < BGCHECK_SUBDIV_OVERLAP) && (*sx > 0)) {
        *sx -= 1;
    }

    if (((s32)dy % (s32)colCtx->subdivLength.y < BGCHECK_SUBDIV_OVERLAP) && (*sy > 0)) {
        *sy -= 1;
    }

    if (((s32)dz % (s32)colCtx->subdivLength.z < BGCHECK_SUBDIV_OVERLAP) && (*sz > 0)) {
        *sz -= 1;
    }
}

/**
 * Get positive bias subdivision indices
 * increments indices if `pos` is within BGCHECK_SUBDIV_OVERLAP units of the positive subdivision boundary
 * `sx`, `sy`, `sz` returns the subdivision x, y, z indices
 */
void BgCheck_GetSubdivisionMaxBounds(CollisionContext* colCtx, Vec3f* pos, s32* sx, s32* sy, s32* sz) {
    f32 dx = pos->x - colCtx->minBounds.x;
    f32 dy = pos->y - colCtx->minBounds.y;
    f32 dz = pos->z - colCtx->minBounds.z;

    *sx = dx * colCtx->subdivLengthInv.x;
    *sy = dy * colCtx->subdivLengthInv.y;
    *sz = dz * colCtx->subdivLengthInv.z;

    if (((s32)colCtx->subdivLength.x - BGCHECK_SUBDIV_OVERLAP < (s32)dx % (s32)colCtx->subdivLength.x) &&
        (*sx < colCtx->subdivAmount.x - 1)) {
        *sx += 1;
    }

    if (((s32)colCtx->subdivLength.y - BGCHECK_SUBDIV_OVERLAP < (s32)dy % (s32)colCtx->subdivLength.y) &&
        (*sy < colCtx->subdivAmount.y - 1)) {
        *sy += 1;
    }

    if (((s32)colCtx->subdivLength.z - BGCHECK_SUBDIV_OVERLAP < (s32)dz % (s32)colCtx->subdivLength.z) &&
        (*sz < colCtx->subdivAmount.z - 1)) {
        *sz += 1;
    }
}

/**
 * Calculate the subdivision index bounding box for CollisionPoly `polyId`
 * `subdivMinX`, `subdivMinY`, `subdivMinZ` returns the minimum subdivision x, y, z indices
 * `subdivMaxX`, `subdivMaxY`, `subdivMaxZ` returns the maximum subdivision x, y, z indices
 */
void BgCheck_GetPolySubdivisionBounds(CollisionContext* colCtx, Vec3s* vtxList, CollisionPoly* polyList,
                                      s32* subdivMinX, s32* subdivMinY, s32* subdivMinZ, s32* subdivMaxX,
                                      s32* subdivMaxY, s32* subdivMaxZ, u16 polyId) {
    u16 vtxId = COLPOLY_VTX_INDEX(polyList[polyId].vtxData[0]);
    Vec3f minVtx;
    Vec3f maxVtx;
    minVtx.x = maxVtx.x = vtxList[vtxId].x;
    minVtx.y = maxVtx.y = vtxList[vtxId].y;
    minVtx.z = maxVtx.z = vtxList[vtxId].z;

    for (s32 i = 1; i < 3; i++) {
        Vec3s* vtx = &vtxList[COLPOLY_VTX_INDEX(polyList[polyId].vtxData[i])];

        for (s32 j = 0; j < 3; j++) {
            f32 e = vtx->a[j];
            if (minVtx.a[j] > e) {
                minVtx.a[j] = e;
            } else if (maxVtx.a[j] < e) {
                maxVtx.a[j] = e;
            }
        }
    }
    BgCheck_GetSubdivisionMinBounds(colCtx, &minVtx, subdivMinX, subdivMinY, subdivMinZ);
    BgCheck_GetSubdivisionMaxBounds(colCtx, &maxVtx, subdivMaxX, subdivMaxY, subdivMaxZ);
}

/**
 * Test if poly `polyList`[`polyId`] intersects cube `min` `max`
 * returns true if the poly intersects the cube, else false
 */
s32 BgCheck_PolyIntersectsSubdivision(Vec3f* min, Vec3f* max, CollisionPoly* polyList, Vec3s* vtxList, u16 polyId) {
    CollisionPoly* poly = &polyList[polyId];
    Vec3f polyVerts[3];
    s32 flags[3];
    u32 i;

    for (i = 0; i < 3; i++) {
        Vec3s* vtx = &vtxList[COLPOLY_VTX_INDEX(poly->vtxData[i])];
        polyVerts[i].x = vtx->x;
        polyVerts[i].y = vtx->y;
        polyVerts[i].z = vtx->z;

        flags[i] = Math3D_PointRelativeToCubeFaces(&polyVerts[i], min, max);
        if (flags[i] == 0) {
            return true;
        }
    }

    if (flags[0] & flags[1] & flags[2]) {
        return false;
    }

    for (i = 0; i < 3; i++) {
        flags[i] |= Math3D_PointRelativeToCubeEdges(&polyVerts[i], min, max) << 8;
    }

    if (flags[0] & flags[1] & flags[2]) {
        return false;
    }

    for (i = 0; i < 3; i++) {
        flags[i] |= Math3D_PointRelativeToCubeVertices(&polyVerts[i], min, max) << 24;
    }

    if (flags[0] & flags[1] & flags[2]) {
        return false;
    }

    f32 nx, ny, nz;
    CollisionPoly_GetNormalF(poly, &nx, &ny, &nz);
    f32 dist = poly->dist;
    f32 intersect;

    // Unwraps to:
    // min->y, min->z, min->x, max->x
    // min->y, max->z, min->x, max->x
    // max->y, min->z, min->x, max->x
    // max->y, max->z, min->x, max->x
    //
    // min->z, min->x, min->y, max->y
    // max->z, min->x, min->y, max->y
    // min->z, max->x, min->y, max->y
    // max->z, max->x, min->y, max->y
    //
    // min->x, min->y, min->z, max->z
    // min->x, max->y, min->z, max->z
    // max->x, min->y, min->z, max->z
    // max->x, max->y, min->z, max->z
    for (i = 0; i < 12; i++) {
        // 0b00 -> min, min
        // 0b01 -> min, max
        // 0b10 -> max, min
        // 0b11 -> max, max
        Vec3f* in1 = (i & (1 << 1)) ? max : min;
        Vec3f* in2 = (i & (1 << 0)) ? max : min;

        // x, y, z
        // y, z, x
        // z, x, y
        u32 a = ((i >> 2) + 0) % 3;
        u32 b = ((i >> 2) + 1) % 3;
        u32 c = ((i >> 2) + 2) % 3;

        if (Math3D_TriChkLineSegParaYIntersect(&polyVerts[0], &polyVerts[1], &polyVerts[2], nx, ny, nz, dist, in1->a[b],
                                               in2->a[c], &intersect, min->a[a], max->a[a])) {
            return true;
        }
    }

    return Math3D_LineVsCube(min, max, &polyVerts[0], &polyVerts[1]) ||
           Math3D_LineVsCube(min, max, &polyVerts[1], &polyVerts[2]) ||
           Math3D_LineVsCube(min, max, &polyVerts[2], &polyVerts[0]);
}

/**
 * Initialize StaticLookup Table
 * returns size of table, in bytes
 */
u32 BgCheck_InitializeStaticLookup(CollisionContext* colCtx, PlayState* play, SSLookup* lookupTbl) {
    Vec3s* vtxList;
    CollisionPoly* polyList;
    u16 polyMax;
    u16 polyId;
    s32 sx;
    s32 sy;
    s32 sz;
    // subdivMin indices
    s32 sxMin;
    s32 syMin;
    s32 szMin;
    // subdivMax indices
    s32 sxMax;
    s32 syMax;
    s32 szMax;
    // subdiv min/max bounds for adding a poly
    Vec3f curSubdivMin;
    Vec3f curSubdivMax;
    CollisionHeader* colHeader;
    SSLookup* lookupTblXY;
    SSLookup* lookupTblX;
    SSLookup* lookup;
    s32 subdivAmountXY;
    f32 subdivLengthX;
    f32 subdivLengthY;
    f32 subdivLengthZ;
    u32 nBins = colCtx->subdivAmount.x * colCtx->subdivAmount.y * colCtx->subdivAmount.z;

    static_assert((u8)SS_NULL == (u8)(SS_NULL >> 8), "SS_NULL halves must be equal");
    memset(lookupTbl, SS_NULL, nBins * sizeof(SSLookup));

    subdivAmountXY = colCtx->subdivAmount.x * colCtx->subdivAmount.y;
    subdivLengthX = colCtx->subdivLength.x + (2 * BGCHECK_SUBDIV_OVERLAP);
    subdivLengthY = colCtx->subdivLength.y + (2 * BGCHECK_SUBDIV_OVERLAP);
    subdivLengthZ = colCtx->subdivLength.z + (2 * BGCHECK_SUBDIV_OVERLAP);

    colHeader = colCtx->colHeader;
    polyMax = colHeader->numPolygons;
    vtxList = colHeader->vtxList;
    polyList = colHeader->polyList;

#if DO_INSTRUMENTATION
    u32 numFloors = 0;
    u32 numWalls = 0;
    u32 numCeilings = 0;
#endif

    for (polyId = 0; polyId < polyMax; polyId++) {
        BgCheck_GetPolySubdivisionBounds(colCtx, vtxList, polyList, &sxMin, &syMin, &szMin, &sxMax, &syMax, &szMax,
                                         polyId);
        lookupTblXY = lookupTbl + szMin * subdivAmountXY;
        curSubdivMin.z = (colCtx->subdivLength.z * szMin + colCtx->minBounds.z) - BGCHECK_SUBDIV_OVERLAP;
        curSubdivMax.z = curSubdivMin.z + subdivLengthZ;

        for (sz = szMin; sz < szMax + 1; sz++) {
            lookupTblX = lookupTblXY + colCtx->subdivAmount.x * syMin;
            curSubdivMin.y = (colCtx->subdivLength.y * syMin + colCtx->minBounds.y) - BGCHECK_SUBDIV_OVERLAP;
            curSubdivMax.y = curSubdivMin.y + subdivLengthY;

            for (sy = syMin; sy < syMax + 1; sy++) {
                lookup = lookupTblX + sxMin;
                curSubdivMin.x = (colCtx->subdivLength.x * sxMin + colCtx->minBounds.x) - BGCHECK_SUBDIV_OVERLAP;
                curSubdivMax.x = curSubdivMin.x + subdivLengthX;

                for (sx = sxMin; sx < sxMax + 1; sx++) {
                    if (BgCheck_PolyIntersectsSubdivision(&curSubdivMin, &curSubdivMax, polyList, vtxList, polyId)) {
                        StaticLookup_AddPoly(lookup, colCtx, polyList, vtxList, polyId);
#if DO_INSTRUMENTATION
                        if (SNORMAL_IS_FLOOR(polyList[polyId].normal.y)) {
                            numFloors++;
                        } else if (SNORMAL_IS_CEILING(polyList[polyId].normal.y)) {
                            numCeilings++;
                        } else {
                            numWalls++;
                        }
#endif
                    }
                    curSubdivMin.x += colCtx->subdivLength.x;
                    curSubdivMax.x += colCtx->subdivLength.x;
                    lookup++;
                }
                curSubdivMin.y += colCtx->subdivLength.y;
                curSubdivMax.y += colCtx->subdivLength.y;
                lookupTblX += colCtx->subdivAmount.x;
            }
            curSubdivMin.z += colCtx->subdivLength.z;
            curSubdivMax.z += colCtx->subdivLength.z;
            lookupTblXY += subdivAmountXY;
        }
    }

#if DO_INSTRUMENTATION
    u32 nBinsFilled = 0;
    for (u32 i = 0; i < nBins; i++) {
        nBinsFilled += lookupTbl[i].ceiling.head != SS_NULL ||
                       lookupTbl[i].wall.head != SS_NULL ||
                       lookupTbl[i].floor.head != SS_NULL;
    }

    PRINTF("Binned %u polys into %u nodes (%u floors, %u walls, %u ceilings) across %u / %u bins\n",
           polyMax, colCtx->polyNodes.count, numFloors, numWalls, numCeilings, nBinsFilled, nBins);
#endif

    return colCtx->polyNodes.count * sizeof(SSNode);
}

/**
 * Is current scene a SPOT scene
 */
s32 BgCheck_IsSpotScene(PlayState* play) {
    static s16 spotScenes[] = {
        SCENE_HYRULE_FIELD,          SCENE_KAKARIKO_VILLAGE,     SCENE_GRAVEYARD,     SCENE_ZORAS_RIVER,
        SCENE_KOKIRI_FOREST,         SCENE_SACRED_FOREST_MEADOW, SCENE_LAKE_HYLIA,    SCENE_ZORAS_DOMAIN,
        SCENE_ZORAS_FOUNTAIN,        SCENE_GERUDO_VALLEY,        SCENE_LOST_WOODS,    SCENE_DESERT_COLOSSUS,
        SCENE_GERUDOS_FORTRESS,      SCENE_HAUNTED_WASTELAND,    SCENE_HYRULE_CASTLE, SCENE_DEATH_MOUNTAIN_TRAIL,
        SCENE_DEATH_MOUNTAIN_CRATER, SCENE_GORON_CITY,           SCENE_LON_LON_RANCH,
    };
    s16* i;

    for (i = spotScenes; i < spotScenes + ARRAY_COUNT(spotScenes); i++) {
        if (play->sceneId == *i) {
            return true;
        }
    }
    return false;
}

typedef struct BgCheckSceneMemEntry {
    u32 sceneId : 8;
    u32 memSize : 24;
} BgCheckSceneMemEntry;

/**
 * Get custom scene memSize
 */
s32 BgCheck_TryGetCustomMemsize(s32 sceneId, u32* memSize) {
    static BgCheckSceneMemEntry sceneMemList[] = {
        { SCENE_HYRULE_FIELD, 0xB798 },         { SCENE_GANONS_TOWER_COLLAPSE_EXTERIOR, 0x78C8 },
        { SCENE_GANON_BOSS, 0x70C8 },           { SCENE_SPIRIT_TEMPLE_BOSS, 0xACC8 },
        { SCENE_CHAMBER_OF_THE_SAGES, 0x70C8 }, { SCENE_SPIRIT_TEMPLE, 0x16CC8 },
        { SCENE_FIRE_TEMPLE, 0x198C8 },         { SCENE_GANONDORF_BOSS, 0x84C8 },
    };

    for (s32 i = 0; i < ARRAY_COUNT(sceneMemList); i++) {
        if (sceneId == sceneMemList[i].sceneId) {
            *memSize = sceneMemList[i].memSize;
            return true;
        }
    }
    return false;
}

/**
 * Compute subdivLength for scene mesh lookup, for a single dimension
 */
void BgCheck_SetSubdivisionDimension(f32 min, s32 subdivAmount, f32* max, f32* subdivLength, f32* subdivLengthInv) {
    f32 length = (*max - min);

    *subdivLength = (s32)(length / subdivAmount) + 1;
    *subdivLength = CLAMP_MIN(*subdivLength, BGCHECK_SUBDIV_MIN);
    *subdivLengthInv = 1.0f / *subdivLength;

    *max = *subdivLength * subdivAmount + min;
}

typedef struct BgCheckSceneSubdivisionEntry {
    s16 sceneId;
    Vec3s subdivAmount;
    s32 nodeListMax; // if <= 0, dynamically compute max nodes
} BgCheckSceneSubdivisionEntry;

/**
 * Allocate CollisionContext
 */
void BgCheck_Allocate(CollisionContext* colCtx, PlayState* play, CollisionHeader* colHeader) {
    static BgCheckSceneSubdivisionEntry sceneSubdivisionList[] = {
        { SCENE_SHADOW_TEMPLE, { 23, 7, 14 }, 0 },
        { SCENE_FOREST_TEMPLE, { 38, 1, 38 }, 0 },
    };
    s32 customNodeListMax = 0;

    colCtx->colHeader = colHeader;

    PRINTF(T("/*---------------- BGCheck バッファーメモリサイズ -------------*/\n",
             "/*---------------- BGCheck Buffer Memory Size -------------*/\n"));

    if ((R_SCENE_CAM_TYPE == SCENE_CAM_TYPE_FIXED_SHOP_VIEWPOINT) ||
        (R_SCENE_CAM_TYPE == SCENE_CAM_TYPE_FIXED_TOGGLE_VIEWPOINT) || (R_SCENE_CAM_TYPE == SCENE_CAM_TYPE_FIXED) ||
        (R_SCENE_CAM_TYPE == SCENE_CAM_TYPE_FIXED_MARKET)) {
        if (play->sceneId == SCENE_STABLE) {
            colCtx->memSize = 0x3520;
            PRINTF(T("/* BGCheck LonLonサイズ %dbyte */\n", "/* BGCheck LonLon Size %dbyte */\n"), colCtx->memSize);
        } else {
            colCtx->memSize = 0x4E20;
            PRINTF(T("/* BGCheck ミニサイズ %dbyte */\n", "/* BGCheck Mini Size %dbyte */\n"), colCtx->memSize);
        }
        colCtx->subdivAmount.x = 2;
        colCtx->subdivAmount.y = 2;
        colCtx->subdivAmount.z = 2;
    } else if (BgCheck_IsSpotScene(play)) {
        colCtx->memSize = 0xF000;
        PRINTF(T("/* BGCheck Spot用サイズ %dbyte */\n", "/* BGCheck Spot Size %dbyte */\n"), colCtx->memSize);
        colCtx->subdivAmount.x = 16;
        colCtx->subdivAmount.y = 8;
        colCtx->subdivAmount.z = 16;
    } else {
        u32 customMemSize;
        if (BgCheck_TryGetCustomMemsize(play->sceneId, &customMemSize)) {
            colCtx->memSize = customMemSize;
        } else {
            colCtx->memSize = 0x1CC00;
        }
        PRINTF(T("/* BGCheck ノーマルサイズ %dbyte  */\n", "/* BGCheck Normal Size %dbyte  */\n"), colCtx->memSize);
        s32 useCustomSubdivisions = false;

        for (s32 i = 0; i < ARRAY_COUNT(sceneSubdivisionList); i++) {
            if (play->sceneId == sceneSubdivisionList[i].sceneId) {
                colCtx->subdivAmount.x = sceneSubdivisionList[i].subdivAmount.x;
                colCtx->subdivAmount.y = sceneSubdivisionList[i].subdivAmount.y;
                colCtx->subdivAmount.z = sceneSubdivisionList[i].subdivAmount.z;
                useCustomSubdivisions = true;
                customNodeListMax = sceneSubdivisionList[i].nodeListMax;
            }
        }
        if (!useCustomSubdivisions) {
            colCtx->subdivAmount.x = 16;
            colCtx->subdivAmount.y = 8;
            colCtx->subdivAmount.z = 16;
        }
    }

    u32 numSubdivisions = colCtx->subdivAmount.x * colCtx->subdivAmount.y * colCtx->subdivAmount.z;
    u32 memSize = numSubdivisions * sizeof(SSLookup);
#if USE_POLY_CHK_TBL
    memSize += BITSET_SIZE_BYTES(colCtx->colHeader->numPolygons, colCtx->polyCheckTbl[0]);
#endif

    if (memSize > colCtx->memSize) {
        LogUtils_HungupThread(__FILE__, __LINE__);
    }

    colCtx->lookupTbl =
        THA_AllocTailAlign(&play->state.tha, numSubdivisions * sizeof(SSLookup), ALIGNOF_MASK(SSLookup));
    assert(colCtx->lookupTbl != NULL);

    u32 tblMax;
    if (customNodeListMax > 0) {
        tblMax = customNodeListMax;
        assert(tblMax <= (colCtx->memSize - memSize) / sizeof(SSNode));
    } else {
        tblMax = (colCtx->memSize - memSize) / sizeof(SSNode);
    }

    colCtx->polyNodes.count = 0;
    colCtx->polyNodes.max = tblMax;
    colCtx->polyNodes.tbl = THA_AllocTailAlign(&play->state.tha, tblMax * sizeof(SSNode), ALIGNOF_MASK(SSNode));
    assert(colCtx->polyNodes.tbl != NULL);

#if USE_POLY_CHK_TBL
    u32 polyCheckSize = BITSET_SIZE_BYTES(colCtx->colHeader->numPolygons, colCtx->polyCheckTbl[0]);
    colCtx->polyCheckTbl = THA_AllocTailAlign16(&play->state.tha, polyCheckSize);
    assert(colCtx->polyCheckTbl != NULL);
#endif

    colCtx->minBounds.x = colCtx->colHeader->minBounds.x;
    colCtx->minBounds.y = colCtx->colHeader->minBounds.y;
    colCtx->minBounds.z = colCtx->colHeader->minBounds.z;
    colCtx->maxBounds.x = colCtx->colHeader->maxBounds.x;
    colCtx->maxBounds.y = colCtx->colHeader->maxBounds.y;
    colCtx->maxBounds.z = colCtx->colHeader->maxBounds.z;
    BgCheck_SetSubdivisionDimension(colCtx->minBounds.x, colCtx->subdivAmount.x, &colCtx->maxBounds.x,
                                    &colCtx->subdivLength.x, &colCtx->subdivLengthInv.x);
    BgCheck_SetSubdivisionDimension(colCtx->minBounds.y, colCtx->subdivAmount.y, &colCtx->maxBounds.y,
                                    &colCtx->subdivLength.y, &colCtx->subdivLengthInv.y);
    BgCheck_SetSubdivisionDimension(colCtx->minBounds.z, colCtx->subdivAmount.z, &colCtx->maxBounds.z,
                                    &colCtx->subdivLength.z, &colCtx->subdivLengthInv.z);
    // TODO ideally we'd move generating the acceleration structure to scene export so we aren't spending time on
    // every scene load binning the polys, and we could use a better acceleration structure that doesn't involve
    // linked list traversals and uniform grids (this post was made by Bounding Volume Hierarchies gang)
    u32 lookupTblMemSize = BgCheck_InitializeStaticLookup(colCtx, play, colCtx->lookupTbl);

    PRINTF(VT_FGCOL(GREEN) "/*--- BGCheck used 0x%X + 0x%X bytes of maximum 0x%X bytes ---*/\n" VT_RST,
           memSize, lookupTblMemSize, colCtx->memSize);

    DynaCollisionContext* dyna = &colCtx->dyna;
    for (s32 i = 0; i < BG_ACTOR_MAX; i++) {
        dyna->bgActorFlags[i] = 0;
        BgActor_Initialize(play, &dyna->bgActors[i]);
        dyna->bgActorFlags[i] |= BGACTOR_INVALIDATE_LOOKUP;
    }
}

/**
 * Get CollisionHeader
 * original name: T_BGCheck_getBGDataInfo
 */
CollisionHeader* BgCheck_GetCollisionHeader(CollisionContext* colCtx, s32 bgId) {
    if (bgId == BGCHECK_SCENE) {
        return colCtx->colHeader;
    }
    if (bgId < 0 || bgId > BG_ACTOR_MAX) {
        return NULL;
    }
    if (!(colCtx->dyna.bgActorFlags[bgId] & BGACTOR_IN_USE)) {
        PRINTF_COLOR_WARNING();
        PRINTF(T("T_BGCheck_getBGDataInfo():そのbg_actor_indexは使われておりません。index=%d\n",
                 "T_BGCheck_getBGDataInfo(): That bg_actor_index is not in use. index=%d\n"));
        PRINTF_RST();
        return NULL;
    }
    return colCtx->dyna.bgActors[bgId].colHeader;
}

/**
 * Test if pos is near collision boundaries
 */
s32 BgCheck_PosInStaticBoundingBox(CollisionContext* colCtx, Vec3f* pos) {
    if (pos->x < (colCtx->minBounds.x - BGCHECK_SUBDIV_OVERLAP) ||
        (colCtx->maxBounds.x + BGCHECK_SUBDIV_OVERLAP) < pos->x ||
        pos->y < (colCtx->minBounds.y - BGCHECK_SUBDIV_OVERLAP) ||
        (colCtx->maxBounds.y + BGCHECK_SUBDIV_OVERLAP) < pos->y ||
        pos->z < (colCtx->minBounds.z - BGCHECK_SUBDIV_OVERLAP) ||
        (colCtx->maxBounds.z + BGCHECK_SUBDIV_OVERLAP) < pos->z) {
        return false;
    }
    return true;
}

/**
 * Raycast Downward
 * If `actor` != null, bgcheck will be skipped for that actor
 * returns the yIntersect of the nearest poly found directly below `pos`, or BGCHECK_Y_MIN if no floor detected
 * returns the poly found in `outPoly`, and the bgId of the entity in `outBgId`
 */
f32 BgCheck_RaycastDownImpl(PlayState* play, CollisionContext* colCtx, u16 xpFlags, CollisionPoly** outPoly,
                            s32* outBgId, Vec3f* pos, Actor* actor, u32 downChkFlags) {
    SSLookup* lookupTbl = colCtx->lookupTbl;
    f32 yIntersect = BGCHECK_Y_MIN;

    INSTRUMENTATION(gBgCheckStats.numFloorTests++);

    *outBgId = BGCHECK_SCENE;
    *outPoly = NULL;

    // Do the static raycast

    // Descend through subdivisions until we fall out the bottom of the collision bounding
    // box or until we find a collision
    for (Vec3f checkPos = *pos; checkPos.y >= colCtx->minBounds.y; checkPos.y -= colCtx->subdivLength.y) {
        // Get the linked list for this subdivision
        SSLookup* lookup = BgCheck_GetStaticLookup(colCtx, lookupTbl, &checkPos);
        if (lookup == NULL) {
            continue;
        }

        // Do the downwards raycast inside this subidivision
        yIntersect = BgCheck_RaycastDownStatic(lookup, colCtx, xpFlags, outPoly, pos, downChkFlags, BGCHECK_Y_MIN);
        // If we found an intersection we're done, since any future subdivision is strictly lower than this one
        if (yIntersect > BGCHECK_Y_MIN) {
            break;
        }
    }

    // Do the dyna raycast

    DynaRaycastDown dynaRaycastDown;
    dynaRaycastDown.play = play;
    dynaRaycastDown.colCtx = colCtx;
    dynaRaycastDown.xpFlags = xpFlags;
    dynaRaycastDown.resultPoly = outPoly;
    dynaRaycastDown.yIntersect = yIntersect;
    dynaRaycastDown.pos = pos;
    dynaRaycastDown.bgId = outBgId;
    dynaRaycastDown.actor = actor;
    dynaRaycastDown.downChkFlags = downChkFlags;

    f32 yIntersectDyna = BgCheck_RaycastDownDyna(&dynaRaycastDown);
    if (yIntersect < yIntersectDyna) {
        yIntersect = yIntersectDyna;
    }

    if (yIntersect != BGCHECK_Y_MIN && SurfaceType_IsSoft(colCtx, *outPoly, *outBgId)) {
        yIntersect -= 1.0f;
    }
    return yIntersect;
}

/**
 * Public raycast downward, ground check (UNUSED)
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_CameraRaycastDown1(CollisionContext* colCtx, CollisionPoly** outGroundPoly, Vec3f* pos) {
    s32 bgId;

    return BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_CAMERA, outGroundPoly, &bgId, pos, NULL,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS |
                                       BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);
}

/**
 * Public raycast downward, ground check
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_EntityRaycastDown1(CollisionContext* colCtx, CollisionPoly** outGroundPoly, Vec3f* pos) {
    s32 bgId;

    return BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_ENTITY, outGroundPoly, &bgId, pos, NULL,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS |
                                       BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);
}

/**
 * Public raycast downward, ground check
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_EntityRaycastDown2(PlayState* play, CollisionContext* colCtx, CollisionPoly** outGroundPoly, Vec3f* pos) {
    s32 bgId;

    return BgCheck_RaycastDownImpl(play, colCtx, COLPOLY_IGNORE_ENTITY, outGroundPoly, &bgId, pos, NULL,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS |
                                       BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);
}

/**
 * Public raycast downward, ground check
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_EntityRaycastDown3(CollisionContext* colCtx, CollisionPoly** outGroundPoly, s32* bgId, Vec3f* pos) {
    return BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_ENTITY, outGroundPoly, bgId, pos, NULL,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS |
                                       BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);
}

/**
 * Public raycast downward, ground check
 * If `actor` != null, bgcheck will be skipped for that actor
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_EntityRaycastDown4(CollisionContext* colCtx, CollisionPoly** outGroundPoly, s32* bgId, Actor* actor,
                               Vec3f* pos) {
    return BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_ENTITY, outGroundPoly, bgId, pos, actor,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS |
                                       BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);
}

/**
 * Public raycast downward, ground check
 * If `actor` != null, bgcheck will be skipped for that actor
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_EntityRaycastDown5(PlayState* play, CollisionContext* colCtx, CollisionPoly** outGroundPoly, s32* bgId,
                               Actor* actor, Vec3f* pos) {
    return BgCheck_RaycastDownImpl(play, colCtx, COLPOLY_IGNORE_ENTITY, outGroundPoly, bgId, pos, actor,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS |
                                       BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);
}

/**
 * Public raycast downward, ground check
 * If `actor` != null, bgcheck will be skipped for that actor
 * `chkDist` is the distance beyond the poly's boundary where the check will still pass
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_EntityRaycastDown6(CollisionContext* colCtx, CollisionPoly** outGroundPoly, s32* bgId, Actor* actor,
                               Vec3f* pos, f32 chkDist) {
    return BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_ENTITY, outGroundPoly, bgId, pos, actor,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS |
                                       BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);
}

/**
 * Public raycast downward, floor and exhaustive wall check (UNUSED)
 * If `actor` != null, bgcheck will be skipped for that actor
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_EntityRaycastDown7(CollisionContext* colCtx, CollisionPoly** outPoly, s32* bgId, Actor* actor, Vec3f* pos) {
    return BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_ENTITY, outPoly, bgId, pos, actor,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS);
}

/**
 * Public raycast downward, ground check
 * `outGroundPoly` returns original value if no poly detected
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_AnyRaycastDown1(CollisionContext* colCtx, CollisionPoly* outGroundPoly, Vec3f* pos) {
    CollisionPoly* checkResultPoly;
    s32 bgId;
    f32 result = BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_NONE, &checkResultPoly, &bgId, pos, NULL,
                                         BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS |
                                             BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);
    if (checkResultPoly != NULL) {
        *outGroundPoly = *checkResultPoly;
    }
    return result;
}

/**
 * Public raycast downward, ground check (UNUSED)
 * `outGroundPoly` returns original value if no poly detected
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_AnyRaycastDown2(CollisionContext* colCtx, CollisionPoly* outGroundPoly, s32* bgId, Vec3f* pos) {
    CollisionPoly* checkResultPoly;
    f32 result = BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_NONE, &checkResultPoly, bgId, pos, NULL,
                                         BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS |
                                             BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);
    if (checkResultPoly != NULL) {
        *outGroundPoly = *checkResultPoly;
    }
    return result;
}

/**
 * Public raycast downward, floor and exhaustive wall check
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_CameraRaycastDown2(CollisionContext* colCtx, CollisionPoly** outPoly, s32* bgId, Vec3f* pos) {
    return BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_CAMERA, outPoly, bgId, pos, NULL,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS);
}

/**
 * Public raycast downward, exhaustive wall check (UNUSED)
 * If `actor` != null, bgcheck will be skipped for that actor
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_EntityRaycastDownWalls(CollisionContext* colCtx, CollisionPoly** outPoly, s32* bgId, Actor* actor,
                                   Vec3f* pos) {
    return BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_ENTITY, outPoly, bgId, pos, actor,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS);
}

/**
 * Public raycast downward, floor and exhaustive wall check (UNUSED)
 * returns yIntersect of the poly found, or BGCHECK_Y_MIN if no poly detected
 */
f32 BgCheck_EntityRaycastDown9(CollisionContext* colCtx, CollisionPoly** outPoly, s32* bgId, Vec3f* pos) {
    return BgCheck_RaycastDownImpl(NULL, colCtx, COLPOLY_IGNORE_ENTITY, outPoly, bgId, pos, NULL,
                                   BGCHECK_RAYCAST_DOWN_CHECK_WALLS | BGCHECK_RAYCAST_DOWN_CHECK_FLOORS);
}

/**
 * Tests if moving from `posPrev` to `posNext` will collide with a "wall"
 * `radius` is used to form a sphere for collision detection purposes
 * `checkHeight` is the positive height above posNext to perform certain checks
 * returns true if a collision is detected, else false
 * `outPoly` returns the closest poly detected, while `outBgId` returns the poly owner
 */
s32 BgCheck_CheckWallImpl(CollisionContext* colCtx, u16 xpFlags, Vec3f* posResult, Vec3f* posNext, Vec3f* posPrev,
                          f32 radius, CollisionPoly** outPoly, s32* outBgId, Actor* actor, f32 checkHeight,
                          u8 disableXZCorrection) {
    SSLookup* lookupTbl = colCtx->lookupTbl;
    f32 dx = posNext->x - posPrev->x;
    f32 dy = posNext->y - posPrev->y;
    f32 dz = posNext->z - posPrev->z;
    s32 result = false;
    f32 nx, nz;
    f32 nXZDistSQ;
    f32 proportionXZ;
    CollisionPoly* poly;
    Vec3f posIntersect;
    s32 bgId;

    *outBgId = BGCHECK_SCENE;
    *outPoly = NULL;
    *posResult = *posNext;

    // if there's movement on the xz plane, and disableXZCorrection is false
    if ((dx != 0.0f || dz != 0.0f) && !disableXZCorrection) {
        if (checkHeight + dy < 5.0f) {
            //! @bug checkHeight is not applied to posPrev/posNext
            result = BgCheck_CheckLineImpl(colCtx, xpFlags, COLPOLY_IGNORE_NONE, posPrev, posNext, &posIntersect, &poly,
                                           &bgId, actor, BGCHECK_CHECK_ALL & ~BGCHECK_CHECK_CEILING);
            if (result) {
                f32 ny = COLPOLY_GET_NORMAL(poly->normal.y);

                // if poly is floor, push result underneath the floor
                if (ny > 0.5f) {
                    posResult->x = posIntersect.x;
                    posResult->y = posIntersect.y - ((checkHeight > 1.0f) ? 1.0f : checkHeight);
                    posResult->z = posIntersect.z;
                } else { // poly is wall
                    nx = COLPOLY_GET_NORMAL(poly->normal.x);
                    nz = COLPOLY_GET_NORMAL(poly->normal.z);
                    posResult->x = radius * nx + posIntersect.x;
                    posResult->y = radius * ny + posIntersect.y;
                    posResult->z = radius * nz + posIntersect.z;
                }
                *outPoly = poly;
                *outBgId = bgId;
            }
        } else {
            // if the radius is less than the distance travelled on the xz plane, also test for floor collisions
            s32 bccFlags = SQ(radius) < (SQ(dx) + SQ(dz))
                               ? (BGCHECK_CHECK_ALL & ~BGCHECK_CHECK_CEILING)
                               : (BGCHECK_CHECK_ALL & ~(BGCHECK_CHECK_FLOOR | BGCHECK_CHECK_CEILING));

            // perform a straight line test to see if a line at posNext.y + checkHeight from posPrev.xz to posNext.xz
            // passes through any wall and possibly floor polys
            Vec3f checkLineNext = *posNext;
            checkLineNext.y += checkHeight;
            Vec3f checkLinePrev = *posPrev;
            checkLinePrev.y = checkLineNext.y;
            result = BgCheck_CheckLineImpl(colCtx, xpFlags, COLPOLY_IGNORE_NONE, &checkLinePrev, &checkLineNext,
                                           &posIntersect, &poly, &bgId, actor, bccFlags);

            if (result) {
                nx = COLPOLY_GET_NORMAL(poly->normal.x);
                nz = COLPOLY_GET_NORMAL(poly->normal.z);
                nXZDistSQ = SQ(nx) + SQ(nz);

                // if poly is not a "flat" floor or "flat" ceiling
                if (!IS_ZERO(nXZDistSQ)) {
                    // normalize nx,nz and multiply each by the radius to go back to the other side of the wall
                    proportionXZ = radius / sqrtf(nXZDistSQ);
                    posResult->x = proportionXZ * nx + posIntersect.x;
                    posResult->z = proportionXZ * nz + posIntersect.z;
                    *outPoly = poly;
                    *outBgId = bgId;
                }
            }
        }
    }

    Vec3f sphCenter = *posResult;
    sphCenter.y += checkHeight;

    s32 dynaPolyCollision = false;

    // test if sphere (sphCenter, radius) collides with a dynamic wall, displacing the x/z coordinates
    if (BgCheck_SphVsDynaWall(colCtx, xpFlags, &posResult->x, &posResult->z, &sphCenter, radius, outPoly, outBgId,
                              actor)) {
        result = true;
        dynaPolyCollision = true;
        sphCenter = *posResult;
        sphCenter.y += checkHeight;
    }

    // test if sphere (sphCenter, radius) collides with a static wall, displacing the x/z coordinates
    // possible bug? if the sphere's radius is smaller than the distance to a subdivision boundary, some static
    // polys will be missed
    if (BgCheck_PosInStaticBoundingBox(colCtx, posNext)) {
#if 1
        SSLookup* lookup = BgCheck_GetNearestStaticLookup(colCtx, lookupTbl, posResult);
        if (BgCheck_WallCheck(colCtx->colHeader->surfaceTypeList, &sphCenter, &posResult->x, &posResult->z,
                              colCtx->colHeader->vtxList, colCtx->colHeader->polyList, colCtx->polyNodes.tbl,
                              &lookup->wall, radius, xpFlags, &bgId, BGCHECK_SCENE, outPoly)) {
            *outBgId = BGCHECK_SCENE;
            result = true;
        }
#else
        // Bugfix for the above bug comment, this is clearly slower however it allows removing
        // the BGCHECK_SUBDIV_OVERLAP so could be worth it overall. TODO: Needs assessment.

        CollisionHeader* colHeader = colCtx->colHeader;
        Vec3f min = {
            sphCenter.x - radius,
            sphCenter.y - radius,
            sphCenter.z - radius,
        };
        Vec3f max = {
            sphCenter.x - radius,
            sphCenter.y - radius,
            sphCenter.z - radius,
        };
        Vec3i sectorMin;
        Vec3i sectorMax;

        BgCheck_GetStaticLookupIndicesFromPos(colCtx, &min, &sectorMin);
        BgCheck_GetStaticLookupIndicesFromPos(colCtx, &max, &sectorMax);

        s32 subdivX = colCtx->subdivAmount.x;
        s32 subdivY = colCtx->subdivAmount.y;
        s32 subdivXY = subdivX * colCtx->subdivAmount.y;

        SSLookup* lookupMin = &lookupTbl[sectorMin.x + subdivX * (sectorMin.y + subdivY * sectorMin.z)];
        SSLookup* lookupMax = &lookupTbl[sectorMax.x + subdivX * (sectorMax.y + subdivY * sectorMax.z)];

        if (lookupMin == lookupMax) {
            // Just one subdiv to check, should be the most common case
            if (BgCheck_WallCheck(colHeader->surfaceTypeList, &sphCenter, &posResult->x, &posResult->z,
                                  colHeader->vtxList, colHeader->polyList, colCtx->polyNodes.tbl, &lookupMin->wall,
                                  radius, xpFlags, &bgId, BGCHECK_SCENE, outPoly)) {
                *outBgId = BGCHECK_SCENE;
                result = true;
            }
        } else {
            // Need to loop through several subdivs
            for (s32 cz = sectorMin.z; cz <= sectorMax.z; cz++) {
                SSLookup* zlookup = &lookupTbl[cz * subdivXY];

                for (s32 cy = sectorMin.y; cy <= sectorMax.y; cy++) {
                    SSLookup* ylookup = &zlookup[cy * subdivX];

                    for (s32 cx = sectorMin.x; cx <= sectorMax.x; cx++) {
                        SSLookup* xlookup = &ylookup[cx];

                        if (BgCheck_WallCheck(colHeader->surfaceTypeList, &sphCenter, &posResult->x, &posResult->z,
                                              colHeader->vtxList, colHeader->polyList, colCtx->polyNodes.tbl,
                                              &xlookup->wall, radius, xpFlags, &bgId, BGCHECK_SCENE, outPoly)) {
                            // Update sphere center for next check
                            sphCenter = *posResult;
                            *outBgId = BGCHECK_SCENE;
                            result = true;
                        }
                    }
                }
            }
        }
#endif
    }

    dynaPolyCollision |= *outBgId != BGCHECK_SCENE;

    // if a collision with a dyna poly was detected
    if (dynaPolyCollision &&
        BgCheck_CheckLineImpl(colCtx, xpFlags, COLPOLY_IGNORE_NONE, posPrev, posResult, &posIntersect, &poly, &bgId,
                              actor, BGCHECK_CHECK_ONE_FACE | BGCHECK_CHECK_WALL)) {
        nx = COLPOLY_GET_NORMAL(poly->normal.x);
        nz = COLPOLY_GET_NORMAL(poly->normal.z);
        nXZDistSQ = SQ(nx) + SQ(nz);

        // if poly is not a "flat" floor or "flat" ceiling
        if (!IS_ZERO(nXZDistSQ)) {
            // normalize nx,nz and multiply each by the radius to go back to the other side of the wall
            proportionXZ = radius / sqrtf(nXZDistSQ);
            posResult->x = proportionXZ * nx + posIntersect.x;
            posResult->z = proportionXZ * nz + posIntersect.z;
            *outPoly = poly;
            *outBgId = bgId;
            result = true;
        }
    }
    return result;
}

/**
 * Public. Tests if moving from `posPrev` to `posNext` will collide with a "wall"
 * `radius` is used to form a sphere for collision detection purposes
 * `checkHeight` is the positive height above posNext to perform certain checks
 * returns true if a collision is detected, else false
 * `outPoly` returns the closest poly detected
 */
s32 BgCheck_EntitySphVsWall1(CollisionContext* colCtx, Vec3f* posResult, Vec3f* posNext, Vec3f* posPrev, f32 radius,
                             CollisionPoly** outPoly, f32 checkHeight) {
    s32 bgId;

    return BgCheck_CheckWallImpl(colCtx, COLPOLY_IGNORE_ENTITY, posResult, posNext, posPrev, radius, outPoly, &bgId,
                                 NULL, checkHeight, false);
}

/**
 * Public. Tests if moving from `posPrev` to `posNext` will collide with a "wall"
 * `radius` is used to form a sphere for collision detection purposes
 * `checkHeight` is the positive height above posNext to perform certain checks
 * returns true if a collision is detected, else false
 * `outPoly` returns the closest poly detected, while `outBgId` returns the poly owner
 */
s32 BgCheck_EntitySphVsWall2(CollisionContext* colCtx, Vec3f* posResult, Vec3f* posNext, Vec3f* posPrev, f32 radius,
                             CollisionPoly** outPoly, s32* outBgId, f32 checkHeight) {
    return BgCheck_CheckWallImpl(colCtx, COLPOLY_IGNORE_ENTITY, posResult, posNext, posPrev, radius, outPoly, outBgId,
                                 NULL, checkHeight, false);
}

/**
 * Public. Tests if moving from `posPrev` to `posNext` will collide with a "wall"
 * `radius` is used to form a sphere for collision detection purposes
 * `checkHeight` is the positive height above posNext to perform certain checks
 * `actor` is the actor performing the check, allowing it to be skipped
 * returns true if a collision is detected, else false
 * `outPoly` returns the closest poly detected, while `outBgId` returns the poly owner
 */
s32 BgCheck_EntitySphVsWall3(CollisionContext* colCtx, Vec3f* posResult, Vec3f* posNext, Vec3f* posPrev, f32 radius,
                             CollisionPoly** outPoly, s32* outBgId, Actor* actor, f32 checkHeight) {
    return BgCheck_CheckWallImpl(colCtx, COLPOLY_IGNORE_ENTITY, posResult, posNext, posPrev, radius, outPoly, outBgId,
                                 actor, checkHeight, false);
}

/***
 * Public. Tests if moving from `posPrev` to `posNext` will collide with a "wall"
 * Skips a check that occurs only when moving on the xz plane
 * `radius` is used to form a sphere for collision detection purposes
 * `checkHeight` is the positive height above posNext to perform certain checks
 * `actor` is the actor performing the check, allowing it to be skipped
 * returns true if a collision is detected, else false
 * `outPoly` returns the closest poly detected, while `outBgId` returns the poly owner
 */
s32 BgCheck_EntitySphVsWall4(CollisionContext* colCtx, Vec3f* posResult, Vec3f* posNext, Vec3f* posPrev, f32 radius,
                             CollisionPoly** outPoly, s32* outBgId, Actor* actor, f32 checkHeight) {
    return BgCheck_CheckWallImpl(colCtx, COLPOLY_IGNORE_ENTITY, posResult, posNext, posPrev, radius, outPoly, outBgId,
                                 actor, checkHeight, true);
}

/***
 * Tests for collision with a ceiling poly
 * `checkHeight` should be a positive value
 * returns true if a collision occurs, else false
 * `outPoly` returns the poly collided with, while `outBgId` returns the owner of the poly
 * `outY` returns the y coordinate of pos needed to not collide with `outPoly`
 */
s32 BgCheck_CheckCeilingImpl(CollisionContext* colCtx, u16 xpFlags, f32* outY, Vec3f* pos, f32 checkHeight,
                             CollisionPoly** outPoly, s32* outBgId, Actor* actor) {
    SSLookup* lookupTbl = colCtx->lookupTbl;

    INSTRUMENTATION(gBgCheckStats.numCeilingTests++);

    *outBgId = BGCHECK_SCENE;
    *outY = pos->y;

    if (!BgCheck_PosInStaticBoundingBox(colCtx, pos)) {
        return false;
    }

    // Do static check

    SSLookup* lookup = BgCheck_GetNearestStaticLookup(colCtx, lookupTbl, pos);
    s32 result = BgCheck_CheckStaticCeiling(lookup, xpFlags, colCtx, outY, pos, checkHeight, outPoly);

    // Do dyna check

    Vec3f posTemp = (Vec3f){ pos->x, *outY, pos->z };
    f32 tempY = *outY;
    if (BgCheck_CheckDynaCeiling(colCtx, xpFlags, &tempY, &posTemp, checkHeight, outPoly, outBgId, actor)) {
        *outY = tempY;
        result = true;
    }
    return result;
}

/**
 * Tests for collision with any ceiling poly
 * `checkHeight` must be a positive value
 * returns true if a collision occurs, else false
 * `outY` returns the displaced y coordinate needed to not collide with the poly
 */
s32 BgCheck_AnyCheckCeiling(CollisionContext* colCtx, f32* outY, Vec3f* pos, f32 checkHeight) {
    CollisionPoly* poly;
    s32 bgId;

    return BgCheck_CheckCeilingImpl(colCtx, COLPOLY_IGNORE_NONE, outY, pos, checkHeight, &poly, &bgId, NULL);
}

/**
 * Tests for collision with any entity solid ceiling poly
 * `checkHeight` must be a positive value
 * returns true if a collision occurs, else false
 * `outY` returns the displaced y coordinate needed to not collide with the poly
 */
s32 BgCheck_EntityCheckCeiling(CollisionContext* colCtx, f32* outY, Vec3f* pos, f32 checkHeight,
                               CollisionPoly** outPoly, s32* outBgId, Actor* actor) {
    return BgCheck_CheckCeilingImpl(colCtx, COLPOLY_IGNORE_ENTITY, outY, pos, checkHeight, outPoly, outBgId, actor);
}

/**
 * Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 * `posB`? `posResult` returns the point of intersection
 * `outPoly` returns the pointer to the intersected poly, while `outBgId` returns the entity the poly belongs to
 */
s32 BgCheck_CheckLineImpl(CollisionContext* colCtx, u16 xpFlags1, u16 xpFlags2, Vec3f* posA, Vec3f* posB,
                          Vec3f* posResult, CollisionPoly** outPoly, s32* outBgId, Actor* actor, u32 bccFlags) {
    SSLookup* lookupTbl = colCtx->lookupTbl;
    SSLookup* iLookup;
    s32 subdivMin[3];
    s32 subdivMax[3];
    s32 i;
    s32 result;
    f32 distSq;
    Vec3f posBTemp = *posB;
    Vec3f sectorMin;
    Vec3f sectorMax;

    INSTRUMENTATION(gBgCheckStats.numLineTests++);

    *outBgId = BGCHECK_SCENE;

#if USE_POLY_CHK_TBL
    // Resets the "poly check table", tracks which static polys have already been checked in this line test.
    // NOTE: This is meant to be an optimization to avoid re-checking polys that were already checked in a different
    // subdivision, but it ends up slower due to the huge memory bandwidth involved in clearing and update the table
    // every time a new line test begins.
    u32 polyCheckSize = BITSET_SIZE_BYTES(colCtx->colHeader->numPolygons, colCtx->polyCheckTbl[0]);
    memset(colCtx->polyCheckTbl, 0, polyCheckSize);
#endif

    // The line test is implemented as an exhaustive search over a cuboid of subdivisions (horrifying)
    BgCheck_GetStaticLookupIndicesFromPos(colCtx, posA, (Vec3i*)&subdivMin);
    BgCheck_GetStaticLookupIndicesFromPos(colCtx, &posBTemp, (Vec3i*)&subdivMax);
    *posResult = *posB;
    result = false;
    distSq = MAXFLOAT;
    *outPoly = NULL;

    if (subdivMin[0] != subdivMax[0] || subdivMin[1] != subdivMax[1] || subdivMin[2] != subdivMax[2]) {
        // If the line crosses multiple subdivisions, do an exhaustive search over all subdivisions..
        // TODO this is horrific, but I tried to do a marching line algorithm and sometimes arrows would fly
        // straight through solid walls probably due to accuracy issues at subdivision boundaries. The hope is
        // that a line check usually won't span more than at most 2 subdivisions so there isn't actually much
        // redundant computation happening, except for arrows but arrows don't live very long in the first place
        s32 k;
        s32 subdivXY;
        s32 j;

        for (i = 0; i < 3; i++) {
            if (subdivMax[i] < subdivMin[i]) {
                j = subdivMax[i];
                subdivMax[i] = subdivMin[i];
                subdivMin[i] = j;
            }
        }
        subdivXY = colCtx->subdivAmount.x * colCtx->subdivAmount.y;
        iLookup = lookupTbl + subdivMin[2] * subdivXY;
        sectorMin.z = subdivMin[2] * colCtx->subdivLength.z + colCtx->minBounds.z;
        sectorMax.z = colCtx->subdivLength.z + sectorMin.z;

        for (i = subdivMin[2]; i <= subdivMax[2]; i++) {
            SSLookup* jLookup = iLookup + subdivMin[1] * colCtx->subdivAmount.x;

            sectorMin.y = subdivMin[1] * colCtx->subdivLength.y + colCtx->minBounds.y;
            sectorMax.y = colCtx->subdivLength.y + sectorMin.y;

            for (j = subdivMin[1]; j <= subdivMax[1]; j++) {
                SSLookup* lookup = jLookup + subdivMin[0];

                sectorMin.x = subdivMin[0] * colCtx->subdivLength.x + colCtx->minBounds.x;
                sectorMax.x = colCtx->subdivLength.x + sectorMin.x;

                for (k = subdivMin[0]; k <= subdivMax[0]; k++) {
                    if (Math3D_LineVsCube(&sectorMin, &sectorMax, posA, &posBTemp) &&
                        BgCheck_CheckLineInSubdivision(lookup, colCtx, xpFlags1, xpFlags2, posA, &posBTemp, posResult,
                                                       outPoly, &distSq, bccFlags)) {
                        result = true;
                    }

                    lookup++;
                    sectorMin.x += colCtx->subdivLength.x;
                    sectorMax.x += colCtx->subdivLength.x;
                }

                jLookup += colCtx->subdivAmount.x;
                sectorMin.y += colCtx->subdivLength.y;
                sectorMax.y += colCtx->subdivLength.y;
            }

            iLookup += subdivXY;
            sectorMin.z += colCtx->subdivLength.z;
            sectorMax.z += colCtx->subdivLength.z;
        }
    } else if (!BgCheck_PosInStaticBoundingBox(colCtx, posA)) {
        return false;
    } else {
        result =
            BgCheck_CheckLineInSubdivision(BgCheck_GetNearestStaticLookup(colCtx, lookupTbl, posA), colCtx, xpFlags1,
                                           xpFlags2, posA, &posBTemp, posResult, outPoly, &distSq, bccFlags);
        if (result) {
            Vec3f v = VSUB(posResult, posA);
            distSq = VMAGSQ(&v);
        }
    }

    // Do dyna test

    if ((bccFlags & BGCHECK_CHECK_DYNA) && BgCheck_CheckLineAgainstDyna(colCtx, xpFlags1, posA, &posBTemp, posResult,
                                                                        outPoly, &distSq, outBgId, actor, bccFlags)) {
        result = true;
    }
    return result;
}

/**
 * Get bccFlags
 */
static u32 BgCheck_GetBccFlags(s32 chkWall, s32 chkFloor, s32 chkCeil, s32 chkOneFace, s32 chkDyna) {
    u32 result = 0;
    result |= chkWall * BGCHECK_CHECK_WALL;
    result |= chkFloor * BGCHECK_CHECK_FLOOR;
    result |= chkCeil * BGCHECK_CHECK_CEILING;
    result |= chkOneFace * BGCHECK_CHECK_ONE_FACE;
    result |= chkDyna * BGCHECK_CHECK_DYNA;
    return result;
}

/**
 * Public. Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 */
s32 BgCheck_CameraLineTest1(CollisionContext* colCtx, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                            CollisionPoly** outPoly, s32 chkWall, s32 chkFloor, s32 chkCeil, s32 chkOneFace,
                            s32* bgId) {
    return BgCheck_CheckLineImpl(colCtx, COLPOLY_IGNORE_CAMERA, COLPOLY_IGNORE_NONE, posA, posB, posResult, outPoly,
                                 bgId, NULL, BgCheck_GetBccFlags(chkWall, chkFloor, chkCeil, chkOneFace, true));
}

/**
 * Public. Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 */
s32 BgCheck_CameraLineTest2(CollisionContext* colCtx, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                            CollisionPoly** outPoly, s32 chkWall, s32 chkFloor, s32 chkCeil, s32 chkOneFace,
                            s32* bgId) {
    return BgCheck_CheckLineImpl(colCtx, COLPOLY_IGNORE_NONE, COLPOLY_IGNORE_CAMERA, posA, posB, posResult, outPoly,
                                 bgId, NULL, BgCheck_GetBccFlags(chkWall, chkFloor, chkCeil, chkOneFace, true));
}

/**
 * Public. Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 */
s32 BgCheck_EntityLineTest1(CollisionContext* colCtx, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                            CollisionPoly** outPoly, s32 chkWall, s32 chkFloor, s32 chkCeil, s32 chkOneFace,
                            s32* bgId) {
    return BgCheck_CheckLineImpl(colCtx, COLPOLY_IGNORE_ENTITY, COLPOLY_IGNORE_NONE, posA, posB, posResult, outPoly,
                                 bgId, NULL, BgCheck_GetBccFlags(chkWall, chkFloor, chkCeil, chkOneFace, true));
}

/**
 * Public. Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 */
s32 BgCheck_EntityLineTest2(CollisionContext* colCtx, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                            CollisionPoly** outPoly, s32 chkWall, s32 chkFloor, s32 chkCeil, s32 chkOneFace, s32* bgId,
                            Actor* actor) {
    return BgCheck_CheckLineImpl(colCtx, COLPOLY_IGNORE_ENTITY, COLPOLY_IGNORE_NONE, posA, posB, posResult, outPoly,
                                 bgId, actor, BgCheck_GetBccFlags(chkWall, chkFloor, chkCeil, chkOneFace, true));
}

/**
 * Public. Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 */
s32 BgCheck_EntityLineTest3(CollisionContext* colCtx, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                            CollisionPoly** outPoly, s32 chkWall, s32 chkFloor, s32 chkCeil, s32 chkOneFace, s32* bgId,
                            Actor* actor, f32 chkDist) {
    return BgCheck_CheckLineImpl(colCtx, COLPOLY_IGNORE_ENTITY, COLPOLY_IGNORE_NONE, posA, posB, posResult, outPoly,
                                 bgId, actor, BgCheck_GetBccFlags(chkWall, chkFloor, chkCeil, chkOneFace, true));
}

/**
 * Public. Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 */
s32 BgCheck_ProjectileLineTest(CollisionContext* colCtx, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                               CollisionPoly** outPoly, s32 chkWall, s32 chkFloor, s32 chkCeil, s32 chkOneFace,
                               s32* bgId) {
    return BgCheck_CheckLineImpl(colCtx, COLPOLY_IGNORE_PROJECTILES, COLPOLY_IGNORE_NONE, posA, posB, posResult,
                                 outPoly, bgId, NULL,
                                 BgCheck_GetBccFlags(chkWall, chkFloor, chkCeil, chkOneFace, true));
}

/**
 * Public. Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 */
s32 BgCheck_AnyLineTest1(CollisionContext* colCtx, Vec3f* posA, Vec3f* posB, Vec3f* posResult, CollisionPoly** outPoly,
                         s32 chkOneFace) {
    return BgCheck_AnyLineTest2(colCtx, posA, posB, posResult, outPoly, true, true, true, chkOneFace);
}

/**
 * Public. Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 */
s32 BgCheck_AnyLineTest2(CollisionContext* colCtx, Vec3f* posA, Vec3f* posB, Vec3f* posResult, CollisionPoly** outPoly,
                         s32 chkWall, s32 chkFloor, s32 chkCeil, s32 chkOneFace) {
    s32 bgId;

    return BgCheck_CheckLineImpl(colCtx, COLPOLY_IGNORE_NONE, COLPOLY_IGNORE_NONE, posA, posB, posResult, outPoly,
                                 &bgId, NULL, BgCheck_GetBccFlags(chkWall, chkFloor, chkCeil, chkOneFace, true));
}

/**
 * Public. Tests if a line from `posA` to `posB` intersects with a poly
 * returns true if it does, else false
 */
s32 BgCheck_AnyLineTest3(CollisionContext* colCtx, Vec3f* posA, Vec3f* posB, Vec3f* posResult, CollisionPoly** outPoly,
                         s32 chkWall, s32 chkFloor, s32 chkCeil, s32 chkOneFace, s32* bgId) {
    return BgCheck_CheckLineImpl(colCtx, COLPOLY_IGNORE_NONE, COLPOLY_IGNORE_NONE, posA, posB, posResult, outPoly, bgId,
                                 NULL, BgCheck_GetBccFlags(chkWall, chkFloor, chkCeil, chkOneFace, true));
}

/**
 * Get first poly intersecting sphere `center` `radius`
 * ignores `actor` dyna poly
 * returns true if any poly intersects the sphere, else false
 * `outPoly` returns the pointer of the first poly found that intersects
 * `outBgId` returns the bgId of the entity that owns `outPoly`
 */
s32 BgCheck_SphVsFirstPolyImpl(CollisionContext* colCtx, u16 xpFlags, CollisionPoly** outPoly, s32* outBgId,
                               Vec3f* center, f32 radius, Actor* actor, u16 bciFlags) {
    *outBgId = BGCHECK_SCENE;

    // Get the linked list for this subdivision
    SSLookup* lookup = BgCheck_GetStaticLookup(colCtx, colCtx->lookupTbl, center);
    if (lookup == NULL) {
        return false;
    }

    // Check static then dyna
    if (BgCheck_SphVsFirstStaticPoly(lookup, xpFlags, colCtx, center, radius, outPoly, bciFlags) ||
        BgCheck_SphVsFirstDynaPoly(colCtx, xpFlags, outPoly, outBgId, center, radius, actor, bciFlags)) {
        return true;
    }
    return false;
}

/**
 * Public get first poly intersecting sphere `center` `radius`
 */
s32 BgCheck_SphVsFirstPoly(CollisionContext* colCtx, Vec3f* center, f32 radius) {
    CollisionPoly* poly;
    s32 bgId;

    return BgCheck_SphVsFirstPolyImpl(colCtx, COLPOLY_IGNORE_NONE, &poly, &bgId, center, radius, NULL,
                                      BGCHECK_IGNORE_NONE);
}

/**
 * Public get first wall poly intersecting sphere `center` `radius`
 */
s32 BgCheck_SphVsFirstWall(CollisionContext* colCtx, Vec3f* center, f32 radius) {
    CollisionPoly* poly;
    s32 bgId;

    return BgCheck_SphVsFirstPolyImpl(colCtx, COLPOLY_IGNORE_NONE, &poly, &bgId, center, radius, NULL,
                                      BGCHECK_IGNORE_FLOOR | BGCHECK_IGNORE_CEILING);
}

/**
 * ScaleRotPos equality test
 */
s32 ScaleRotPos_Equals(ScaleRotPos* a, ScaleRotPos* b) {
    return (a->scale.x == b->scale.x && a->scale.y == b->scale.y && a->scale.z == b->scale.z && a->rot.x == b->rot.x &&
            a->rot.y == b->rot.y && a->rot.z == b->rot.z && a->pos.x == b->pos.x && a->pos.y == b->pos.y &&
            a->pos.z == b->pos.z);
}

/**
 * Initialize BgActor
 */
void BgActor_Initialize(PlayState* play, BgActor* bgActor) {
    bzero(bgActor, sizeof(BgActor));
    bgActor->dynaLookup.ceiling.head = SS_NULL;
    bgActor->dynaLookup.wall.head = SS_NULL;
    bgActor->dynaLookup.floor.head = SS_NULL;
}

/**
 * Is BgActor Id
 */
s32 DynaPoly_IsBgIdBgActor(s32 bgId) {
    return !(bgId < 0 || bgId >= BG_ACTOR_MAX);
}

/**
 * Set BgActor
 * original name: DynaPolyInfo_setActor
 */
s32 DynaPoly_SetBgActor(PlayState* play, DynaCollisionContext* dyna, Actor* actor, CollisionHeader* colHeader) {
    s32 bgId;
    s32 foundSlot = false;

    for (bgId = 0; bgId < BG_ACTOR_MAX; bgId++) {
        if (dyna->bgActorFlags[bgId] & BGACTOR_IN_USE) {
            continue;
        }
        dyna->bgActorFlags[bgId] |= BGACTOR_IN_USE;
        foundSlot = true;
        break;
    }

    if (!foundSlot) {
        return BG_ACTOR_MAX;
    }

    BgActor* bgActor = &dyna->bgActors[bgId];
    bgActor->actor = actor;
    bgActor->colHeader = colHeader;
    bgActor->prevTransform.scale = actor->scale;
    bgActor->prevTransform.rot = actor->shape.rot;
    bgActor->prevTransform.rot.x--; // make them different to trigger a recalculation
    bgActor->prevTransform.pos = actor->world.pos;
    bgActor->curTransform.scale = actor->scale;
    bgActor->curTransform.rot = actor->shape.rot;
    bgActor->curTransform.pos = actor->world.pos;

    u32 allocSize =
        colHeader->numPolygons * (sizeof(CollisionPoly) + sizeof(SSNode)) + colHeader->numVertices * sizeof(Vec3s);
    // PRINTF(VT_FGCOL(RED) "BgActor actor=%08X bgId=%d allocated 0x%X\n" VT_RST, actor, bgId, allocSize);
    bgActor->polyList = ZELDA_ARENA_MALLOC(allocSize, __FILE__, __LINE__);
    assert(bgActor->polyList != NULL);
    bgActor->polyNodes.tbl = (SSNode*)(bgActor->polyList + colHeader->numPolygons);
    bgActor->polyNodes.max = colHeader->numPolygons;
    bgActor->polyNodes.count = 0;
    bgActor->vtxList = (Vec3s*)(bgActor->polyNodes.tbl + colHeader->numPolygons);

    dyna->bgActorFlags[bgId] |= BGACTOR_INVALIDATE_LOOKUP;
    dyna->bgActorFlags[bgId] &= ~BGACTOR_MARKED_FOR_DELETION;
    return bgId;
}

/**
 * Gets the actor assigned to `bgId`
 * possible orginal name: DynaPolyInfo_getActor
 */
DynaPolyActor* DynaPoly_GetActor(CollisionContext* colCtx, s32 bgId) {
    if (!DynaPoly_IsBgIdBgActor(bgId) || !(colCtx->dyna.bgActorFlags[bgId] & BGACTOR_IN_USE) ||
        (colCtx->dyna.bgActorFlags[bgId] & BGACTOR_MARKED_FOR_DELETION)) {
        return NULL;
    }
    return (DynaPolyActor*)colCtx->dyna.bgActors[bgId].actor;
}

void DynaPoly_DisableCollision(PlayState* play, DynaCollisionContext* dyna, s32 bgId) {
    if (DynaPoly_IsBgIdBgActor(bgId) && !(dyna->bgActorFlags[bgId] & BGACTOR_COLLISION_DISABLED)) {
        dyna->bgActorFlags[bgId] |= BGACTOR_COLLISION_DISABLED;
        dyna->bgActorFlags[bgId] |= BGACTOR_INVALIDATE_LOOKUP;
    }
}

void DynaPoly_EnableCollision(PlayState* play, DynaCollisionContext* dyna, s32 bgId) {
    if (DynaPoly_IsBgIdBgActor(bgId) && (dyna->bgActorFlags[bgId] & BGACTOR_COLLISION_DISABLED)) {
        dyna->bgActorFlags[bgId] &= ~BGACTOR_COLLISION_DISABLED;
        dyna->bgActorFlags[bgId] |= BGACTOR_INVALIDATE_LOOKUP;
    }
}

void DynaPoly_DisableCeilingCollision(PlayState* play, DynaCollisionContext* dyna, s32 bgId) {
    if (DynaPoly_IsBgIdBgActor(bgId) && !(dyna->bgActorFlags[bgId] & BGACTOR_CEILING_COLLISION_DISABLED)) {
        dyna->bgActorFlags[bgId] |= BGACTOR_CEILING_COLLISION_DISABLED;
        dyna->bgActorFlags[bgId] |= BGACTOR_INVALIDATE_LOOKUP;
    }
}

void DynaPoly_EnableCeilingCollision(PlayState* play, DynaCollisionContext* dyna, s32 bgId) {
    if (DynaPoly_IsBgIdBgActor(bgId) && (dyna->bgActorFlags[bgId] & BGACTOR_CEILING_COLLISION_DISABLED)) {
        dyna->bgActorFlags[bgId] &= ~BGACTOR_CEILING_COLLISION_DISABLED;
        dyna->bgActorFlags[bgId] |= BGACTOR_INVALIDATE_LOOKUP;
    }
}

/**
 * original name: DynaPolyInfo_delReserve
 */
void DynaPoly_DeleteBgActor(PlayState* play, DynaCollisionContext* dyna, s32 bgId) {
    DynaPolyActor* actor;

    if (!DynaPoly_IsBgIdBgActor(bgId)) {
        if (bgId == -1) {
            // double free
        } else {
            // was never allocated
        }
        return;
    }

    BgActor* bgActor = &dyna->bgActors[bgId];

    actor = DynaPoly_GetActor(&play->colCtx, bgId);
    if (actor != NULL) {
        actor->bgId = BGACTOR_NEG_ONE;
        bgActor->actor = NULL;
        dyna->bgActorFlags[bgId] |= BGACTOR_MARKED_FOR_DELETION;
    }
}

void DynaPoly_InvalidateLookup(PlayState* play, DynaCollisionContext* dyna, s32 bgId) {
    if (!DynaPoly_IsBgIdBgActor(bgId) || !(dyna->bgActorFlags[bgId] & BGACTOR_IN_USE)) {
        return;
    }
    dyna->bgActorFlags[bgId] |= BGACTOR_INVALIDATE_LOOKUP;
}

#if 0
// Build a 3x3 rotation matrix that maps [0,1,0] to the normal direction
// Not used now but may be used in future for transforming normals when a dynapoly changes rotation
void DynaPoly_BuildRotationMatrixForNormal(f32 dst[3][3], Vec3s* restrict rot) {
    f32 (* restrict mtx)[3]  = dst;
    cf32 eiY = Math_SinCosS(rot->y);
    f32 sinY = SIN(eiY);
    f32 cosY = COS(eiY);

    mtx[0][0] = cosY; // xx
    dst[2][0] = -sinY; // zx

    if (rot->x != 0) {
        cf32 eiX = Math_SinCosS(rot->x);
        f32 sinX = SIN(eiX);
        f32 cosX = COS(eiX);

        dst[0][1] = sinY * sinX;
        dst[0][2] = sinY * cosX;
        dst[1][1] = cosX;
        dst[1][2] = -sinX;
        dst[2][1] = cosY * sinX;
        dst[2][2] = cosY * cosX;
    } else {
        dst[0][1] = 0.0f;
        dst[0][2] = sinY;
        dst[1][1] = CF(1.0f);
        dst[1][2] = 0.0f;
        dst[2][1] = 0.0f;
        dst[2][2] = cosY;
    }

    if (rot->z != 0) {
        cf32 eiZ = Math_SinCosS(rot->z);
        f32 sinZ = SIN(eiZ);
        f32 cosZ = COS(eiZ);

        f32 xx = dst[0][0];
        f32 xy = dst[0][1];
        dst[0][0] = xx * cosZ + xy * sinZ;
        dst[0][1] = xy * cosZ - xx * sinZ;

        f32 zx = dst[2][0];
        f32 zy = dst[2][1];
        dst[2][0] = zx * cosZ + zy * sinZ;
        dst[2][1] = zy * cosZ - zx * sinZ;

        f32 yy = dst[1][1];
        dst[1][0] = yy * sinZ;
        dst[1][1] = yy * cosZ;
    } else {
        dst[1][0] = 0.0f;
    }
}
#endif

/**
 * original name: DynaPolyInfo_expandSRT
 */
void DynaPoly_AddBgActorToLookup(PlayState* play, DynaCollisionContext* dyna, s32 bgId) {
    BgActor* bgActor = &dyna->bgActors[bgId];
    CollisionHeader* colHeader = bgActor->colHeader;
    Actor* actor = bgActor->actor;
    s32 i;

    Vec3f pos = actor->world.pos;
    pos.y += actor->shape.yOffset * actor->scale.y;

    bgActor->curTransform.scale = actor->scale;
    bgActor->curTransform.rot = actor->shape.rot;
    bgActor->curTransform.pos = pos;

    if (dyna->bgActorFlags[bgId] & BGACTOR_COLLISION_DISABLED) {
        bgActor->dynaLookup.ceiling.head = SS_NULL;
        bgActor->dynaLookup.wall.head = SS_NULL;
        bgActor->dynaLookup.floor.head = SS_NULL;
        return;
    }

    if (!(dyna->bgActorFlags[bgId] & BGACTOR_INVALIDATE_LOOKUP) &&
        ScaleRotPos_Equals(&bgActor->prevTransform, &bgActor->curTransform)) {
        // Not invalid and the transform is unchanged, no need to rebuild anything.
        return;
    }
    // TODO if only translation changed, updating the dynapoly is significantly simplified, worth checking for?
    // In particular, bounding sphere radius and normals do not need recalculating, but see below for why this might
    // not be possible..

    // TODO in theory we could probably defer rebuilding the lookup until something decides it needs to test against
    // the dynapoly in question.. not sure if it's worth it though since it may be less cache friendly

    // PRINTF(VT_FGCOL(RED) "Dynapoly rebuilding lookup for bgId = %d (%d)\n" VT_RST, bgId,
    //        dyna->bgActorFlags[bgId] & BGACTOR_INVALIDATE_LOOKUP);

    dyna->bgActorFlags[bgId] &= ~BGACTOR_INVALIDATE_LOOKUP;
    dyna->bgActorFlags[bgId] |= BGACTOR_TRANSFORM_NEEDS_UPDATE | BGACTOR_TRANSFORM_NEEDS_UPDATE_D;

    MtxF srpMtxF;
    ScaleRotPos* curTransform = &bgActor->curTransform;
    SkinMatrix_SetTranslateRotateYXZScale(&srpMtxF, curTransform->scale.x, curTransform->scale.y, curTransform->scale.z,
                                          curTransform->rot.x, curTransform->rot.y, curTransform->rot.z,
                                          curTransform->pos.x, curTransform->pos.y, curTransform->pos.z);

    // TODO ideally we'd test dynapoly collisions in model space and then return a pointer to the poly in world space,
    // however:
    // - bounding primitives need to be in world space for faster rejection against dynapolies we don't care about
    // - something about actors being able to modify the underlying model space geometry
#if 0
    // Model -> World Transformation
    SkinMatrix_SetTranslateRotateYXZScale(&bgActor->model2world,
                                          curTransform->scale.x, curTransform->scale.y, curTransform->scale.z,
                                          curTransform->rot.x, curTransform->rot.y, curTransform->rot.z,
                                          curTransform->pos.x, curTransform->pos.y, curTransform->pos.z);

    // Model -> World Transformation, Rotation only
    DynaPoly_BuildRotationMatrixForNormal(bgActor->model2worldRot, &curTransform->rot);

    Matrix_Push();
    {
        // World -> Model Transformation
        Matrix_Scale(1.0f / curTransform->scale.x, 1.0f / curTransform->scale.y, 1.0f / curTransform->scale.z, MTXMODE_NEW);
        Matrix_RotateZ(BINANG_TO_RAD(-curTransform->rot.z), MTXMODE_APPLY);
        Matrix_RotateX(BINANG_TO_RAD(-curTransform->rot.x), MTXMODE_APPLY);
        Matrix_RotateY(BINANG_TO_RAD(-curTransform->rot.y), MTXMODE_APPLY);
        Matrix_Translate(-curTransform->pos.x, -curTransform->pos.y, -curTransform->pos.z, MTXMODE_APPLY);
        Matrix_Get(&bgActor->world2model);
    }
    Matrix_Pop();
#endif

    // ================================================================================================================
    // TODO remove this, have a bounding sphere in model space that becomes a bounding ellipsoid in world space
    // (and do the test in model space?? the lingering problem here is that actors are allowed to change the underlying
    //  model space geometry (cf. bg_sst_floor, bg_ydan_sp) in any way they like so we can't really assume that the
    //  current bounding primitive is still valid after such a modification has been made...)
    Vec3f newCenterPoint;
    newCenterPoint.x = newCenterPoint.y = newCenterPoint.z = 0.0f;
    for (i = 0; i < colHeader->numVertices; i++) {
        Vec3f vtx = VCVT(&colHeader->vtxList[i]);
        Vec3f vtxT; // Vtx after mtx transform
        SkinMatrix_Vec3fMtxFMultXYZ(&srpMtxF, &vtx, &vtxT);
        bgActor->vtxList[i].x = vtxT.x;
        bgActor->vtxList[i].y = vtxT.y;
        bgActor->vtxList[i].z = vtxT.z;

        if (i == 0) {
            bgActor->minY = bgActor->maxY = vtxT.y;
        } else if (vtxT.y < bgActor->minY) {
            bgActor->minY = vtxT.y;
        } else if (bgActor->maxY < vtxT.y) {
            bgActor->maxY = vtxT.y;
        }
        newCenterPoint.x += vtxT.x;
        newCenterPoint.y += vtxT.y;
        newCenterPoint.z += vtxT.z;
    }

    f32 numVtxInverse = 1.0f / colHeader->numVertices;
    newCenterPoint.x *= numVtxInverse;
    newCenterPoint.y *= numVtxInverse;
    newCenterPoint.z *= numVtxInverse;
    Sphere16* sphere = &bgActor->boundingSphere;
    sphere->center.x = newCenterPoint.x;
    sphere->center.y = newCenterPoint.y;
    sphere->center.z = newCenterPoint.z;

    f32 newRadiusSq = 0.0f;
    for (i = 0; i < colHeader->numVertices; i++) {
        Vec3f newVtx = VCVT(&bgActor->vtxList[i]);
        Vec3f diff = VSUB(&newVtx, &newCenterPoint);
        f32 radiusSq = VMAGSQ(&diff);
        if (newRadiusSq < radiusSq) {
            newRadiusSq = radiusSq;
        }
    }

    sphere->radius = sqrtf(newRadiusSq) * 1.1f; // Slightly enlarge the sphere to curb fp errors (TODO why not additive?)
    // ================================================================================================================

    bgActor->polyNodes.count = 0;
    bgActor->dynaLookup.ceiling.head = SS_NULL;
    bgActor->dynaLookup.wall.head = SS_NULL;
    bgActor->dynaLookup.floor.head = SS_NULL;

    for (i = 0; i < colHeader->numPolygons; i++) {
        CollisionPoly* restrict newPoly = &bgActor->polyList[i];

        *newPoly = colHeader->polyList[i];

        Vec3f polyVerts[3];
        for (s32 j = 0; j < 3; j++) {
            polyVerts[j].x = bgActor->vtxList[COLPOLY_VTX_INDEX(newPoly->vtxData[j])].x;
            polyVerts[j].y = bgActor->vtxList[COLPOLY_VTX_INDEX(newPoly->vtxData[j])].y;
            polyVerts[j].z = bgActor->vtxList[COLPOLY_VTX_INDEX(newPoly->vtxData[j])].z;
        }

        Vec3f newNormal;
        Math3D_SurfaceNorm(&polyVerts[0], &polyVerts[1], &polyVerts[2], &newNormal);

        f32 magSQ = SQ(newNormal.x) + SQ(newNormal.y) + SQ(newNormal.z);
        if (!IS_ZERO(magSQ)) {
            f32 invMag = 1.0f / sqrtf(magSQ);
            newNormal.x *= invMag;
            newNormal.y *= invMag;
            newNormal.z *= invMag;
            newPoly->normal.x = COLPOLY_SNORMAL(newNormal.x);
            newPoly->normal.y = COLPOLY_SNORMAL(newNormal.y);
            newPoly->normal.z = COLPOLY_SNORMAL(newNormal.z);
        }
        newPoly->dist = -DOTXYZ(newNormal, bgActor->vtxList[COLPOLY_VTX_INDEX(newPoly->vtxData[0])]);

        SSList* target;
        if (NORMAL_IS_FLOOR(newNormal.y)) {
            target = &bgActor->dynaLookup.floor;
        } else if (NORMAL_IS_CEILING(newNormal.y)) {
            target = &bgActor->dynaLookup.ceiling;
        } else {
            target = &bgActor->dynaLookup.wall;
        }
        SSNodeList_SetSSListHead(&bgActor->polyNodes, target, i);
    }
}

void DynaPoly_UnsetAllInteractFlags(PlayState* play, DynaCollisionContext* dyna, Actor* actor) {
    for (s32 i = 0; i < BG_ACTOR_MAX; i++) {
        if (!(dyna->bgActorFlags[i] & BGACTOR_IN_USE)) {
            continue;
        }
        DynaPolyActor* dynaActor = DynaPoly_GetActor(&play->colCtx, i);
        if (dynaActor != NULL && &dynaActor->actor == actor) {
            DynaPolyActor_UnsetAllInteractFlags(dynaActor);
            break;
        }
    }
}

/**
 * Original name: "DynaPolyInfo_setup"
 */
void DynaPoly_UpdateContext(PlayState* play, DynaCollisionContext* dyna) {
    s32 i;

    for (i = 0; i < BG_ACTOR_MAX; i++) {
        s32 delete = false;
        BgActor* bgActor = &dyna->bgActors[i];

        if (dyna->bgActorFlags[i] & BGACTOR_MARKED_FOR_DELETION) {
            // Marked for deletion in a destroy function
            delete = true;
        }

        if (bgActor->actor != NULL && bgActor->actor->update == NULL) {
            // Delete BgActor because the actor is about to be deleted
            DynaPolyActor* actor = DynaPoly_GetActor(&play->colCtx, i);
            if (actor == NULL) {
                return;
            }
            actor->bgId = BGACTOR_NEG_ONE;
            delete = true;
        }

        if (delete) {
            // PRINTF(VT_FGCOL(RED) "BgActor bgId=%d freed polynodes\n" VT_RST, i);
            assert(bgActor->polyList != NULL);
            ZELDA_ARENA_FREE(bgActor->polyList, __FILE__, __LINE__);
            dyna->bgActorFlags[i] = 0;
            BgActor_Initialize(play, bgActor);
            dyna->bgActorFlags[i] |= BGACTOR_INVALIDATE_LOOKUP;
        }
    }

    for (i = 0; i < BG_ACTOR_MAX; i++) {
        if (dyna->bgActorFlags[i] & BGACTOR_IN_USE) {
            DynaPoly_AddBgActorToLookup(play, dyna, i);
        }
    }
}

/**
 * Update all BgActor's previous ScaleRotPos
 */
void DynaPoly_UpdateBgActorTransforms(PlayState* play, DynaCollisionContext* dyna) {
    for (s32 i = 0; i < BG_ACTOR_MAX; i++) {
        if (dyna->bgActorFlags[i] & BGACTOR_IN_USE) {
            dyna->bgActors[i].prevTransform = dyna->bgActors[i].curTransform;
        }
    }
}

#define DYNA_RAYCAST_FLOORS 1
#define DYNA_RAYCAST_WALLS 2
#define DYNA_RAYCAST_CEILINGS 4

/**
 * Performs a downward raycast check on a list of floor, wall, or ceiling dyna polys
 * `listType` specifies the poly list type (e.g. DYNA_RAYCAST_FLOORS)
 */
f32 BgCheck_RaycastDownDynaList(DynaRaycastDown* dynaRaycastDown, u32 listType) {
    DynaCollisionContext* dyna = dynaRaycastDown->dyna;
    CollisionPoly* polyList = dynaRaycastDown->bgActor->polyList;
    Vec3s* vtxList = dynaRaycastDown->bgActor->vtxList;
    f32 result = dynaRaycastDown->yIntersect;
    s32 groundOnly = (listType & (DYNA_RAYCAST_WALLS | DYNA_RAYCAST_CEILINGS)) &&
                     (dynaRaycastDown->downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_GROUND_ONLY);

    POLYLIST_FOREACH(dynaRaycastDown->bgActor->polyNodes.tbl, curNode, dynaRaycastDown->ssList) {
        CollisionPoly* poly = &polyList[curNode->polyId];

        INSTRUMENTATION(gBgCheckStats.numFloorPolysTraversed++);

        if (COLPOLY_VTX_CHECK_FLAGS_ANY(poly->flags_vIA, dynaRaycastDown->xpFlags)) {
            continue;
        }
        if (groundOnly && poly->normal.y < 0) {
            continue;
        }
        if (ABS(poly->normal.y) < COLPOLY_SNORMAL(IS_ZERO_EPS)) {
            continue;
        }

        INSTRUMENTATION(gBgCheckStats.numFloorPolysTested++);

        f32 yIntersect;
        if (CollisionPoly_CheckYIntersectDyna(poly, vtxList, dynaRaycastDown->pos, &yIntersect) &&
            yIntersect < dynaRaycastDown->pos->y && result < yIntersect) {
            INSTRUMENTATION(gBgCheckStats.numFloorPolysPassed++);
            result = yIntersect;
            *dynaRaycastDown->resultPoly = poly;
        }
    }
    return result;
}

/**
 * Performs a downward raycast check on dyna polys
 * returns the yIntersect of the poly found, or BGCHECK_Y_MIN if no poly is found
 */
f32 BgCheck_RaycastDownDyna(DynaRaycastDown* dynaRaycastDown) {
    CollisionContext* colCtx = dynaRaycastDown->colCtx;
    s32 i;
    f32 intersect;
    f32 result = BGCHECK_Y_MIN;

    dynaRaycastDown->dyna = &colCtx->dyna;
    *dynaRaycastDown->bgId = BGCHECK_SCENE;

    // Run the raycast on all nearby dynapolies

    for (i = 0; i < BG_ACTOR_MAX; i++) {
        if (!(colCtx->dyna.bgActorFlags[i] & BGACTOR_IN_USE)) {
            continue;
        }
        BgActor* bgActor = &colCtx->dyna.bgActors[i];

        if (dynaRaycastDown->actor == bgActor->actor || dynaRaycastDown->pos->y < bgActor->minY) {
            continue;
        }

        f32 dx = bgActor->boundingSphere.center.x - dynaRaycastDown->pos->x;
        f32 dz = bgActor->boundingSphere.center.z - dynaRaycastDown->pos->z;
        if (SQ(dx) + SQ(dz) > SQ(bgActor->boundingSphere.radius)) {
            continue;
        }

        dynaRaycastDown->bgActor = bgActor;

        if (dynaRaycastDown->downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_FLOORS) {
            dynaRaycastDown->ssList = &bgActor->dynaLookup.floor;
            intersect = BgCheck_RaycastDownDynaList(dynaRaycastDown, DYNA_RAYCAST_FLOORS);

            if (dynaRaycastDown->yIntersect < intersect) {
                dynaRaycastDown->yIntersect = intersect;
                *dynaRaycastDown->bgId = i;
                result = intersect;
            }
        }

        if ((dynaRaycastDown->downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_WALLS) ||
            (*dynaRaycastDown->resultPoly == NULL &&
             (dynaRaycastDown->downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_WALLS_SIMPLE))) {
            dynaRaycastDown->ssList = &bgActor->dynaLookup.wall;
            intersect = BgCheck_RaycastDownDynaList(dynaRaycastDown, DYNA_RAYCAST_WALLS);

            if (dynaRaycastDown->yIntersect < intersect) {
                dynaRaycastDown->yIntersect = intersect;
                *dynaRaycastDown->bgId = i;
                result = intersect;
            }
        }

        if (dynaRaycastDown->downChkFlags & BGCHECK_RAYCAST_DOWN_CHECK_CEILINGS) {
            dynaRaycastDown->ssList = &bgActor->dynaLookup.ceiling;
            intersect = BgCheck_RaycastDownDynaList(dynaRaycastDown, DYNA_RAYCAST_CEILINGS);

            if (dynaRaycastDown->yIntersect < intersect) {
                dynaRaycastDown->yIntersect = intersect;
                *dynaRaycastDown->bgId = i;
                result = intersect;
            }
        }
    }

    // For the closest, if it was found: (TODO wtf is all of this for? it seems to check for whether the bg was marked
    // for deletion so maybe it's a final update of some kind..)

    s32 bgId = *dynaRaycastDown->bgId;
    BgActor* bgActor = &dynaRaycastDown->dyna->bgActors[bgId];

    if (result == BGCHECK_Y_MIN || DynaPoly_GetActor(colCtx, bgId) == NULL || dynaRaycastDown->play == NULL) {
        return result;
    }

    if (IS_PAUSED(&dynaRaycastDown->play->pauseCtx) ||
        !(colCtx->dyna.bgActorFlags[bgId] & BGACTOR_MARKED_FOR_DELETION)) {
        return result;
    }

    // Build a matrix to transform positions from dyna model -> world

    MtxF mtxF;
    ScaleRotPos* curTransform = &bgActor->curTransform;
    SkinMatrix_SetTranslateRotateYXZScale(&mtxF, curTransform->scale.x, curTransform->scale.y, curTransform->scale.z,
                                          curTransform->rot.x, curTransform->rot.y, curTransform->rot.z,
                                          curTransform->pos.x, curTransform->pos.y, curTransform->pos.z);

    CollisionPoly* polyMin = &dynaRaycastDown->bgActor->polyList[0];
    CollisionPoly* poly = &bgActor->colHeader->polyList[*dynaRaycastDown->resultPoly - polyMin];

    // Update the vertex positions
    Vec3f polyVerts[3];
    Vec3s* vtxList = bgActor->colHeader->vtxList;
    for (i = 0; i < 3; i++) {
        Vec3f vtx = VCVT(&vtxList[COLPOLY_VTX_INDEX(poly->vtxData[i])]);
        SkinMatrix_Vec3fMtxFMultXYZ(&mtxF, &vtx, &polyVerts[i]);
    }

    // Compute the normal
    Vec3f polyNorm;
    Math3D_SurfaceNorm(&polyVerts[0], &polyVerts[1], &polyVerts[2], &polyNorm);

    f32 magSQ = SQ(polyNorm.x) + SQ(polyNorm.y) + SQ(polyNorm.z);
    if (!IS_ZERO(magSQ)) {
        f32 invMagnitude = 1.0f / sqrtf(magSQ);
        polyNorm.x *= invMagnitude;
        polyNorm.y *= invMagnitude;
        polyNorm.z *= invMagnitude;
        // Compute an intersection with the updated triangle?
        if (Math3D_TriChkPointParaYIntersectInsideTri(&polyVerts[0], &polyVerts[1], &polyVerts[2], polyNorm.x,
                                                      polyNorm.y, polyNorm.z, -DOTXYZ(polyNorm, polyVerts[0]),
                                                      dynaRaycastDown->pos->z, dynaRaycastDown->pos->x, &intersect,
                                                      1.0f) &&
            fabsf(intersect - result) < 1.0f) {
            result = intersect;
        }
    }
    return result;
}

/**
 * Performs collision detection on all dyna poly walls using sphere `pos`, `radius`
 * returns true if a collision was detected
 * `outX` `outZ` return the displaced x,z coordinates
 * `outPoly` returns the pointer to the nearest poly collided with, or NULL
 * `outBgId` returns the index of the BgActor that owns `outPoly`
 * If `actor` is not NULL, an BgActor bound to that actor will be ignored
 */
s32 BgCheck_SphVsDynaWall(CollisionContext* colCtx, u16 xpFlags, f32* outX, f32* outZ, Vec3f* pos, f32 radius,
                          CollisionPoly** outPoly, s32* outBgId, Actor* actor) {
    DynaCollisionContext* dyna = &colCtx->dyna;
    SurfaceType* surfaceTypes = colCtx->colHeader->surfaceTypeList;
    s32 result = false;
    Vec3f resultPos = *pos;
    s16 radius16 = (s16)radius;

    for (s32 i = 0; i < BG_ACTOR_MAX; i++) {
        if (!(dyna->bgActorFlags[i] & BGACTOR_IN_USE)) {
            continue;
        }
        BgActor* bgActor = &dyna->bgActors[i];

        if (bgActor->actor == actor) {
            continue;
        }
        // Note the vanilla bug: radius is not checked, only the sphere center
        if (bgActor->minY > resultPos.y || bgActor->maxY < resultPos.y) {
            continue;
        }

        // Check overlap between bounding sphere and collision sphere
        f32 rsq = SQ(bgActor->boundingSphere.radius + radius16);
        f32 dx = bgActor->boundingSphere.center.x - resultPos.x;
        f32 dy = bgActor->boundingSphere.center.y - resultPos.y;
        f32 dz = bgActor->boundingSphere.center.z - resultPos.z;
        // Either they overlap in xz or they overlap in both xy and yz
        if (rsq < SQ(dx) + SQ(dz) || (rsq < SQ(dx) + SQ(dy) && rsq < SQ(dy) + SQ(dz))) {
            continue;
        }

        if (BgCheck_WallCheck(surfaceTypes, &resultPos, outX, outZ, bgActor->vtxList, bgActor->polyList,
                              bgActor->polyNodes.tbl, &bgActor->dynaLookup.wall, radius, xpFlags, outBgId, i,
                              outPoly)) {
            resultPos.x = *outX;
            resultPos.z = *outZ;
            result = true;
        }
    }
    return result;
}

/**
 * Tests for collision with a dyna poly ceiling, starting at `ssList`
 * returns true if a collision occurs, else false
 * `outPoly` returns the poly collided with
 * `outY` returns the y coordinate needed to not collide with `outPoly`
 */
s32 BgCheck_CheckDynaCeilingList(CollisionContext* colCtx, u16 xpFlags, DynaCollisionContext* dyna, BgActor* bgActor,
                                 SSList* ssList, f32* outY, Vec3f* pos, f32 checkHeight, CollisionPoly** outPoly) {
    CollisionPoly* polyList = bgActor->polyList;
    Vec3s* vtxList = bgActor->vtxList;
    s32 result = false;
    Vec3f testPos = *pos;

    POLYLIST_FOREACH(bgActor->polyNodes.tbl, curNode, ssList) {
        CollisionPoly* poly = &polyList[curNode->polyId];

        INSTRUMENTATION(gBgCheckStats.numCeilingPolysTraversed++);

        if (COLPOLY_VTX_CHECK_FLAGS_ANY(poly->flags_vIA, xpFlags)) {
            continue;
        }
        if (ABS(poly->normal.y) < COLPOLY_SNORMAL(IS_ZERO_EPS)) {
            continue;
        }

        Vec3f n;
        CollisionPoly_GetNormalF(poly, &n.x, &n.y, &n.z);
        // Note: Assumes normal is unit-length or close enough
        if (checkHeight < fabsf(DOTXYZ(n, testPos) + poly->dist)) {
            continue;
        }

        INSTRUMENTATION(gBgCheckStats.numCeilingPolysTested++);

        f32 ceilingY;
        if (CollisionPoly_CheckYIntersect(poly, vtxList, testPos.x, testPos.z, &ceilingY)) {
            INSTRUMENTATION(gBgCheckStats.numCeilingPolysPassed++);
            f32 intersectDist = ceilingY - testPos.y;
            if (testPos.y < ceilingY && intersectDist < checkHeight && intersectDist * n.y <= 0.0f) {
                f32 sign = sgn(n.y);
                testPos.y = (sign * checkHeight) + ceilingY;
                result = true;
                *outPoly = poly;
            }
        }
    }

    *outY = testPos.y;
    return result;
}

/**
 * Tests collision with a dyna poly ceiling
 * returns true if a collision occurs, else false
 * `outPoly` returns the poly collided with, while `outBgId` returns the id of the BgActor that owns the poly
 * `outY` returns the y coordinate needed to not collide with `outPoly`, or `pos`.y + `chkDist` if no collision occurs
 */
s32 BgCheck_CheckDynaCeiling(CollisionContext* colCtx, u16 xpFlags, f32* outY, Vec3f* pos, f32 chkDist,
                             CollisionPoly** outPoly, s32* outBgId, Actor* actor) {
    f32 tempY = chkDist + pos->y;
    f32 resultY = tempY;
    s32 result = false;

    for (s32 i = 0; i < BG_ACTOR_MAX; i++) {
        if (!(colCtx->dyna.bgActorFlags[i] & BGACTOR_IN_USE)) {
            continue;
        }
        BgActor* bgActor = &colCtx->dyna.bgActors[i];

        if (bgActor->actor == actor) {
            continue;
        }

        f32 dx = bgActor->boundingSphere.center.x - pos->x;
        f32 dz = bgActor->boundingSphere.center.z - pos->z;
        if (SQ(dx) + SQ(dz) > SQ(bgActor->boundingSphere.radius)) {
            continue;
        }

        CollisionPoly* poly;
        if (BgCheck_CheckDynaCeilingList(colCtx, xpFlags, &colCtx->dyna, bgActor, &bgActor->dynaLookup.ceiling, &tempY,
                                         pos, chkDist, &poly) &&
            tempY < resultY) {
            resultY = tempY;
            *outPoly = poly;
            *outBgId = i;
            result = true;
        }
    }
    *outY = resultY;
    return result;
}

/**
 * Tests if DynaLineTest intersects with a poly
 * returns true if a poly was intersected, else false
 */
s32 BgCheck_CheckLineAgainstBgActorSSList(DynaLineTest* dynaLineTest) {
    DynaCollisionContext* dyna = dynaLineTest->dyna;
    BgActor* bgActor = dynaLineTest->bgActor;
    CollisionPoly* polyList = bgActor->polyList;
    Vec3s* vtxList = bgActor->vtxList;
    u16 xpFlags = dynaLineTest->xpFlags;
    s32 result = false;

    POLYLIST_FOREACH(bgActor->polyNodes.tbl, curNode, dynaLineTest->ssList) {
        CollisionPoly* curPoly = &polyList[curNode->polyId];

        INSTRUMENTATION(gBgCheckStats.numLinePolysTraversed++);

        if (COLPOLY_VTX_CHECK_FLAGS_ANY(curPoly->flags_vIA, xpFlags)) {
            continue;
        }

        Vec3f polyIntersect;
        if (CollisionPoly_LineVsPoly(curPoly, vtxList, dynaLineTest->posA, dynaLineTest->posB, &polyIntersect,
                                     dynaLineTest->chkOneFace)) {
            INSTRUMENTATION(gBgCheckStats.numLinePolysPassed++);
            Vec3f diff = VSUB(dynaLineTest->posA, &polyIntersect);
            f32 distSq = VMAGSQ(&diff);
            if (distSq < *dynaLineTest->distSq) {
                *dynaLineTest->distSq = distSq;
                *dynaLineTest->posResult = polyIntersect;
                *dynaLineTest->posB = polyIntersect;
                *dynaLineTest->resultPoly = curPoly;
                result = true;
            }
        }
    }
    return result;
}

/**
 * Tests if line `posA` `posB` intersects with a dyna poly within BgActor `bgId`
 * `distSq` is the maximum squared distance to check for a collision
 * returns true if an intersection occurred, else false
 * `posB`? and `posResult` return the point of intersection
 * `outPoly` returns the poly intersected
 * `distSq` returns the squared distance of the intersection
 */
s32 BgCheck_CheckLineAgainstBgActor(CollisionContext* colCtx, u16 xpFlags, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                                    CollisionPoly** outPoly, f32* distSq, s32 bgId, s32 bccFlags) {
    s32 result = false;
    DynaLineTest dynaLineTest;

    dynaLineTest.colCtx = colCtx;
    dynaLineTest.xpFlags = xpFlags;
    dynaLineTest.dyna = &colCtx->dyna;
    dynaLineTest.posA = posA;
    dynaLineTest.posB = posB;
    dynaLineTest.posResult = posResult;
    dynaLineTest.resultPoly = outPoly;
    dynaLineTest.chkOneFace = (bccFlags & BGCHECK_CHECK_ONE_FACE) != 0;
    dynaLineTest.distSq = distSq;
    dynaLineTest.bgActor = &colCtx->dyna.bgActors[bgId];

    dynaLineTest.ssList = &dynaLineTest.bgActor->dynaLookup.wall;
    result |= (bccFlags & BGCHECK_CHECK_WALL) && BgCheck_CheckLineAgainstBgActorSSList(&dynaLineTest);

    dynaLineTest.ssList = &dynaLineTest.bgActor->dynaLookup.floor;
    result |= (bccFlags & BGCHECK_CHECK_FLOOR) && BgCheck_CheckLineAgainstBgActorSSList(&dynaLineTest);

    dynaLineTest.ssList = &dynaLineTest.bgActor->dynaLookup.ceiling;
    result |= (bccFlags & BGCHECK_CHECK_CEILING) && BgCheck_CheckLineAgainstBgActorSSList(&dynaLineTest);

    return result;
}

/**
 * Tests if line from `posA` to `posB` passes through a dyna poly.
 * returns true if so, otherwise false
 * `outPoly` returns the pointer of the poly intersected.
 * `outBgId` returns the BgActor index of the poly
 */
s32 BgCheck_CheckLineAgainstDyna(CollisionContext* colCtx, u16 xpFlags, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                                 CollisionPoly** outPoly, f32* distSq, s32* outBgId, Actor* actor, s32 bccFlags) {
    s32 result = false;

    for (s32 i = 0; i < BG_ACTOR_MAX; i++) {
        if (!(colCtx->dyna.bgActorFlags[i] & BGACTOR_IN_USE)) {
            continue;
        }
        BgActor* bgActor = &colCtx->dyna.bgActors[i];

        if (actor == bgActor->actor) {
            continue;
        }

        f32 ay = posA->y;
        f32 by = posB->y;

        if ((ay < bgActor->minY && by < bgActor->minY) || (ay > bgActor->maxY && by > bgActor->maxY)) {
            continue;
        }

        Linef line;
        line.a = *posA;
        line.b = *posB;
        if (Math3D_LineVsSph(&bgActor->boundingSphere, &line) &&
            BgCheck_CheckLineAgainstBgActor(colCtx, xpFlags, posA, posB, posResult, outPoly, distSq, i, bccFlags)) {
            *outBgId = i;
            result = true;
        }
    }
    return result;
}

/**
 * Get first dyna poly intersecting sphere `center` `radius` from list `ssList`
 * returns true if any poly intersects the sphere, else returns false
 * `outPoly` returns the pointer of the first poly found that intersects
 */
s32 BgCheck_SphVsFirstDynaPolyList(CollisionContext* colCtx, BgActor* bgActor, u16 xpFlags, CollisionPoly** outPoly,
                                   Vec3f* center, f32 radius, SSList* ssList) {
    DynaCollisionContext* dyna = &colCtx->dyna;
    CollisionPoly* polyList = bgActor->polyList;
    Vec3s* vtxList = bgActor->vtxList;

    POLYLIST_FOREACH(bgActor->polyNodes.tbl, curNode, ssList) {
        CollisionPoly* curPoly = &polyList[curNode->polyId];

        if (COLPOLY_VTX_CHECK_FLAGS_ANY(curPoly->flags_vIA, xpFlags)) {
            continue;
        }

        if (CollisionPoly_SphVsPoly(curPoly, vtxList, center, radius)) {
            *outPoly = curPoly;
            return true;
        }
    }
    return false;
}

/**
 * Get first dyna poly intersecting sphere `center` `radius` from BgActor `bgId`
 * returns true if any poly intersects the sphere, else false
 * `outPoly` returns the pointer of the first poly found that intersects
 */
s32 BgCheck_SphVsFirstDynaPolyInBgActor(CollisionContext* colCtx, u16 xpFlags, CollisionPoly** outPoly, Vec3f* center,
                                        f32 radius, s32 bgId, u16 bciFlags) {
    BgActor* bgActor = &colCtx->dyna.bgActors[bgId];

    if (!(bciFlags & BGCHECK_IGNORE_CEILING) &&
        BgCheck_SphVsFirstDynaPolyList(colCtx, bgActor, xpFlags, outPoly, center, radius,
                                       &bgActor->dynaLookup.ceiling)) {
        return true;
    }
    if (!(bciFlags & BGCHECK_IGNORE_WALL) &&
        BgCheck_SphVsFirstDynaPolyList(colCtx, bgActor, xpFlags, outPoly, center, radius, &bgActor->dynaLookup.wall)) {
        return true;
    }
    if (!(bciFlags & BGCHECK_IGNORE_FLOOR) &&
        BgCheck_SphVsFirstDynaPolyList(colCtx, bgActor, xpFlags, outPoly, center, radius, &bgActor->dynaLookup.floor)) {
        return true;
    }
    return false;
}

/**
 * Gets first dyna poly intersecting sphere `center` `radius`
 * returns true if poly detected, else false
 * `outPoly` returns the first intersecting poly, while `outBgId` returns the BgActor index of that poly
 */
s32 BgCheck_SphVsFirstDynaPoly(CollisionContext* colCtx, u16 xpFlags, CollisionPoly** outPoly, s32* outBgId,
                               Vec3f* center, f32 radius, Actor* actor, u16 bciFlags) {
    for (s32 i = 0; i < BG_ACTOR_MAX; i++) {
        if (!(colCtx->dyna.bgActorFlags[i] & BGACTOR_IN_USE)) {
            continue;
        }
        BgActor* bgActor = &colCtx->dyna.bgActors[i];

        if (bgActor->actor == actor) {
            continue;
        }
        if (!SphereVsSphere16(center, radius, &bgActor->boundingSphere)) {
            continue;
        }
        if (BgCheck_SphVsFirstDynaPolyInBgActor(colCtx, xpFlags, outPoly, center, radius, i, bciFlags)) {
            return true;
        }
    }
    return false;
}

/**
 * SEGMENTED_TO_VIRTUAL CollisionHeader members
 */
void CollisionHeader_SegmentedToVirtual(CollisionHeader* colHeader) {
    colHeader->vtxList = SEGMENTED_TO_VIRTUAL(colHeader->vtxList);
    colHeader->polyList = SEGMENTED_TO_VIRTUAL(colHeader->polyList);
    colHeader->surfaceTypeList = SEGMENTED_TO_VIRTUAL(colHeader->surfaceTypeList);
    colHeader->bgCamList = SEGMENTED_TO_VIRTUAL(colHeader->bgCamList);
    colHeader->waterBoxes = SEGMENTED_TO_VIRTUAL(colHeader->waterBoxes);
}

/**
 * Convert CollisionHeader Segmented to Virtual addressing
 */
void CollisionHeader_GetVirtual(void* colHeader, CollisionHeader** dest) {
    *dest = SEGMENTED_TO_VIRTUAL(colHeader);
    CollisionHeader_SegmentedToVirtual(*dest);
}

/**
 * SEGMENT_TO_VIRTUAL all active BgActor CollisionHeaders
 */
void func_800418D0(CollisionContext* colCtx, PlayState* play) {
    DynaCollisionContext* dyna = &colCtx->dyna;

    for (s32 i = 0; i < BG_ACTOR_MAX; i++) {
        u16 flags = dyna->bgActorFlags[i];
        if ((flags & BGACTOR_IN_USE) && !(flags & BGACTOR_MARKED_FOR_DELETION)) {
            Actor_SetObjectDependency(play, dyna->bgActors[i].actor);
            CollisionHeader_SegmentedToVirtual(dyna->bgActors[i].colHeader);
        }
    }
}

/**
 * Get SurfaceType property set
 */
u32 SurfaceType_GetData(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId, s32 dataIdx) {
    CollisionHeader* colHeader = BgCheck_GetCollisionHeader(colCtx, bgId);
    if (colHeader == NULL || poly == NULL) {
        return 0;
    }
    SurfaceType* surfaceTypes = colHeader->surfaceTypeList;
    if (surfaceTypes == NULL) {
        return 0;
    }
    return surfaceTypes[poly->type].data[dataIdx];
}

u32 SurfaceType_GetBgCamIndex(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 0) & 0xFF;
}

/**
 * BgCam get setting of bgCam
 */
u16 BgCheck_GetBgCamSettingImpl(CollisionContext* colCtx, u32 bgCamIndex, s32 bgId) {
    CollisionHeader* colHeader = BgCheck_GetCollisionHeader(colCtx, bgId);
    if (colHeader == NULL) {
        return CAM_SET_NONE;
    }
    return colHeader->bgCamList[bgCamIndex].setting;
}

/**
 * BgCam Get the camera setting of bgCam
 */
u16 BgCheck_GetBgCamSetting(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    CollisionHeader* colHeader = BgCheck_GetCollisionHeader(colCtx, bgId);
    if (colHeader == NULL) {
        return CAM_SET_NONE;
    }
    if (colHeader->bgCamList == NULL) {
        return CAM_SET_NONE;
    }
    if (colHeader->surfaceTypeList == NULL) {
        return CAM_SET_NONE;
    }
    return BgCheck_GetBgCamSettingImpl(colCtx, SurfaceType_GetBgCamIndex(colCtx, poly, bgId), bgId);
}

/**
 * BgCam Get the total count of Vec3s data from bgCamFuncData
 */
u16 BgCheck_GetBgCamCountImpl(CollisionContext* colCtx, u32 bgCamIndex, s32 bgId) {
    CollisionHeader* colHeader = BgCheck_GetCollisionHeader(colCtx, bgId);
    if (colHeader == NULL) {
        return 0;
    }
    BgCamInfo* bgCamList = colHeader->bgCamList;
    if (bgCamList == NULL) {
        return 0;
    }
    return bgCamList[bgCamIndex].count;
}

/**
 * BgCam Get the total count of Vec3s data from bgCamFuncData
 */
u16 BgCheck_GetBgCamCount(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    CollisionHeader* colHeader = BgCheck_GetCollisionHeader(colCtx, bgId);
    if (colHeader == NULL) {
        return 0;
    }
    if (colHeader->bgCamList == NULL) {
        return 0;
    }
    if (colHeader->surfaceTypeList == NULL) {
        return 0;
    }
    return BgCheck_GetBgCamCountImpl(colCtx, SurfaceType_GetBgCamIndex(colCtx, poly, bgId), bgId);
}

/**
 * BgCam Get Vec3s data from bgCamFuncData
 */
Vec3s* BgCheck_GetBgCamFuncDataImpl(CollisionContext* colCtx, s32 bgCamIndex, s32 bgId) {
    CollisionHeader* colHeader = BgCheck_GetCollisionHeader(colCtx, bgId);
    if (colHeader == NULL) {
        return NULL;
    }
    BgCamInfo* bgCamList = colHeader->bgCamList;
    if (bgCamList == NULL) {
        return NULL;
    }
    return (Vec3s*)SEGMENTED_TO_VIRTUAL(bgCamList[bgCamIndex].bgCamFuncData);
}

/**
 * BgCam Get Vec3s data from bgCamFuncData
 */
Vec3s* BgCheck_GetBgCamFuncData(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    CollisionHeader* colHeader = BgCheck_GetCollisionHeader(colCtx, bgId);
    if (colHeader == NULL) {
        return NULL;
    }
    if (colHeader->bgCamList == NULL) {
        return NULL;
    }
    if (colHeader->surfaceTypeList == NULL) {
        return NULL;
    }
    return BgCheck_GetBgCamFuncDataImpl(colCtx, SurfaceType_GetBgCamIndex(colCtx, poly, bgId), bgId);
}

u32 SurfaceType_GetExitIndex(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 0) >> 8 & 0x1F;
}

u32 SurfaceType_GetFloorType(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 0) >> 13 & 0x1F;
}

/**
 * SurfaceType Get ? Property (& 0x001C 0000)
 */
u32 func_80041D70(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 0) >> 18 & 7;
}

u32 SurfaceType_GetWallType(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 0) >> 21 & 0x1F;
}

s32 SurfaceType_GetWallFlags(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return D_80119D90[SurfaceType_GetWallType(colCtx, poly, bgId)];
}

s32 SurfaceType_CheckWallFlag0(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return (SurfaceType_GetWallFlags(colCtx, poly, bgId) & WALL_FLAG_0) ? true : false;
}

s32 SurfaceType_CheckWallFlag1(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return (SurfaceType_GetWallFlags(colCtx, poly, bgId) & WALL_FLAG_1) ? true : false;
}

s32 SurfaceType_CheckWallFlag2(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return (SurfaceType_GetWallFlags(colCtx, poly, bgId) & WALL_FLAG_2) ? true : false;
}

u32 SurfaceType_GetFloorProperty2(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 0) >> 26 & 0xF;
}

u32 SurfaceType_GetFloorProperty(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 0) >> 26 & 0xF;
}

u32 SurfaceType_IsSoft(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 0) >> 30 & 1;
}

u32 SurfaceType_IsHorseBlocked(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 0) >> 31 & 1;
}

u32 SurfaceType_GetMaterial(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 1) & 0xF;
}

u16 SurfaceType_GetSfxOffset(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    s32 surfaceMaterial = SurfaceType_GetMaterial(colCtx, poly, bgId);

    if ((surfaceMaterial < 0) || (surfaceMaterial >= ARRAY_COUNT(sSurfaceMaterialToSfxOffset))) {
        return SURFACE_SFX_OFFSET_DIRT;
    }
    return sSurfaceMaterialToSfxOffset[surfaceMaterial];
}

u32 SurfaceType_GetFloorEffect(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 1) >> 4 & 3;
}

u32 SurfaceType_GetLightSetting(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 1) >> 6 & 0x1F;
}

u32 SurfaceType_GetEcho(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 1) >> 11 & 0x3F;
}

u32 SurfaceType_CanHookshot(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 1) >> 17 & 1;
}

/**
 * CollisionPoly is ignored by entities
 * Returns true if poly is ignored by entities, else false
 */
s32 SurfaceType_IsIgnoredByEntities(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    if (BgCheck_GetCollisionHeader(colCtx, bgId) == NULL) {
        return true;
    }
    return !!COLPOLY_VTX_CHECK_FLAGS_ANY(poly->flags_vIA, COLPOLY_IGNORE_ENTITY);
}

/**
 * CollisionPoly is ignored by projectiles
 * Returns true if poly is ignored by projectiles, else false
 */
s32 SurfaceType_IsIgnoredByProjectiles(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    if (BgCheck_GetCollisionHeader(colCtx, bgId) == NULL) {
        return true;
    }
    return !!COLPOLY_VTX_CHECK_FLAGS_ANY(poly->flags_vIA, COLPOLY_IGNORE_PROJECTILES);
}

/**
 * Checks if poly is a floor conveyor
 *
 * A conveyor surface is enabled with non-zero speed.
 * When enabled, the conveyor will exhibit two types of behaviour depending on the return value:
 *
 * If true, then it is a floor conveyor and will push player only while being stood on
 * If false, then it is a water conveyor and will push player only while in water
 */
s32 SurfaceType_IsFloorConveyor(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    if (BgCheck_GetCollisionHeader(colCtx, bgId) == NULL) {
        return true;
    }
    return !!COLPOLY_VTX_CHECK_FLAGS_ANY(poly->flags_vIB, COLPOLY_IS_FLOOR_CONVEYOR);
}

u32 SurfaceType_GetConveyorSpeed(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 1) >> 18 & 7;
}

/**
 * returns a value between 0-63, representing 360 / 64 degrees of rotation
 */
u32 SurfaceType_GetConveyorDirection(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return SurfaceType_GetData(colCtx, poly, bgId, 1) >> 21 & 0x3F;
}

u32 func_80042108(CollisionContext* colCtx, CollisionPoly* poly, s32 bgId) {
    return !!(SurfaceType_GetData(colCtx, poly, bgId, 1) & 0x08000000);
}

/**
 * Zora's Domain WaterBox in King Zora's Room
 */
WaterBox sZorasDomainWaterBox = { -348, 877, -1746, 553, 780, 0x2104 };
#define ZORAS_DOMAIN_WATERBOX_HEIGHT 100

/**
 * Public. Get the water surface at point (`x`, `ySurface`, `z`). `ySurface` doubles as position y input
 * returns true if point is within the xz boundaries of an active water box, else false
 * `ySurface` returns the water box's surface, while `outWaterBox` returns a pointer to the WaterBox
 */
s32 WaterBox_GetSurface1(PlayState* play, CollisionContext* colCtx, f32 x, f32 z, f32* ySurface,
                         WaterBox** outWaterBox) {
    if (play->sceneId == SCENE_ZORAS_DOMAIN) {
        s16 sx = x;
        s16 sy = *ySurface;
        s16 sz = z;

        if (sZorasDomainWaterBox.xMin < sx && sx < sZorasDomainWaterBox.xMin + sZorasDomainWaterBox.xLength &&
            sZorasDomainWaterBox.ySurface < sy + ZORAS_DOMAIN_WATERBOX_HEIGHT &&
            sy - ZORAS_DOMAIN_WATERBOX_HEIGHT < sZorasDomainWaterBox.ySurface && sZorasDomainWaterBox.zMin < sz &&
            sz < sZorasDomainWaterBox.zMin + sZorasDomainWaterBox.zLength) {
            *outWaterBox = &sZorasDomainWaterBox;
            *ySurface = sZorasDomainWaterBox.ySurface;
            return true;
        }
    }
    return WaterBox_GetSurfaceImpl(play, colCtx, x, z, ySurface, outWaterBox);
}

/**
 * Internal. Get the water surface at point (`x`, `ySurface`, `z`). `ySurface` doubles as position y input
 * returns true if point is within the xz boundaries of an active water box, else false
 * `ySurface` returns the water box's surface, while `outWaterBox` returns a pointer to the WaterBox
 */
s32 WaterBox_GetSurfaceImpl(PlayState* play, CollisionContext* colCtx, f32 x, f32 z, f32* ySurface,
                            WaterBox** outWaterBox) {
    CollisionHeader* colHeader = colCtx->colHeader;
    if (colHeader->numWaterBoxes == 0 || colHeader->waterBoxes == NULL) {
        return false;
    }

    for (s32 i = 0; i < colHeader->numWaterBoxes; i++) {
        WaterBox* waterBox = &colHeader->waterBoxes[i];
        s32 room = WATERBOX_ROOM(waterBox->properties);

        if (room != play->roomCtx.curRoom.num && room != WATERBOX_ROOM_ALL) {
            continue;
        }
        if (waterBox->properties & WATERBOX_FLAG_19) {
            continue;
        }

        if (waterBox->xMin < x && x < waterBox->xMin + waterBox->xLength && waterBox->zMin < z &&
            z < waterBox->zMin + waterBox->zLength) {
            *outWaterBox = waterBox;
            *ySurface = waterBox->ySurface;
            return true;
        }
    }
    return false;
}

/**
 * Gets the first active WaterBox at `pos` with WATERBOX_FLAG_19 not set
 * `surfaceChkDist` is the absolute y distance from the water surface to check
 * returns the index of the waterbox found, or -1 if no waterbox is found
 * `outWaterBox` returns the pointer to the waterbox found, or NULL if none is found
 */
s32 WaterBox_GetSurface2(PlayState* play, CollisionContext* colCtx, Vec3f* pos, f32 surfaceChkDist,
                         WaterBox** outWaterBox) {
    CollisionHeader* colHeader = colCtx->colHeader;
    if (colHeader->numWaterBoxes == 0 || colHeader->waterBoxes == NULL) {
        *outWaterBox = NULL;
        return -1;
    }

    for (s32 i = 0; i < colHeader->numWaterBoxes; i++) {
        WaterBox* waterBox = &colHeader->waterBoxes[i];
        s32 room = WATERBOX_ROOM(waterBox->properties);

        if (room != play->roomCtx.curRoom.num && room != WATERBOX_ROOM_ALL) {
            continue;
        }
        if (waterBox->properties & WATERBOX_FLAG_19) {
            continue;
        }

        if (waterBox->xMin < pos->x && pos->x < waterBox->xMin + waterBox->xLength && waterBox->zMin < pos->z &&
            pos->z < waterBox->zMin + waterBox->zLength && pos->y - surfaceChkDist < waterBox->ySurface &&
            waterBox->ySurface < pos->y + surfaceChkDist) {
            *outWaterBox = waterBox;
            return i;
        }
    }

    *outWaterBox = NULL;
    return -1;
}

/**
 * WaterBox get BgCam index
 */
u32 WaterBox_GetBgCamIndex(CollisionContext* colCtx, WaterBox* waterBox) {
    return waterBox->properties & 0xFF;
}

/**
 * WaterBox get BgCam setting
 */
u16 WaterBox_GetBgCamSetting(CollisionContext* colCtx, WaterBox* waterBox) {
    BgCamInfo* bgCamList = colCtx->colHeader->bgCamList;
    if (bgCamList == NULL) {
        return CAM_SET_NONE;
    }
    return bgCamList[WaterBox_GetBgCamIndex(colCtx, waterBox)].setting;
}

/**
 * WaterBox get lighting settings
 */
u32 WaterBox_GetLightIndex(CollisionContext* colCtx, WaterBox* waterBox) {
    return (waterBox->properties >> 8) & 0x1F;
}

/**
 * Get the water surface at point (`x`, `ySurface`, `z`). `ySurface` doubles as position y input
 * same as WaterBox_GetSurfaceImpl, but tests if WATERBOX_FLAG_19 is set
 * returns true if point is within the xz boundaries of an active water box, else false
 * `ySurface` returns the water box's surface, while `outWaterBox` returns a pointer to the WaterBox
 */
s32 func_800425B0(PlayState* play, CollisionContext* colCtx, f32 x, f32 z, f32* ySurface, WaterBox** outWaterBox) {
    CollisionHeader* colHeader = colCtx->colHeader;
    if (colHeader->numWaterBoxes == 0 || colHeader->waterBoxes == NULL) {
        return false;
    }

    for (s32 i = 0; i < colHeader->numWaterBoxes; i++) {
        WaterBox* waterBox = &colHeader->waterBoxes[i];
        s32 room = WATERBOX_ROOM(waterBox->properties);

        if (room != play->roomCtx.curRoom.num && room != WATERBOX_ROOM_ALL) {
            continue;
        }
        if (!(waterBox->properties & WATERBOX_FLAG_19)) {
            continue;
        }

        if (waterBox->xMin < x && x < waterBox->xMin + waterBox->xLength && waterBox->zMin < z &&
            z < waterBox->zMin + waterBox->zLength) {
            *outWaterBox = waterBox;
            *ySurface = waterBox->ySurface;
            return true;
        }
    }
    return false;
}

/**
 * Gets the `closestPoint` to `point` on the line formed from the intesection of planes `polyA` and `polyB`
 * returns true if the `closestPoint` exists, else returns false
 */
s32 func_80042708(CollisionPoly* polyA, CollisionPoly* polyB, Vec3f* point, Vec3f* closestPoint) {
    f32 n1X;
    f32 n1Y;
    f32 n1Z;
    f32 n2X;
    f32 n2Y;
    f32 n2Z;

    CollisionPoly_GetNormalF(polyA, &n1X, &n1Y, &n1Z);
    CollisionPoly_GetNormalF(polyB, &n2X, &n2Y, &n2Z);
    return Math3D_PlaneVsPlaneVsLineClosestPoint(n1X, n1Y, n1Z, polyA->dist, n2X, n2Y, n2Z, polyB->dist, point,
                                                 closestPoint);
}

/**
 * Get the `closestPoint` to line (`pointA`, `pointB`) formed from the intersection of planes `polyA` and `polyB`
 * returns true if the `closestPoint` exists, else returns false
 */
s32 func_800427B4(CollisionPoly* polyA, CollisionPoly* polyB, Vec3f* pointA, Vec3f* pointB, Vec3f* closestPoint) {
    f32 n1X;
    f32 n1Y;
    f32 n1Z;
    f32 n2X;
    f32 n2Y;
    f32 n2Z;
    s32 result;

    CollisionPoly_GetNormalF(polyA, &n1X, &n1Y, &n1Z);
    CollisionPoly_GetNormalF(polyB, &n2X, &n2Y, &n2Z);
    result = Math3D_PlaneVsLineSegClosestPoint(n1X, n1Y, n1Z, polyA->dist, n2X, n2Y, n2Z, polyB->dist, pointA, pointB,
                                               closestPoint);
    return result;
}

#if DEBUG_FEATURES
/**
 * Draw a list of dyna polys, specified by `ssList`
 */
void BgCheck_DrawDynaPolyList(PlayState* play, CollisionContext* colCtx, DynaCollisionContext* dyna, BgActor* bgActor,
                              SSList* ssList, u8 r, u8 g, u8 b) {
    POLYLIST_FOREACH(bgActor->polyNodes.tbl, curNode, ssList) {
        CollisionPoly* poly = &bgActor->polyList[curNode->polyId];
        Vec3f polyVerts[3];

        CollisionPoly_GetVertices(poly, bgActor->vtxList, polyVerts);
        if (AREG(26)) {
            f32 nx = COLPOLY_GET_NORMAL(poly->normal.x);
            f32 ny = COLPOLY_GET_NORMAL(poly->normal.y);
            f32 nz = COLPOLY_GET_NORMAL(poly->normal.z);
            polyVerts[0].x += AREG(26) * nx;
            polyVerts[0].y += AREG(26) * ny;
            polyVerts[0].z += AREG(26) * nz;
            polyVerts[1].x += AREG(26) * nx;
            polyVerts[1].y += AREG(26) * ny;
            polyVerts[1].z += AREG(26) * nz;
            polyVerts[2].x += AREG(26) * nx;
            polyVerts[2].y += AREG(26) * ny;
            polyVerts[2].z += AREG(26) * nz;
        }
        Collider_DrawPoly(play->state.gfxCtx, &polyVerts[0], &polyVerts[1], &polyVerts[2], r, g, b);
    }
}

/**
 * Draw a BgActor's dyna polys
 * `bgId` is the BgActor index that should be drawn
 */
void BgCheck_DrawBgActor(PlayState* play, CollisionContext* colCtx, s32 bgId) {
    BgActor* bgActor = &colCtx->dyna.bgActors[bgId];

    if (AREG(21)) {
        BgCheck_DrawDynaPolyList(play, colCtx, &colCtx->dyna, bgActor, &bgActor->dynaLookup.ceiling, 255, 0, 0);
    }
    if (AREG(22)) {
        BgCheck_DrawDynaPolyList(play, colCtx, &colCtx->dyna, bgActor, &bgActor->dynaLookup.wall, 0, 255, 0);
    }
    if (AREG(23)) {
        BgCheck_DrawDynaPolyList(play, colCtx, &colCtx->dyna, bgActor, &bgActor->dynaLookup.floor, 0, 0, 255);
    }
}

/**
 * Draw all dyna polys
 */
void BgCheck_DrawDynaCollision(PlayState* play, CollisionContext* colCtx) {
    for (s32 bgId = 0; bgId < BG_ACTOR_MAX; bgId++) {
        if (!(colCtx->dyna.bgActorFlags[bgId] & BGACTOR_IN_USE)) {
            continue;
        }
        BgCheck_DrawBgActor(play, colCtx, bgId);
    }
}

/**
 * Draw a static poly
 */
void BgCheck_DrawStaticPoly(PlayState* play, CollisionContext* colCtx, CollisionPoly* poly, u8 r, u8 g, u8 b) {
    Vec3f polyVerts[3];

    CollisionPoly_GetVertices(poly, colCtx->colHeader->vtxList, polyVerts);
    if (AREG(26) != 0) {
        f32 nx = COLPOLY_GET_NORMAL(poly->normal.x);
        f32 ny = COLPOLY_GET_NORMAL(poly->normal.y);
        f32 nz = COLPOLY_GET_NORMAL(poly->normal.z);
        polyVerts[0].x += AREG(26) * nx;
        polyVerts[0].y += AREG(26) * ny;
        polyVerts[0].z += AREG(26) * nz;
        polyVerts[1].x += AREG(26) * nx;
        polyVerts[1].y += AREG(26) * ny;
        polyVerts[1].z += AREG(26) * nz;
        polyVerts[2].x += AREG(26) * nx;
        polyVerts[2].y += AREG(26) * ny;
        polyVerts[2].z += AREG(26) * nz;
    }
    Collider_DrawPoly(play->state.gfxCtx, &polyVerts[0], &polyVerts[1], &polyVerts[2], r, g, b);
}

/**
 * Draw a list of static polys, specified by `ssList`
 */
void BgCheck_DrawStaticPolyList(PlayState* play, CollisionContext* colCtx, SSList* ssList, u8 r, u8 g, u8 b) {
    CollisionPoly* polyList = colCtx->colHeader->polyList;

    POLYLIST_FOREACH(colCtx->polyNodes.tbl, curNode, ssList) {
        BgCheck_DrawStaticPoly(play, colCtx, &polyList[curNode->polyId], r, g, b);
    }
}

/**
 * Draw scene collision
 */
void BgCheck_DrawStaticCollision(PlayState* play, CollisionContext* colCtx) {
    Player* player = GET_PLAYER(play);
    SSLookup* lookup = BgCheck_GetNearestStaticLookup(colCtx, colCtx->lookupTbl, &player->actor.world.pos);

    if (AREG(23) != 0) {
        BgCheck_DrawStaticPolyList(play, colCtx, &lookup->floor, 0, 0, 255);
    }
    if (AREG(22) != 0) {
        BgCheck_DrawStaticPolyList(play, colCtx, &lookup->wall, 0, 255, 0);
    }
    if (AREG(21) != 0) {
        BgCheck_DrawStaticPolyList(play, colCtx, &lookup->ceiling, 255, 0, 0);
    }
}
#endif
