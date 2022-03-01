
/* Copyright (c) 2020, Peter Barrett
**
** Permission to use, copy, modify, and/or distribute this software for
** any purpose with or without fee is hereby granted, provided that the
** above copyright notice and this permission notice appear in all copies.
**
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
** SOFTWARE.
*/

/**
 * Change log:
 *
 * Oct 2021 - Original code from https://github.com/rossumur/esp_8_bit
 *            Modified by Marcio Teixeira to extract Atari framebuffer
 *            and make it compatible with Bitluni's code. I don't
 *            really understand this code very well, but there are
 *            some implementation details in the esp_8_bit repo.
 */

#define VIDEO_PIN   26
#define AUDIO_PIN   18  // can be any pin
//#define IR_PIN      0   // TSOP4838 or equivalent on any pin if desired

#include "esp_types.h"
#include "esp_heap_caps.h"
#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "esp_err.h"
#include "soc/gpio_reg.h"
#include "soc/rtc.h"
#include "soc/soc.h"
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "soc/ledc_struct.h"
#include "soc/rtc_io_reg.h"
#include "soc/io_mux_reg.h"
#include "rom/gpio.h"
#include "rom/lldesc.h"
#include "driver/periph_ctrl.h"
#include "driver/dac.h"
#include "driver/gpio.h"
#include "driver/i2s.h"

#if !defined(SUPPORT_NTSC) && !defined(SUPPORT_PAL)
    #define SUPPORT_NTSC 1
    #define SUPPORT_PAL 1
#endif

#ifdef IR_PIN
#include "ir_input.h"  // ir peripherals
#endif

#define COLORBURST 1

