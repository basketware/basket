#include <stdlib.h>
#include <strings.h>
#include "SDL2/SDL.h"
#include "basket.h"
#include "lib/ttf2mesh.h"


int fnt_init(Font *font, const char* data, u32 length, float size) {
    ttf_t* ttf;
    if (ttf_load_from_mem((u8 *)data, length, &ttf, false) != TTF_DONE) {
        return 0;
    }

    Glyph *array = calloc(sizeof(Glyph), ttf->nchars);

    size_t fallback = 0;

    for (int i = 0; i < ttf->nchars; i++) {
        ttf_glyph_t *g = ttf->glyphs + ttf->char2glyph[i];

        Glyph *character = &array[i];

        character->character = ttf->chars[i];

        ttf_mesh_t *mesh = NULL;
        ttf_glyph2mesh(g, &mesh, 5, 0);

        if (mesh == NULL) continue;

        character->slice.length = mesh->nfaces * 3;
        character->slice.data = malloc(character->slice.length * sizeof(Vertex));
        character->advance = g->advance * 16;

        for (int j = 0; j < mesh->nfaces; j++) {
            for (int k = 0; k < 3; k++) {
                u32 idx = j * 3 + k;
                int vert_idx;
                switch (k) {
                    case 0: vert_idx = mesh->faces[j].v1; break;
                    case 1: vert_idx = mesh->faces[j].v2; break;
                    case 2: vert_idx = mesh->faces[j].v3; break;
                }

                character->slice.data[idx].position[0] = mesh->vert[vert_idx].x * 16;
                character->slice.data[idx].position[1] = mesh->vert[vert_idx].y * -16;
                character->slice.data[idx].position[2] = 0.0f;

                character->slice.data[idx].uv[0] = mesh->vert[vert_idx].x / size;
                character->slice.data[idx].uv[1] = mesh->vert[vert_idx].y / size;

                character->slice.data[idx].color = COLOR_WHITE;
            }
        }

        if (character->character == '?')
            fallback = i;

        ttf_free_mesh(mesh);
    }

    font->size = size;
    font->glyphs = array;
    font->characters = ttf->nchars;
    font->fallback = fallback;

    ttf_free(ttf);
    //free(ttf_data);
    return 0;
}

void copy_string( char* _dst, const char* _src, size_t _n )
{
   size_t i = 0;
   while(i++ != _n && (*_dst++ = *_src++));
}


static uint32_t utf8_next(const char** ptr) {
    const unsigned char* p = (const unsigned char*)*ptr;
    uint32_t character = 0;

    if (*p <= 0x7F) {
        character = *p++;
    } else if (*p <= 0xDF) {
        character = (*p++ & 0x1F) << 6;
        character |= (*p++ & 0x3F);
    } else if (*p <= 0xEF) {
        character = (*p++ & 0x0F) << 12;
        character |= (*p++ & 0x3F) << 6;
        character |= (*p++ & 0x3F);
    } else {
        character = (*p++ & 0x07) << 18;
        character |= (*p++ & 0x3F) << 12;
        character |= (*p++ & 0x3F) << 6;
        character |= (*p++ & 0x3F);
    }

    *ptr = (const char*)p;
    return character;
}

int fnt_free(Font *font) {
    for (int i = 0; i < font->characters; i++) {
        Glyph g = font->glyphs[i];
        free(g.slice.data);
    }
    free(font->glyphs);
    font->size = 0;
    return 0;
}

static Glyph find_glyph(Font font, uint32_t character) {
    for (int i = 0; i < font.characters; i++) {
        if (font.glyphs[i].character == character) {
            return font.glyphs[i];
        }
    }
    return font.glyphs[font.fallback];
}

static f32 get_advance(Font font, uint32_t character) {
    Glyph glyph = find_glyph(font, character);
    return (character == ' ' || character == '\t' || character == '\n')
           ? font.glyphs[font.fallback].advance
           : glyph.advance;
}

static void render_character(Font font, uint32_t character, Color color, f32 x, f32 y) {
    Glyph glyph = find_glyph(font, character);
    RenderCall call = {
        .model = QUICK_TRANSLATION_MATRIX(x, font.size + y, 0),
        .mesh = glyph.slice,
        .texture = { 0, 0, 1, 1 },
        .tint = color
    };
    ren_draw(call);
}


void fnt_size(Font font, const char *_text, f32 *w, f32 *h) {
    const char *text = _text;
    f32 x = 0;
    f32 y = 0;
    f32 max_width = 0;
    f32 line_width = 0;

    while (*text) {
        uint32_t character = utf8_next(&text);
        f32 char_advance = get_advance(font, character);

        if (character == '\n') {
            y += font.size;
            max_width = (line_width > max_width) ? line_width : max_width;
            line_width = 0;
        } else {
            line_width += char_advance;
        }
    }

    // Check the last line
    max_width = (line_width > max_width) ? line_width : max_width;

    // Set the width and height
    if (w != NULL)
        *w = max_width;
    if (h != NULL)
        *h = y + font.size; // Add one more line height for the last line
}

// Assume fnt_size and get_advance are defined elsewhere

// Helper function to render text without any wrapping
static void render_text_naive(Font font, const char *text, f32 x, f32 y, Color color) {
    const f32 original_x = x;

    while (*text) {
        uint32_t character = utf8_next(&text);
        render_character(font, character, color, x, y);
        x += get_advance(font, character);

        if (*text == '\n') {
            y += font.size * 1.5;
            x = original_x;
        }
    }
}

void fnt_print(Font font, const char *text, Color color, f32 x, f32 y, f32 wrap) {
    if (wrap <= 0) {
        render_text_naive(font, text, x, y, color);
        return;
    }

    const f32 original_x = x;
    f32 space_width = get_advance(font, ' ');
    const char *word_start = text;

    while (*text) {
        if (*text == ' ' || *text == '\n' || *(text + 1) == '\0') {
            // End of word or end of string
            char word[256] = {0};  // Assuming max word length of 255 characters
            size_t word_len = text - word_start + (*text != ' ' && *text != '\n');
            copy_string(word, word_start, word_len);
            word[word_len] = '\0';

            f32 word_width, word_height;
            fnt_size(font, word, &word_width, &word_height);

            if (x + word_width > original_x + wrap && x > original_x) {
                // Word doesn't fit, wrap to next line
                y += font.size * 1.3;
                x = original_x;
            }

            render_text_naive(font, word, x, y, color);
            x += word_width;

            if (*text == ' ' && x + space_width <= original_x + wrap) {
                x += space_width;
            } else if (*text == '\n') {
                y += font.size * 1.5;
                x = original_x;
            }

            word_start = text + 1;
        }
        text++;
    }
}
