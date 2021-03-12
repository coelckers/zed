#ifndef buildtypes_h__
#define buildtypes_h__

//ceilingstat/floorstat:
//   bit 0: 1 = parallaxing, 0 = not                                 "P"
//   bit 1: 1 = groudraw, 0 = not
//   bit 2: 1 = swap x&y, 0 = not                                    "F"
//   bit 3: 1 = double smooshiness                                   "E"
//   bit 4: 1 = x-flip                                               "F"
//   bit 5: 1 = y-flip                                               "F"
//   bit 6: 1 = Align texture to first wall of sector                "R"
//   bits 8-7:                                                       "T"
//          00 = normal floors
//          01 = masked floors
//          10 = transluscent masked floors
//          11 = reverse transluscent masked floors
//   bit 9: 1 = blocking ceiling/floor
//   bit 10: 1 = YAX'ed ceiling/floor
//   bit 11: 1 = hitscan-sensitive ceiling/floor
//   bits 12-15: reserved

//////////////////// Version 7 map format ////////////////////

//40 bytes
struct sectortype
{
    int16_t wallptr, wallnum;
    int32_t ceilingz, floorz;
    uint16_t ceilingstat, floorstat;
    int16_t ceilingpicnum, ceilingheinum;
    int8_t ceilingshade;
    uint8_t ceilingpal; /*CM_FLOORZ:*/
    int16_t floorpicnum, floorheinum;
    int8_t floorshade;
    uint8_t floorpal;;
    uint8_t /*CM_CEILINGZ:*/ visibility, fogpal;
    int16_t lotag;
    int16_t hitag;
    int16_t extra;

    uint16_t ceilingxpan, ceilingypan, floorxpan, floorypan;
};

//cstat:
//   bit 0: 1 = Blocking wall (use with clipmove, getzrange)         "B"
//   bit 1: 1 = bottoms of invisible walls swapped, 0 = not          "2"
//   bit 2: 1 = align picture on bottom (for doors), 0 = top         "O"
//   bit 3: 1 = x-flipped, 0 = normal                                "F"
//   bit 4: 1 = masking wall, 0 = not                                "M"
//   bit 5: 1 = 1-way wall, 0 = not                                  "1"
//   bit 6: 1 = Blocking wall (use with hitscan / cliptype 1)        "H"
//   bit 7: 1 = Transluscence, 0 = not                               "T"
//   bit 8: 1 = y-flipped, 0 = normal                                "F"
//   bit 9: 1 = Transluscence reversing, 0 = normal                  "T"
//   bits 10 and 11: reserved (in use by YAX)
//   bits 12-15: reserved  (14: temp use by editor)

//32 bytes
struct walltype
{
    int32_t x, y;
    int16_t point2, nextwall, nextsector;
    uint16_t cstat;
    int16_t picnum, overpicnum;
    int8_t shade;
    uint8_t pal, xrepeat, yrepeat;
    int16_t lotag;
    int16_t hitag;
    int16_t extra;
    int16_t xpan, ypan;
};

//cstat:
//   bit 0: 1 = Blocking sprite (use with clipmove, getzrange)       "B"
//   bit 1: 1 = transluscence, 0 = normal                            "T"
//   bit 2: 1 = x-flipped, 0 = normal                                "F"
//   bit 3: 1 = y-flipped, 0 = normal                                "F"
//   bits 5-4: 00 = FACE sprite (default)                            "R"
//             01 = WALL sprite (like masked walls)
//             10 = FLOOR sprite (parallel to ceilings&floors)
//   bit 6: 1 = 1-sided sprite, 0 = normal                           "1"
//   bit 7: 1 = Real centered centering, 0 = foot center             "C"
//   bit 8: 1 = Blocking sprite (use with hitscan / cliptype 1)      "H"
//   bit 9: 1 = Transluscence reversing, 0 = normal                  "T"
//   bit 10: reserved (in use by a renderer hack, see CSTAT_SPRITE_MDHACK)
//   bit 11: 1 = determine shade based only on its own shade member (see CON's spritenoshade command), i.e.
//               don't take over shade from parallaxed ceiling/nonparallaxed floor
//               (NOTE: implemented on the game side)
//   bit 12: reserved
//   bit 13: 1 = does not cast shadow
//   bit 14: 1 = invisible but casts shadow
//   bit 15: 1 = Invisible sprite, 0 = not invisible
#ifndef buildtypes_h__enums
enum
{
    CSTAT_SPRITE_BLOCK = 1u,
    CSTAT_SPRITE_TRANSLUCENT = 1u<<1u,
    CSTAT_SPRITE_XFLIP = 1u<<2u,
    CSTAT_SPRITE_YFLIP = 1u<<3u,
    CSTAT_SPRITE_ALIGNMENT = 1u<<4u | 1u<<5u, // (cstat & CSTAT_SPRITE_ALIGNMENT) == CSTAT_SPRITE_ALIGNMENT_xxxxxx can be used to check sprite alignment
    CSTAT_SPRITE_ONE_SIDED = 1u<<6u,
    CSTAT_SPRITE_YCENTER = 1u<<7u,
    CSTAT_SPRITE_BLOCK_HITSCAN = 1u<<8u,
    CSTAT_SPRITE_BLOCK_ALL = CSTAT_SPRITE_BLOCK_HITSCAN | CSTAT_SPRITE_BLOCK,
    CSTAT_SPRITE_TRANSLUCENT_INVERT = 1u<<9u,

