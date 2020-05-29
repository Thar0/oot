.include "macro.inc"

 # assembler directives
 .set noat      # allow manual use of $at
 .set noreorder # don't insert nops after branches
 .set gp=64     # allow use of 64-bit general purpose registers

.section .data

.balign 16

glabel Demo_Shd_InitVars
 .word 0x01190700, 0x00000030, 0x00010000, 0x00000154
.word DemoShd_Init
.word DemoShd_Destroy
.word DemoShd_Update
.word DemoShd_Draw
glabel D_80991680
 .word 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000005, 0x08000000, 0x00000000, 0x00000000, 0x00000524, 0x31090000, 0x00000000, 0x00000000, 0x00041443, 0x4C120001, 0x03000000, 0x00000500, 0x072D5D88, 0x7C37100D, 0x12030000, 0x00051808, 0x2479B9D7, 0xC37C4C3A, 0x2C060000, 0x00051A0D, 0x4ABEF1FC, 0xEEAE8D79, 0x4B0A0000, 0x00041612, 0x68E0FEFF, 0xF5BBB8B6, 0x6D100000, 0x00031423, 0x84EEFFFF, 0xF4C4D1DB, 0x8A1D0300, 0x00041C3C, 0xA0F4FFFE, 0xF1DAE7E9, 0x93200400, 0x00083154, 0xB6F7FFFA, 0xF4F7FBEA, 0x84140100, 0x000C4767, 0xC3F9FCEA, 0xEAFCFFED, 0x7C0F0200, 0x000D4E77, 0xCCFAFBE5, 0xE3FAFFEE, 0x7D0F0200, 0x000A3F83, 0xD6FBFEF0, 0xE4F6FEF0, 0x84110200, 0x00052688, 0xE5FDFDF0, 0xE6F9FFEF, 0x7F100200, 0x0002147E, 0xECFFFCE6, 0xDFFAFFEC, 0x710E0200, 0x00020C65, 0xE2FEFAD9, 0xCFF7FFEA, 0x660B0200, 0x00020953, 0xD2FBFADA, 0xC9F5FFEB, 0x6B0D0200, 0x00020951, 0xCBFAFAE0, 0xD4F6FFED, 0x750F0200, 0x00020C5C, 0xD4FAF1E2, 0xEBFCFFEB, 0x740F0100, 0x00010B60, 0xDEF8DDD7, 0xF6FFFEE9, 0x690D0100, 0x00010B60, 0xE2F8D7CF, 0xF1FDFFE9, 0x5F0A0100, 0x00000C63, 0xDAFAE6D4, 0xE1F1FDE7, 0x5B0A0000, 0x00000C5F, 0xC6F8F8E1, 0xDAE9FAD9, 0x52090000, 0x00000B59, 0xB4F0FBE0, 0xD1DDF4C0, 0x44070000, 0x00000A54, 0xAFEFF3C6, 0xBBC0EDB0, 0x40070000, 0x00000B57, 0xB1E8DAA4, 0xA092DCA6, 0x45080000, 0x00000C5A, 0xAEDAB480, 0x8C68BF92, 0x40080000, 0x00000C52, 0xA2C99264, 0x814FA06F, 0x23030000, 0x00010F48, 0x97BC7B4A, 0x75459760, 0x0F000000, 0x00010E41, 0x95B46B36, 0x683E9663, 0x0D000000, 0x00000941, 0x9AAD602B, 0x5A328C64, 0x0E000000, 0x00000841, 0x989E5524, 0x54347951, 0x0B000000, 0x0000073D, 0x88834C22, 0x4F3D6330, 0x07000000, 0x0000073B, 0x73624726, 0x463E4B17, 0x0B010000, 0x00000941, 0x6748482C, 0x392A3413, 0x1C050000, 0x00000A46, 0x5D303B2E, 0x3310211B, 0x2D070000, 0x00000941, 0x4B131B2C, 0x3F0D1821, 0x33080000, 0x0000093F, 0x4109092D, 0x490D0C20, 0x32080000, 0x00000A43, 0x3F09062C, 0x450B061E, 0x2B070000, 0x00000B47, 0x410A0722, 0x3108081E, 0x20040000, 0x00000838, 0x35090C19, 0x21050920, 0x20040000, 0x00000522, 0x1F060B0A, 0x1B04061E, 0x29060000, 0x00000313, 0x0B020608, 0x28070319, 0x34090000, 0x0000020E, 0x03000108, 0x2F080214, 0x2F090000, 0x0000020D, 0x03000007, 0x2407020C, 0x0B000000, 0x00000311, 0x06010001, 0x0D010413, 0x04000000, 0x0000051C, 0x0B010001, 0x0801020C, 0x02000000, 0x00000623, 0x0D010003, 0x12030002, 0x00000000, 0x0000051D, 0x0A010005, 0x1B050000, 0x00000000, 0x0000020C, 0x03000005, 0x18050000, 0x00000000, 0x00000002, 0x00000000, 0x05000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
