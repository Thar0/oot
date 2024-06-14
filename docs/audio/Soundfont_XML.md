# Soundfont XML Format Specification

Soundfont xmls always open with a Soundfont tag:
```xml
<Soundfont
    Name="<C Identifier>"
    Index="<uint>"
    Medium="<Medium>"
    CachePolicy="<CachePolicy>"
    SampleBank="<Path>"
    Indirect="[uint]"
    SampleBankDD="[Path]"
    IndirectDD="[uint]"
    LoopsHaveFrames="[bool]"
    PadToSize="[uint]"
    NumInstruments="[uint]"
>
<!-- Begins a new soundfont. -->
<!--    Name: Soundfont symbol name. Must be a valid C identifier. -->
<!--    Index: Soundfont index. Must be an integer. -->
<!--    Medium: Storage medium. Must be an enum name from `SampleMedium`. -->
<!--    CachePolicy: Cache policy. Must be an enum name from `AudioCacheLoadType`. -->
<!--    SampleBank: Path to samplebank xml used by this soundfont. -->
<!--    Indirect[Optional]: Pointer index if the samplebank is referenced indirectly. -->
<!--    SampleBankDD[Optional]: Path to samplebank xml used for DD medium. -->
<!--    IndirectDD[Optional]: Pointer index if the DD samplebank is referenced indirectly. -->
<!--    LoopsHaveFrames[Optional]: Whether loops in this soundfont store the total frame count of the sample. Must be a boolean. -->
<!--    PadToSize[Optional]: For matching only. Specifies the total file size the result output should be padded to. -->
<!--    NumInstruments[Optional]: For matching only. Specifies the total number of instrument pointers. Usually this is automatically assigned based on `max(program_number) + 1` but some vanilla banks don't match this way. -->

    <Envelopes>

        <Envelope
            Name="<C Identifier>"
            Release="<u8>"
        >
        <!-- Starts a new envelope. -->
        <!--    Name: Unique name for this envelope. Must be a valid C identifier. -->
        <!--    Release: Release rate index for this envelope -->

            <Point
                Delay="<s16>"
                Arg="<s16>"
            />
            <!-- Add a point to the envelope at (delay, arg) -->
            <!--    Delay: Duration until the next point -->
            <!--    Arg: Value of the envelope at this point -->

            <Disable/>
            <!-- Insert a ADSR_DISABLE command -->

            <Hang/>
            <!-- Insert a ADSR_HANG command -->

            <Goto
                Index="<uint>"
            />
            <!-- Insert a ADSR_GOTO command -->
            <!--    Index: Index of the envelope point to jump to -->

        </Envelope>

    </Envelopes>

    <Samples
        IsDD="[Bool]"
        Cached="[Bool]"
    >
    <!--  -->
    <!-- IsDD[Optional]:  -->
    <!-- Cached[Optional]:  -->

        <Sample
            Name="<C Identifier>"
            SampleRate="[Sample Rate]"
            BaseNote="[Note Number]"
            IsDD="[Bool]"
            Cached="[Bool]"
        />
        <!--  -->
        <!--    Name:  -->
        <!--    SampleRate[Optional]:  -->
        <!--    BaseNote[Optional]:  -->
        <!--    IsDD[Optional]:  -->
        <!--    Cached[Optional]:  -->

    </Samples>

    <Effects>

        <Effect
            Name="<C Identifier>"
            Sample="<Sample Name>"
            SampleRate="[Sample Rate]"
            BaseNote="[Note Number]"
        />
        <!--  -->
        <!--    Name:  -->
        <!--    Sample:  -->
        <!--    SampleRate[Optional]:  -->
        <!--    BaseNote[Optional]:  -->

    </Effects>

    <Drums>

        <Drum
            Name="<C Identifier>"
            Note="[Note Number]"
            NoteStart="[Note Number]"
            NoteEnd="[Note Number]"
            Pan="<u8>"
            Envelope="<Envelope Name>"
            Release="[u8]"
            Sample="<Sample Name>"
            SampleRate="[Sample Rate]"
            BaseNote="[Note Number]"
        />
        <!--  -->
        <!--    Name:  -->
        <!--    Note[Optional]:  -->
        <!--    NoteStart[Optional]:  -->
        <!--    NoteEnd[Optional]:  -->
        <!--    Pan:  -->
        <!--    Envelope:  -->
        <!--    Release[Optional]:  -->
        <!--    Sample:  -->
        <!--    SampleRate[Optional]:  -->
        <!--    BaseNote[Optional]:  -->

    </Drums>

    <Instruments>

        <Instrument
            ProgramNumber="<>"
            Name="<C Identifier>"
            Envelope="<Envelope Name>"
            Release="[u8]"

            Sample="<Sample Name>"
            SampleRate="[Sample Rate]"
            BaseNote="[Note Number]"

            RangeLo="[Note Number]"
            SampleLo="[Sample Name]"
            SampleRateLo="[Sample Rate]"
            BaseNoteLo="[Note Number]"

            RangeHi="[Note Number]"
            SampleHi="[Sample Name]"
            SampleRateHi="[Sample Rate]"
            BaseNoteHi="[Note Number]"
        />
        <!--  -->
        <!--    ProgramNumber: MIDI Program Number for this instrument. Must be in the range 0 <= n <= 125 -->
        <!--    Name: The name of this instrument. -->
        <!--    Envelope: Envelope to use, identified by name. -->
        <!--    Release[Optional]: Release rate index override -->
        <!--    Sample:  -->
        <!--    SampleRate[Optional]: Sample rate override -->
        <!--    BaseNote[Optional]: Base note override -->
        <!--    RangeLo[Optional]:  -->
        <!--    SampleLo[Optional]:  -->
        <!--    SampleRateLo[Optional]:  -->
        <!--    BaseNoteLo[Optional]:  -->
        <!--    RangeHi[Optional]:  -->
        <!--    SampleHi[Optional]:  -->
        <!--    SampleRateHi[Optional]:  -->
        <!--    BaseNoteHi[Optional]:  -->

    </Instruments>

</Soundfont>
