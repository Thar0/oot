# Samplebank XML Format Specification

Samplebank XMLs describe which sample files to include in a particular samplebank.

---

```xml
<SampleBank
    Name="<C Indentifier>"
    Index="<uint>"
    Medium="<Medium>"
    CachePolicy="<CachePolicy>"
    BufferBug="[bool]"
>
```
Begins a new samplebank.

**Properties**

- **Name**: The name of the samplebank.
- **Index**: The index of the samplebank in the samplebank table. Must be a unique index.
- **Medium**: The storage medium, from the SampleMedium enum.
- **CachePolicy**: The cache policy, from the AudioCacheLoadType enum.
- <ins>[Optional]</ins> **BufferBug**: Whether this samplebank suffers from a buffer clearing bug present in the original audio tools. For matching only.

**Sub-Attributes**

-
    ```xml
    <Pointer
        Index="<uint>"
    />
    ```
    Create an alternate index that refers to this samplebank.

    **Properties**

    - **Index**: The alternative index, must be unique like the 'true' index.

    ---

-
    ```xml
    <Sample
        Name="<C Identifier>"
        Path="<Path>"
    />
    ```
    Adds a sample to the samplebank. Should be a VADPCM compressed AIFC file.

    **Properties**

    - **Name**: Name of this sample.
    - **Path**: Path to aifc file (typically in `$(BUILD_DIR)/assets/samples/`)

    ---

-
    ```xml
    <Blob
        Name="<C Identifier>"
        Path="<Path>"
    />
    ```
    Adds a binary blob to the samplebank. Intended for matching only when data cannot be identified.

    **Properties**

    - **Name**: Name of this blob.
    - **Path**: Path to binary file, relative to project root (typically in `assets/samples/`)

    ---

```xml
</SampleBank>
```
---