glabel D_80991E80
 .word 0x2F3E56AB, 0xA4582102, 0x10417AB0, 0x805C312A, 0x32376DD4, 0xA2633907, 0x0B387DA2, 0x6D81470C, 0x3C5C60AC, 0xA5571801, 0x154C97B9, 0x68533236, 0x304276D6, 0x9B533108, 0x154A96B7, 0x7C9B400B, 0x3E756FAE, 0xAF5D1301, 0x1D59B4BF, 0x53432C38, 0x2F487DCC, 0x8A4A2406, 0x1F5DB0C8, 0x90B55110, 0x408B7FAA, 0xB76D1503, 0x286ACCBE, 0x43312030, 0x2B4C82BA, 0x77431607, 0x2B70C7D5, 0xA6B95B13, 0x409C93A6, 0xB87D1D08, 0x367FE1C1, 0x3C261629, 0x2A508AAB, 0x69410C0C, 0x3986DDDB, 0xB5AE6316, 0x40A0A7A1, 0xAA7D2610, 0x4596F1CD, 0x47281E36, 0x365C97A3, 0x63450713, 0x499DF0D8, 0xB49E721A, 0x409AB59F, 0x8A662D1C, 0x57AEFBDF, 0x6B404456, 0x536EA49B, 0x6A4D061E, 0x5CB6FCC6, 0x9A8E7F20, 0x428EBEA0, 0x623F2E2A, 0x69C6FFF0, 0x9A686569, 0x6F83AC92, 0x7F5D0B2A, 0x70CDFFB2, 0x77848D29, 0x4787C4A0, 0x401D2E3E, 0x7ED9FFFC, 0xC48F7973, 0x8598AE86, 0x9A701339, 0x85E1FFA6, 0x5E859534, 0x5189C99B, 0x2C0B3455, 0x92E9FFFA, 0xDCAB8071, 0x94ACAD7A, 0xB1882249, 0x9AF1FFB0, 0x6A8F9746, 0x6394CD8D, 0x290C416F, 0xA5EEFFF4, 0xDCB46B6D, 0xA1BFAD77, 0xB6A43A58, 0xAAF8FFC8, 0x84959963, 0x7BA4CE78, 0x351A588E, 0xB5E3F9F1, 0xCFAC6077, 0xABCFAC77, 0xA8C05A6A, 0xB2F3FFE1, 0xA3959471, 0x94B9D065, 0x473274A9, 0xBDCDECEF, 0xC2A05F86, 0xBADDAC78, 0x92D87C7D, 0xAFDEFDF1, 0xBC8C8971, 0xA6D2D45A, 0x584B95BB, 0xC1B3D5ED, 0xB8956697, 0xC5E9A670, 0x7AE6978F, 0xA4BEF0EF, 0xB9797360, 0xB1E8DD5D, 0x5D5FAFC7, 0xC49CB8E3, 0xAF8C78A5, 0xCBF19D5F, 0x6AE5A6A3, 0x9398DBE7, 0xB46A5841, 0xB8F2E871, 0x586DC1CD, 0xC78C9DD5, 0xAA878CB1, 0xD8F89249, 0x63DCABB3, 0x8576C5D8, 0xA95E4430, 0xC0EBEE91, 0x5378CACE, 0xC58087C5, 0xA4859EAE, 0xE1FD8A36, 0x60D0ACBD, 0x7765B9C5, 0x9C563A2B, 0xCED7E7B4, 0x5484C7CC, 0xB66F7CBB, 0x9F81A898, 0xE3FF8D2E, 0x63C8B0BA, 0x6D69B8B0, 0x93573B2E, 0xE0C2CED2, 0x6898C0C3, 0x97587EBC, 0x9D749570, 0xDEFF9D36, 0x67C8B4A8, 0x6682BD9B, 0x93643E37, 0xF0B4A8DE, 0x86A9B6B0, 0x70448BBE, 0x99617545, 0xD1FFB84E, 0x70CAB48C, 0x64A4BD8D, 0x96763D41, 0xFAAC82DA, 0xA7B6AE9A, 0x4D359AC0, 0x90494F27, 0xC0FFD675, 0x80CCAD6D, 0x63C1B681, 0x9C85374E, 0xFFA069C9, 0xC1BAA785, 0x3936A4B6, 0x7D342E18, 0xA9FFEEA5, 0x9AC59F56, 0x60D0A873, 0x9288315E, 0xFB8669B6, 0xD0B6A473, 0x3C47A39A, 0x6228241B, 0x8BFFF6CD, 0xB7BB8F47, 0x57CFA169, 0x77782C6B, 0xE56478AA, 0xD6AC9E64, 0x4E61977C, 0x4B232428, 0x6FFFEBE5, 0xD0AF813C, 0x44C2A266, 0x5B61296E, 0xC04188A8, 0xD4A09051, 0x5D788B61, 0x3E202234, 0x58F9D7DF, 0xDEA27332, 0x31AE9A6C, 0x3E4B2F62, 0x902688B3, 0xD0937A3F, 0x5D7D8B58, 0x37241F35, 0x46F2C5BF, 0xD7956528, 0x1E918E76, 0x3442424A, 0x601774C3, 0xCB8F6630, 0x48698261, 0x333C2729, 0x3CECBF95, 0xC2885420, 0x11717B78, 0x4952673A, 0x3D1153CD, 0xC994602F, 0x2D496F6D, 0x31674418, 0x32E7CA6C, 0xA77D4318, 0x09526672, 0x6B6F963E, 0x2B1036D0, 0xCAA26E3C, 0x1B2A5A7B, 0x3798700F, 0x2BDFD94D, 0x8C773712, 0x043C606C, 0x9086B955, 0x281225C9, 0xC7AD8954, 0x141B4085, 0x53CB9C10, 0x26D3E63A, 0x757A3B11, 0x0131666F, 0xAB87BA6A, 0x2E151FBF, 0xC1AEA16A, 0x1721388C, 0x81ECBB17, 0x23C1EE30, 0x6688561E, 0x02317381, 0xA36A9566, 0x3B171FB9, 0xB5A5AF7A, 0x1B384395, 0xABFBC81D, 0x20AAEB28, 0x61987838, 0x05358299, 0x85405D4E, 0x45181FB5, 0xA695B083, 0x1C55569C, 0xCEFFCF1E, 0x1A8CDA24, 0x62A39755, 0x0A398FAE, 0x601A2A2C, 0x49161FB1, 0x9A8BAC8A, 0x23736DA0, 0xE0FFD61E, 0x136BAF23, 0x70ABAD6D, 0x143D95BD, 0x41030D16, 0x41151DAA, 0x938BAA94, 0x2E8A8BA7, 0xE6FFD921, 0x0C4A7D2D, 0x7CACAB70, 0x244093C3, 0x38020F11, 0x2F151CA3, 0x9293ABA1, 0x3995AAB7, 0xEDF7D924, 0x072F5842, 0x83AE9C66, 0x3F498EC7, 0x420D2812, 0x1E151F9B, 0x989CB0B2, 0x4697C5CE, 0xF5F0D626, 0x061B435B, 0x85B18A5B, 0x5F568BCD, 0x54245213, 0x12172593, 0x9FA1BAC0, 0x5291D4E1, 0xFAF0D123, 0x0711416F, 0x82B2765B, 0x84668FCB, 0x684D8212, 0x1318308A, 0xA3A2C2C6, 0x5F88D3EE, 0xFEF6C81B, 0x0B104276, 0x85B06A72, 0xA97896C3, 0x7C81AC19, 0x24194083, 0xA2A0C6C4, 0x6A82CCF1, 0xFFFFB910, 0x14173E77, 0x8BAE6992, 0xC6879BB4, 0x92B3C933, 0x3C1D4F7D, 0x9DA2CCBE, 0x7381C6E6, 0xFFFFB111, 0x26233975, 0x92AF6DAF, 0xDB949AA3, 0xADDBDD64, 0x5927597B, 0x99AAD4B9, 0x7687C6D5, 0xFCFFB736, 0x432D3A78, 0x98B371BD, 0xE5A0989E, 0xCCF2EAA0, 0x753C5C80, 0x9DB9E3BC, 0x7896CFBF, 0xF6FFCB72, 0x64323F81, 0x9CBC70B4, 0xE3AC9EAD, 0xE6FBF6CF, 0x8B5C5C85, 0xA7C8F4C2, 0x7AA6D9A3, 0xF0FFE2A9, 0x8032448D, 0xA4C970A0, 0xDBB5AAC6, 0xF7FEFDE7, 0xA3806187, 0xB0D6FFC9, 0x7FB5D686, 0xE7FFF0D2, 0x8D2F4E98, 0xABD7768D, 0xCFB9B4E1, 0xF8F8FFE7, 0xB29B7089, 0xB7E4FFCD, 0x8CB8BF65, 0xD5FFF0D7, 0x8D34609D, 0xB4E48385, 0xC3BAB0F1, 0xE2E8FEDC, 0xB5A07A8D, 0xBDEFFFD0, 0x9AAB9543, 0xB9FFE9C3, 0x80447497, 0xC0EF9C90, 0xBDB997E5, 0xC2CCFBD4, 0xAB8F7387, 0xC8F8FFD4, 0xA4936324, 0x98FFE5B3, 0x6F577E8A, 0xCEFABCA8, 0xC0BC71C7, 0x9EADF4D2, 0x916D5D7B, 0xD4FEFFD6, 0xA575390F, 0x76FFE4A6, 0x656A7F7D, 0xDDFFDABF, 0xC8BF4CA1, 0x8095E8CC, 0x744A3B66, 0xDAFFFFD6, 0x9D5B1E04, 0x5DFDDF9A, 0x6075787A, 0xECFFF1CD, 0xCEC42E7C, 0x7087D6BF, 0x5A311F49, 0xD4FFFFD0, 0x8E4B1100, 0x50F7D892, 0x68767788, 0xF6FFFDCE, 0xCDC31E65, 0x6E88C3A8, 0x4922153A, 0xC3FFFDC4, 0x7F410B03, 0x4BEED08B, 0x747682A0, 0xFDFFFEBF, 0xBCB61456, 0x7792B18B, 0x3E19153D, 0xB5FFF8B2, 0x733B0908, 0x4DE0C487, 0x827797BC, 0xFFFFF8AB, 0x9E9B0E50, 0x869E9F6E, 0x3714184D, 0xB5FFF09E, 0x67380C11, 0x56CFB285, 0x8F7EADD4, 0xFFFFED96, 0x78730A53, 0x9CA98F57, 0x310F1C69, 0xC8FFE18A, 0x5D35161D, 0x64BC9E84, 0x978BBFE7, 0xFFFFDD81, 0x5348075A, 0xB5B07F48, 0x290A2088, 0xE1FFCF75, 0x5335272B, 0x74AD877F, 0x9C9DCFF3, 0xFFFEC86E, 0x35260864, 0xC6B16F3E, 0x200422A6, 0xF8FCB95F, 0x47373A3C, 0x86A36F73, 0x9FB0D9FA, 0xFFF7B15B, 0x21110E70, 0xCDAB6139, 0x160027C1, 0xFFF09F4C, 0x3B3A4B4D, 0x989D5B5D, 0x9DB5D9FE, 0xFFEA994C, 0x1509177B, 0xC8A15C3C, 0x0E022BD1, 0xFFE1873B, 0x2F3E5460, 0xA8954F44, 0x96A6CEFF, 0xFFD7823C, 0x0C0A2784, 0xB9956343, 0x0B0731DB, 0xFFCE722C, 0x22415672, 0xB58C4A31, 0x8A88BAFF, 0xFAC56E2E, 0x060F398F, 0xA989764A, 0x0A0C3AE1, 0xFFB85F20, 0x17425787, 0xBF824826, 0x775FA3FF, 0xF1B66121, 0x03164B98, 0x9A818E4A, 0x0A1040D4, 0xE8974D14, 0x11405495, 0xBB74401F, 0x603D86F1, 0xD7A35414, 0x02195694, 0x8476943E, 0x171A46C5, 0xCC7D3D0C, 0x0D3E599E, 0xAE6C371C, 0x4A2D72E3, 0xC38F4C0E, 0x03205F96, 0x766A792A, 0x242B4FB8, 0xB66A2E06, 0x0D3E66AA, 0x9B663322, 0x3B2D6CDD, 0xB47B450A, 0x062B6E9C, 0x6E705D17
