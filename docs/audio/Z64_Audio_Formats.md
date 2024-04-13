# Zelda64 Audio Files Format

This document aims to provide accurate information on the structure of the audio asset files and tables in Zelda 64.

## Audio Tables

The audio tables are located in the main code file, towards the end of the `.rodata` section. These tables contain an entry for the individual files that make up `Audioseq`, `Audiobank` and `Audiotable`.

The tables follow a common structure:
```c
typedef struct {
    /* 0x00 */ u32 romAddr;
    /* 0x04 */ u32 size;
    /* 0x08 */ s8 medium;
    /* 0x09 */ s8 cachePolicy;
    /* 0x0A */ s16 shortData1;
    /* 0x0C */ s16 shortData2;
    /* 0x0E */ s16 shortData3;
} AudioTableEntry; // size = 0x10

typedef struct {
    /* 0x00 */ s16 numEntries;
    /* 0x02 */ s16 unkMediumParam;
    /* 0x04 */ u32 romAddr;
    /* 0x08 */ char pad[0x8];
    /* 0x10 */ AudioTableEntry entries[1/* numEntries */];
} AudioTable; // size = 0x10 + 0x10 * numEntries
```

 - `romAddr` is an offset from the start of the audio segment, and is converted to an absolute ROM address at runtime.
 - `size` is the size of the entry in bytes.
 - `medium` is the storage medium for the data this entry points to, either RAM, Cartridge, or N64 Disk Drive.
 - `cachePolicy` determines how long the data will be allocated for when loaded, it may be `PERMANENT`, `PERSISTENT`, `TEMPORARY` or `EITHER`.

