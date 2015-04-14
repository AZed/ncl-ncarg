/*
 * $Id: crotiff.c,v 1.1 2010/04/02 16:36:16 brownrig Exp $
 *
 * crotiff.c
 *
 * A simplified TIFF writer, tailored to the requirements for writing images
 * as generated by the cairo driver. Specifically, the images are assumed to be RGBa, and we
 * are not tiling the output.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <cairo/cairo.h>
#include "gksc.h"

/* constants from the tiff spec. */
const uint16_t MagicNum     = 42;

#ifdef ByteSwapped
const uint16_t endianness = 0x4949;   /* "II" */
#else
const uint16_t endianness = 0x4D4D;   /* "MM" */
#endif

/* tiff Image File Header */
typedef struct IFHeader {
    uint16_t   byteOrder;
    uint16_t   magicNum;
    uint32_t   ifdOffset;
} IFHeader;

/* Entries in the tiff Image File Directory */
typedef struct IFDEntry {
    uint16_t   tag;
    uint16_t   type;
    uint32_t   count;
    uint32_t   valOffset;
} IFDEntry;

/* tiff Image File Directory */
typedef struct IFDirectory {
    uint16_t   numEntries;
    uint16_t   maxEntries;
    IFDEntry*   entries;
    uint32_t   nextIFD;
} IFDirectory;

/* a utility struct to swizzle the RGBa components; cairo and tiff have differing conventions. */
typedef union {
    uint32_t value;
    unsigned char swizzle[4];
} Argb2RgbA;

const int IFD_GROW_SIZE = 25;  /* amount by which to grow the IFD table, as needed */
const int RowsPerStrip = 8;    /* our strip size; 8 is pretty common, esp. for JPG compression */

/* The type fields for IFDEntries. This is all we're dealing with for the moment;
 * if this list changes, see addIFDEntry() below for proper packing of values/offsets
 * into the IFDEntry (ex., recall values smaller than 4 bytes are packed into the upper part
 * of valOffset field).
 */
const uint16_t SHORT_TYPE    = 3;
const uint16_t LONG_TYPE     = 4;

/* Tiff-Tags of interest here... */
const uint16_t TAG_ImageWidth      = 256;
const uint16_t TAG_ImageLength     = 257;
const uint16_t TAG_BitsPerSample   = 258;
const uint16_t TAG_Compression     = 259;
const uint16_t TAG_PhotoInterp     = 262;
const uint16_t TAG_StripOffsets    = 273;
const uint16_t TAG_SamplesPerPixel = 277;
const uint16_t TAG_RowsPerStrip    = 278;
const uint16_t TAG_StripByteCount  = 279;
const uint16_t TAG_XResolution     = 282;
const uint16_t TAG_TYresolution    = 283;
const uint16_t TAG_PlanarConfig    = 284;
const uint16_t TAG_ResolutionUnit  = 296;
const uint16_t TAG_ExtraSamples    = 338;

/* forward ref. */
static void addIFDEntry(IFDirectory* ifd, int16_t tag, uint16_t type,
        uint32_t count, uint32_t value);
static void writeIFD(IFDirectory* ifd, FILE* out);

