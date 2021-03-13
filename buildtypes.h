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

#ifndef MAKE_ID
#ifndef __BIG_ENDIAN__
#define MAKE_ID(a,b,c,d)	((uint32_t)((a)|((b)<<8)|((c)<<16)|((d)<<24)))
#else
#define MAKE_ID(a,b,c,d)	((uint32_t)((d)|((c)<<8)|((b)<<16)|((a)<<24)))
#endif
#endif

// Blood map format - designed by idiots.
#pragma pack(push, 1)
// This is the on-disk format. Only Blood still needs this for its retarded encryption that has to read this in as a block so that it can be decoded.
// Keep it local so that the engine's sprite type is no longer limited by file format restrictions.
struct spritetypedisk
{
    int32_t x, y, z;
    uint16_t cstat;
    int16_t picnum;
    int8_t shade;
    uint8_t pal, clipdist, detail;
    uint8_t xrepeat, yrepeat;
    int8_t xoffset, yoffset;
    int16_t sectnum, statnum;
    int16_t ang, owner;
    int16_t index, yvel, inittype;
    int16_t type;
    int16_t hitag;
    int16_t extra;
};

struct sectortypedisk
{
    int16_t wallptr, wallnum;
    int32_t ceilingz, floorz;
    uint16_t ceilingstat, floorstat;
    int16_t ceilingpicnum, ceilingheinum;
    int8_t ceilingshade;
    uint8_t ceilingpal, ceilingxpanning, ceilingypanning;
    int16_t floorpicnum, floorheinum;
    int8_t floorshade;
    uint8_t floorpal, floorxpanning, floorypanning;
    uint8_t visibility, fogpal;
    int16_t type;
    int16_t hitag;
    int16_t extra;
};

struct walltypedisk
{
    int32_t x, y;
    int16_t point2, nextwall, nextsector;
    uint16_t cstat;
    int16_t picnum, overpicnum;
    int8_t shade;
    uint8_t pal, xrepeat, yrepeat, xpanning, ypanning;
    int16_t type;
    int16_t hitag;
    int16_t extra;
};

#pragma pack(pop)

struct XSPRITE {

    union
    {
        uint32_t flags;
        struct {
            unsigned int state : 1;             // State 0
            unsigned int triggerOn : 1;         // going ON
            unsigned int triggerOff : 1;        // going OFF
            unsigned int restState : 1;         // restState
            unsigned int Interrutable : 1;      // Interruptable
            unsigned int Decoupled : 1;         // Decoupled
            unsigned int triggerOnce : 1;       // 1-shot
            unsigned int isTriggered : 1;       // works in case if triggerOnce selected
            unsigned int Push : 1;              // Push
            unsigned int Vector : 1;            // Vector
            unsigned int Impact : 1;            // Impact
            unsigned int Pickup : 1;            // Pickup
            unsigned int Touch : 1;             // Touch
            unsigned int Sight : 1;             // Sight
            unsigned int Proximity : 1;         // Proximity
            unsigned int lS : 1;                // Single
            unsigned int lB : 1;                // Bloodbath
            unsigned int lT : 1;                // Launch Team
            unsigned int lC : 1;                // Coop
            unsigned int DudeLockout : 1;       // DudeLockout
            unsigned int locked : 1;            // Locked
            unsigned int dudeDeaf : 1;          // dudeDeaf
            unsigned int dudeAmbush : 1;        // dudeAmbush
            unsigned int dudeGuard : 1;         // dudeGuard
            unsigned int dudeFlag4 : 1;         // unused
            unsigned int wave : 2;              // Wave
            unsigned int medium : 2;            // medium
            unsigned int respawn : 2;           // Respawn option
        };
    };
    int32_t targetX;          // target x
    int32_t targetY;          // target y
    int32_t targetZ;          // target z
    int32_t sysData1;            // used to keep here various system data, so user can't change it in map editor
    int32_t sysData2;            //
    int32_t scale;                   // used for scaling SEQ size on sprites
    uint32_t physAttr;         // currently used by additional physics sprites to keep it's attributes.
    uint32_t health;
    uint32_t busy;

    int16_t reference;
    int16_t data1;            // Data 1
    int16_t data2;            // Data 2
    int16_t data3;            // Data 3
    int16_t target;           // target sprite
    int16_t burnSource;
    uint16_t txID;             // TX ID
    uint16_t rxID;             // RX ID
    uint16_t command;           // Cmd
    uint16_t busyTime;         // busyTime
    uint16_t waitTime;         // waitTime
    uint16_t data4;            // Data 4
    uint16_t goalAng;          // Dude goal ang
    uint16_t burnTime;
    uint16_t height;
    uint16_t stateTimer;       // ai timer

    uint8_t respawnPending;    // respawnPending
    uint8_t dropMsg;           // Drop Item
    uint8_t key;               // Key
    uint8_t lSkill;            // Launch 12345
    uint8_t lockMsg;           // Lock msg
    int8_t dodgeDir;          // Dude dodge direction

};

struct XSECTOR {

