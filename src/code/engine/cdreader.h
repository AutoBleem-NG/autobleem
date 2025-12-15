// CD image reader classes for scanner
//
// Provides abstract interface for reading CD-ROM images in various formats:
// - CDReader: Base class for raw BIN/ISO images
// - CHDReader: Subclass for MAME CHD (Compressed Hunks of Data) format
//
// Used by the game scanner to extract serial numbers and metadata from
// PlayStation disc images.

#pragma once

#include "../main.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <libchdr/chd.h>

#include <stdio.h>
#include <string.h>

// CD-ROM sector sizes
#define SECTOR_SIZE 2352      // Raw sector size (sync + header + data + ECC/EDC)
#define DATA_SIZE 2048        // User data portion of a sector
#define MAX_OFFSET 500        // Maximum bytes to search when calibrating
#define CD_FRAME_SIZE 2448    // Full frame with subcode (2352 + 96)

using namespace std;

// Helper class to read bin file as "CDROM" data
class CDReader
{
private:
    ifstream *stream = nullptr;
    int offset = 0;
    char *buffer = nullptr;
    int sectorpos = 0;
    int currentSector = 0;
    bool opened = false;

public:
    void setOffset(int off)
    {
        offset = off;
    }
    int getOffset()
    {
        return offset;
    }
    void setBuffer(char *buff)
    {
        buffer = buff;
    }
    char *getBuffer()
    {
        return buffer;
    }
    int getSectorPos()
    {
        return sectorpos;
    }
    void setSectorpos(int pos)
    {
        sectorpos = pos;
    }

    void setCurrentSector(int pos)
    {
        currentSector = pos;
    }
    int getCurrentSector()
    {
        return currentSector;
    }
    bool isOpen()
    {
        return opened;
    }
    void setOpen(bool state)
    {
        opened =state;
    }

    int calibrate(int maxOffset)
    {
        unsigned char c = 0;
        selectSector(16); // go to sector 16
        for (int i = 0; i < maxOffset; i++)
        {
            c = readChar();
            if (c != 1)
            {
                c = readChar();
            }
            else
            {
                int sectorPosNow = getSectorPos();
                int curSectorNow = getSelSector();
                string str = readString(5);
                rev(5);
                if (str == "CD001")
                {
                    cout << "CD001 found in sector:" << curSectorNow << " at pos " << sectorPosNow << endl;
                    offset = (curSectorNow - 16) * SECTOR_SIZE + sectorPosNow - 1;
                    return 0;
                }
                else
                {
                    c = readChar();
                }
            }
        }
        return -1;
    }
    void ffd(int size)
    {
        for (int i = 0; i < size; i++)
        {
            sectorpos++;
            if (sectorpos >= DATA_SIZE)
            {
                selectSector(currentSector + 1);
            }
        }
    }
    void rev(int size)
    {
        for (int i = 0; i < size; i++)
        {
            sectorpos--;
            if (sectorpos < 0)
            {
                selectSector(currentSector - 1);
                sectorpos = (DATA_SIZE - 1);
            }
        }
    }
    unsigned char readChar()
    {
        unsigned char x;
        x = buffer[sectorpos];
        sectorpos++;
        if (sectorpos >= DATA_SIZE)
        {
            selectSector(currentSector + 1);
        }
        return x;
    }
    std::string readString(int size)
    {
        char str[size + 1];
        str[size] = 0;
        for (int i = 0; i < size; i++)
        {
            str[i] = readChar();
        }
        return str;
    }
    unsigned long readDword()
    {
        unsigned long res = 0;
        unsigned long c;
        c = readChar();
        res += c;
        c = readChar();
        res += c << (1 * 8);
        c = readChar();
        res += c << (2 * 8);
        c = readChar();
        res += c << (3 * 8);
        return res;
    }

    virtual void selectSector(int sectorNum)
    {
        int addr = sectorNum * SECTOR_SIZE + offset;
        stream->seekg(addr, ios::beg);
        stream->read(buffer, SECTOR_SIZE);
        sectorpos = 0;
        currentSector = sectorNum;
    }
    int getSelSector()
    {
        return currentSector;
    }
    virtual int openImage(string imagePath)
    {
        buffer = new char[CD_FRAME_SIZE];
        cout << "Opening ISO image" << endl;
        offset = 0;
        opened = false;
        sectorpos = 0;
        currentSector = 0;
        stream = new ifstream(imagePath, ios::binary | ios::in);
        if (!stream->good())
        {
            delete stream;
            return -1;
        }
        if (!stream->is_open())
        {
            delete stream;
            return -1;
        }
        stream->seekg(0);
        if (calibrate(MAX_OFFSET) != 0)
        {
            return -1;
        };
        opened = true;

        return 0;
    }
    virtual void closeImage()
    {
        if (buffer != nullptr) {
            delete[] buffer;
            buffer = nullptr;
        }
        if (stream != nullptr) {
            delete stream;
            stream = nullptr;
        }
        opened = false;
    }