namespace RawCompositeVideoBlitter {

enum VideoStandard {NTSC, PAL};

int _pal_ = 0;

//====================================================================================================
//====================================================================================================
// Color palettes
//

const static uint32_t atari_palette_rgb[256] = {
    0x00000000,0x000F0F0F,0x001B1B1B,0x00272727,0x00333333,0x00414141,0x004F4F4F,0x005E5E5E,
    0x00686868,0x00787878,0x00898989,0x009A9A9A,0x00ABABAB,0x00BFBFBF,0x00D3D3D3,0x00EAEAEA,
    0x00001600,0x000F2100,0x001A2D00,0x00273900,0x00334500,0x00405300,0x004F6100,0x005D7000,
    0x00687A00,0x00778A17,0x00899B29,0x009AAC3B,0x00ABBD4C,0x00BED160,0x00D2E574,0x00E9FC8B,
    0x001C0000,0x00271300,0x00331F00,0x003F2B00,0x004B3700,0x00594500,0x00675300,0x00756100,
    0x00806C12,0x008F7C22,0x00A18D34,0x00B29E45,0x00C3AF56,0x00D6C36A,0x00EAD77E,0x00FFEE96,
    0x002F0000,0x003A0000,0x00460F00,0x00521C00,0x005E2800,0x006C3600,0x007A4416,0x00885224,
    0x00925D2F,0x00A26D3F,0x00B37E50,0x00C48F62,0x00D6A073,0x00E9B487,0x00FDC89B,0x00FFDFB2,
    0x00390000,0x00440000,0x0050000A,0x005C0F17,0x00681B23,0x00752931,0x0084373F,0x0092464E,
    0x009C5058,0x00AC6068,0x00BD7179,0x00CE838A,0x00DF949C,0x00F2A7AF,0x00FFBBC3,0x00FFD2DA,
    0x00370020,0x0043002C,0x004E0037,0x005A0044,0x00661350,0x0074215D,0x0082306C,0x00903E7A,
    0x009B4984,0x00AA5994,0x00BC6AA5,0x00CD7BB6,0x00DE8CC7,0x00F1A0DB,0x00FFB4EF,0x00FFCBFF,
    0x002B0047,0x00360052,0x0042005E,0x004E006A,0x005A1276,0x00672083,0x00762F92,0x00843DA0,
    0x008E48AA,0x009E58BA,0x00AF69CB,0x00C07ADC,0x00D18CED,0x00E59FFF,0x00F9B3FF,0x00FFCAFF,
    0x0016005F,0x0021006A,0x002D0076,0x00390C82,0x0045198D,0x0053279B,0x006135A9,0x006F44B7,
    0x007A4EC2,0x008A5ED1,0x009B6FE2,0x00AC81F3,0x00BD92FF,0x00D0A5FF,0x00E4B9FF,0x00FBD0FF,
    0x00000063,0x0000006F,0x00140C7A,0x00201886,0x002C2592,0x003A329F,0x004841AE,0x00574FBC,
    0x00615AC6,0x00716AD6,0x00827BE7,0x00948CF8,0x00A59DFF,0x00B8B1FF,0x00CCC5FF,0x00E3DCFF,
    0x00000054,0x00000F5F,0x00001B6A,0x00002776,0x00153382,0x00234190,0x0031509E,0x00405EAC,
    0x004A68B6,0x005A78C6,0x006B89D7,0x007D9BE8,0x008EACF9,0x00A1BFFF,0x00B5D3FF,0x00CCEAFF,
    0x00001332,0x00001E3E,0x00002A49,0x00003655,0x00004261,0x0012506F,0x00205E7D,0x002F6D8B,
    0x00397796,0x004987A6,0x005B98B7,0x006CA9C8,0x007DBAD9,0x0091CEEC,0x00A5E2FF,0x00BCF9FF,
    0x00001F00,0x00002A12,0x0000351E,0x0000422A,0x00004E36,0x000B5B44,0x00196A53,0x00287861,
    0x0033826B,0x0043927B,0x0054A38C,0x0065B49E,0x0077C6AF,0x008AD9C2,0x009EEDD6,0x00B5FFED,
    0x00002400,0x00003000,0x00003B00,0x00004700,0x0000530A,0x00106118,0x001E6F27,0x002D7E35,
    0x00378840,0x00479850,0x0059A961,0x006ABA72,0x007BCB84,0x008FDE97,0x00A3F2AB,0x00BAFFC2,
    0x00002300,0x00002F00,0x00003A00,0x00004600,0x00115200,0x001F6000,0x002E6E00,0x003C7C12,
    0x0047871C,0x0057972D,0x0068A83E,0x0079B94F,0x008ACA61,0x009EDD74,0x00B2F189,0x00C9FFA0,
    0x00001B00,0x00002700,0x000F3200,0x001C3E00,0x00284A00,0x00365800,0x00446600,0x00527500,
    0x005D7F00,0x006D8F19,0x007EA02B,0x008FB13D,0x00A0C24E,0x00B4D662,0x00C8EA76,0x00DFFF8D,
    0x00110E00,0x001D1A00,0x00292500,0x00353100,0x00413D00,0x004F4B00,0x005D5A00,0x006B6800,
    0x0076720B,0x0085821B,0x0097932D,0x00A8A43E,0x00B9B650,0x00CCC963,0x00E0DD77,0x00F7F48F,
};

// swizzed ntsc palette in RAM
const static DRAM_ATTR uint32_t atari_4_phase_ntsc[256] = {
    0x18181818,0x1A1A1A1A,0x1C1C1C1C,0x1F1F1F1F,0x21212121,0x24242424,0x27272727,0x2A2A2A2A,
    0x2D2D2D2D,0x30303030,0x34343434,0x38383838,0x3B3B3B3B,0x40404040,0x44444444,0x49494949,
    0x1A15210E,0x1C182410,0x1E1A2612,0x211D2915,0x231F2B18,0x26222E1A,0x2925311D,0x2C283420,
    0x2F2B3723,0x322E3A27,0x36323E2A,0x3A36412E,0x3D394532,0x423D4936,0x46424E3A,0x4B46523F,
    0x151A210E,0x171D2310,0x191F2613,0x1C222815,0x1E242B18,0x21272E1B,0x242A311D,0x272D3420,
    0x2A303724,0x2E333A27,0x31373D2A,0x353A412E,0x393E4532,0x3D424936,0x41474D3A,0x464B523F,
    0x101F1F10,0x13212113,0x15232315,0x18262618,0x1A28281A,0x1D2B2B1D,0x202E2E20,0x23313123,
    0x26343426,0x29383729,0x2D3B3B2D,0x303F3F31,0x34434234,0x38474738,0x3D4B4B3D,0x41505041,
    0x0E211A15,0x10231D17,0x13261F19,0x1528221C,0x182B241F,0x1B2E2721,0x1D312A24,0x20342D27,
    0x2337302A,0x273A332E,0x2A3E3731,0x2E413A35,0x32453E39,0x3649423D,0x3A4D4741,0x3F524B46,
    0x0E21151A,0x1024181C,0x12261A1E,0x15291D21,0x182B1F24,0x1A2E2226,0x1D312529,0x2034282C,
    0x23372B2F,0x273A2E33,0x2A3E3236,0x2E41353A,0x3245393E,0x36493D42,0x3A4E4246,0x3F52464B,
    0x101F111E,0x12211320,0x15241623,0x17261825,0x1A291B28,0x1D2C1E2B,0x202F202E,0x23322331,
    0x26352634,0x29382A37,0x2C3B2D3B,0x303F313E,0x34433542,0x38473946,0x3C4B3D4A,0x4150424F,
    0x141B0E21,0x161D1023,0x19201326,0x1B221528,0x1E25182B,0x21281B2E,0x242A1E30,0x272E2133,
    0x2A312436,0x2D34273A,0x30372B3D,0x343B2E41,0x383F3245,0x3C433649,0x40473A4D,0x454C3F52,
    0x19160E21,0x1B181024,0x1E1B1226,0x201D1529,0x2320172B,0x26231A2E,0x29261D31,0x2C292034,
    0x2F2C2337,0x322F273A,0x35322A3E,0x39362E41,0x3D3A3245,0x413E3649,0x45423A4E,0x4A473F52,
    0x1E11101F,0x20141222,0x22161424,0x25191727,0x271B1929,0x2A1E1C2C,0x2D211F2F,0x30242232,
    0x33272535,0x362A2838,0x3A2E2C3C,0x3E323040,0x41353343,0x46393847,0x4A3E3C4C,0x4F424150,
    0x210E131C,0x2311161E,0x25131820,0x28161B23,0x2A181D26,0x2D1B2028,0x301E232B,0x3321262E,
    0x36242931,0x3A272C35,0x3D2B3038,0x412E333C,0x45323740,0x49363B44,0x4D3B4048,0x523F444D,
    0x210E1817,0x24101B19,0x26121D1B,0x29151F1E,0x2B172221,0x2E1A2523,0x311D2826,0x34202B29,
    0x37232E2C,0x3A263130,0x3E2A3533,0x422E3837,0x45313C3B,0x4936403F,0x4E3A4543,0x523F4948,
    0x200F1D12,0x22111F14,0x25142217,0x27162419,0x2A19271C,0x2D1C2A1F,0x2F1F2C22,0x32222F25,
    0x35253328,0x3928362B,0x3C2C392F,0x402F3D32,0x44334136,0x4837453A,0x4C3B493E,0x51404E43,
    0x1C13200F,0x1F152311,0x21172513,0x241A2816,0x261D2A19,0x291F2D1B,0x2C22301E,0x2F253321,
    0x32283624,0x352C3928,0x392F3D2B,0x3C33402F,0x40374433,0x443B4837,0x493F4D3B,0x4D445140,
    0x1818220E,0x1A1A2410,0x1C1C2612,0x1F1F2915,0x21212B17,0x24242E1A,0x2727311D,0x2A2A3420,
    0x2D2D3723,0x30303A26,0x34343E2A,0x3838422E,0x3B3B4531,0x40404A36,0x44444E3A,0x4949533F,
    0x131C200F,0x151F2311,0x17212513,0x1A242816,0x1D262A19,0x1F292D1B,0x222C301E,0x252F3321,
    0x28323624,0x2C353928,0x2F393D2B,0x333C402F,0x37404433,0x3B444837,0x3F494D3B,0x444D5140,
};

const static DRAM_ATTR uint32_t atari_4_phase_pal[] = {
    0x18181818,0x1B1B1B1B,0x1E1E1E1E,0x21212121,0x25252525,0x28282828,0x2B2B2B2B,0x2E2E2E2E,
    0x32323232,0x35353535,0x38383838,0x3B3B3B3B,0x3F3F3F3F,0x42424242,0x45454545,0x49494949,
    0x16271A09,0x192A1D0C,0x1C2D200F,0x1F302312,0x23342716,0x26372A19,0x293A2D1C,0x2C3D301F,
    0x30413423,0x33443726,0x36473A29,0x394A3D2C,0x3D4E4130,0x40514433,0x43544736,0x47584B3A,
    0x0F24210C,0x1227240F,0x152A2712,0x182D2A15,0x1C312E19,0x1F34311C,0x2237341F,0x253A3722,
    0x293E3B26,0x2C413E29,0x2F44412C,0x3247442F,0x364B4833,0x394E4B36,0x3C514E39,0x4055523D,
    0x0B1F2511,0x0E222814,0x11252B17,0x14282E1A,0x182C321E,0x1B2F3521,0x1E323824,0x21353B27,
    0x25393F2B,0x283C422E,0x2B3F4531,0x2E424834,0x32464C38,0x35494F3B,0x384C523E,0x3C505642,
    0x09182718,0x0C1B2A1B,0x0F1E2D1E,0x12213021,0x16253425,0x19283728,0x1C2B3A2B,0x1F2E3D2E,
    0x23324132,0x26354435,0x29384738,0x2C3B4A3B,0x303F4E3F,0x33425142,0x36455445,0x3A495849,
    0x0B11251F,0x0E142822,0x11172B25,0x141A2E28,0x181E322C,0x1B21352F,0x1E243832,0x21273B35,
    0x252B3F39,0x282E423C,0x2B31453F,0x2E344842,0x32384C46,0x353B4F49,0x383E524C,0x3C425650,
    0x0F0C2124,0x120F2427,0x1512272A,0x18152A2D,0x1C192E31,0x1F1C3134,0x221F3437,0x2522373A,
    0x29263B3E,0x2C293E41,0x2F2C4144,0x322F4447,0x3633484B,0x39364B4E,0x3C394E51,0x403D5255,
    0x15091B27,0x180C1E2A,0x1B0F212D,0x1E122430,0x22162834,0x25192B37,0x281C2E3A,0x2B1F313D,
    0x2F233541,0x32263844,0x35293B47,0x382C3E4A,0x3C30424E,0x3F334551,0x42364854,0x463A4C58,
    0x1C0A1426,0x1F0D1729,0x22101A2C,0x25131D2F,0x29172133,0x2C1A2436,0x2F1D2739,0x32202A3C,
    0x36242E40,0x39273143,0x3C2A3446,0x3F2D3749,0x43313B4D,0x46343E50,0x49374153,0x4D3B4557,
    0x220D0E23,0x25101126,0x28131429,0x2B16172C,0x2F1A1B30,0x321D1E33,0x35202136,0x38232439,
    0x3C27283D,0x3F2A2B40,0x422D2E43,0x45303146,0x4934354A,0x4C37384D,0x4F3A3B50,0x533E3F54,
    0x26130A1D,0x29160D20,0x2C191023,0x2F1C1326,0x3320172A,0x36231A2D,0x39261D30,0x3C292033,
    0x402D2437,0x4330273A,0x46332A3D,0x49362D40,0x4D3A3144,0x503D3447,0x5340374A,0x57443B4E,
    0x271A0916,0x2A1D0C19,0x2D200F1C,0x3023121F,0x34271623,0x372A1926,0x3A2D1C29,0x3D301F2C,
    0x41342330,0x44372633,0x473A2936,0x4A3D2C39,0x4E41303D,0x51443340,0x54473643,0x584B3A47,
    0x24200C10,0x27230F13,0x2A261216,0x2D291519,0x312D191D,0x34301C20,0x37331F23,0x3A362226,
    0x3E3A262A,0x413D292D,0x44402C30,0x47432F33,0x4B473337,0x4E4A363A,0x514D393D,0x55513D41,
    0x1F25110B,0x2228140E,0x252B1711,0x282E1A14,0x2C321E18,0x2F35211B,0x3238241E,0x353B2721,
    0x393F2B25,0x3C422E28,0x3F45312B,0x4248342E,0x464C3832,0x494F3B35,0x4C523E38,0x5056423C,
    0x19271709,0x1C2A1A0C,0x1F2D1D0F,0x22302012,0x26342416,0x29372719,0x2C3A2A1C,0x2F3D2D1F,
    0x33413123,0x36443426,0x39473729,0x3C4A3A2C,0x404E3E30,0x43514133,0x46544436,0x4A58483A,
    0x12261E0A,0x1529210D,0x182C2410,0x1B2F2713,0x1F332B17,0x22362E1A,0x2539311D,0x283C3420,
    0x2C403824,0x2F433B27,0x32463E2A,0x3549412D,0x394D4531,0x3C504834,0x3F534B37,0x43574F3B,
    //odd
    0x18181818,0x1B1B1B1B,0x1E1E1E1E,0x21212121,0x25252525,0x28282828,0x2B2B2B2B,0x2E2E2E2E,
    0x32323232,0x35353535,0x38383838,0x3B3B3B3B,0x3F3F3F3F,0x42424242,0x45454545,0x49494949,
    0x1A271609,0x1D2A190C,0x202D1C0F,0x23301F12,0x27342316,0x2A372619,0x2D3A291C,0x303D2C1F,
    0x34413023,0x37443326,0x3A473629,0x3D4A392C,0x414E3D30,0x44514033,0x47544336,0x4B58473A,
    0x21240F0C,0x2427120F,0x272A1512,0x2A2D1815,0x2E311C19,0x31341F1C,0x3437221F,0x373A2522,
    0x3B3E2926,0x3E412C29,0x41442F2C,0x4447322F,0x484B3633,0x4B4E3936,0x4E513C39,0x5255403D,
    0x251F0B11,0x28220E14,0x2B251117,0x2E28141A,0x322C181E,0x352F1B21,0x38321E24,0x3B352127,
    0x3F39252B,0x423C282E,0x453F2B31,0x48422E34,0x4C463238,0x4F49353B,0x524C383E,0x56503C42,
    0x27180918,0x2A1B0C1B,0x2D1E0F1E,0x30211221,0x34251625,0x37281928,0x3A2B1C2B,0x3D2E1F2E,
    0x41322332,0x44352635,0x47382938,0x4A3B2C3B,0x4E3F303F,0x51423342,0x54453645,0x58493A49,
    0x25110B1F,0x28140E22,0x2B171125,0x2E1A1428,0x321E182C,0x35211B2F,0x38241E32,0x3B272135,
    0x3F2B2539,0x422E283C,0x45312B3F,0x48342E42,0x4C383246,0x4F3B3549,0x523E384C,0x56423C50,
    0x210C0F24,0x240F1227,0x2712152A,0x2A15182D,0x2E191C31,0x311C1F34,0x341F2237,0x3722253A,
    0x3B26293E,0x3E292C41,0x412C2F44,0x442F3247,0x4833364B,0x4B36394E,0x4E393C51,0x523D4055,
    0x1B091527,0x1E0C182A,0x210F1B2D,0x24121E30,0x28162234,0x2B192537,0x2E1C283A,0x311F2B3D,
    0x35232F41,0x38263244,0x3B293547,0x3E2C384A,0x42303C4E,0x45333F51,0x48364254,0x4C3A4658,
    0x140A1C26,0x170D1F29,0x1A10222C,0x1D13252F,0x21172933,0x241A2C36,0x271D2F39,0x2A20323C,
    0x2E243640,0x31273943,0x342A3C46,0x372D3F49,0x3B31434D,0x3E344650,0x41374953,0x453B4D57,
    0x0E0D2223,0x11102526,0x14132829,0x17162B2C,0x1B1A2F30,0x1E1D3233,0x21203536,0x24233839,
    0x28273C3D,0x2B2A3F40,0x2E2D4243,0x31304546,0x3534494A,0x38374C4D,0x3B3A4F50,0x3F3E5354,
    0x0A13261D,0x0D162920,0x10192C23,0x131C2F26,0x1720332A,0x1A23362D,0x1D263930,0x20293C33,
    0x242D4037,0x2730433A,0x2A33463D,0x2D364940,0x313A4D44,0x343D5047,0x3740534A,0x3B44574E,
    0x091A2716,0x0C1D2A19,0x0F202D1C,0x1223301F,0x16273423,0x192A3726,0x1C2D3A29,0x1F303D2C,
    0x23344130,0x26374433,0x293A4736,0x2C3D4A39,0x30414E3D,0x33445140,0x36475443,0x3A4B5847,
    0x0C202410,0x0F232713,0x12262A16,0x15292D19,0x192D311D,0x1C303420,0x1F333723,0x22363A26,
    0x263A3E2A,0x293D412D,0x2C404430,0x2F434733,0x33474B37,0x364A4E3A,0x394D513D,0x3D515541,
    0x11251F0B,0x1428220E,0x172B2511,0x1A2E2814,0x1E322C18,0x21352F1B,0x2438321E,0x273B3521,
    0x2B3F3925,0x2E423C28,0x31453F2B,0x3448422E,0x384C4632,0x3B4F4935,0x3E524C38,0x4256503C,
    0x17271909,0x1A2A1C0C,0x1D2D1F0F,0x20302212,0x24342616,0x27372919,0x2A3A2C1C,0x2D3D2F1F,
    0x31413323,0x34443626,0x37473929,0x3A4A3C2C,0x3E4E4030,0x41514333,0x44544636,0x48584A3A,
    0x1E26120A,0x2129150D,0x242C1810,0x272F1B13,0x2B331F17,0x2E36221A,0x3139251D,0x343C2820,
    0x38402C24,0x3B432F27,0x3E46322A,0x4149352D,0x454D3931,0x48503C34,0x4B533F37,0x4F57433B,
};

const uint32_t* ntsc_palette() {return atari_4_phase_ntsc;};
const uint32_t* pal_palette() {return atari_4_phase_pal;}

//====================================================================================================
//====================================================================================================
// Frame buffer
//
// (The following code isn't actually used when combined with bitluni's CompositeGraphics code)

void* fb_malloc(size_t size, const char* label, uint32_t caps) {
    printf("fb_malloc %d free, %d biggest, allocating %s:%d\n",
      heap_caps_get_free_size(caps), heap_caps_get_largest_free_block(caps), label, size);
    void *r = heap_caps_malloc(size, caps);
    if (!r) {
        printf("fb_malloc failed allocation of %s:%d!\n", label, size);
        esp_restart();
    }
    else
        printf("fb_malloc allocation of %s:%d %08X\n", label, size, r);
    return r;
}

constexpr unsigned int Screen_WIDTH = 336;
constexpr unsigned int Screen_HEIGHT = 240;

uint32_t  * _fb;
uint8_t  ** _lines;

void frame_clear(uint8_t color)
{
    uint32_t color_x4 = (color << 24) | (color << 16) | (color << 8) | (color << 0);
    int i = Screen_WIDTH*Screen_HEIGHT/4;
    while (i--)
        _fb[i] = color_x4;
}

/**
 * Creates a framebuffer. The caps argument can be one of the following:
 *
 *   MALLOC_CAP_8BIT   - Accessible in single bytes
 *   MALLOC_CAP_32BIT  - All access must be 32-bit aligned
 *
 * The latter allows the ESP32 to store the data in IRAM (instruction RAM),
 * but makes access to individual bytes more difficult.
 */
void frame_init(uint32_t caps = MALLOC_CAP_8BIT) {
    _fb    = (uint32_t*) fb_malloc(Screen_WIDTH * Screen_HEIGHT,     "_fb",    caps);
    _lines = (uint8_t**) fb_malloc(Screen_HEIGHT * sizeof(uint8_t*), "_lines", MALLOC_CAP_32BIT);

    uint8_t* s = (uint8_t*)_fb;
    for (int y = 0; y < Screen_HEIGHT; y++) {
        _lines[y] = s;
        s += Screen_WIDTH;
    }
    frame_clear(0x00);
}

//====================================================================================================
//====================================================================================================
//
// low level HW setup of DAC/DMA/APLL/PWM
//

lldesc_t _dma_desc[4] = {0};
intr_handle_t _isr_handle;

extern "C"
void IRAM_ATTR video_isr(volatile void* buf);

// simple isr
static void IRAM_ATTR i2s_intr_handler_video(void *arg) {
    if (I2S0.int_st.out_eof)
        video_isr(((lldesc_t*)I2S0.out_eof_des_addr)->buf); // get the next line of video
    I2S0.int_clr.val = I2S0.int_st.val;                     // reset the interrupt
}

static esp_err_t start_dma(int line_width,int samples_per_cc, int ch = 1)
{
    periph_module_enable(PERIPH_I2S0_MODULE);

    // setup interrupt
    if (esp_intr_alloc(ETS_I2S0_INTR_SOURCE, ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_INTRDISABLED,
        i2s_intr_handler_video, 0, &_isr_handle) != ESP_OK)
        return -1;

    // reset conf
    I2S0.conf.val = 1;
    I2S0.conf.val = 0;
    I2S0.conf.tx_right_first = 1;
    I2S0.conf.tx_mono = (ch == 2 ? 0 : 1);

    I2S0.conf2.lcd_en = 1;
    I2S0.fifo_conf.tx_fifo_mod_force_en = 1;
    I2S0.sample_rate_conf.tx_bits_mod = 16;
    I2S0.conf_chan.tx_chan_mod = (ch == 2) ? 0 : 1;

    // Create TX DMA buffers
    for (int i = 0; i < 2; i++) {
        int n = line_width*2*ch;
        if (n >= 4092) {
            printf("DMA chunk too big:%s\n",n);
            return -1;
        }
        _dma_desc[i].buf = (uint8_t*)heap_caps_calloc(1, n, MALLOC_CAP_DMA);
        if (!_dma_desc[i].buf)
            return -1;
        
        _dma_desc[i].owner = 1;
        _dma_desc[i].eof = 1;
        _dma_desc[i].length = n;
        _dma_desc[i].size = n;
        _dma_desc[i].empty = (uint32_t)(i == 1 ? _dma_desc : _dma_desc+1);
    }
    I2S0.out_link.addr = (uint32_t)_dma_desc;

    //  Setup up the apll: See ref 3.2.7 Audio PLL
    //  f_xtal = (int)rtc_clk_xtal_freq_get() * 1000000;
    //  f_out = xtal_freq * (4 + sdm2 + sdm1/256 + sdm0/65536); // 250 < f_out < 500
    //  apll_freq = f_out/((o_div + 2) * 2)
    //  operating range of the f_out is 250 MHz ~ 500 MHz
    //  operating range of the apll_freq is 16 ~ 128 MHz.
    //  select sdm0,sdm1,sdm2 to produce nice multiples of colorburst frequencies

    //  see calc_freq() for math: (4+a)*10/((2 + b)*2) mhz
    //  up to 20mhz seems to work ok:
    //  rtc_clk_apll_enable(1,0x00,0x00,0x4,0);   // 20mhz for fancy DDS

    #if SUPPORT_NTSC
        if (!_pal_) {
            switch (samples_per_cc) {
                case 3: rtc_clk_apll_enable(1,0x46,0x97,0x4,2);   break;    // 10.7386363636 3x NTSC (10.7386398315mhz)
                case 4: rtc_clk_apll_enable(1,0x46,0x97,0x4,1);   break;    // 14.3181818182 4x NTSC (14.3181864421mhz)
            }
        }
    #endif
    #if SUPPORT_PAL
        if (_pal_) {
            rtc_clk_apll_enable(1,0x04,0xA4,0x6,1);     // 17.734476mhz ~4x PAL
        }
    #endif

    I2S0.clkm_conf.clkm_div_num = 1;            // I2S clock divider’s integral value.
    I2S0.clkm_conf.clkm_div_b = 0;              // Fractional clock divider’s numerator value.
    I2S0.clkm_conf.clkm_div_a = 1;              // Fractional clock divider’s denominator value
    I2S0.sample_rate_conf.tx_bck_div_num = 1;
    I2S0.clkm_conf.clka_en = 1;                 // Set this bit to enable clk_apll.
    I2S0.fifo_conf.tx_fifo_mod = (ch == 2) ? 0 : 1; // 32-bit dual or 16-bit single channel data

    dac_output_enable(DAC_CHANNEL_1);           // DAC, video on GPIO25
    dac_i2s_enable();                           // start DAC!

    I2S0.conf.tx_start = 1;                     // start DMA!
    I2S0.int_clr.val = 0xFFFFFFFF;
    I2S0.int_ena.out_eof = 1;
    I2S0.out_link.start = 1;
    return esp_intr_enable(_isr_handle);        // start interruprs!
}

void video_init_hw(int line_width, int samples_per_cc)
{
    // setup apll 4x NTSC or PAL colorburst rate
    start_dma(line_width,samples_per_cc,1);

    // Now ideally we would like to use the decoupled left DAC channel to produce audio
    // But when using the APLL there appears to be some clock domain conflict that causes
    // nasty digitial spikes and dropouts. You are also limited to a single audio channel.
    // So it is back to PWM/PDM and a 1 bit DAC for us. Good news is that we can do stereo
    // if we want to and have lots of different ways of doing nice noise shaping etc.

    // PWM audio out of pin 18 -> can be anything
    // lots of other ways, PDM by hand over I2S1, spi circular buffer etc
    // but if you would like stereo the led pwm seems like a fine choice
    // needs a simple rc filter (1k->1.2k resistor & 10nf->15nf cap work fine)

    // 18 ----/\/\/\/----|------- a out
    //          1k       |
    //                  ---
    //                  --- 10nf
    //                   |
    //                   v gnd

    //  IR input if used
#ifdef IR_PIN
    pinMode(IR_PIN,INPUT);
#endif
}

#if SUPPORT_AUDIO
    ledcSetup(0,2000000,7);    // 625000 khz is as fast as we go w 7 bits
    ledcAttachPin(AUDIO_PIN, 0);
    ledcWrite(0,0);