Some entries in the table have zero `size` and act as pointers to other entries in the table, in which case the `romAddr` field contains the new ID to use. ([Code](https://github.com/zeldaret/oot/blob/e68f321777be140726591b9a5dc4c45fe127d6d3/src/code/audio_load.c#L741))

The last three `shortData` fields are used only in the soundfont table associated with `Audiobank`, they are 0 in the other tables. In the soundfont table they represent:

`shortData1 = (sampleBankId1 << 8) | sampleBankId2`
`shortData2 = (numInstruments << 8) | numDrums`
`shortData3 = numSfx`

## Audiotable

### About

The Audiotable contains compressed audio samples. It is split into multiple "sample banks". The table contains 7 entries:
```c
SAMPLE_BANK_SFX
SAMPLE_BANK_ORCHESTRA
SAMPLE_BANK_DEKU_TREE
SAMPLE_BANK_JABU_JABU
SAMPLE_BANK_FOREST_TEMPLE
SAMPLE_BANK_GORON_CITY
SAMPLE_BANK_SPIRIT_TEMPLE
```
There are however only 6 samplebank binaries, `SAMPLE_BANK_ORCHESTRA` acts as a "pointer" to `SAMPLE_BANK_SFX` when accessed externally.

#### N64 Audio Interface

To understand why the samples are stored in the way they are, it is useful to know about the Audio Interface (AI) to know what the aim is.

The AI consumes raw uncompressed 16-bit pulse-code-modulation (PCM) audio data. The audio driver's task is to prepare a stream of PCM to send to the AI as soon as it requires it. The AI receives PCM via DMA that is programmed by a set of memory-mapped registers

| Address      | Name               | R/W | Size   | Description |
| ------------ | ------------------ | --- | ------ | ----------- |
| `0x04500000` | `AI_DRAM_ADDR_REG` | W   | 24-bit | Audio DMA Base Address (8-byte aligned)
| `0x04500004` | `AI_LEN_REG`       | R/W | 18-bit | Audio DMA Length (8-byte multiples)
| `0x04500008` | `AI_CONTROL_REG`   | W   |  1-bit | Audio DMA Control, bit 0=DMA Enable
| `0x0450000C` | `AI_STATUS_REG`    | R/W | 32-bit | Audio Interface Status, bits 31/0=DMA Full, bit 30=DMA Busy, writing clears AI interrupt
| `0x04500010` | `AI_DACRATE_REG`   | W   | 14-bit | DAC Sample Period, set to `(vi_clock / samplerate) - 1`
| `0x04500014` | `AI_BITRATE_REG`   | W   |  4-bit | ABUS Clock Half Period, set to `MIN(dperiod / 66, 16) - 1`

Side note: Rather misleadingly, `AI_BITRATE_REG` does not mean the AI can play any bitrate from 1 to 16, the AI is only capable of signed 16-bit stereo. In order to play 8-bit PCM properly, the effective samplerate used to program `AI_DACRATE_REG` must also be half of what it would be for the same sample encoded at 16-bit PCM. The Zelda64 audio driver is unconcerned with this, it will only ever provide 16-bit PCM, it has support for 8-bit PCM however it is converted on the RSP before it is fed to the AI.

#### VADPCM

Storing raw PCM would be far too costly in ROM and it must reside in RAM as the AI DMA engine can only access RAM. To alleviate the ROM space cost the raw sample data is instead stored compressed. One of the RSP Audio commands the Audio driver will issue is responsible for decompressing them when a sample is loaded at runtime.

VADPCM is the lossy compression algorithm used to store the audio samples. The `V` likely stands for `Vector`, the decompression algorithm is particularly suited to be parallelized using 8-lane 16-bit SIMD instructions, which are present in the N64 RSP processor. `AD` stands for Adaptive-Differential, rather than storing raw values it stores the difference from a "predicted" value, selecting the prediction coefficients adaptively from a codebook.

#### AIFF and AIFC

AIFF files are split into chunks, identified by four characters ("[FourCC](https://en.wikipedia.org/wiki/FourCC)"). We are interested in a couple of the chunks found in these files:
 - `SSND`: Sound data
 - `COMM`: "Common" chunk, contains number of channels, number of samples, number of bits per sample, and sample rate. For our purposes, we expect that the number of channels is 1 and the number of bits per sample is 16.
 - `MARK`: Markers, for loop points
 - `INST`: "Instrument" chunk, contains some additional metadata about a sample such as the base note value (using MIDI convention, Middle C = 60)
 - `APPL`: Application-specific sections

For more information on the structure of chunks in an AIFF file, refer to http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/AIFF/Docs/AIFF-1.3.pdf

An AIFC file is structured in essentially the same way, however the `SSND` data is now compressed with VADPCM and there are two `APPL` sections, `VADPCMCODES` for the codebook and `VADPCMLOOPS` for loop point information. The compression type in the `COMM` header is `VAPC` to indicate VADPCM.

The data in the Audiotable is the `SSND` chunk of an AIFC file. In order to reconstruct the AIFC file, data from the Audiobank is also required to fill in the codebook and loops information.

Decoding the AIFC file to AIFF can be accomplished using either `aifc_decode.c` or [sampleconv](../../tools/audio/sampleconv). As VADPCM is a lossy compression method, information from the original AIFF has been lost, the decoder finds decompressed data that re-compresses to matching through a brute-force method.

Converting AIFF to AIFC either uses two programs from the N64 SDK, `tabledesign` ([manual](http://n64devkit.square7.ch/n64man/tool/tabledesign.htm)) and `vadpcm_enc` ([manual](http://n64devkit.square7.ch/n64man/tool/vadpcm_enc.htm)), or [sampleconv](../../tools/audio/sampleconv). For the SDK tools, `tabledesign` generates the codebook which is passed to `vadpcm_enc` alongside the AIFF file to produce the AIFC file.

### Table (gSampleBankTable)

The Sample Bank table is a listing of the offsets and sizes of all sample banks present in the Audiotable. Also determines cache policy.

### Files

The Audiotable is partitioned into files for each sample bank. Each sample bank contains a collection of compressed audio samples. There is no other data, the starts and codecs of samples are recorded in structures in the Audiobank.

## Audiobank

### About

The Audiobank file houses "soundfont" data that organizes samples in the sample banks. Each soundfont is it's own file. In OoT there are 38 soundfonts, each has their own ID to be referenced by sequences. A list of named Soundfont IDs is [here](https://github.com/zeldaret/oot/blob/3e298fa818569f9323138ec34f9e3127fa435b9f/include/sequence.inc#L145).

#### Envelopes

Part of the Audiobank data is envelope data used to modulate sample waveforms, e.g. for ADSR ('attack-delay-sustain-release') for which the effect on the sample is pictured below:

![ADSR Envelope](https://upload.wikimedia.org/wikipedia/commons/e/ea/ADSR_parameter.svg)
[(Image Source)](https://commons.wikimedia.org/wiki/File:ADSR_parameter.svg)

Envelopes in the Zelda 64 audio driver are not limited to ADSR shapes, they are capable of building arbitrary piecewise-linear functions to act as an envelope.

A single envelope is comprised of multiple `EnvelopePoint` structures in an array

```c
typedef struct {
    /* 0x0 */ s16 delay;
    /* 0x2 */ s16 arg;
} EnvelopePoint; // size = 0x4
```

A negative or zero `delay` value indicates a special event:
| Delay Value | Name      | Result                     | Meaning of `arg` |
| ----------- | --------- | -------------------------- | ---------------- |
| ` 0`        | `DISABLE` | Stop processing            | Ignored, always 0.
| `-1`        | `HANG`    | Stop processing            | Ignored, always 0.
| `-2`        | `GOTO`    | Jump to another point      | Index of envelope point to jump to.
| `-3`        | `RESTART` | Restart from the beginning | Ignored, always 0.

A positive `delay` value indicates that the envelope curve should (linearly) approach `arg` over a period of `delay` update ticks. Many of these points are strung together to produce a full envelope, ending with one of the special commands listed above. In OoT audio data they all end with `HANG`.

### Table (gSoundFontTable)

The Audiobank table has an entry for each of the 38 soundfonts. The entry stores information such as the number of instruments, drums and Sfx in the soundfont file.

### Files

The structure of a file in the Audiobank can vary somewhat, however the overall structure is consistent.

| Data Type                                   | Description                                                                      |
| ------------------------------------------- | -------------------------------------------------------------------------------- |
| `Drum* (*)[]`                               | Pointer to array of Drum pointers, may be NULL if there are no Drums. |
| `SoundFontSound (*)[]`                      | Pointer to array of Sfx, may also be NULL if there are no Sfx. |
| `Instrument* []`                            | Array of Instrument pointers, the array length is in the `gSoundFontTable` entry for this soundfont. |
| **Padding to 0x10**                         | ====================================          |
| `Sample`, `AdpcmBook`, `AdpcmLoop` | Samples, their codebooks and their loop points. Identical codebooks are deduplicated. Book and loop structures are individually aligned to 0x10-byte boundaries. |
| `EnvelopePoint []`                           | Envelopes, varying array size, referenced by Instruments and Drums. |
| `Instrument []`                             | Instrument structures. |
| `Drum []`                                   | Drum structures. |
| `Drum* []`                                  | Array of pointers to Drums, the array length is in the `gSoundFontTable` entry for this soundfont, may have NULL entries. |
| **Padding to 0x10**                         | ====================================          |
| `SoundFontSound []`                         | Sfx structures, the number of structures is in the `gSoundFontTable` entry for this soundfont. |

It is possible that there is more padding to 0x10 than is indicated. For example, the following could make sense but all of the data in every soundfont happens to align there already generating no visible padding:
- Between samples/books/loops and envelopes
- Between envelopes and instruments
- Between instruments and drums

Occasionally, some structures such as envelopes and Instruments may go unreferenced and unused.

**Visual representation of the Spirit Temple Soundfont by engineer**
![Spirit Temple Soundfont](https://hackmd.io/_uploads/SyA1RVC92.png)

The various structures are described in greater detail in the following sections.

#### Soundfont Samples

Samples in the audiotable are referenced from a `Sample` structure:
```c
typedef struct {
    /* 0x00 */ u32 codec : 4;
    /* 0x00 */ u32 medium : 2;
    /* 0x00 */ u32 unk_bit26 : 1; // always 0 in OoT, can be non-zero in MM
    /* 0x00 */ u32 isRelocated : 1; // always 0 in ROM
    /* 0x01 */ u32 size : 24; // sample data size
    /* 0x04 */ u8* sampleAddr;
    /* 0x08 */ AdpcmLoop* loop;
    /* 0x0C */ AdpcmBook* book;
} Sample; // size = 0x10
```

In both OoT and MM, the codec is either `ADPCM (0)` or `SHORT_ADPCM (3)`. The audio driver has additional support for `S8 (1)` (8-bit PCM, decompressed on the RSP) and `S16 (2 or 5)` (raw 16-bit PCM).

To get the offset into `Audiotable` where the sample is actually stored:
 - If the `medium` is 0, take `sampleBankId1` as the `sampleBankId`.
   If the `medium` is 1, take `sampleBankId2` as the `sampleBankId`.
 - If `sampleBankId` is not -1, then the offset into Audiobank is `gSampleBankTable[sampleBankId].romAddr + sampleAddr`
   Otherwise it is `sampleAddr`.

In both OoT and MM, `medium` is always 0.

Note that in ROM, `medium` does not correspond to the `SampleMedium` enum, i.e. 0 is not `MEDIUM_RAM`.

([AudioLoad_RelocateSample](https://github.com/zeldaret/oot/blob/95b431793182bd021cb4234255f90b996edd8d4d/src/code/audio_load.c#L1574))

Accompanying every `Sample` are structures to hold the codebook and loop point information:

```c
typedef struct {
    /* 0x00 */ u32 start;
    /* 0x04 */ u32 end;
    /* 0x08 */ u32 count;
    /* 0x0C */ char unk_0C[0x4]; // padding to 8 alignment
    /* 0x10 */ s16 state[16]; // only exists if count != 0. 8-byte aligned
} AdpcmLoop; // size = (count != 0) ? 0x30 : 0x10

typedef struct {
    /* 0x00 */ s32 order; // tabledesign -o [order]
    /* 0x04 */ s32 npredictors;
    /* 0x08 */ s16 book[1/* 8 * order * npredictors */]; // 8-byte aligned
} AdpcmBook; // size = 8 + 16 * order * npredictors
```

If a book structure is the same as another in the same sound font, it will only be emitted once.

#### Sound Effects

Sound effects are the simplest way to define a single sound, they contain only the minimal information to access and play the associated sample.

```c
typedef struct {
    /* 0x00 */ Sample* sample;
    /* 0x04 */ f32 tuning;
} TunedSample; // size = 0x8

typedef struct {
    /* 0x00 */ TunedSample tunedSample;
} SoundEffect; // size = 0x08
```

The `tuning` parameter is a frequency scale factor that encodes information about the note scale the sample should be played at. It can be modelled by

$$\tau = \frac{R}{32 \ \mathrm{kHz}} \cdot 2^{\frac{1}{12} \left ( 5 \cdot 12 - N + 0.01 F \right )}$$<!--`-->

where
 - $R$ is the sample rate
 - $32 \ \mathrm{kHz}$ is the playback sample rate
 - $5 \cdot 12$ is the center key (C4/Middle C) in midi note number
 - $N$ is the root note for the sample in midi note number
 - $F$ is a fine-tuning parameter in cents (100th of a semitone)

(Source: [z64audio](https://github.com/z64tools/z64audio/blob/0436649b0b2bf8cfa1a7fe1d87d23532018d1722/lib/AudioConvert.c#L872))

A tuning of 1.0 with $R = 32 \ \mathrm{kHz}$ and $F = 0$ results in $N = 60$, the sample will be played at C4. A tuning larger than 1.0 will be a lower sound, and a tuning smaller than 1.0 will be a higher sound.

Given only the tuning value there is clearly no way to uniquely solve for all of `R`,`N`,`F`. Restricting all three to integers allows recovery up to a multiple of 2, however naively recomputing the above formula generally causes the resulting 32-bit floats to fail to match the originals.

It is possible to match in almost all cases using a lookup table approach

$$\tau = \frac{R}{32 \ \mathrm{kHz}} \cdot \mathrm{TBL}[N]$$<!--`-->where we have assumed that $F = 0$ and $R \in \mathbb{N}, N \in [0,128) \subset \mathbb{N}$.

The table $\mathrm{TBL}[N]$ is defined such that $\mathrm{TBL}[39] = 1.0$ and each adjacent entry differs by approximately $2^{1/12}$ for $N \in [0,128)$. It would be possible in principle to fine-tune a table via brute-force over a large collection of tuning data, however `gPitchFrequencies` (in [src/audio/lib/data.c](https://github.com/zeldaret/oot/blob/d307a3723313507726d5ebcfda0b5b9cef224d91/src/audio/lib/data.c#L407)) was found to work well for matching.

Using this method `R` and `N` can both be recovered, up to a multiple of 2, by using the floating-point error when recomputing the tuning as an oracle. The procedure is (working in 32-bit floats):
```
INPUT F32 tuning
OUTPUT TUPLE[INT, F32] match_candidates

FOR freq IN gPitchFrequencies:
    nominal_rate = INT((tuning / freq) * 32000.0)

    FOR test_rate IN {nominal_rate, nominal_rate - 1, nominal_rate + 1}:
        tuning2 = (test_rate / 32000.0) * freq
        diff = ABS(F32_BITS(tuning2) - F32_BITS(tuning))
        IF diff == 0:
            match_candidates.add(test_rate, freq)
```
Once all candidates that match on round-trip are determined, they should be sorted by some appropriate ranking scheme to produce a single value.

Candidates can be further narrowed down for a sample with multiple associated tuning values by assuming that the sample rate should be the same if the sample is the same. In this case the intersection of the sample rate results for each tuning must contain the correct sample rate.

Based on testing, the above seems to be untrue in general. In OoT, sample 33 of bank 0 has two tuning values that have no intersection in their results of the above procedure so the sample rates cannot be reconciled.

#### Drums

Drums are a single sound modulated by ADSR. In all but the first soundfont, the number of Drum pointers is 64. The list of Drum pointers is indexed by a semitone value, each Drum entry corresponds to that particular semitone. The Drum pointers in the Drum pointer list may be NULL, or multiple pointers will reference the same Drum structure, associating it with a range of semitones.

```c
typedef struct {
    /* 0x00 */ u8 adsrDecayIndex; // index for adsrDecayTable
    /* 0x01 */ u8 pan;
    /* 0x02 */ u8 isRelocated; // 0 in ROM
    /* 0x04 */ TunedSample tunedSample;
    /* 0x0C */ EnvelopePoint* envelope;
} Drum; // size = 0x10
```

Many drum entries share a sample but differ by a tuning value. The tuning values for drums sharing the same sample differ by a semitone while the samplerate is a property of the sample itself.

For example, soundfont 3 has 5 distinct samples that cover all 64 indices
![Plot of Drum Group from Soundfont 3](https://hackmd.io/_uploads/Hy--REA53.png)

These "drum groups" and their semitone ranges are enough information to fully reconstruct the entire drums array without listing every single entry.

#### Instruments

Instruments are sounds that are both modulated by ADSR and split across multiple samples for low, "normal" and high notes.

```c
typedef struct {
    /* 0x00 */ u8 isRelocated; // 0 in ROM
    /* 0x01 */ u8 normalRangeLo; // semitone, below this lowNotesSound is used
    /* 0x02 */ u8 normalRangeHi; // semitone, above this highNotesSound is used
    /* 0x03 */ u8 adsrDecayIndex; // index for adsrDecayTable
    /* 0x04 */ EnvelopePoint* envelope;
    /* 0x08 */ TunedSample lowPitchTunedSample;
    /* 0x10 */ TunedSample normalPitchTunedSample;
    /* 0x18 */ TunedSample highPitchTunedSample;
} Instrument; // size = 0x20
```

`normalRangeLo` and `normalRangeHi` are semitones that determine when to use the low notes sound, normal notes sound or high notes sound.

Some instruments may not have a low or high note sound, the sample pointer within `SoundFontSound` will be `NULL` and either `normalRangeLo` will be 0 for no low note sound or `normalRangeHi` will be 127 (0x7F) for no high note sound.

## Audioseq

### About

Audio sequences are responsible for combining various sounds together to generate sound effects and background music for the game to use. Audio sequences are made up of a series of commands for the "Music Macro Language", a custom bytecode that is interpreted and ran by the Audio driver. It is a cross between MIDI and a scripting language, it contains commands to select instruments, play and modulate notes, branch, loop and call subroutines.

Sequence commands are variable-length, anywhere from 1 to 5 bytes long. The command id is either the most significant 4 or 8 bits of the first byte. Command ids are different based on the current "section", that is whether it is a sequence, channel or layer that is currently interpreting the bytecode. In a well-formed sequence these sections would never overlap. Sequences begin in the sequence section.

There are 7 section types:
 - Sequence
    - Contains bytecode commands
    - Can spawn channels via `ldchan`
    - Can self-modify at runtime via `ldseq` and `stseq`
    - Can call routines dynmically through pointer tables via `dyncall`
 - Channel
    - Contains bytecode commands
    - Can spawn layers via `ldlayer`
    - Can call routines dynmically through pointer tables via `dyncall`
 - Layer
    - Contains bytecode commands
 - Array
    - Contains arbitrary bytes of data with arbitrary length
 - Table
    - Contains 16-bit pointer tables for use via `dyntbl` and `dyncall`
 - Envelope
    - Contains envelope data, same format as in the soundfonts
 - Filter
    - Contains filter data, 8x2 bytes per filter

The rough process for sequence disassembly is to start at the beginning of the file in a sequence section and proceed until a command terminating control-flow or an unconditional loop is encountered. As each command is decoded, track the channels and layers that are spawned and add them to a queue of addresses to start disassembling at next. Repeat until this no longer gives new regions.
This method works for many sequences but fails for sequences with more complex control flow such as tables. Tables are not bounds-checked so there is no concrete way to identify them.

For more details on individual commands refer to the [MML doc](https://github.com/MNGoldenEagle/oot/blob/7c7b4cea149f8b67af7c011037fb2ad7552dfda8/docs/Music%20Macro%20Language.md).

A complete list of the audio sequences can be found in [sequence.h](https://github.com/zeldaret/oot/blob/b8b983dd7fcc0870544c586bddda02bc85050f4c/include/sequence.h#L6).

#### "Program" sequences

Audio sequences can communicate with code via IO ports. Some sequences take advantage of this to do special things:
 - Sequence 0 uses IO ports to perform sound effects (`NA_SE_*` in `sfx.h`)
 - Sequence 1 is ambient/environment sounds, they change based on current area.
 - Sequence 2 is the Hyrule Field dynamic track switching
 - Sequence 109 is various cutscene effects, IO port 0 is used to communicate which effect to play ([list](https://github.com/MNGoldenEagle/oot/blob/3e298fa818569f9323138ec34f9e3127fa435b9f/src/audio/109_Cutscene_Effects.mus#L849-L865))

These sequences are more like programs than part of the soundtrack.

### Table (gSequenceTable)

The Audioseq table contains an entry for each audio sequence, allowing the sequence to be located in Audioseq and describing the cache policy.

#### gSequenceFontTable

Audioseq has another table in the main code file that follows a different format to the other tables. `gSequenceFontTable` controls which Soundfonts the audio sequence will use.

The table first contains a list of 16-bit offsets from the start of the table for all sequences. The data pointed to by the offset starts with an 8-bit number specifying the number of Soundfonts this sequence uses, followed by a list of 8-bit Soundfont IDs.

### Files

The files that make up Audioseq each contain a single audio sequence, made up of Music Macro Language commands and related data such as Envelopes and Channels. For an example, see the [disassembled Sequence 0](https://github.com/zeldaret/oot/blob/3e298fa818569f9323138ec34f9e3127fa435b9f/src/audio/000_Sound_Effects.mus).
