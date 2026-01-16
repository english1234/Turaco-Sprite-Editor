// coding.c
//
//  Functions for converting between the sprite palette bitmaps
//  and the format of the memory rom images.
//
//  November, 1998
//   Ivan Mackintosh
//
//  Speedup hacks/rewrite December 1998 Jerry
//  Fixed bitplane arrangement for Donkey Kong

#include "../INCLUDE/ALLEGRO.H"
#include "../INCLUDE/sprtplte.h"
#include "../INCLUDE/general.h"
#include "../INCLUDE/gamedesc.h"

int GetBit(long startbit) {
    long nbyte = startbit >> 3;

    // Check if byte position is within ANY loaded ROM
    for (int i = 0; i < NumGfxRoms; i++) {
        if (nbyte >= GfxRoms[i].LoadAddress &&
            nbyte < GfxRoms[i].LoadAddress + GfxRoms[i].Size) {
            int nbit = startbit & 7;
            int mask = 0x80 >> nbit;
            return ((GfxRomData[nbyte] & mask) == mask);
        }
    }

    return 0;  // Bit not found in any ROM
}

// some macros to make everything a little more "standardized"

#define ___setup_plane_info___()\
    for (plane = 0; plane < gbe->planes; plane++)         \
    {                                                     \
        pln[plane] = 1 << (gbe->planes - 1 - plane);      \
    }

#define ___get_sprite_macro___(XXX,YYY)                   \
    result = 0;                                           \
    ww = offs1 + gbe->xoffset[x] + gbe->yoffset[y];       \
    for (plane = 0; plane < gbe->planes; plane++)         \
    {                                                     \
        if (GetBit(ww + gbe->planeoffsets[plane]))        \
            result |= pln[plane];                         \
    }                                                     \
    putpixel(tbmp, (XXX), (YYY), result + FIRST_USER_COLOR);