    virtual bool endStream()
    {
        return stream->tellg() == -1;
    }
};

// CHDReader - reads MAME CHD (Compressed Hunks of Data) disc images
//
// CHD files store CD-ROM data in compressed "hunks". Each hunk contains
// multiple frames, and each frame represents one CD sector plus subcode.
// The libchdr library handles decompression transparently.
//
// CHD structure:
//   - Header: Contains hunkbytes (bytes per hunk) and unitbytes (bytes per frame)
//   - Metadata: Track information stored as tagged strings (CDROM_TRACK_METADATA_TAG)
//   - Data: Compressed hunks containing frame data
//
// Frame layout depends on track type:
//   - MODE1:      2048 bytes user data (no header)
//   - MODE1_RAW:  12 sync + 4 header + 2048 data + 288 ECC = 2352 bytes
//   - MODE2_RAW:  12 sync + 4 header + 8 subheader + 2048 data + 280 ECC = 2352 bytes
//
class CHDReader : public CDReader
{
private:
    chd_file *chd = nullptr;              // libchdr file handle
    uint8_t *hunkCache = nullptr;         // Cached decompressed hunk data
    uint32_t cachedHunk = static_cast<uint32_t>(-1); // Currently cached hunk number (-1 = none)
    uint32_t hunkBytes = 0;            // Bytes per hunk (from CHD header)
    uint32_t unitBytes = 0;            // Bytes per frame/unit (from CHD header)
    uint32_t framesPerHunk = 0;        // Number of frames in each hunk
    uint32_t trackFrames = 0;          // Total frames in track 1
    uint32_t trackStartOffset = 0;     // Byte offset to user data within frame

    // Parse track metadata from CHD to get frame count and track type.
    // Tries CDROM_TRACK_METADATA2_TAG first (has pregap/postgap info),
    // falls back to CDROM_TRACK_METADATA_TAG (simpler format).
    // Only reads track 0 (first/data track) since we just need game data.
    bool parseTrackMetadata()
    {
        char metadata[256];
        uint32_t metalen;
        chd_error err;

        err = chd_get_metadata(chd, CDROM_TRACK_METADATA2_TAG, 0,
                               metadata, sizeof(metadata), &metalen, nullptr, nullptr);
        if (err == CHDERR_NONE)
        {
            int tracknum;
            char type[32], subtype[32], pgtype[32], pgsub[32];
            int frames, pregap, postgap;
            if (sscanf(metadata, CDROM_TRACK_METADATA2_FORMAT,
                       &tracknum, type, subtype, &frames, &pregap, pgtype, pgsub, &postgap) >= 4)
            {
                trackFrames = frames;
                parseTrackType(type);
                return true;
            }
        }

        err = chd_get_metadata(chd, CDROM_TRACK_METADATA_TAG, 0,
                               metadata, sizeof(metadata), &metalen, nullptr, nullptr);
        if (err == CHDERR_NONE)
        {
            int tracknum;
            char type[32], subtype[32];
            int frames;
            if (sscanf(metadata, CDROM_TRACK_METADATA_FORMAT,
                       &tracknum, type, subtype, &frames) >= 4)
            {
                trackFrames = frames;
                parseTrackType(type);
                return true;
            }
        }

        return false;
    }

    // Set trackStartOffset based on track type string from metadata.
    // This offset indicates where user data begins within each frame:
    //   - MODE1/MODE2/MODE2_FORM1: Data starts at offset 0 (no sync/header)
    //   - MODE1_RAW: Skip 16 bytes (12 sync + 4 header)
    //   - MODE2_RAW: Skip 24 bytes (12 sync + 4 header + 8 subheader)
    void parseTrackType(const char *type)
    {
        trackStartOffset = 0;

        if (strcmp(type, "MODE1") == 0 || strcmp(type, "MODE1/2048") == 0)
        {
            trackStartOffset = 0;
        }
        else if (strcmp(type, "MODE1_RAW") == 0 || strcmp(type, "MODE1/2352") == 0)
        {
            trackStartOffset = 16;
        }
        else if (strcmp(type, "MODE2") == 0 || strcmp(type, "MODE2/2336") == 0)
        {
            trackStartOffset = 0;
        }
        else if (strcmp(type, "MODE2_RAW") == 0 || strcmp(type, "MODE2/2352") == 0)
        {
            trackStartOffset = 24;
        }
        else if (strcmp(type, "MODE2_FORM1") == 0)
        {
            trackStartOffset = 0;
        }
    }

