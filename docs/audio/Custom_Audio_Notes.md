# Custom Audio Notes

## BGM

To add bgm you would:
- If using any custom samples, add the wav to the samplebank that the soundfont will use
  - Put wav file in one of the subdirectories in `assets/samples`
  - Add path to wav file to the corresponding samplebank xml in `assets/samplebanks`, prefixing it with `$(BUILD_DIR)` and replacing `.wav` with `.aifc` to match the other paths
- If using a custom soundfont, add the soundfont `.xml` file to `assets/soundfonts`
- Add the sequence `.seq` file to `assets/sequences`
- Add both to the spec in the appropriate locations
- Add an entry in `include/tables/sequence_table.h`

There is no current tooling to produce `.seq` from `.midi` or `.xml` from `.sf2/.sfz/other`.

It is possible to convert seq64 output to `.seq` by using a sequence assembly file that resembles
```mips
#include "aseq.h"
#include "MySoundfont.h"

.startseq MySequence

.incbin "MySequence_Seq64.aseq"

.endseq MySequence
```
where
 - `MySoundfont` is the name of the soundfont to use
 - `MySequence` is the name of the sequence
 - `MySequence_Seq64.aseq` is the seq64 output binary

This is not recommended, but it is at least better than disassembling seq64 output with `disassemble_sequence.py` since there is no guarantee that the output will be correct.

There is no convenient way to produce a soundfont `.xml` automatically. If using a custom soundfont it must be created manually, either from scratch or by referencing an existing soundfont. It is possible in principle to convert a `.sf2` or `.sfz` to a soundfont `.xml`, but this is so far incomplete.

---

## Sound Effects

To add a new sound effect in the way the driver expects is a somewhat lengthy process:
- Add the wav to Samplebank 0
  - Put wav file in `assets/samples/Samplebank_0`
  - Add path to wav file to `assets/samplebanks/Samplebank_0.xml`, prefixing it with `$(BUILD_DIR)` and replacing `.wav` with `.aifc` to match the other paths
- Add an sfx entry to one of the headers in `include/tables/sfx/`, whichever feels most appropriate
  - The first entry in `DEFINE_SFX` is the name of the assembly routine that will play the sound effect
  - The second entry in `DEFINE_SFX` is the name you'll use to trigger your sound effect in code (that is, it's the enum name you'll pass to `Audio_PlaySfxGeneral` or similar functions)
- Register the sample as an Effect in either Soundfont 0 or 1
  - Use Soundfont 1 if this effect is in the enemy bank, otherwise use Soundfont 0
  - Add the sample name to the `<Samples>` region of the xml, e.g. `<Sample Name="samplename"/>`
  - In the `<Effects>` region add a new entry, e.g. `<Effect Name="EFFECT_MY_EFFECT" Sample="samplename"/>`
- Finally, add the assembly routine to `seq_0.prg.seq` somewhere in the file above the `SEQ_0_END` label, e.g. if your assembly routine was called `my_sound_effect` and the effect name in the Soundfont is `EFFECT_MY_EFFECT` then you'd want to write something like (note that the effect name in the Soundfont is prefixed by `SFn_` in the sequence file, where `n` is the Index of the Soundfont e.g. `SF0_` for Soundfont 0 or `SF1_` for Soundfont 1)
    ```mips
    .channel my_sound_effect
        ldlayer     0, my_sound_effect_layer
        end

    .layer my_sound_effect_layer
        instr       FONTANY_INSTR_SFX
        notedv      SF0_EFFECT_MY_EFFECT, 0, 100
        end
    ```
  - If you get an error on the `notedv` line involving `"non-constant expression"` and/or `"invalid operands (*ABS* and *UND* sections)"`, double-check the Soundfont steps.
  - If you get `"Error: value [...] out of range for unsigned 6-bit argument"` on the `notedv` line, the note is out of range of `0..63` and needs to be transposed into that range:
    ```
    .layer my_sound_effect_layer
        transpose   K
        instr       FONTANY_INSTR_SFX
        notedv      (SF0_EFFECT_MY_EFFECT - K * 64), 0, 100
        end
    ```
    `K` should be chosen to be the smallest number such that the result of `SF0_EFFECT_MY_EFFECT - K * 64` is less than 64, which is immediately found by calculating `floor(SF0_EFFECT_MY_EFFECT / 64)`. e.g. if `SF0_EFFECT_MY_EFFECT` was valued at 136, `K` should be `2`. You'd then write
    ```
    .layer my_sound_effect_layer
        transpose   2
        instr       FONTANY_INSTR_SFX
        notedv      (SF0_EFFECT_MY_EFFECT - 2 * 64), 0, 100
        end
    ```
