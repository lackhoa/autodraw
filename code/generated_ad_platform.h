typedef void GameInitialize(Arena & init_arena, PlatformCode & platform, String autodraw_path, Codepoint * codepoints);

typedef GameOutput GameUpdateAndRender(GameInput & input);