    union
    {
        uint64_t flags;
        struct {
            unsigned int state : 1;             // State
            unsigned int triggerOn : 1;         // Send at ON
            unsigned int triggerOff : 1;        // Send at OFF
            unsigned int restState : 1;
            unsigned int interruptable : 1;     // Interruptable
            unsigned int reTriggerA : 1;        // OFF->ON wait
            unsigned int reTriggerB : 1;        // ON->OFF wait
            unsigned int shadeAlways : 1;       // Lighting shadeAlways
            unsigned int shadeFloor : 1;        // Lighting floor
            unsigned int shadeCeiling : 1;      // Lighting ceiling
            unsigned int shadeWalls : 1;        // Lighting walls
            unsigned int panAlways : 1;         // Pan always
            unsigned int panFloor : 1;          // Pan floor
            unsigned int panCeiling : 1;        // Pan ceiling
            unsigned int Drag : 1;              // Pan drag
            unsigned int Underwater : 1;        // Underwater
            unsigned int decoupled : 1;         // Decoupled
            unsigned int triggerOnce : 1;       // 1-shot
            unsigned int isTriggered : 1;
            unsigned int Push : 1;              // Push
            unsigned int Vector : 1;            // Vector
            unsigned int Reserved : 1;          // Reserved
            unsigned int Enter : 1;             // Enter
            unsigned int Exit : 1;              // Exit
            unsigned int Wallpush : 1;          // WallPush
            unsigned int color : 1;             // Color Lights
            unsigned int stopOn : 1;
            unsigned int stopOff : 1;
            unsigned int Crush : 1;             // Crush
            unsigned int locked : 1;            // Locked
            unsigned int windAlways : 1;        // Wind always
            unsigned int dudeLockout : 1;
            unsigned int bobAlways : 1;         // Motion always
            unsigned int bobFloor : 1;          // Motion bob floor
            unsigned int bobCeiling : 1;        // Motion bob ceiling
            unsigned int bobRotate : 1;         // Motion rotate

        };
    };

    uint32_t busy;
    int32_t offCeilZ;
    int32_t onCeilZ;
    int32_t offFloorZ;
    int32_t onFloorZ;
    uint32_t windVel;          // Wind vel (changed from 10 bit to use higher velocity values)

    uint16_t reference;
    uint16_t data;             // Data
    uint16_t txID;             // TX ID
    uint16_t rxID;             // RX ID
    uint16_t busyTimeA;        // OFF->ON busyTime
    uint16_t waitTimeA;        // OFF->ON waitTime
    uint16_t panAngle;         // Motion angle
    uint16_t busyTimeB;        // ON->OFF busyTime
    uint16_t waitTimeB;        // ON->OFF waitTime
    uint16_t marker0;
    uint16_t marker1;
    uint16_t windAng;          // Wind ang
    uint16_t bobTheta;         // Motion Theta
    int16_t bobSpeed;           // Motion speed

    uint8_t busyWaveA;         // OFF->ON wave
    uint8_t busyWaveB;         // ON->OFF wave
    uint8_t command;           // Cmd
    int8_t amplitude;           // Lighting amplitude
    uint8_t freq;              // Lighting freq
    uint8_t phase;             // Lighting phase
    uint8_t wave;              // Lighting wave
    int8_t shade;               // Lighting value
    uint8_t panVel;            // Motion speed
    uint8_t Depth;             // Depth
    uint8_t Key;               // Key
    uint8_t ceilpal;           // Ceil pal2
    uint8_t damageType;        // DamageType
    uint8_t floorpal;          // Floor pal2
    uint8_t bobZRange;         // Motion Z range
};

struct XWALL {

    union
    {
        uint32_t flags;
        struct {
            unsigned int state : 1;             // State
            unsigned int triggerOn : 1;         // going ON
            unsigned int triggerOff : 1;        // going OFF
            unsigned int restState : 1;         // restState
            unsigned int interruptable : 1;     // Interruptable
            unsigned int panAlways : 1;         // panAlways
            unsigned int decoupled : 1;         // Decoupled
            unsigned int triggerOnce : 1;       // 1-shot
            unsigned int isTriggered : 1;
            unsigned int triggerPush : 1;       // Push
            unsigned int triggerVector : 1;     // Vector
            unsigned int triggerTouch : 1;      // by NoOne: renamed from Reserved to Touch as it works with Touch now.
            unsigned int locked : 1;            // Locked
            unsigned int dudeLockout : 1;       // DudeLockout
        };
    };
    uint32_t busy;

    int16_t reference;
    int16_t data;               // Data
    uint16_t txID;             // TX ID
    uint16_t rxID;             // RX ID
    uint16_t busyTime;         // busyTime
    uint16_t waitTime;         // waitTime

    uint8_t command;           // Cmd
    int8_t panXVel;           // panX
    int8_t panYVel;           // panY
    uint8_t key;               // Key
};

extern XSECTOR xsector[4096];
extern XWALL xwall[16384];
extern XSPRITE xsprite[16384];

#pragma pack(push, 1)
struct MAPHEADER {
    int x; // x
    int y; // y
    int z; // z
    short ang; // ang
    short sect; // sect
    short pskybits; // pskybits
    int visibility; // visibility
    int mattid; // song id, Matt
    char parallax; // parallaxtype
    int revision; // map revision
    short numsectors; // numsectors
    short numwalls; // numwalls
    short numsprites; // numsprites
};

struct MAPHEADER2 {
    char name[64];
    int numxsprites; // xsprite size
    int numxwalls; // xwall size
    int numxsectors; // xsector size
    char pad[52];
};

const int nXSectorSize = 60;
const int nXSpriteSize = 56;
const int nXWallSize = 24;

#pragma pack(pop)

#endif // buildtypes_h__
