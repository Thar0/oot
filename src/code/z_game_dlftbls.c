#include "global.h"

#define DEFINE_GAMESTATE_INTERNAL(typeName) \
    { NULL, 0, 0, NULL, NULL, NULL, typeName##_Init, typeName##_Destroy, NULL, NULL, 0, sizeof(typeName##State) },

#define DEFINE_GAMESTATE(typeName, name)       \
    { NULL,                                    \
      (uintptr_t)_ovl_##name##SegmentRomStart, \
      (uintptr_t)_ovl_##name##SegmentRomEnd,   \
      _ovl_##name##SegmentStart,               \
      _ovl_##name##SegmentEnd,                 \
      NULL,                                    \
      typeName##_Init,                         \
      typeName##_Destroy,                      \
      NULL,                                    \
      NULL,                                    \
      0,                                       \
      sizeof(typeName##State) },

GameStateOverlay gGameStateOverlayTable[] = {
#include "tables/gamestate_table.h"
};

#undef DEFINE_GAMESTATE
#undef DEFINE_GAMESTATE_INTERNAL