glabel D_80992680
 .word 0x01950624, 0xF9950000, 0x0191F272, 0x007800FF, 0x01D605D0, 0xF8F90000, 0x0200F155, 0x007800FF, 0x00CF0520, 0xFB760000, 0x00FBF2D8, 0x007800FF, 0x00CF0334, 0xFB760000, 0x003DF2CB, 0x007800FF, 0x03CD0334, 0xFCB10000, 0x0200F51C, 0x007800FF, 0x00CF0520, 0xFB760000, 0x00FCF2D8, 0x007800FF, 0x03CD0520, 0xFCB10000, 0x0200F439, 0x007800FF, 0x053C03F4, 0xFF2F0000, 0x0200FE39, 0x007800FF, 0x053C00DC, 0xFF2F0000, 0x0200FBC7, 0x007800FF, 0x03AB0208, 0xFF2F0000, 0x00C9FA84, 0x007800FF, 0x03AB00DC, 0xFF2F0000, 0x0187FA91, 0x007800FF, 0x04C700B1, 0x033F0000, 0x000006AB, 0x007800FF, 0x051D00DC, 0x033F0000, 0x008F068F, 0x007800FF, 0x04C701F4, 0x033F0000, 0x00000555, 0x007800FF, 0x074F00DC, 0x033F0000, 0x02000639, 0x007800FF, 0x03CD0520, 0xFCB10000, 0x0200F439, 0x007800FF, 0x02640624, 0xF9EA0000, 0x0200F31C, 0x007800FF, 0x02640624, 0xF9EA0000, 0x0200F31C, 0x007800FF, 0x034703F4, 0xFF2F0000, 0x008FFD8E, 0x007800FF, 0x064204F8, 0x01380000, 0x0200011C, 0x007800FF, 0x03470498, 0x00770000, 0x0000FEE4, 0x007800FF, 0x03B904F8, 0x01380000, 0x00000072, 0x007800FF, 0x04C703F6, 0x033F0000, 0x00000355, 0x007800FF, 0x074F03F6, 0x033F0000, 0x02000355, 0x007800FF, 0x02B30208, 0xFF2F0000, 0x0000FA00, 0x007800FF, 0x034702A4, 0x00770000, 0x0000FC39, 0x007800FF, 0x03470208, 0xFF2F0000, 0x007FFA54, 0x007800FF, 0x077100DC, 0x04020000, 0x018C09B1, 0x007800FF, 0x067A00DC, 0x04910000, 0x006509B7, 0x007800FF, 0x07710127, 0x04020000, 0x019A0A39, 0x007800FF, 0x067A0127, 0x04910000, 0x00660A39, 0x007800FF, 0x077B0127, 0x04810000, 0x019A0B55, 0x007800FF