    CSTAT_SPRITE_RESERVED1 = 1u<<10u, // used by Duke 3D (Polymost)
    CSTAT_SPRITE_RESERVED2 = 1u<<11u, // used by Duke 3D (EDuke32 game code extension)
    CSTAT_SPRITE_RESERVED3 = 1u<<12u, // used by Shadow Warrior, Blood
    CSTAT_SPRITE_RESERVED4 = 1u<<13u, // used by Duke 3D (Polymer), Shadow Warrior, Blood
    CSTAT_SPRITE_RESERVED5 = 1u<<14u, // used by Duke 3D (Polymer), Shadow Warrior, Blood

    CSTAT_SPRITE_INVISIBLE = 1u<<15u,
};
enum
{
    CSTAT_SPRITE_ALIGNMENT_FACING = 0,
    CSTAT_SPRITE_ALIGNMENT_WALL   = 1u<<4u,
    CSTAT_SPRITE_ALIGNMENT_FLOOR  = 1u<<5u,
    CSTAT_SPRITE_ALIGNMENT_SLAB   = 1u<<4u | 1u<<5u,

    CSTAT_SPRITE_ALIGNMENT_MASK   = 1u<<4u | 1u<<5u,
};

enum
{
    CSTAT_WALL_BLOCK         = 1u,
    CSTAT_WALL_BOTTOM_SWAP   = 1u<<1u,
    CSTAT_WALL_ALIGN_BOTTOM  = 1u<<2u,
    CSTAT_WALL_XFLIP         = 1u<<3u,
    CSTAT_WALL_MASKED        = 1u<<4u,
    CSTAT_WALL_1WAY          = 1u<<5u,
    CSTAT_WALL_BLOCK_HITSCAN = 1u<<6u,
    CSTAT_WALL_TRANSLUCENT   = 1u<<7u,
    CSTAT_WALL_YFLIP         = 1u<<8u,
    CSTAT_WALL_TRANS_FLIP    = 1u<<9u,

    CSTAT_WALL_YAX_UPWALL    = 1u<<10u, // EDuke32 extension
    CSTAT_WALL_YAX_DOWNWALL  = 1u<<11u, // EDuke32 extension
    CSTAT_WALL_ROTATE_90     = 1u<<12u, // EDuke32 extension

    CSTAT_WALL_RESERVED1     = 1u<<13u,
    CSTAT_WALL_RESERVED2     = 1u<<14u, // used by Shadow Warrior, Blood
    CSTAT_WALL_RESERVED3     = 1u<<15u, // used by Shadow Warrior, Blood
};
#endif

#ifndef buildtypes_h__enums


struct spritetype
{
    int32_t x, y, z;
    uint16_t cstat;
    int16_t picnum;
    int8_t shade;
    uint8_t pal, clipdist, blend;
    uint8_t xrepeat, yrepeat;
    int8_t xoffset, yoffset;
    int16_t sectnum, statnum;
    int16_t ang, owner;
	int16_t xvel, yvel, zvel;
    int16_t lotag, hitag;
    int16_t extra;
    int16_t detail;

};

#endif

//////////////////// END Version 7 map format ////////////////

#ifndef buildtypes_h__enums
#define buildtypes_h__enums
#endif


extern int numsectors, numwalls, numsprites;
extern sectortype sector[4096];
extern walltype wall[16384];
extern spritetype sprite[16384];

class FileReader;
void engineLoadBoard(FileReader& fr, spritetype& startpos);

#endif // buildtypes_h__