    // send an audio sample every scanline (15720hz for ntsc, 15600hz for PAL)
    inline void IRAM_ATTR audio_sample(uint8_t s)
    {
        auto& reg = LEDC.channel_group[0].channel[0];
        reg.duty.duty = s << 4; // 25 bit (21.4)
        reg.conf0.sig_out_en = 1; // This is the output enable control bit for channel
        reg.conf1.duty_start = 1; // When duty_num duty_cycle and duty_scale has been configured. these register won't take effect until set duty_start. this bit is automatically cleared by hardware
        reg.conf0.clk_en = 1;
    }
#endif

//  Appendix

/*
static
void calc_freq(double f)
{
    f /= 1000000;
    printf("looking for sample rate of %fmhz\n",(float)f);
    int xtal_freq = 40;
    for (int o_div = 0; o_div < 3; o_div++) {
        float f_out = 4*f*((o_div + 2)*2);          // 250 < f_out < 500
        if (f_out < 250 || f_out > 500)
            continue;
        int sdm = round((f_out/xtal_freq - 4)*65536);
        float apll_freq = 40 * (4 + (float)sdm/65536)/((o_div + 2)*2);    // 16 < apll_freq < 128 MHz
        if (apll_freq < 16 || apll_freq > 128)
            continue;
        printf("f_out:%f %d:0x%06X %fmhz %f\n",f_out,o_div,sdm,apll_freq/4,f/(apll_freq/4));
    }
    printf("\n");
}

static void freqs()
{
    calc_freq(PAL_FREQUENCY*3);
    calc_freq(PAL_FREQUENCY*4);
    calc_freq(NTSC_FREQUENCY*3);
    calc_freq(NTSC_FREQUENCY*4);
    calc_freq(20000000);
}
*/

//====================================================================================================
//====================================================================================================


uint32_t cpu_ticks()
{
  return xthal_get_ccount();
}

uint32_t us() {
    return cpu_ticks()/240;
}

// Color clock frequency is 315/88 (3.57954545455)
// DAC_MHZ is 315/11 or 8x color clock
// 455/2 color clocks per line, round up to maintain phase
// HSYNCH period is 44/315*455 or 63.55555..us
// Field period is 262*44/315*455 or 16651.5555us

#define IRE(_x)          ((uint32_t)(((_x)+40)*255/3.3/147.5) << 8)   // 3.3V DAC
#define SYNC_LEVEL       IRE(-40)
#define BLANKING_LEVEL   IRE(0)
#define BLACK_LEVEL      IRE(7.5)
#define GRAY_LEVEL       IRE(50)
#define WHITE_LEVEL      IRE(100)

#define P0 (color >> 16)
#define P1 (color >> 8)
#define P2 (color)
#define P3 (color << 8)

volatile int _line_counter = 0;
volatile int _frame_counter = 0;

int _active_lines;
int _line_count;

int _line_width;
int _samples_per_cc = 4; // 3 or 4
const uint32_t* _palette;

float _sample_rate;

int _hsync;
int _hsync_long;
int _hsync_short;
int _burst_start;
int _burst_width;
int _active_start;

int16_t* _burst0 = 0; // pal bursts
int16_t* _burst1 = 0;

static int usec(float us)
{
    uint32_t r = (uint32_t)(us*_sample_rate);
    return ((r + _samples_per_cc)/(_samples_per_cc << 1))*(_samples_per_cc << 1);  // multiple of color clock, word align
}

#define NTSC_COLOR_CLOCKS_PER_SCANLINE 228       // really 227.5 for NTSC but want to avoid half phase fiddling for now
#define NTSC_FREQUENCY (315000000.0/88)
#define NTSC_LINES 262

#define PAL_COLOR_CLOCKS_PER_SCANLINE 284        // really 283.75 ?
#define PAL_FREQUENCY 4433618.75
#define PAL_LINES 312

void ntsc_init();
void pal_init();

void video_init(VideoStandard standard)
{
    _pal_ = standard == PAL;
    
    #if SUPPORT_PAL
        if (_pal_) pal_init();
    #endif
    #if SUPPORT_NTSC
        if (!_pal_) ntsc_init();
    #endif
    
    _active_lines = 240;
    video_init_hw(_line_width,_samples_per_cc);    // init the hardware
}

//===================================================================================================
//===================================================================================================
// Performance Testing

#ifdef PERF
    #define BEGIN_TIMING()  uint32_t t = cpu_ticks()
    #define END_TIMING() t = cpu_ticks() - t; _blit_ticks_min = min(_blit_ticks_min,t); _blit_ticks_max = max(_blit_ticks_max,t);
    #define ISR_BEGIN() uint32_t t = cpu_ticks()
    #define ISR_END() t = cpu_ticks() - t;_isr_us += (t+120)/240;
    uint32_t _blit_ticks_min = 0;
    uint32_t _blit_ticks_max = 0;
    uint32_t _isr_us = 0;
    void perf()
    {
      static int _next = 0;
      if (_drawn >= _next) {
        float elapsed_us = 120*1000000/(_emu->standard ? 60 : 50);
        _next = _drawn + 120;
        
        printf("frame_time:%d drawn:%d displayed:%d blit_ticks:%d->%d, isr time:%2.2f%%\n",
          _frame_time/240,_drawn,_frame_counter,_blit_ticks_min,_blit_ticks_max,(_isr_us*100)/elapsed_us);
          
        _blit_ticks_min = 0xFFFFFFFF;
        _blit_ticks_max = 0;
        _isr_us = 0;
      }
    }
#else
    #define BEGIN_TIMING()
    #define END_TIMING()
    #define ISR_BEGIN()
    #define ISR_END()
    void perf(){};
#endif

//===================================================================================================
//===================================================================================================
// PAL

#if SUPPORT_PAL
    void pal_init()
    {
        _palette = pal_palette();
        _sample_rate = PAL_FREQUENCY*_samples_per_cc/1000000.0;       // DAC rate in mhz
        _line_width = PAL_COLOR_CLOCKS_PER_SCANLINE*_samples_per_cc;
        _line_count = PAL_LINES;
        _hsync_short = usec(2);
        _hsync_long = usec(30);
        _hsync = usec(4.7);
        _burst_start = usec(5.6);
        _burst_width = (int)(10*_samples_per_cc + 4) & 0xFFFE;
        _active_start = usec(10.4);

        // make colorburst tables for even and odd lines
        _burst0 = new int16_t[_burst_width];
        _burst1 = new int16_t[_burst_width];
        float phase = 2*M_PI/2;
        for (int i = 0; i < _burst_width; i++)
        {
            _burst0[i] = BLANKING_LEVEL + sin(phase + 3*M_PI/4) * BLANKING_LEVEL/1.5;
            _burst1[i] = BLANKING_LEVEL + sin(phase - 3*M_PI/4) * BLANKING_LEVEL/1.5;
            phase += 2*M_PI/_samples_per_cc;
        }
    }