int
crotiff_writeImage(const char* filename, cairo_surface_t* surface)
{
    int i, j, k;

    if (!filename || !surface) {
        ESprintf(ERR_CRO_BADARG, "crotiff_writeImage");
        return ERR_CRO_BADARG;
    }

    FILE* out = fopen(filename, "wb");
    if (!out) {
        ESprintf(ERR_CRO_OPN, "crotiff: could not open output file: %s", filename);
        return ERR_CRO_OPN;
    }

    /* determine how many strips we need... */
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    int stride = cairo_image_surface_get_stride(surface);
    int numStrips = height / RowsPerStrip;
    int samplesPerPixel = 4;

    uint32_t* stripOffsets = (uint32_t*) malloc(numStrips * sizeof(uint32_t));
    uint32_t* stripCounts = (uint32_t*) malloc(numStrips * sizeof(uint32_t));
    if (!stripOffsets || !stripCounts) {
        ESprintf(ERR_CRO_MEMORY, "crotiff: malloc of strip offsets/count");
        return ERR_CRO_MEMORY;
    }

    int fileOffset = 0;

    IFHeader hdr;
    hdr.byteOrder = endianness;
    hdr.magicNum = MagicNum;
    hdr.ifdOffset = sizeof(IFHeader) + (samplesPerPixel*width*height) + (2*sizeof(uint32_t)*numStrips);

    /* write these piecemeal, in case of padding in the struct */
    fwrite(&hdr.byteOrder, sizeof(uint16_t), 1, out);
    fwrite(&hdr.magicNum, sizeof(uint16_t), 1, out);
    fwrite(&hdr.ifdOffset, sizeof(uint32_t), 1, out);
    fileOffset += 8;

    /* for uncompressed output, we can compute the strip offsets/counts outright... */
    uint32_t numImageBytes = width * height * samplesPerPixel;
    for (i=0; i<numStrips; i++) {
        stripOffsets[i] = fileOffset + i*samplesPerPixel*width*RowsPerStrip;
        int count = width*RowsPerStrip*samplesPerPixel;
        int numBytesAccum = i*count;
        stripCounts[i] = (numBytesAccum > numImageBytes) ? numBytesAccum - numImageBytes : count;
    }

    /* begin writing the image data by strips... */
    unsigned char* imageData = cairo_image_surface_get_data(surface);
    int imageOffset = 0;
    Argb2RgbA swizzler;
    uint32_t* stripBuff = (uint32_t*) malloc(sizeof(uint32_t) * width * RowsPerStrip);
    if (!stripBuff) {
        ESprintf(ERR_CRO_MEMORY, "crotiff: malloc of strip buffer");
        return ERR_CRO_MEMORY;
    }

    for (k=0; k<numStrips; k++) {
        int buffOffset = 0;
        for (j=0; j<RowsPerStrip; j++) {
            if ((k*RowsPerStrip + j) >= height) break;
            for (i=0; i<width; i++) {
                /* Our cairo images are argb, whereas we need rgba for tiff. */
#ifdef ByteSwapped
                swizzler.swizzle[2] = imageData[imageOffset];
                swizzler.swizzle[1] = imageData[imageOffset+1];
                swizzler.swizzle[0] = imageData[imageOffset+2];
                swizzler.swizzle[3] = imageData[imageOffset+3];
#else
                swizzler.swizzle[3] = imageData[imageOffset];
                swizzler.swizzle[0] = imageData[imageOffset+1];
                swizzler.swizzle[1] = imageData[imageOffset+2];
                swizzler.swizzle[2] = imageData[imageOffset+3];
#endif
                stripBuff[buffOffset] = swizzler.value;
                imageOffset += 4;
                ++buffOffset;
            }
        }

        fwrite(stripBuff, 1, stripCounts[k], out);
    }
    free(stripBuff);

    /* Write the strip offsets, followed by the strip counts... */
    long stripOffsetsPos = ftell(out);
    fwrite(stripOffsets, sizeof(uint32_t), numStrips, out);
    long stripCountsPos = ftell(out);
    fwrite(stripCounts, sizeof(uint32_t), numStrips, out);
    free(stripOffsets);
    free(stripCounts);

    /* Create and write the Image File Directory. Recall that entries are supposed to
     * be sorted by tag value -- we'll manually manage that here for the time being.
     */
    IFDirectory ifd;
    memset(&ifd, 0, sizeof(ifd));

    addIFDEntry(&ifd, TAG_ImageWidth, SHORT_TYPE, 1, width);
    addIFDEntry(&ifd, TAG_ImageLength, SHORT_TYPE, 1, height);
    addIFDEntry(&ifd, TAG_BitsPerSample, SHORT_TYPE, 1, 8);
    addIFDEntry(&ifd, TAG_Compression, SHORT_TYPE, 1, 1);    /* 1 denotes "uncompressed" */
    addIFDEntry(&ifd, TAG_PhotoInterp, SHORT_TYPE, 1, 2);    /* 2 denotes "RGB" */
    addIFDEntry(&ifd, TAG_StripOffsets, LONG_TYPE, numStrips, stripOffsetsPos);
    addIFDEntry(&ifd, TAG_SamplesPerPixel, SHORT_TYPE, 1, 4);
    addIFDEntry(&ifd, TAG_RowsPerStrip, SHORT_TYPE, 1, RowsPerStrip);
    addIFDEntry(&ifd, TAG_StripByteCount, LONG_TYPE, numStrips, stripCountsPos);
    addIFDEntry(&ifd, TAG_PlanarConfig, SHORT_TYPE, 1, 1);  /* 1 denotes "chunky" */
    addIFDEntry(&ifd, TAG_ExtraSamples, SHORT_TYPE, 1, 1);  /* alpha component */

    writeIFD(&ifd, out);
    fclose(out);

    free(ifd.entries);
    return 0;
}


/*
 * addIFDEntry()
 *
 * Adds an entry to the given IFD. Grows the IFD entries table as necessary.
 *
 */
static void
addIFDEntry(IFDirectory* ifd, int16_t tag, uint16_t type, uint32_t count, uint32_t value)
{
    if (ifd->numEntries >= ifd->maxEntries) {
        /* need space for more entries... */
        ifd->entries = (IFDEntry*) realloc(ifd->entries,
                (ifd->maxEntries+IFD_GROW_SIZE)*sizeof(IFDEntry));
        ifd->maxEntries += IFD_GROW_SIZE;
    }

    int indx = ifd->numEntries;
    ifd->entries[indx].tag = tag;
    ifd->entries[indx].type = type;
    ifd->entries[indx].count = count;
    ifd->entries[indx].valOffset = value;
    ifd->numEntries++;
}

/*
 * writeIFD()
 *
 * Writes the Image File Directory to the given file.
 *
 */
void
writeIFD(IFDirectory* ifd, FILE* out)
{
    int i;

    fwrite(&ifd->numEntries, sizeof(uint16_t), 1, out);
    for (i=0; i<ifd->numEntries; i++) {
        /* write each field individually, rather than by IFEntry, in case there's padding
         * in the struct...
         */
        fwrite(&ifd->entries[i].tag, sizeof(uint16_t), 1, out);
        fwrite(&ifd->entries[i].type, sizeof(uint16_t), 1, out);
        fwrite(&ifd->entries[i].count, sizeof(uint32_t), 1, out);
        if (ifd->entries[i].type == SHORT_TYPE) {
            /* Recall that valOffset types < 4 bytes are packed into upper-half of the valOffset field. */
            uint16_t* tmp = (uint16_t*) &ifd->entries[i].valOffset;
#ifdef ByteSwapped
            fwrite(tmp, sizeof(uint16_t), 1, out);
#else
            fwrite(tmp+1, sizeof(uint16_t), 1, out);
#endif
            tmp = 0;  /* zero pad lower part */
            fwrite(&tmp, sizeof(uint16_t), 1, out);
        }
        else
            fwrite(&ifd->entries[i].valOffset, sizeof(uint32_t), 1, out);
    }
    fwrite(&ifd->nextIFD, sizeof(uint32_t), 1, out);
}