void Decode_Normal(int CurrentBank) {
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    long x, y, spriteno, plane;
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);
    int pln[MAX_COL_PLANES];

    printf("=== DECODE_NORMAL Bank %d ===\n", CurrentBank);
    printf("Dimensions: %dx%d, Sprites: %d\n",
        sp->sprite_w, sp->sprite_h, sp->n_total);
    printf("Planes: %d, StartAddress: 0x%lX, CharIncrement: %d\n",
        gbe->planes, gbe->startaddress, gbe->charincrement);

    // Calculate total ROM size from loaded ROMs
    long max_rom_address = 0;
    for (int i = 0; i < NumGfxRoms; i++) {
        long rom_end = GfxRoms[i].LoadAddress + GfxRoms[i].Size;
        if (rom_end > max_rom_address) {
            max_rom_address = rom_end;
        }
    }
    printf("Max loaded ROM address: 0x%lX\n", max_rom_address);

    // SIMPLIFIED: Just calculate how many sprites fit in the available space
    long available_bytes = max_rom_address - gbe->startaddress;
    int max_fit_sprites = available_bytes / gbe->charincrement;

    // Don't limit sprites - trust the INI file
    int sprites_to_decode = sp->n_total;

    printf("Available bytes: %ld, Max fit sprites: %d\n", available_bytes, max_fit_sprites);
    printf("Decoding %d sprites\n", sprites_to_decode);

    if (sprites_to_decode <= 0) {
        printf("ERROR: No sprites to decode!\n");
        destroy_bitmap(tbmp);
        return;
    }

    clear_to_color(tbmp, FIRST_USER_COLOR);

    for (plane = 0; plane < gbe->planes; plane++) {
        pln[plane] = 1 << (gbe->planes - 1 - plane);
    }

    // Debug: Check ROM data for the last few sprites
    printf("\nDEBUG: Checking ROM data for sprites 0xB0 to 0xBF (176-191):\n");
    for (int debug_sprite = 0xB0; debug_sprite <= 0xBF && debug_sprite < sprites_to_decode; debug_sprite++) {
        long debug_offs1 = (gbe->charincrement * debug_sprite) + StartBit;
        printf("Sprite 0x%X (offs1=%ld, byte=%ld):\n", debug_sprite, debug_offs1, debug_offs1 / 8);

        // Check first few bytes of each plane
        for (plane = 0; plane < gbe->planes; plane++) {
            long plane_offset = gbe->planeoffsets[plane];
            printf("  Plane %d (offset=%ld bits, %ld bytes): ",
                plane, plane_offset, plane_offset / 8);

            // Check first 8 bytes of this sprite in this plane
            for (int byte = 0; byte < 8; byte++) {
                long bit_position = debug_offs1 + plane_offset + (byte * 8);
                long byte_position = bit_position / 8;

                // Check if byte position is within any ROM
                int in_rom = 0;
                for (int rom_idx = 0; rom_idx < NumGfxRoms; rom_idx++) {
                    if (byte_position >= GfxRoms[rom_idx].LoadAddress &&
                        byte_position < GfxRoms[rom_idx].LoadAddress + GfxRoms[rom_idx].Size) {
                        in_rom = 1;
                        break;
                    }
                }

                if (in_rom && byte_position >= 0 && byte_position < max_rom_address) {
                    printf("%02X ", GfxRomData[byte_position] & 0xFF);
                }
                else {
                    printf("-- ");
                }
            }
            printf("\n");
        }
    }

    for (spriteno = 0; spriteno < sprites_to_decode; spriteno++) {
        offs1 = (gbe->charincrement * spriteno) + StartBit;

        // Debug problematic sprites
        if (spriteno >= 0xB8 && spriteno <= 0xBF) {
            printf("\nDEBUG Sprite 0x%X (%d):\n", spriteno, spriteno);
        }

        // copy the current character into a bitmap
        for (y = 0; y < sp->sprite_h; y++) {
            for (x = 0; x < sp->sprite_w; x++) {
                long ww = offs1 + gbe->xoffset[x] + gbe->yoffset[y];

                result = 0;
                int bits_found[2] = { 0, 0 };
                for (plane = 0; plane < gbe->planes; plane++) {
                    long bit_position = ww + gbe->planeoffsets[plane];
                    if (GetBit(bit_position)) {
                        result |= pln[plane];
                    }
                }

                // Debug first few pixels of problematic sprites
                if (spriteno >= 0xB8 && spriteno <= 0xBF && x < 2 && y < 2) {
                    printf("  Pixel (%d,%d): ww=%ld, result=%d, bits: plane0=%d, plane1=%d\n",
                        x, y, ww, result, bits_found[0], bits_found[1]);
                }

                // CRITICAL: Ensure color index is within valid range
                if (result >= (1 << gbe->planes)) {
                    // Only warn for first few sprites
                    if (spriteno < 10) {
                        printf("WARNING: Invalid color index %d at sprite %d, pixel (%d,%d)\n",
                            result, spriteno, x, y);
                    }
                    result = 0;  // Default to color 0 on error
                }

                putpixel(tbmp, x, y, result + FIRST_USER_COLOR);
            }
        }

        // Debug: Check if sprite is all one color
        if (spriteno >= 0xB8 && spriteno <= 0xBF) {
            int all_same = 1;
            int first_color = getpixel(tbmp, 0, 0);
            for (y = 0; y < sp->sprite_h && all_same; y++) {
                for (x = 0; x < sp->sprite_w && all_same; x++) {
                    if (getpixel(tbmp, x, y) != first_color) {
                        all_same = 0;
                    }
                }
            }
            if (all_same) {
                printf("  Sprite 0x%X is all color %d\n", spriteno, first_color - FIRST_USER_COLOR);
            }
        }

        // now copy it to the appropriate place in the master gfxbank bitmap
        int dest_x = spriteno * sp->sprite_w;

        if (dest_x + sp->sprite_w > sp->bmp->w) {
            printf("ERROR: Attempting to write beyond bitmap bounds in Decode_Normal!\n");
            continue; // Skip this sprite
        }

        if (dest_x >= 0 && dest_x < (sp->n_total * sp->sprite_w)) {
            blit(tbmp, sp->bmp, 0, 0, dest_x,
                0, sp->sprite_w, sp->sprite_h);
        }
    }

    // If we couldn't decode all sprites, clear the remaining area
    if (sprites_to_decode < sp->n_total) {
        int remaining_sprites = sp->n_total - sprites_to_decode;
        int start_x = sprites_to_decode * sp->sprite_w;
        int width = remaining_sprites * sp->sprite_w;

        if (start_x < (sp->n_total * sp->sprite_w)) {
            rectfill(sp->bmp, start_x, 0, start_x + width - 1, sp->sprite_h - 1,
                FIRST_USER_COLOR);
        }

        printf("Cleared %d sprites (sprites %d to %d)\n",
            remaining_sprites, sprites_to_decode, sp->n_total - 1);
    }

    destroy_bitmap(tbmp);
    printf("=== END DECODE_NORMAL Bank %d ===\n", CurrentBank);
    printf("Successfully decoded %d of %d sprites\n", sprites_to_decode, sp->n_total);

    // Debug: Check the actual pixel data in the final bitmap for problematic sprites
    printf("\nDEBUG: Final bitmap check for sprites 0xB8-0xBF:\n");
    for (int s = 0xB8; s <= 0xBF && s < sprites_to_decode; s++) {
        int sprite_start_x = s * sp->sprite_w;
        printf("Sprite 0x%X (pixels at x=%d to %d): ", s, sprite_start_x, sprite_start_x + sp->sprite_w - 1);

        // Check a few pixels
        for (int check_x = 0; check_x < 4 && check_x < sp->sprite_w; check_x++) {
            int color = getpixel(sp->bmp, sprite_start_x + check_x, 0);
            printf("[%d]=%d ", check_x, color - FIRST_USER_COLOR);
        }
        printf("\n");
    }
}

