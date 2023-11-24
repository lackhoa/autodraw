#pragma once

struct ReadFileResult
{
    u32 content_size;
    u8 *content;
};

ReadFileResult platformReadEntireFile(const char *file_name);