    void IRAM_ATTR blit_pal(uint8_t* src, uint16_t* dst)
    {
        const bool even = _line_counter & 1;
        const uint32_t* p = even ? _palette : _palette + 256;
        uint32_t c,color;
        uint8_t c0,c1,c2,c3,c4;
        uint8_t y1,y2,y3;

        // pal is 5/4 wider than ntsc to account for pal 288 color clocks per line vs 228 in ntsc
        // so do an ugly stretch on pixels (actually luma) to accomodate -> 384 pixels are now 240 pal color clocks wide

        const int left = 0;
        const int right = 336;
        dst += 40;
        for (int i = left; i < right; i += 4) {
            c = *((uint32_t*)(src+i));

            // make 5 colors out of 4 by interpolating y: 0000 0111 1122 2223 3333
            c0 = c;
            c1 = c >> 8;
            c3 = c >> 16;
            c4 = c >> 24;
            y1 = (((c1 & 0xF) << 1) + ((c0 + c1) & 0x1F) + 2) >> 2;    // (c0 & 0xF)*0.25 + (c1 & 0xF)*0.75;
            y2 = ((c1 + c3 + 1) >> 1) & 0xF;                           // (c1 & 0xF)*0.50 + (c2 & 0xF)*0.50;
            y3 = (((c3 & 0xF) << 1) + ((c3 + c4) & 0x1F) + 2) >> 2;    // (c2 & 0xF)*0.75 + (c3 & 0xF)*0.25;
            c1 = (c1 & 0xF0) + y1;
            c2 = (c1 & 0xF0) + y2;
            c3 = (c3 & 0xF0) + y3;

            color = p[c0];
            dst[0^1] = P0;
            dst[1^1] = P1;
            color = p[c1];
            dst[2^1] = P2;
            dst[3^1] = P3;
            color = p[c2];
            dst[4^1] = P0;
            dst[5^1] = P1;
            color = p[c3];
            dst[6^1] = P2;
            dst[7^1] = P3;
            color = p[c4];
            dst[8^1] = P0;
            dst[9^1] = P1;

            i += 4;
            c = *((uint32_t*)(src+i));
            
            // make 5 colors out of 4 by interpolating y: 0000 0111 1122 2223 3333
            c0 = c;
            c1 = c >> 8;
            c3 = c >> 16;
            c4 = c >> 24;
            y1 = (((c1 & 0xF) << 1) + ((c0 + c1) & 0x1F) + 2) >> 2;    // (c0 & 0xF)*0.25 + (c1 & 0xF)*0.75;
            y2 = ((c1 + c3 + 1) >> 1) & 0xF;                           // (c1 & 0xF)*0.50 + (c2 & 0xF)*0.50;
            y3 = (((c3 & 0xF) << 1) + ((c3 + c4) & 0x1F) + 2) >> 2;    // (c2 & 0xF)*0.75 + (c3 & 0xF)*0.25;
            c1 = (c1 & 0xF0) + y1;
            c2 = (c1 & 0xF0) + y2;
            c3 = (c3 & 0xF0) + y3;

            color = p[c0];
            dst[10^1] = P2;
            dst[11^1] = P3;
            color = p[c1];
            dst[12^1] = P0;
            dst[13^1] = P1;
            color = p[c2];
            dst[14^1] = P2;
            dst[15^1] = P3;
            color = p[c3];
            dst[16^1] = P0;
            dst[17^1] = P1;
            color = p[c4];
            dst[18^1] = P2;
            dst[19^1] = P3;
            dst += 20;
        }
    }