glabel D_80992880
 .word 0x07710127, 0x04020000, 0x019A0A39, 0x007800FF, 0x07940127, 0x04170000, 0x02000A72, 0x007800FF, 0x077100DC, 0x04020000, 0x018C09B1, 0x007800FF, 0x079400DC, 0x04170000, 0x02000A00, 0x007800FF, 0x067A00DC, 0x04910000, 0x006509B7, 0x007800FF, 0x067A00DC, 0x04BF0000, 0x00000A00, 0x007800FF, 0x067A0127, 0x04910000, 0x00660A39, 0x007800FF, 0x067A0127, 0x04BF0000, 0x00000A72, 0x007800FF, 0x07B00127, 0x04940000, 0x02000B8E, 0x007800FF, 0x077B0127, 0x04810000, 0x019A0B55, 0x007800FF, 0x06E30127, 0x04D90000, 0x00660B55, 0x007800FF, 0x06D90127, 0x050F0000, 0x00000B8E, 0x007800FF, 0x03CD0334, 0xFCB10000, 0x0200F51C, 0x007800FF, 0x00CF0334, 0xFB760000, 0x003DF2CB, 0x007800FF, 0x03CD0208, 0xFCB10000, 0x0200F6AB, 0x007800FF, 0x008E0208, 0xFB5B0000, 0x0000F439, 0x007800FF, 0x045C0208, 0xFD820000, 0x0200F7C7, 0x007800FF, 0x03AB0208, 0xFF2F0000, 0x00C9FA84, 0x007800FF, 0x045C00DC, 0xFD820000, 0x0200F91C, 0x007800FF, 0x03AB00DC, 0xFF2F0000, 0x0187FA91, 0x007800FF, 0x051D00DC, 0x033F0000, 0x008F068F, 0x007800FF, 0x077300DC, 0x03980000, 0x0200091C, 0x007800FF, 0x074F00DC, 0x033F0000, 0x02000639, 0x007800FF, 0x04C700B1, 0x033F0000, 0x000006AB, 0x007800FF, 0x051D00DC, 0x038E0000, 0x0000071C, 0x007800FF, 0x078C00DC, 0x03F70000, 0x020009C7, 0x007800FF, 0x053C00DC, 0xFF2F0000, 0x0200FBC7, 0x007800FF, 0x066400DC, 0x04AA0000, 0x000009C7, 0x007800FF, 0x02B30208, 0xFF2F0000, 0x0000FA00, 0x007800FF, 0x03470208, 0xFF2F0000, 0x007FFA54, 0x007800FF, 0x077BFF38, 0x04810000, 0x019A1000, 0x007800FF, 0x07B0FF38, 0x04940000, 0x02001000, 0x007800FF
