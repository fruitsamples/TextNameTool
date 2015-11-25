/*    File: PixOps.c        Description:        Routines for simple graphics file creation and manipulation.      Copyright:        Copyright (c) 2003 Apple Computer, Inc. All rights reserved.        Disclaimer:        IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.        ("Apple") in consideration of your agreement to the following terms, and your        use, installation, modification or redistribution of this Apple software        constitutes acceptance of these terms.  If you do not agree with these terms,        please do not use, install, modify or redistribute this Apple software.        In consideration of your agreement to abide by the following terms, and subject        to these terms, Apple grants you a personal, non-exclusive license, under Apple�s        copyrights in this original Apple software (the "Apple Software"), to use,        reproduce, modify and redistribute the Apple Software, with or without        modifications, in source and/or binary forms; provided that if you redistribute        the Apple Software in its entirety and without modifications, you must retain        this notice and the following text and disclaimers in all such redistributions of        the Apple Software.  Neither the name, trademarks, service marks or logos of        Apple Computer, Inc. may be used to endorse or promote products derived from the        Apple Software without specific prior written permission from Apple.  Except as        expressly stated in this notice, no other rights or licenses, express or implied,        are granted by Apple herein, including but not limited to any patent rights that        may be infringed by your derivative works or by other works in which the Apple        Software may be incorporated.        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO        WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED        WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR        PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN        COMBINATION WITH YOUR PRODUCTS.        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)        ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION        OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT        (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN        ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    Change History (most recent first):        Wed, Feb 16, 2001 -- created        Wed, Apr 02, 2001 -- added unicode only routines        Wed, Aug 27, 2003 -- morphed to work in TextNameTool sample*/#include <ApplicationServices/ApplicationServices.h>#include <QuickTime/ImageCompression.h>#include <QuickTime/QuickTimeComponents.h>#include "PixOps.h"OSStatus SaveAsJPG(GWorldPtr gworld, char* filepath, int jpgquality) {	GraphicsExportComponent theExportComponent;	OSStatus err, result;	Handle dataHandle;	FILE* dataFile;	CodecQ spatialQuality;	switch (jpgquality) {		case minQuality: spatialQuality = codecMinQuality; break;		case lowQuality: spatialQuality = codecLowQuality; break;		case normalQuality: spatialQuality = codecNormalQuality; break;		case highQuality: spatialQuality = codecHighQuality; break;		case maxQuality: spatialQuality = codecMaxQuality; break;		default: return paramErr;	}		/* find the exporter */	result = paramErr;	err = OpenADefaultComponent( GraphicsExporterComponentType, kQTFileTypeJPEG, &theExportComponent );	if (err == noErr) {		err = GraphicsExportSetInputGWorld(theExportComponent, gworld); 		if (err == noErr) {				/* perform the export */			result = memFullErr;			dataHandle = NewHandle(0);			if (dataHandle != NULL) {				err = GraphicsExportSetOutputHandle( theExportComponent, dataHandle);				if (err == noErr) {					err = GraphicsExportSetCompressionQuality( theExportComponent, spatialQuality );					if (err == noErr) {						err = GraphicsExportDoExport( theExportComponent, NULL );						if (err == noErr) {								/* write the file */							result = fnfErr;							dataFile = fopen(filepath, "w+");							if (dataFile != NULL) {								HLock(dataHandle);								fwrite(*dataHandle, GetHandleSize(dataHandle), 1, dataFile);								fclose(dataFile);								result = noErr;							}						}					}				}				DisposeHandle(dataHandle);			}		}		CloseComponent( theExportComponent );	}	return result;}OSStatus SaveAsPNG(GWorldPtr gworld, char* filepath) {	GraphicsExportComponent theExportComponent;	OSStatus err, result;	Handle dataHandle;	FILE* dataFile;		/* find the exporter */	result = paramErr;	err = OpenADefaultComponent( GraphicsExporterComponentType, kQTFileTypePNG, &theExportComponent );	if (err == noErr) {		err = GraphicsExportSetInputGWorld(theExportComponent, gworld); 		if (err == noErr) {				/* perform the export */			result = memFullErr;			dataHandle = NewHandle(0);			if (dataHandle != NULL) {				err = GraphicsExportSetOutputHandle( theExportComponent, dataHandle);				if (err == noErr) {					err = GraphicsExportSetDepth( theExportComponent, 4); /* 40 for grayscale */					if (err == noErr) {						err = GraphicsExportDoExport( theExportComponent, NULL );						if (err == noErr) {								/* write the file */							result = fnfErr;							dataFile = fopen(filepath, "w+");							if (dataFile != NULL) {								HLock(dataHandle);								fwrite(*dataHandle, GetHandleSize(dataHandle), 1, dataFile);								fclose(dataFile);								result = noErr;							}						}					}				}				DisposeHandle(dataHandle);			}		}		CloseComponent( theExportComponent );	}	return result;}GWorldPtr MakeGWorld(Rect *bounds) {	GWorldPtr result;	result = NULL;	if (NewGWorld(&result, sizeof(PixelDataType) * 8, bounds, NULL, NULL, 0) != noErr) result = NULL;	if (result != NULL) LockPixels(GetGWorldPixMap(result));	return result;}GWorldPtr MakeGSubWorld(GWorldPtr sourceWorld, Rect *bounds) {	Rect r;	GWorldPtr sub;	r = *bounds;	OffsetRect(&r, -r.left, -r.top);	sub = MakeGWorld(&r);	if (sub != NULL) {		SetGWorld(sub, NULL);		CopyBits(GetPortBitMapForCopyBits(sourceWorld), 			GetPortBitMapForCopyBits(sub), bounds, &r, srcCopy, NULL);	}	return sub;}GWorldPtr MakeGrayscaleGWorld(Rect *bounds) {	GWorldPtr result;	CTabHandle theClut;	result = NULL;	theClut = GetCTable(40);	if (theClut != NULL) {		if (NewGWorld(&result, 8, bounds, theClut, NULL, 0) == noErr) {			LockPixels(GetGWorldPixMap(result));		} else result = NULL;		DisposeCTable(theClut);	}	return result;}void UnmakeGWorld(GWorldPtr theWorld) {	UnlockPixels(GetGWorldPixMap(theWorld));	DisposeGWorld(theWorld);}Rect *DiscoverGWorldUsedBounds(GWorldPtr newWorld, Rect *boundingBox, Rect *r) {	PixMapHandle thePix;	unsigned char *baseAddr;	long rowBytes;	PixelDataType zeroElt, *rover;	Boolean isEmpty;	long x, y;	Rect box;	thePix = GetGWorldPixMap(newWorld);	baseAddr = (unsigned char*) GetPixBaseAddr(thePix);	rowBytes = GetPixRowBytes(thePix);	zeroElt = * (PixelDataType*) baseAddr;	box = *boundingBox;	        /* down from top */    for ( y = box.top; y < box.bottom ; y++) {        for (isEmpty=true, rover = (PixelDataType*) (baseAddr + y*rowBytes), x=box.left; x<box.right; x++, rover++) {            if ( *rover != zeroElt ) { isEmpty = false; break; }        }        if (! isEmpty) break;    }    if (y == box.bottom) return NULL;    box.top = y;        /* up from bottom */	for ( y = box.bottom; y > box.top ; y--) {        for (isEmpty=true, rover = (PixelDataType*) (baseAddr + (y-1)*rowBytes), x=box.left; x<box.right; x++, rover++) {            if ( *rover != zeroElt ) { isEmpty = false; break; }        }        if (! isEmpty) break;    }    box.bottom = y;        /* from left to right */	for ( x = box.left; x < box.right ; x++) {        for (isEmpty=true, y=box.top; y<box.bottom; y++) {            rover = ((PixelDataType*) (baseAddr + y*rowBytes)) + x;            if ( *rover != zeroElt ) { isEmpty = false; break; }        }        if (! isEmpty) break;    }    box.left = x;        /* from right to left */	for ( x = box.right; x > box.left ; x--) {        for (isEmpty=true, y=box.top; y<box.bottom; y++) {            rover = ((PixelDataType*) (baseAddr + y*rowBytes)) + (x-1);            if ( *rover != zeroElt ) { isEmpty = false; break; }        }        if (! isEmpty) break;    }    box.right = x;		/* return the bounding box */	*r = box;	return r;}OSStatus GetGraphicsFileBounds(char* filepath, short *width, short *height) {	ComponentInstance gi;	OSStatus err, result;	Rect naturalBounds;		FILE* theFile;	long theFileSize;	void* fileData;		PointerDataRef pdr;		/* look for the file */	result = fnfErr;	theFile = fopen(filepath, "r");	if (theFile) {		fseek(theFile, 0, SEEK_END);		theFileSize = ftell(theFile);		fseek(theFile, 0, SEEK_SET);			/* read it into a buffer, make a dataref to the buffer */		result = memFullErr;		fileData = malloc(theFileSize);		if (fileData != NULL) {			fread(fileData, theFileSize, 1, theFile);		    pdr = (PointerDataRef) NewHandle(sizeof(PointerDataRefRecord));			if (pdr != NULL) {				(**pdr).data = fileData;				(**pdr).dataLength = theFileSize;					/* identify the file format */				result = paramErr;				err = GetGraphicsImporterForDataRef( (Handle) pdr, PointerDataHandlerSubType, &gi);				if (err == noErr) {					err = GraphicsImportGetNaturalBounds(gi, &naturalBounds);					if (err == noErr) {						*width = naturalBounds.right - naturalBounds.left;						*height = naturalBounds.bottom - naturalBounds.top;						result = noErr;					}					CloseComponent( gi );				}				DisposeHandle((Handle) pdr);			}			free(fileData);		}		fclose(theFile);	}		/* done */	return result;}OSStatus LoadGraphicsFile(char* filepath, GWorldPtr *theWorld, Rect *r) {	ComponentInstance gi;	OSStatus err, result;	Rect naturalBounds;	FILE* theFile;	long theFileSize;	void* fileData;	PointerDataRef pdr;		/* look for the file */	result = fnfErr;	theFile = fopen(filepath, "r");	if (theFile) {		fseek(theFile, 0, SEEK_END);		theFileSize = ftell(theFile);		fseek(theFile, 0, SEEK_SET);			/* read it into a buffer, make a dataref to the buffer */		result = memFullErr;		fileData = malloc(theFileSize);		if (fileData != NULL) {			fread(fileData, theFileSize, 1, theFile);		    pdr = (PointerDataRef) NewHandle(sizeof(PointerDataRefRecord));			if (pdr != NULL) {				(**pdr).data = fileData;				(**pdr).dataLength = theFileSize;					/* identify the file format */				result = paramErr;				err = GetGraphicsImporterForDataRef( (Handle) pdr, PointerDataHandlerSubType, &gi);				if (err == noErr) {					err = GraphicsImportGetNaturalBounds(gi, &naturalBounds);					if (err == noErr) {						GWorldPtr localGWorld;						OffsetRect(&naturalBounds, -naturalBounds.left, -naturalBounds.top);						if ((localGWorld = MakeGWorld(&naturalBounds)) != NULL) {							err = GraphicsImportSetGWorld( gi, localGWorld, GetGWorldDevice(localGWorld));							if (err == noErr) {								err = GraphicsImportDraw(gi);								if (err == noErr) {									*r = naturalBounds;									*theWorld = localGWorld;									result = noErr;								}							}							if (err != noErr) {								UnmakeGWorld(localGWorld);								result = err;							}						}					}					CloseComponent( gi );				}				DisposeHandle((Handle) pdr);			}			free(fileData);		}		fclose(theFile);	}		/* done */	return result;}