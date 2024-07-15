# Soundfont XML Format Specification

Soundfont XMLs

---

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
```
Begins a new soundfont.

**Properties**
- **Name**: Soundfont symbol name. Must be a valid C identifier.
- **Index**: Soundfont index. Must be an integer.
- **Medium**: Storage medium. Must be an enum name from `SampleMedium`.
- **CachePolicy**: Cache policy. Must be an enum name from `AudioCacheLoadType`.
- **SampleBank**: Path to samplebank xml used by this soundfont.
- <ins>[Optional]</ins> **Indirect**: Pointer index if the samplebank is referenced indirectly.
- <ins>[Optional]</ins> **SampleBankDD**: Path to samplebank xml used for DD medium.
- <ins>[Optional]</ins> **IndirectDD**: Pointer index if the DD samplebank is referenced indirectly.
- <ins>[Optional]</ins> **LoopsHaveFrames**: Whether loops in this soundfont store the total frame count of the sample. Must be a boolean.
- <ins>[Optional]</ins> **PadToSize**: For matching only. Specifies the total file size the result output should be padded to.
- <ins>[Optional]</ins> **NumInstruments**: For matching only. Specifies the total number of instrument pointers. Usually this is automatically assigned based on `max(program_number) + 1` but some vanilla banks don't match this way.

**Sub-Attributes**

-
    ```xml
    <Envelopes>
    ```
    TODO

    **Properties**

    N/A

    **Sub-Attributes**

    -
        ```xml
        <Envelope
            Name="<C Identifier>"
            Release="<u8>"
        >
        ```
        Starts a new envelope.

        **Properties**

        - **Name**: Unique name for this envelope. Must be a valid C identifier.
        - **Release**: Release rate index for this envelope

        **Sub-Attributes**

        -
            ```xml
            <Point
                Delay="<s16>"
                Arg="<s16>"
            />
            ```
            Add a point to the envelope at (delay, arg)

            **Properties**

            - **Delay**: Duration until the next point
            - **Arg**: Value of the envelope at this point

            ---

        -
            ```xml
            <Disable/>
            ```
            Insert a ADSR_DISABLE command

            ---

        -
            ```xml
            <Hang/>
            ```
            Insert a ADSR_HANG command

            ---

        -
            ```xml
            <Goto
                Index="<uint>"
            />
            ```
            Insert a ADSR_GOTO command

            **Properties**

            - **Index**: Index of the envelope point to jump to

            ---

        ```xml
        </Envelope>
        ```
        ---

    ```xml
    </Envelopes>
    ```
    ---

-
    ```xml
    <Samples
        IsDD="[Bool]"
        Cached="[Bool]"
    >
    ```
    TODO

    **Properties**

    - <ins>[Optional]</ins> **IsDD**: TODO
    - <ins>[Optional]</ins> **Cached**: TODO

    **Sub-Attributes**

    -
        ```xml
        <Sample
            Name="<C Identifier>"
            SampleRate="[Sample Rate]"
            BaseNote="[Note Number]"
            IsDD="[Bool]"
            Cached="[Bool]"
        />
        ```
        TODO

        **Properties**

        - **Name**: TODO
        - <ins>[Optional]</ins> **SampleRate**: TODO
        - <ins>[Optional]</ins> **BaseNote**: TODO
        - <ins>[Optional]</ins> **IsDD**: TODO
        - <ins>[Optional]</ins> **Cached**: TODO

        ---

    ```xml
    </Samples>
    ```
    ---

-
    ```xml
    <Effects>
    ```
    TODO

    **Properties**

    N/A

    **Sub-Attributes**

    -
        ```xml
        <Effect
            Name="<C Identifier>"
            Sample="<Sample Name>"
            SampleRate="[Sample Rate]"
            BaseNote="[Note Number]"
        />
        ```
        TODO

        **Properties**
        - **Name**: TODO
        - **Sample**: TODO
        - <ins>[Optional]</ins> **SampleRate**: TODO
        - <ins>[Optional]</ins> **BaseNote**: TODO

        ---

    ```xml
    </Effects>
    ```
    ---

-
    ```xml
    <Drums>
    ```
    TODO

    **Properties**

    N/A

    **Sub-Attributes**

    -
        ```xml
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
        ```
        TODO

        **Properties**
        - **Name**: TODO
        - <ins>[Optional]</ins> **Note**: TODO
        - <ins>[Optional]</ins> **NoteStart**: TODO
        - <ins>[Optional]</ins> **NoteEnd**: TODO
        - **Pan**: TODO
        - **Envelope**: TODO
        - <ins>[Optional]</ins> **Release**: TODO
        - **Sample**: TODO
        - <ins>[Optional]</ins> **SampleRate**: TODO
        - <ins>[Optional]</ins> **BaseNote**: TODO

        ---

    ```xml
    </Drums>
    ```
    ---

-
    ```xml
    <Instruments>
    ```
    TODO

    **Properties**

    N/A

    **Sub-Attributes**

    -
        ```xml
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
        ```
        TODO

        **Properties**
        - **ProgramNumber**: MIDI Program Number for this instrument. Must be in the range `0 <= n <= 125`
        - **Name**: The name of this instrument.
        - **Envelope**: Envelope to use, identified by name.
        - <ins>[Optional]</ins> **Release**: Release rate index override
        - **Sample**: TODO
        - <ins>[Optional]</ins> **SampleRate**: Sample rate override
        - <ins>[Optional]</ins> **BaseNote**: Base note override
        - <ins>[Optional]</ins> **RangeLo**: TODO
        - <ins>[Optional]</ins> **SampleLo**: TODO
        - <ins>[Optional]</ins> **SampleRateLo**: TODO
        - <ins>[Optional]</ins> **BaseNoteLo**: TODO
        - <ins>[Optional]</ins> **RangeHi**: TODO
        - <ins>[Optional]</ins> **SampleHi**: TODO
        - <ins>[Optional]</ins> **SampleRateHi**: TODO
        - <ins>[Optional]</ins> **BaseNoteHi**: TODO

        ---

    ```xml
    </Instruments>
    ```
    ---

```xml
</Soundfont>
```
---
