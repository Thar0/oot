# Soundfont XML Format Specification

Soundfont xmls always open with a Soundfont tag:
```xml
<Soundfont
    Name="<C Identifier>"
    Index="<uint>"
    Medium="<Medium>"
    CachePolicy="<CachePolicy>"
    SampleBank="<Path>"
    Indirect="<uint>"
    SampleBankDD="<Path>"
    IndirectDD="<uint>"
    LoopsHaveFrames="<bool>"
    PadToSize="<uint>"
    NumInstruments="<uint>"
>
<!-- Begins a new soundfont. -->
<!--    Name:  -->
<!--    Index:  -->
<!--    Medium:  -->
<!--    CachePolicy:  -->
<!--    SampleBank:  -->
<!--    Indirect:  -->
<!--    SampleBankDD:  -->
<!--    IndirectDD:  -->
<!--    LoopsHaveFrames:  -->
<!--    PadToSize:  -->
<!--    NumInstruments:  -->

    <Envelopes>

        <Envelope
            Name=""
            Release=""
        >

            <Point
                Delay=""
                Arg=""
            />

            <Disable/>

            <Hang/>

            <Goto
                Index=""
            />
        </Envelope>

    </Envelopes>

    <Samples>

        <Sample
            Name=""
            SampleRate=""
            BaseNote=""
            IsDD=""
            Cached=""
        />
    </Samples>

    <Effects>

        <Effect
            Name=""
            Sample=""
            SampleRate=""
            BaseNote=""
        />

    </Effects>

    <Drums>

        <Drum
            Name=""
            Note=""
            NoteStart=""
            NoteEnd=""
            Pan=""
            Envelope=""
            Release=""
            Sample=""
            SampleRate=""
            BaseNote=""
        />

    </Drums>

    <Instruments>

        <Instrument
            ProgramNumber=""
            Name=""
            Envelope=""
            Release=""

            Sample=""
            SampleRate=""
            BaseNote=""

            RangeLo=""
            SampleLo=""
            SampleRateLo=""
            BaseNoteLo=""

            RangeHi=""
            SampleHi=""
            SampleRateHi=""
            BaseNoteHi=""
        />

    </Instruments>

</Soundfont>
```
- `Name`            : Soundfont symbol name. Must be a valid C identifier.
- `Index`           : Soundfont index. Must be an integer.
- `Medium`          : Storage medium. Must be an enum name from `SampleMedium`.
- `CachePolicy`     : Cache policy. Must be an enum name from `AudioCacheLoadType`.
- `SampleBank`      : Path to samplebank xml used by this soundfont.
- `Indirect`        : Pointer index if the samplebank is referenced indirectly.
- `SampleBankDD`    : Path to samplebank xml used for DD medium.
- `IndirectDD`      : Pointer index if the DD samplebank is referenced indirectly.
- `LoopsHaveFrames` : Whether loops in this soundfont store the total frame count of the sample. Must be a boolean.
- `PadToSize`       : For matching only. Specifies the total file size the result output should be padded to.
- `NumInstruments`  : For matching only. Specifies the total number of instrument pointers. Usually this is automatically assigned based on `max(program_number) + 1` but some vanilla banks don't match this way.