    void IRAM_ATTR burst_pal(uint16_t* line)
    {
        line += _burst_start;
        int16_t* b = (_line_counter & 1) ? _burst0 : _burst1;
        for (int i = 0; i < _burst_width; i += 2) {
            line[i^1] = b[i];
            line[(i+1)^1] = b[i+1];
        }
    }

    // Fancy pal non-interlace
    // http://martin.hinner.info/vga/pal.html
    void IRAM_ATTR pal_sync2(uint16_t* line, int width, int swidth)
    {
        swidth = swidth ? _hsync_long : _hsync_short;
        int i;
        for (i = 0; i < swidth; i++)
            line[i] = SYNC_LEVEL;
        for (; i < width; i++)
            line[i] = BLANKING_LEVEL;
    }

    uint8_t DRAM_ATTR _sync_type[8] = {0,0,0,3,3,2,0,0};
    void IRAM_ATTR pal_sync(uint16_t* line, int i)
    {
        uint8_t t = _sync_type[i-304];
        pal_sync2(line,_line_width/2, t & 2);
        pal_sync2(line+_line_width/2,_line_width/2, t & 1);
    }

#endif

//===================================================================================================
//===================================================================================================
// ntsc tables
// AA AA                // 2 pixels, 1 color clock - atari
// AA AB BB             // 3 pixels, 2 color clocks - nes
// AAA ABB BBC CCC      // 4 pixels, 3 color clocks - sms

// cc == 3 gives 684 samples per line, 3 samples per cc, 3 pixels for 2 cc
// cc == 4 gives 912 samples per line, 4 samples per cc, 2 pixels per cc

#if SUPPORT_NTSC
    void ntsc_init() {
        _palette = ntsc_palette();
        _sample_rate = 315.0/88 * _samples_per_cc;   // DAC rate
        _line_width = NTSC_COLOR_CLOCKS_PER_SCANLINE*_samples_per_cc;
        _line_count = NTSC_LINES;
        _hsync_long = usec(63.555-4.7);
        _active_start = usec(_samples_per_cc == 4 ? 10 : 10.5);
        _hsync = usec(4.7);
    }