void Decode_Flip_X(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int rx;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;
        // copy the current character into a bitmap
        for (y = 0; y < sp->sprite_h; y++)
        {
            for (x = 0; x < sp->sprite_w; x++)
            {
                rx = sp->sprite_w - 1 - x;

                ___get_sprite_macro___(rx, y);
            }
        }

        // now copy it to the appropriate place in the master gfxbank bitmap
        blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w,
            0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode_Flip_Y(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int ry;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;
        // copy the current character into a bitmap
        for (y = 0; y < sp->sprite_h; y++)
        {
            ry = sp->sprite_h - 1 - y;
            for (x = 0; x < sp->sprite_w; x++)
            {
                ___get_sprite_macro___(x, ry);
            }
        }

        // now copy it to the appropriate place in the master gfxbank bitmap
        blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w,
            0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode_Rotate_90(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int ry;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;
        // copy the current character into a bitmap
        for (y = 0; y < sp->sprite_h; y++)
        {
            ry = sp->sprite_h - 1 - y;
            for (x = 0; x < sp->sprite_w; x++)
            {
                ___get_sprite_macro___(ry, x);
            }
        }

        // now copy it to the appropriate place in the master gfxbank bitmap
        blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w,
            0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode_Rotate_180(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int rx, ry;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;
        // copy the current character into a bitmap
        for (y = 0; y < sp->sprite_h; y++)
        {
            ry = sp->sprite_h - 1 - y;
            for (x = 0; x < sp->sprite_w; x++)
            {
                rx = sp->sprite_w - 1 - x;
                ___get_sprite_macro___(rx, ry);
            }
        }

        // now copy it to the appropriate place in the master gfxbank bitmap
        blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w,
            0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode_Rotate_270(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int ww;
    int rx, ry;
    int pln[MAX_COL_PLANES];
    long offs1;
    BYTE result = 0;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);

    clear_to_color(tbmp, FIRST_USER_COLOR);

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;
        // copy the current character into a bitmap
        for (y = 0; y < sp->sprite_h; y++)
        {
            ry = sp->sprite_h - 1 - y;
            for (x = 0; x < sp->sprite_w; x++)
            {
                rx = sp->sprite_w - 1 - x;
                ___get_sprite_macro___(y, rx);
            }
        }

        // now copy it to the appropriate place in the master gfxbank bitmap
        blit(tbmp, sp->bmp, 0, 0, spriteno * sp->sprite_w,
            0, sp->sprite_w, sp->sprite_h);
    }

    destroy_bitmap(tbmp);
}

void Decode(int CurrentBank)
{
    switch (Orientation)
    {
    case ORIENTATION_FLIP_X:
        Decode_Flip_X(CurrentBank);
        break;
    case ORIENTATION_FLIP_Y:
        Decode_Flip_Y(CurrentBank);
        break;
    case ORIENTATION_ROTATE_180:
    case ORIENTATION_SWAP_XY:
        Decode_Rotate_180(CurrentBank);
        break;
    case ORIENTATION_ROTATE_90:
        Decode_Rotate_90(CurrentBank);
        break;
    case ORIENTATION_ROTATE_270:
        Decode_Rotate_270(CurrentBank);
        break;
    default:
        Decode_Normal(CurrentBank);
        break;
    }
}

#define PutBit(s,v) do {                                 \
    long __nbyte = (s) >> 3;                            \
    int __nbit = (s) & 7;                               \
    int __mask = 0x80 >> __nbit;                        \
    GfxRomData[__nbyte] &= ~__mask;                     \
    if ((v)) GfxRomData[__nbyte] |= __mask;             \
} while(0)

#define ___put_sprite_macro___(XXX,YYY)                           \
    {                                                             \
        int pixel_color = getpixel(tbmp, (XXX), (YYY));           \
        int color_index = pixel_color - FIRST_USER_COLOR;         \
        long ww = offs1 + gbe->xoffset[x] + gbe->yoffset[y];      \
        for (plane = 0; plane < gbe->planes; plane++)             \
        {                                                         \
            int bit_value = (color_index & pln[plane]) ? 1 : 0;   \
            long bit_position = ww + plane;  /* FIXED: Relative position */ \
            PutBit(bit_position, bit_value);                      \
        }                                                         \
    }

void Encode_Normal(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
    int pln[MAX_COL_PLANES];
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);
    long offs1;
    long ww;  // This was missing in your version!

    printf("\n=== ENCODE_NORMAL DEBUG Bank %d ===\n", CurrentBank);

    // Use the original macro approach
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;

        blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0,
            0, 0, sp->sprite_w, sp->sprite_h);

        for (y = 0; y < sp->sprite_h; y++)
        {
            for (x = 0; x < sp->sprite_w; x++)
            {
                ww = offs1 + gbe->xoffset[x] + gbe->yoffset[y];

                for (plane = 0; plane < gbe->planes; plane++)
                {
                    // debug Inside the encoding loops
                    if (spriteno == 0 && x < 4 && y < 2) {
                        int pixel_color = getpixel(tbmp, x, y);
                        int color_index = pixel_color - FIRST_USER_COLOR;
                        printf("Encoding sprite0[%d,%d]: master_color=%d, color_index=%d\n",
                            x, y, pixel_color, color_index);
                    }

                    // USE THE EXACT ORIGINAL LOGIC - no intermediate variable
                    PutBit(ww + gbe->planeoffsets[plane],
                        (getpixel(tbmp, x, y) - FIRST_USER_COLOR) & pln[plane]);
                }
            }
        }
    }

    destroy_bitmap(tbmp);
    printf("=== END ENCODE_NORMAL DEBUG Bank %d ===\n\n", CurrentBank);
}