glabel D_80992A80
 .word 0x077B0127, 0x04810000, 0x019A0B55, 0x007800FF, 0x07B0FF38, 0x04940000, 0x02001000, 0x007800FF, 0x07B00127, 0x04940000, 0x02000B8E, 0x007800FF, 0x06E30127, 0x04D90000, 0x00660B55, 0x007800FF, 0x06E3FF38, 0x04D90000, 0x00661000, 0x007800FF, 0x077BFF38, 0x04810000, 0x019A1000, 0x007800FF, 0x06D90127, 0x050F0000, 0x00000B8E, 0x007800FF, 0x06D9FF38, 0x050F0000, 0x00001000, 0x007800FF, 0x053C03F4, 0xFF2F0000, 0x0200FE39, 0x007800FF, 0x03AB0208, 0xFF2F0000, 0x00C9FA84, 0x007800FF, 0x034703F4, 0xFF2F0000, 0x008FFD8E, 0x007800FF, 0x03470208, 0xFF2F0000, 0x007FFA54, 0x007800FF, 0x034702A4, 0x00770000, 0x0000FC39, 0x007800FF, 0x03470498, 0x00770000, 0x0000FEE4, 0x007800FF, 0x045C0208, 0xFD820000, 0x0200F7C7, 0x007800FF, 0x01D50334, 0xF8F90000, 0x0200F000, 0x007800FF, 0x00CF0334, 0xFB760000, 0x003DF2CB, 0x007800FF, 0x00CF0520, 0xFB760000, 0x00FBF2D8, 0x007800FF, 0x01D605D0, 0xF8F90000, 0x0200F155, 0x007800FF, 0x04C703F6, 0x033F0000, 0x00000355, 0x007800FF, 0x04C701F4, 0x033F0000, 0x00000555, 0x007800FF, 0x074F03F6, 0x033F0000, 0x02000355, 0x007800FF, 0x074F00DC, 0x033F0000, 0x02000639, 0x007800FF, 0x008E0208, 0xFB5B0000, 0x0000F439, 0x007800FF, 0x008E0334, 0xFB5B0000, 0x0000F2AB, 0x007800FF, 0xFFB40334, 0xF99F0000, 0x0000F000, 0x007800FF