    // draw a line of game in NTSC
    void IRAM_ATTR blit_ntsc(uint8_t* src, uint16_t* dst)
    {
        uint32_t* d = (uint32_t*)dst;
        const uint32_t* p = _palette;
        uint32_t c;
        int i;

        // 2 pixels per color clock, 4 samples per cc, used by atari
        // AA AA
        // 192 color clocks wide
        // only show 336 pixels
        d += 16;
        for (i = 0; i < 336; i += 4) {
            uint32_t c = *((uint32_t*)src); // screen may be in 32 bit mem
            d[0] = p[(uint8_t)c];
            d[1] = p[(uint8_t)(c>>8)] << 8;
            d[2] = p[(uint8_t)(c>>16)];
            d[3] = p[(uint8_t)(c>>24)] << 8;
            d += 4;
            src += 4;
        }
        END_TIMING();
    }
    
    void IRAM_ATTR burst_ntsc(uint16_t* line)
    {
        int i,phase;
        switch (_samples_per_cc) {
            case 4:
                // 4 samples per color clock
                for (i = _hsync; i < _hsync + (4*10); i += 4) {
                    #if COLORBURST
                        line[i+1] = BLANKING_LEVEL;
                        line[i+0] = BLANKING_LEVEL + BLANKING_LEVEL/2;
                        line[i+3] = BLANKING_LEVEL;
                        line[i+2] = BLANKING_LEVEL - BLANKING_LEVEL/2;
                    #else
                        line[i+1] = BLANKING_LEVEL;
                        line[i+0] = BLANKING_LEVEL;
                        line[i+3] = BLANKING_LEVEL;
                        line[i+2] = BLANKING_LEVEL;
                    #endif
                }
                break;
            case 3:
                // 3 samples per color clock
                phase = 0.866025*BLANKING_LEVEL/2;
                for (i = _hsync; i < _hsync + (3*10); i += 6) {
                    line[i+1] = BLANKING_LEVEL;
                    line[i+0] = BLANKING_LEVEL + phase;
                    line[i+3] = BLANKING_LEVEL - phase;
                    line[i+2] = BLANKING_LEVEL;
                    line[i+5] = BLANKING_LEVEL + phase;
                    line[i+4] = BLANKING_LEVEL - phase;
                }
                break;
        }
    }
#endif

void IRAM_ATTR blit(uint8_t* src, uint16_t* dst)
{
    BEGIN_TIMING();
    #if SUPPORT_PAL
    if (_pal_) blit_pal(src,dst);
    #endif
    #if SUPPORT_NTSC
    if (!_pal_) blit_ntsc(src,dst);
    #endif
    END_TIMING();
}

void IRAM_ATTR burst(uint16_t* line)
{
    #if SUPPORT_PAL
        if (_pal_) burst_pal(line);
    #endif
    #if SUPPORT_NTSC
        if(!_pal_) burst_ntsc(line);
    #endif
}

void IRAM_ATTR sync(uint16_t* line, int syncwidth)
{
    for (int i = 0; i < syncwidth; i++)
        line[i] = SYNC_LEVEL;
}

void IRAM_ATTR blanking(uint16_t* line, bool vbl)
{
    int syncwidth = vbl ? _hsync_long : _hsync;
    sync(line,syncwidth);
    for (int i = syncwidth; i < _line_width; i++)
        line[i] = BLANKING_LEVEL;
    if (!vbl)
        burst(line);    // no burst during vbl
}

// Wait for blanking before starting drawing
// avoids tearing in our unsynchonized world
#ifdef ESP_PLATFORM
    void video_sync()
    {
      if (!_lines)
        return;
      int n = 0;
      if (_pal_) {
        if (_line_counter < _active_lines)
          n = (_active_lines - _line_counter)*1000/15600;
      } else {
        if (_line_counter < _active_lines)
          n = (_active_lines - _line_counter)*1000/15720;
      }
      vTaskDelay(n+1);
    }
#endif

// Workhorse ISR handles audio and video updates
extern "C"
void IRAM_ATTR video_isr(volatile void* vbuf)
{
    if (!_lines)
        return;

    ISR_BEGIN();

    #if SUPPORT_AUDIO
        uint8_t s = _audio_r < _audio_w ? _audio_buffer[_audio_r++ & (sizeof(_audio_buffer)-1)] : 0x20;
        audio_sample(s);
        //audio_sample(_sin64[_x++ & 0x3F]);
    #endif

    #ifdef IR_PIN
        ir_sample();
    #endif

    int i = _line_counter++;
    uint16_t* buf = (uint16_t*)vbuf;
    #if SUPPORT_PAL
        if (_pal_) {
            // pal
            if (i < 32) {
                blanking(buf,false);                // pre render/black 0-32
            } else if (i < _active_lines + 32) {    // active video 32-272
                sync(buf,_hsync);
                burst(buf);
                blit(_lines[i-32],buf + _active_start);
            } else if (i < 304) {                   // post render/black 272-304
                if (i < 274)                        // slight optimization here, once you have 2 blanking buffers
                    blanking(buf,false);
            } else {
                pal_sync(buf,i);                    // 8 lines of sync 304-312
            }
        }
    #endif
    #if SUPPORT_NTSC
        if(!_pal_) {
            // ntsc
            if (i < _active_lines) {                // active video
                sync(buf,_hsync);
                burst(buf);
                blit(_lines[i],buf + _active_start);

            } else if (i < (_active_lines + 5)) {   // post render/black
                blanking(buf,false);

            } else if (i < (_active_lines + 8)) {   // vsync
                blanking(buf,true);

            } else {                                // pre render/black
                blanking(buf,false);
            }
        }
    #endif

    if (_line_counter == _line_count) {
        _line_counter = 0;                      // frame is done
        _frame_counter++;
    }

    ISR_END();
}

//===================================================================================================
//===================================================================================================
// sound routines from "src/gui.cpp" and "src/emu.cpp"

#if SUPPORT_AUDIO
    //  audio is buffered as 6 bit unsigned samples
    uint8_t _audio_buffer[1024];
    uint32_t _audio_r = 0;
    uint32_t _audio_w = 0;
    void audio_write_16(const int16_t* s, int len, int channels)
    {
        int b;
        while (len--) {
            if (_audio_w == (_audio_r + sizeof(_audio_buffer)))
                break;
            if (channels == 2) {
                b = (s[0] + s[1]) >> 9;
                s += 2;
            } else
                b = *s++ >> 8;
            if (b < -32) b = -32;
            if (b > 31) b = 31;
            _audio_buffer[_audio_w++ & (sizeof(_audio_buffer)-1)] = b + 32;
        }
    }
#endif

#if 0
    Emu::Emu(const char* n,int w,int h, int st, int aformat, int cc, int f) :
        name(n),width(w),height(h),standard(st),audio_format(aformat),cc_width(cc),flavor(f)
    {
        //audio_frequency = 15625; // requires fixed point sampler
        audio_frequency = standard == 1 ? 15720 : 15600;
        audio_frame_samples = standard ? (audio_frequency << 16)/60 : (audio_frequency << 16)/50;   // fixed point sampler
        audio_fraction = 0;
    }