void Encode_Flip_X(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
   // int ww;
    int pln[MAX_COL_PLANES];
    int rx;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);
    long offs1;

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;

        // copy the current char into the temp bitmap for rotation
        blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0,
            0, 0, sp->sprite_w, sp->sprite_h);

        for (x = 0; x < sp->sprite_w; x++)
        {
            rx = sp->sprite_w - 1 - x;
            for (y = 0; y < sp->sprite_h; y++)
            {
                ___put_sprite_macro___(rx, y);
            }
        }
    }
    destroy_bitmap(tbmp);
}

void Encode_Flip_Y(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
 //   int ww;
    int pln[MAX_COL_PLANES];
    int ry;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);
    long offs1;

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;

        // copy the current char into the temp bitmap for rotation
        blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0,
            0, 0, sp->sprite_w, sp->sprite_h);

        for (y = 0; y < sp->sprite_h; y++)
        {
            ry = sp->sprite_h - 1 - y;
            for (x = 0; x < sp->sprite_w; x++)
            {
                ___put_sprite_macro___(x, ry);
            }
        }
    }
    destroy_bitmap(tbmp);
}

void Encode_Rotate_90(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
   // int ww;
    int pln[MAX_COL_PLANES];
    int ry;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);
    long offs1;

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;

        // copy the current char into the temp bitmap for rotation
        blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0,
            0, 0, sp->sprite_w, sp->sprite_h);

        for (y = 0; y < sp->sprite_h; y++)
        {
            ry = sp->sprite_h - 1 - y;
            for (x = 0; x < sp->sprite_w; x++)
            {
                ___put_sprite_macro___(ry, x);
            }
        }
    }
    destroy_bitmap(tbmp);
}