glabel D_80992C20
 .word 0x067A0127, 0x04910000, 0x00CD051C, 0x007800FF, 0x06E30127, 0x04D90000, 0x00CD05AB, 0x007800FF, 0x077B0127, 0x04810000, 0x033305AB, 0x007800FF
glabel D_80992C50
 .word 0xFE7B0320, 0xF8A40000, 0x00661000, 0x2E0092FF, 0xFC230320, 0xF7AD0000, 0x02001000, 0x2E0092FF, 0xFE7B0520, 0xF8A40000, 0x00660E05, 0x2B2998FF, 0xFC230520, 0xF7AD0000, 0x02000C6F, 0x1C5EBCFF, 0xFA2D05F1, 0xF8A10000, 0x02000A74, 0x146BCFFF, 0xFA0A0625, 0xF9040000, 0x01B009E6, 0x0776EEFF, 0xFC400625, 0xF9ED0000, 0x00000B71, 0x0776EEFF, 0xFE510558, 0xF90A0000, 0x00000D9F, 0x146BCFFF, 0xF2210080, 0x07E10000, 0x0200DE11, 0x007800FF, 0xF2DE006E, 0x07150000, 0x0200DC49, 0x007800FF, 0xF1B100AB, 0x05350000, 0x0000DC49, 0x007800FF, 0xF64E0014, 0x04460000, 0x0000D8EB, 0x007800FF, 0xF04E00C9, 0x06E90000, 0x0000DE11, 0x007800FF, 0xEFA900D2, 0x0A360000, 0x0000E03F, 0x007800FF, 0xF1F40084, 0x09310000, 0x0200E03F, 0x007800FF, 0xF5A10014, 0x068E0000, 0x01FCD9E8, 0x007800FF, 0xEE220106, 0x0AE30000, 0x0000ED20, 0x007800FF, 0xEDB40115, 0x0A760000, 0x0000EDB9, 0x007800FF, 0xEBD00155, 0x0AB10000, 0x0200EE51, 0x007800FF, 0xEBB6015F, 0x081F0000, 0x0200F0E4, 0x007800FF, 0xEC7B013E, 0x0B9F0000, 0x0200ED20, 0x007800FF, 0xED9A011B, 0x076B0000, 0x0000EFE7, 0x007800FF, 0xEDD30123, 0x06C00000, 0x0000F0E4, 0x007800FF, 0xF8560014, 0x03DC0000, 0x0000D722, 0x007800FF, 0xF8E00014, 0x05EC0000, 0x0200D722, 0x007800FF, 0xFF8C0014, 0x038B0000, 0x0000D0FE, 0x007800FF, 0xFF030014, 0x038C0000, 0x0000D196, 0x007800FF, 0xFEFB0014, 0x05B40000, 0x0200D196, 0x007800FF, 0xFB5B0014, 0x05BA0000, 0x0200D48F, 0x007800FF, 0x00690014, 0x03890000, 0x0000D000, 0x007800FF, 0xFFD60014, 0x038A0000, 0x0000D098, 0x007800FF, 0x00700014, 0x04BB0000, 0x0100D000, 0x007800FF