    // Read a single frame from the CHD file.
    // Uses hunk caching to avoid repeated decompression of the same hunk.
    // Each hunk contains multiple frames; we calculate which hunk contains
    // the requested frame and the offset within that hunk.
    chd_error readFrame(uint32_t frameNum, uint8_t *frameBuf)
    {
        uint32_t hunkNum = frameNum / framesPerHunk;
        uint32_t offsetInHunk = (frameNum % framesPerHunk) * unitBytes;

        // Cache the hunk if not already loaded
        if (hunkNum != cachedHunk)
        {
            chd_error err = chd_read(chd, hunkNum, hunkCache);
            if (err != CHDERR_NONE)
                return err;
            cachedHunk = hunkNum;
        }

        // Copy frame data from cached hunk
        memcpy(frameBuf, hunkCache + offsetInHunk, unitBytes);
        return CHDERR_NONE;
    }

public:
    // Release all resources: buffer, hunk cache, and CHD file handle
    void closeImage() override
    {
        if (getBuffer() != nullptr) {
            delete[] getBuffer();
            setBuffer(nullptr);
        }

        if (hunkCache != nullptr)
        {
            delete[] hunkCache;
            hunkCache = nullptr;
        }

        if (chd != nullptr)
        {
            chd_close(chd);
            chd = nullptr;
        }

        setOpen(false);
    }

    // Check if we've reached the end of the track data
    bool endStream() override
    {
        if (getCurrentSector() > static_cast<int>(trackFrames))
            setCurrentSector(trackFrames - 1);
        return (getCurrentSector() == static_cast<int>(trackFrames) - 1) && (getSectorPos() >= DATA_SIZE);
    }

    // Open a CHD file and prepare for reading.
    // Steps:
    //   1. Open CHD file with libchdr
    //   2. Read header to get hunk/unit sizes
    //   3. Validate it's a CD-ROM CHD (unitBytes must divide hunkBytes evenly)
    //   4. Allocate buffers for frame data and hunk cache
    //   5. Parse track metadata to get frame count and track type
    //   6. Calibrate to find ISO9660 volume descriptor (CD001 signature)
    int openImage(string imagePath) override
    {
        cout << "Opening CHD image" << endl;
        setOpen(false);
        setSectorpos(0);
        setCurrentSector(0);

        // Open the CHD file
        chd_error err = chd_open(imagePath.c_str(), CHD_OPEN_READ, nullptr, &chd);
        if (err != CHDERR_NONE)
        {
            cout << "Error opening CHD file: " << chd_error_string(err) << endl;
            return -1;
        }

        // Get CHD header for size information
        const chd_header *header = chd_get_header(chd);
        if (header == nullptr)
        {
            cout << "Error getting CHD header" << endl;
            goto cleanup_chd;
        }

        hunkBytes = header->hunkbytes;
        unitBytes = header->unitbytes;

        // Validate this is a CD-ROM CHD: unitBytes should be frame size,
        // and hunkBytes should contain a whole number of frames
        if (unitBytes == 0 || hunkBytes % unitBytes != 0)
        {
            cout << "Not a CD-ROM CHD (invalid unit size)" << endl;
            goto cleanup_chd;
        }

        // Allocate buffers
        framesPerHunk = hunkBytes / unitBytes;
        setBuffer(new char[unitBytes]);
        hunkCache = new uint8_t[hunkBytes];
        cachedHunk = static_cast<uint32_t>(-1);

        // Parse track metadata to get frame count and determine data offset
        if (!parseTrackMetadata())
        {
            cout << "Failed to parse CD track metadata" << endl;
            goto cleanup_all;
        }

        cout << "TOC found - track has " << trackFrames << " frames" << endl;
        if (trackFrames < 1)
        {
            cout << "Track has no frames" << endl;
            goto cleanup_all;
        }

        // Calibrate: find the ISO9660 primary volume descriptor (CD001)
        // This determines any additional offset needed for reading
        if (calibrate(MAX_OFFSET) != 0)
        {
            cout << "Calibrate failed" << endl;
            goto cleanup_all;
        }

        setOpen(true);
        return 0;

    cleanup_all:
        // Clean up allocated buffers and CHD file
        if (hunkCache != nullptr)
        {
            delete[] hunkCache;
            hunkCache = nullptr;
        }
        if (getBuffer() != nullptr)
        {
            delete[] getBuffer();
            setBuffer(nullptr);
        }
    cleanup_chd:
        // Clean up CHD file handle
        if (chd != nullptr)
        {
            chd_close(chd);
            chd = nullptr;
        }
        return -1;
    }

    // Read a sector and extract the 2048-byte user data portion.
    // The frame is read into the buffer, then user data is moved to
    // the front (skipping sync/header bytes for RAW track types).
    void selectSector(int sectorNum) override
    {
        char *buff = getBuffer();

        if (readFrame(sectorNum, reinterpret_cast<uint8_t*>(buff)) == CHDERR_NONE)
        {
            // For RAW track types, skip the sync/header to get user data
            if (trackStartOffset > 0)
            {
                memmove(buff, buff + trackStartOffset, DATA_SIZE);
            }
        }
        else
        {
            memset(buff, 0, DATA_SIZE);
        }

        setSectorpos(0);
        setCurrentSector(sectorNum);
    }
};
