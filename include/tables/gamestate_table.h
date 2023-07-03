/**
 * Gamestate Table
 *
 * DEFINE_GAMESTATE should be used for gamestates with code loaded from an overlay
 *    - Argument 1: Gamestate type name (without State suffix, also used for Init and Destroy function names)
 *    - Argument 2: Gamestate id enum name
 *    - Argument 3: Gamestate overlay spec segment name
 *
 * DEFINE_GAMESTATE_INTERNAL should be used for gamestates that aren't an overlay, the first two arguments are the same as for DEFINE_GAMESTATE
 */
/* 0x00 */ DEFINE_GAMESTATE_INTERNAL(Setup, GAMESTATE_ID_SETUP)
/* 0x01 */ DEFINE_GAMESTATE(MapSelect, GAMESTATE_ID_MAP_SELECT, select)
/* 0x02 */ DEFINE_GAMESTATE(ConsoleLogo, GAMESTATE_ID_CONSOLE_LOGO, title)
/* 0x03 */ DEFINE_GAMESTATE_INTERNAL(Play, GAMESTATE_ID_PLAY)
/* 0x04 */ DEFINE_GAMESTATE(TitleSetup, GAMESTATE_ID_TITLE_SETUP, opening)
/* 0x05 */ DEFINE_GAMESTATE(FileSelect, GAMESTATE_ID_FILE_SELECT, file_choose)