    int audio_format = (16 | (1 << 8));

    // soft click wave soundy thing
    const uint16_t _wav[16] =  {
        0x0000,0x187D,0x2D41,0x3B20,0x3FFF,0x3B20,0x2D41,0x187D,
        0x0000,0xE783,0xD2BF,0xC4E0,0xC001,0xC4E0,0xD2BF,0xE783
    };

    void update_audio()
    {
        int16_t abuffer[313*2];
        int format = _emu->audio_format >> 8;
        int sample_count = _emu->frame_sample_count();
        if (_visible) {
            format = 1;
            if (_click) {
                _click = 0;
                for (int i = 0; i < sample_count; i++)
                    abuffer[i] = _wav[i&0xF];  // just a signed sine click
            } else
              memset(abuffer,0,sizeof(abuffer));
        } else {
            sample_count = _emu->audio_buffer(abuffer,sizeof(abuffer));
        }
        audio_write_16(abuffer,sample_count,format);
    }
#endif

} // Namespace RawCompositeVideoBlitter

//===================================================================================================
//===================================================================================================
// Glue code for Bitluni's graphics library

class CompositeColorOutput {
    public:
        float pixelAspect = 1.0;
  
        enum Mode {PAL,NTSC};
        static constexpr unsigned int XRES = RawCompositeVideoBlitter::Screen_WIDTH;
        static constexpr unsigned int YRES = RawCompositeVideoBlitter::Screen_HEIGHT;
        
        RawCompositeVideoBlitter::VideoStandard mode;
        
        CompositeColorOutput(Mode mode) : mode(mode == NTSC ? RawCompositeVideoBlitter::NTSC : RawCompositeVideoBlitter::PAL) {
        }
        
        void init() {
            //RawCompositeVideoBlitter::frame_init(); // The CompositeGraphics lib will do this for us
            RawCompositeVideoBlitter::video_init(mode);
        }
        
        void sendFrameHalfResolution(char ***frame) {
            RawCompositeVideoBlitter::_lines = (uint8_t**) *frame;
        }
};