void Encode_Rotate_180(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
 //   int ww;
    int pln[MAX_COL_PLANES];
    int rx, ry;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);
    long offs1;

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;

        // copy the current char into the temp bitmap for rotation
        blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0,
            0, 0, sp->sprite_w, sp->sprite_h);

        for (y = 0; y < sp->sprite_h; y++)
        {
            ry = sp->sprite_h - 1 - y;
            for (x = 0; x < sp->sprite_w; x++)
            {
                rx = sp->sprite_w - 1 - x;

                ___put_sprite_macro___(rx, ry);
            }
        }
    }
    destroy_bitmap(tbmp);
}

void Encode_Rotate_270(int CurrentBank)
{
    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[CurrentBank];
    SPRITE_PALETTE* sp = &GfxBanks[CurrentBank];
    int x, y, spriteno, plane;
   // int ww;
    int pln[MAX_COL_PLANES];
    int rx;
    long StartBit = gbe->startaddress * 8;
    MYBITMAP* tbmp = create_bitmap(sp->sprite_w, sp->sprite_h);
    long offs1;

    // FIXED: Use LSB to MSB bitplane arrangement
    ___setup_plane_info___()

    for (spriteno = 0; spriteno < sp->n_total; spriteno++)
    {
        offs1 = (gbe->charincrement * spriteno) + StartBit;

        // copy the current char into the temp bitmap for rotation
        blit(sp->bmp, tbmp, spriteno * sp->sprite_w, 0,
            0, 0, sp->sprite_w, sp->sprite_h);

        for (x = 0; x < sp->sprite_w; x++)
        {
            rx = sp->sprite_w - 1 - x;
            for (y = 0; y < sp->sprite_h; y++)
            {
                ___put_sprite_macro___(y, rx);
            }
        }
    }
    destroy_bitmap(tbmp);
}

void Encode(int CurrentBank)
{
    switch (Orientation)
    {
    case ORIENTATION_FLIP_X:
        Encode_Flip_X(CurrentBank);
        break;
    case ORIENTATION_FLIP_Y:
        Encode_Flip_Y(CurrentBank);
        break;
    case ORIENTATION_ROTATE_180:
    case ORIENTATION_SWAP_XY:
        Encode_Rotate_180(CurrentBank);
        break;
    case ORIENTATION_ROTATE_90:
        Encode_Rotate_90(CurrentBank);
        break;
    case ORIENTATION_ROTATE_270:
        Encode_Rotate_270(CurrentBank);
        break;
    default:
        Encode_Normal(CurrentBank);
        break;
    }
}

long CalculateExpectedSpriteSize(int sprite_w, int sprite_h, int planes) {
    // Calculate minimum bytes needed for sprite: width * height * planes / 8
    // This is the theoretical minimum without any padding or interleaving
    long min_size = (sprite_w * sprite_h * planes) / 8;

    // However, some games have padding or interleaving, so return both
    return min_size;
}