glabel D_80992E50
 .word 0xFA370522, 0xF8AE0000, 0x02000A74, 0x4D561FFF, 0xF7930712, 0xF9ED0000, 0x0200067D, 0x495628FF, 0xF9540520, 0xFAE90000, 0x00B50828, 0x475331FF, 0xEFE10864, 0xFF330000, 0x0200FCFA, 0x2C5D3DFF, 0xED05072C, 0x02DE0000, 0x0200F975, 0x2C5D3DFF, 0xEEAF05CB, 0x04010000, 0x0000F86C, 0x2C5D3DFF, 0xEBC605BE, 0x06040000, 0x0200F6D0, 0x2C5D3DFF, 0xEBA604EC, 0x07800000, 0x0200F50D, 0x2C5D3DFF, 0xEBB4047D, 0x08200000, 0x0200F475, 0x2C5D3DFF, 0xEDD10464, 0x06C10000, 0x0000F475, 0x2C5D3DFF, 0xF67506C0, 0xFC8C0000, 0x00000384, 0x49552AFF, 0xF9310502, 0xFB550000, 0x00000715, 0x652F2CFF, 0xF9BF0208, 0xFB150000, 0x00000878, 0xD2006EFF, 0xF9BF0520, 0xFB150000, 0x00000943, 0xD52968FF, 0xF9540208, 0xFAE90000, 0x0056082A, 0xD2006EFF, 0xF9540520, 0xFAE90000, 0x00B50828, 0xD2006EFF, 0xF9540208, 0xFAE90000, 0x0056082A, 0x007800FF, 0xF9310208, 0xFB550000, 0x000007E0, 0x007800FF, 0xF9BF0208, 0xFB150000, 0x00000878, 0x007800FF, 0xFC400625, 0xF9ED0000, 0x00000B71, 0x0776EEFF, 0xFA0A0625, 0xF9040000, 0x01B009E6, 0x0776EEFF, 0xFE510320, 0xF90A0000, 0x00001000, 0x6E002EFF, 0xFE7B0520, 0xF8A40000, 0x00660E05, 0x6E002EFF, 0xFE510558, 0xF90A0000, 0x00000D9F, 0x6E002EFF, 0xFE7B0320, 0xF8A40000, 0x00661000, 0x6E002EFF, 0xF9540208, 0xFAE90000, 0x0056082A, 0x720025FF, 0xF9310208, 0xFB550000, 0x000007E0, 0x720025FF, 0xF5170894, 0xFB2D0000, 0x020002EC, 0x3B5A34FF, 0xF5E80702, 0xFCFA0000, 0x000002B9, 0x3B5A34FF, 0xF5630870, 0xFAF50000, 0x02000352, 0x48552AFF
glabel D_80993030
 .word 0xEBB4047D, 0x08200000, 0x0200F475, 0x007800FF, 0xEBB6015F, 0x081F0000, 0x0200F0E4, 0x007800FF, 0xEDD30123, 0x06C00000, 0x0000F0E4, 0x007800FF, 0xEDD10464, 0x06C10000, 0x0000F475, 0x007800FF, 0xEE220106, 0x0AE30000, 0x0000ED20, 0x007800FF, 0xEC7B013E, 0x0B9F0000, 0x0200ED20, 0x007800FF, 0xEC7B0470, 0x0B9F0000, 0x0200E95D, 0x007800FF, 0xEE220458, 0x0AE30000, 0x0000E95D, 0x007800FF, 0xF1F40421, 0x09310000, 0x0200E39E, 0x007800FF, 0xF1F40084, 0x09310000, 0x0200E03F, 0x007800FF, 0xEFA900D2, 0x0A360000, 0x0000E03F, 0x007800FF, 0xEFA90442, 0x0A360000, 0x0000E39E, 0x007800FF, 0xFB5B0014, 0x05BA0000, 0x0200D48F, 0x007800FF, 0xFB560014, 0x03920000, 0x0000D4C1, 0x007800FF, 0xF9D70014, 0x05BC0000, 0x0200D624, 0x007800FF, 0xFF030014, 0x038C0000, 0x0000D196, 0x007800FF, 0xF9B60014, 0x03950000, 0x0000D5F2, 0x007800FF, 0xF8560014, 0x03DC0000, 0x0000D722, 0x007800FF, 0xF8E00014, 0x05EC0000, 0x0200D722, 0x007800FF, 0xEEAF05CB, 0x04010000, 0x0000F86C, 0x2C5D3DFF, 0xF1A206D4, 0x00510000, 0x0000FD93, 0x2C5D3DFF, 0xEFE10864, 0xFF330000, 0x0200FCFA, 0x2C5D3DFF, 0xF1C708A9, 0xFD6F0000, 0x0200FF8E, 0x2C5D3DFF, 0xF5E80702, 0xFCFA0000, 0x000002B9, 0x3B5A34FF, 0xF5E80702, 0xFCFA0000, 0x000002B9, 0x3B5A34FF, 0xF5170894, 0xFB2D0000, 0x020002EC, 0x3B5A34FF, 0xEC7B0470, 0x0B9F0000, 0x0200E95D, 0x007800FF, 0xECF404FF, 0x0C470000, 0x0200E837, 0x007800FF, 0xEE220458, 0x0AE30000, 0x0000E95D, 0x007800FF, 0xEEA304A7, 0x0B2A0000, 0x0000E7ED, 0x007800FF, 0xEE7805EF, 0x0D200000, 0x0200E710, 0x007800FF
glabel D_80993220
 .word 0xEE7805EF, 0x0D200000, 0x0200E710, 0x007800FF, 0xF0740603, 0x0C860000, 0x0200E5EA, 0x007800FF, 0xEF4704A9, 0x0AF10000, 0x0000E67D, 0x007800FF, 0xEEA304A7, 0x0B2A0000, 0x0000E7ED, 0x007800FF, 0xF1C50539, 0x0AE10000, 0x0200E4C4, 0x007800FF, 0xEF95047F, 0x0A970000, 0x0000E50D, 0x007800FF, 0xF1F40421, 0x09310000, 0x0200E39E, 0x007800FF, 0xEFA90442, 0x0A360000, 0x0000E39E, 0x007800FF
