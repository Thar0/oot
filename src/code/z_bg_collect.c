#include "printf.h"
#include "terminal.h"
#include "translation.h"
#include "ultra64.h"
#include "actor.h"
#include "bgcheck.h"
#include "player.h"
#include "skin_matrix.h"
#include "sys_matrix.h"

/**
 * Update the `carriedActor`'s position based on the dynapoly actor identified by `bgId`.
 */
void DynaPolyActor_UpdateCarriedActorPos(CollisionContext* colCtx, s32 bgId, Actor* carriedActor) {
    if (!DynaPoly_IsBgIdBgActor(bgId)) {
        return;
    }

    // We actually need to update the transform for at least 2 frames after the dynapoly has transformed in order to
    // capture the relative transform after a dynapoly stops transforming. Otherwise, the last frame of motion will
    // continue to be applied even after it has come to rest.
    if (colCtx->dyna.bgActorFlags[bgId] & (BGACTOR_TRANSFORM_NEEDS_UPDATE | BGACTOR_TRANSFORM_NEEDS_UPDATE_D)) {
        ScaleRotPos* curTransform = &colCtx->dyna.bgActors[bgId].curTransform;
        ScaleRotPos* prevTransform = &colCtx->dyna.bgActors[bgId].prevTransform;

        if (IS_ZERO(prevTransform->scale.x) || IS_ZERO(prevTransform->scale.y) || IS_ZERO(prevTransform->scale.z)) {
            // Previous transform is not invertible, skip it for now and don't update any actor positions
            return;
        }

        // Computes T2^-1 * R2^-1 * S2^-1 * S1 * R1 * T1
        // TODO is it worth checking whether only translation changed and skipping most of the work in that case?
        Matrix_Push();
        {
            Matrix_SetTranslateRotateYXZ(curTransform->pos.x, curTransform->pos.y, curTransform->pos.z,
                                         &curTransform->rot);
            Matrix_Scale(curTransform->scale.x / prevTransform->scale.x, curTransform->scale.y / prevTransform->scale.y,
                         curTransform->scale.z / prevTransform->scale.z, MTXMODE_APPLY);
            Matrix_RotateZ(BINANG_TO_RAD(-prevTransform->rot.z), MTXMODE_APPLY);
            Matrix_RotateX(BINANG_TO_RAD(-prevTransform->rot.x), MTXMODE_APPLY);
            Matrix_RotateY(BINANG_TO_RAD(-prevTransform->rot.y), MTXMODE_APPLY);
            Matrix_Translate(-prevTransform->pos.x, -prevTransform->pos.y, -prevTransform->pos.z, MTXMODE_APPLY);
            // Get the relative transform result
            Matrix_Get(&colCtx->dyna.bgActors[bgId].relTransform);
        }
        Matrix_Pop();

        if (colCtx->dyna.bgActorFlags[bgId] & BGACTOR_TRANSFORM_NEEDS_UPDATE_D) {
            colCtx->dyna.bgActorFlags[bgId] &= ~BGACTOR_TRANSFORM_NEEDS_UPDATE_D;
        } else {
            colCtx->dyna.bgActorFlags[bgId] &= ~BGACTOR_TRANSFORM_NEEDS_UPDATE;
        }
    }

    // Apply the matrix to the position
    Vec3f pos;
    SkinMatrix_Vec3fMtxFMultXYZ(&colCtx->dyna.bgActors[bgId].relTransform, &carriedActor->world.pos, &pos);
    carriedActor->world.pos = pos;
}

/**
 * Update the `carriedActor`'s Y rotation based on the dynapoly actor identified by `bgId`.
 */
void DynaPolyActor_UpdateCarriedActorRotY(CollisionContext* colCtx, s32 bgId, Actor* carriedActor) {
    if (DynaPoly_IsBgIdBgActor(bgId)) {
        s16 rotY = colCtx->dyna.bgActors[bgId].curTransform.rot.y - colCtx->dyna.bgActors[bgId].prevTransform.rot.y;

        if (carriedActor->id == ACTOR_PLAYER) {
            ((Player*)carriedActor)->yaw += rotY;
        }

        carriedActor->shape.rot.y += rotY;
        carriedActor->world.rot.y += rotY;
    }
}

void func_80043334(CollisionContext* colCtx, Actor* actor, s32 bgId) {
    if (DynaPoly_IsBgIdBgActor(bgId)) {
        DynaPolyActor* dynaActor = DynaPoly_GetActor(colCtx, bgId);
        if (dynaActor != NULL) {
            DynaPolyActor_SetActorOnTop(dynaActor);

            if (ACTOR_FLAGS_CHECK_ALL(actor, ACTOR_FLAG_CAN_PRESS_SWITCHES)) {
                DynaPolyActor_SetSwitchPressed(dynaActor);
            }
        }
    }
}

/**
 * Update the `carriedActor`'s position and Y rotation based on the dynapoly actor identified by `bgId`, according to
 * the dynapoly actor's move flags (see `DYNA_TRANSFORM_POS` and `DYNA_TRANSFORM_ROT_Y`).
 */
s32 DynaPolyActor_TransformCarriedActor(CollisionContext* colCtx, s32 bgId, Actor* carriedActor) {
    s32 result = false;
    DynaPolyActor* dynaActor;

    if (!DynaPoly_IsBgIdBgActor(bgId)) {
        return false;
    }

    if ((colCtx->dyna.bgActorFlags[bgId] & BGACTOR_MARKED_FOR_DELETION) || !(colCtx->dyna.bgActorFlags[bgId] & BGACTOR_IN_USE)) {
        return false;
    }

    dynaActor = DynaPoly_GetActor(colCtx, bgId);

    if (dynaActor == NULL) {
        return false;
    }

    if (dynaActor->transformFlags & DYNA_TRANSFORM_POS) {
        DynaPolyActor_UpdateCarriedActorPos(colCtx, bgId, carriedActor);
        result = true;
    }

    if (dynaActor->transformFlags & DYNA_TRANSFORM_ROT_Y) {
        DynaPolyActor_UpdateCarriedActorRotY(colCtx, bgId, carriedActor);
        result = true;
    }

    return result;
}