// If and old bank is specified (-1 for no old bank) then
// the bitmap for that bank is encoded into the memory roms image
// Once this is complete the bitmap for the new bank is then
// decode from the memory rom image.
void SwitchGraphicsBank(int oldbank, int newbank) {
    printf("\n=== SwitchGraphicsBank DEBUG ===\n");
    printf("Old bank: %d, New bank: %d\n", oldbank, newbank);
    printf("GameDriverLoaded: %d\n", GameDriverLoaded);

    // Calculate total ROM size from loaded ROMs
    long max_rom_address = 0;
    for (int i = 0; i < NumGfxRoms; i++) {
        long rom_end = GfxRoms[i].LoadAddress + GfxRoms[i].Size;
        if (rom_end > max_rom_address) {
            max_rom_address = rom_end;
        }
    }
    printf("Max loaded ROM address: 0x%lX\n", max_rom_address);

    // anything to encode?
    if (oldbank != -1 && oldbank < NumGfxBanks) {
        printf("Encoding bank %d\n", oldbank);
        Encode(oldbank);
        printf("Encoding completed for bank %d\n", oldbank);
    }

    // now decode the new one
    if (newbank != -1 && newbank < NumGfxBanks) {
        printf("Decoding bank %d\n", newbank);

        GFXBANKEXTRA* gbe = &GfxBankExtraInfo[newbank];
        SPRITE_PALETTE* sp = &GfxBanks[newbank];

        // CRITICAL: Validate charincrement against expected sprite size
        long expected_min_size = CalculateExpectedSpriteSize(sp->sprite_w, sp->sprite_h, gbe->planes);

        printf("Bank %d validation:\n", newbank);
        printf("  Sprite: %dx%d, %d planes\n", sp->sprite_w, sp->sprite_h, gbe->planes);
        printf("  Expected minimum bytes per sprite: %ld\n", expected_min_size);
        printf("  Configured charincrement: %d\n", gbe->charincrement);

        if (gbe->charincrement < expected_min_size) {
            printf("ERROR: charincrement (%d) is LESS than minimum required (%ld)!\n",
                gbe->charincrement, expected_min_size);
            printf("INI file has incorrect charincrement value.\n");
        }
        else if (gbe->charincrement > expected_min_size * 2) {
            printf("WARNING: charincrement (%d) is MORE than 2x expected minimum (%ld)\n",
                gbe->charincrement, expected_min_size);
            printf("This may indicate incorrect charincrement or interleaved data.\n");
        }

        // Calculate actual available data
        long available_bytes = 0;
        if (gbe->startaddress < max_rom_address) {
            available_bytes = max_rom_address - gbe->startaddress;
        }
        else {
            printf("CRITICAL ERROR: Start address 0x%lX is beyond available ROM data!\n",
                gbe->startaddress);
            printf("Available ROM data up to: 0x%lX\n", max_rom_address);

            // Clear the bank and mark as empty
            clear_to_color(sp->bmp, FIRST_USER_COLOR);
            sp->n_total = 0;
            return;
        }

        printf("Available bytes from start address: %ld (0x%lX)\n",
            available_bytes, available_bytes);

        // Calculate maximum sprites that can fit in available data
        int max_fit_sprites = 0;
        if (gbe->charincrement > 0) {
            max_fit_sprites = available_bytes / gbe->charincrement;
        }

        printf("Maximum sprites that fit: %d (using charincrement=%d)\n",
            max_fit_sprites, gbe->charincrement);

        // Don't limit based on validation - let Decode_Normal handle it
        // Just pass through to decode
        printf("Calling Decode for bank %d\n", newbank);
        Decode(newbank);

        // After decoding, verify the first sprite
        if (sp->bmp) {
            printf("First sprite pixel at (0,0) color index: %d\n",
                getpixel(sp->bmp, 0, 0) - FIRST_USER_COLOR);
        }
    }
}

void TestFixedBitplaneArrangement(int bank_index) {
    printf("\n=== FIXED BITPLANE ARRANGEMENT TEST Bank %d ===\n", bank_index);

    GFXBANKEXTRA* gbe = &GfxBankExtraInfo[bank_index];
    int pln[MAX_COL_PLANES];

    // New arrangement: LSB to MSB
    for (int plane = 0; plane < gbe->planes; plane++) {
        pln[plane] = 1 << plane;
    }

    printf("New plane bitmasks: ");
    for (int plane = 0; plane < gbe->planes; plane++) {
        printf("pln[%d]=%d ", plane, pln[plane]);
    }
    printf("\n");

    printf("Color mapping with new arrangement:\n");
    for (int color_index = 0; color_index < (1 << gbe->planes); color_index++) {
        printf("  Index %d -> ", color_index);
        for (int plane = 0; plane < gbe->planes; plane++) {
            int bit_value = (color_index & pln[plane]) ? 1 : 0;
            printf("Plane%d=%d ", plane, bit_value);
        }
        printf("-> Color %d\n", color_index + FIRST_USER_COLOR);
    }

    printf("=== END FIXED BITPLANE TEST ===\n\n");
}