glabel D_809932A0
 .word 0x00750014, 0x05B20000, 0x0400E800, 0x007800FF, 0x00700014, 0x04BB0000, 0x0200E800, 0x007800FF, 0xFEFB0014, 0x05B40000, 0x0400E8CB, 0x007800FF
glabel D_809932D0
 .word 0xE7000000, 0x00000000, 0xE3001001, 0x00000000, 0xD7000002, 0xFFFFFFFF, 0xFD900000
.word D_80991680
.word 0xF5900000, 0x0701C640, 0xE6000000, 0x00000000, 0xF3000000, 0x073FF400, 0xE7000000, 0x00000000, 0xF5880400, 0x0001C640, 0xF2000000, 0x0003C1FC, 0xFD900000
.word D_80991E80
.word 0xF5900100, 0x0701BC50, 0xE6000000, 0x00000000, 0xF3000000, 0x073FF200, 0xE7000000, 0x00000000, 0xF5880900, 0x0101BC50, 0xF2000000, 0x0107C0FC, 0xFC71ABFF, 0x5FFEE238, 0xE200001C, 0x0C1849D8, 0xD9F2FFFF, 0x00000000, 0xD9FFFFFF, 0x00020400, 0xFA000000, 0x003264FF, 0xFB000000, 0x32140080, 0xDF000000, 0x00000000
glabel D_80993390
 .word 0x01020040
.word D_80992680
.word 0x06000204, 0x0006080A, 0x06080C0A, 0x000E1012, 0x06101412, 0x0016181A, 0x06181C1A, 0x001E2004, 0x06200004, 0x00220200, 0x060E2426, 0x00242826, 0x06282A26, 0x002C2E2A, 0x062E262A, 0x00303234, 0x0636383A, 0x00383C3A, 0x063A3C3E, 0x00343224, 0x01020040
.word D_80992880
.word 0x06000204, 0x00020604, 0x06080A0C, 0x000A0E0C, 0x06020010, 0x00001210, 0x060C0E14, 0x000E1614, 0x06181A1C, 0x001A1E1C, 0x06202224, 0x00222624, 0x06080428, 0x00042A28, 0x062A2C28, 0x002E3028, 0x06322A04, 0x00063204, 0x06342426, 0x00303628, 0x06360828, 0x00360A08, 0x06383A1E, 0x003A201E, 0x06201C1E, 0x00123C3E, 0x0101A034
.word D_80992A80
.word 0x06000204, 0x0006080A, 0x06060A00, 0x000C0E08, 0x060C0806, 0x00101214, 0x06121614, 0x0014181A, 0x061C1612, 0x001E2022, 0x061E2224, 0x0026282A, 0x06282C2A, 0x002E2030, 0x061E3220, 0x00323020, 0x01003006
.word D_80992C20
.word 0x05000204, 0x00000000, 0xDF000000, 0x00000000
glabel D_809934B8
 .word 0x01020040
.word D_80992C50
.word 0x06000204, 0x00020604, 0x06080A06, 0x000A0C06, 0x060C0E06, 0x000E0406, 0x06101214, 0x00121614, 0x06141810, 0x00181A10, 0x061A1C10, 0x00121E16, 0x06202224, 0x00222624, 0x06242820, 0x00222A26, 0x062A2C26, 0x002E1630, 0x06161E30, 0x00323436, 0x06343836, 0x003A3C3E, 0x063E3C36, 0x003C3236, 0x01020040
.word D_80992E50
.word 0x06000204, 0x0006080A, 0x06080C0A, 0x000C0E0A, 0x060E100A, 0x0010120A, 0x06141602, 0x00160402, 0x06181A1C, 0x001A1E1C, 0x06202224, 0x00041A26, 0x06042628, 0x002A2C2E, 0x062A302C, 0x00163204, 0x06163432, 0x0036383A, 0x0638143A, 0x0014023A, 0x01020040
.word D_80993030
.word 0x06000204, 0x00000406, 0x06080A0C, 0x00080C0E, 0x06101214, 0x00101416, 0x06181A1C, 0x00181E1A, 0x061A201C, 0x0020221C, 0x0622241C, 0x0026282A, 0x06282C2A, 0x00282E2C, 0x0630322C, 0x00343638, 0x06363A38, 0x00363C3A, 0x01008010
.word D_80993220
.word 0x06000204, 0x00000406, 0x0602080A, 0x00020A04, 0x06080C0E, 0x00080E0A, 0x01003006
.word D_809932A0
.word 0x05000204, 0x00000000, 0xDF000000, 0x00000000

