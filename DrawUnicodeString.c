/*    File: DrawUnicodeString.c        Description:        Routines for drawing unicode strings using ATSUI.      Copyright:        Copyright (c) 2003 Apple Computer, Inc. All rights reserved.        Disclaimer:        IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.        ("Apple") in consideration of your agreement to the following terms, and your        use, installation, modification or redistribution of this Apple software        constitutes acceptance of these terms.  If you do not agree with these terms,        please do not use, install, modify or redistribute this Apple software.        In consideration of your agreement to abide by the following terms, and subject        to these terms, Apple grants you a personal, non-exclusive license, under Apple�s        copyrights in this original Apple software (the "Apple Software"), to use,        reproduce, modify and redistribute the Apple Software, with or without        modifications, in source and/or binary forms; provided that if you redistribute        the Apple Software in its entirety and without modifications, you must retain        this notice and the following text and disclaimers in all such redistributions of        the Apple Software.  Neither the name, trademarks, service marks or logos of        Apple Computer, Inc. may be used to endorse or promote products derived from the        Apple Software without specific prior written permission from Apple.  Except as        expressly stated in this notice, no other rights or licenses, express or implied,        are granted by Apple herein, including but not limited to any patent rights that        may be infringed by your derivative works or by other works in which the Apple        Software may be incorporated.        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO        WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED        WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR        PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN        COMBINATION WITH YOUR PRODUCTS.        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)        ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION        OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT        (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN        ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    Change History (most recent first):        Wed, Feb 16, 2001 -- created        Wed, Apr 02, 2001 -- added unicode only routines        Wed, Aug 27, 2003 -- morphed to work in TextNameTool sample*/#include "DrawUnicodeString.h"static int FIDRECCMP(const void *a, const void *b) {	FontNameIDRec *fida, *fidb;	fida = (FontNameIDRec *) a;	fidb = (FontNameIDRec *) b;	return strcmp(fida->name, fidb->name);}OSStatus GetInstalledFontList(			FontNameIDVector *fontList,	/* place to return pointer to list */			long *listLength) {			/* place to return length of list */	OSStatus err;	ItemCount i, numFonts;	ATSUFontID *fontIDList = NULL;	FontNameIDVector fiv;	long fivlen;		/* set up */	fivlen = 0;	fiv = NULL;		/* iterate over installed fonts */	err = ATSUFontCount( &numFonts );	if ( err == noErr ) {		fontIDList = (ATSUFontID*) malloc( numFonts * sizeof(ATSUFontID) );		if (fontIDList == NULL) {			err = memFullErr;		} else {			fiv = (FontNameIDVector) malloc( numFonts * sizeof(FontNameIDRec) );			if (fiv == NULL) {				err = memFullErr;			} else {				err = ATSUGetFontIDs( fontIDList, numFonts, NULL );				if (err == noErr) {					for ( i = 0; i < numFonts; i++ ) {						ByteCount namelen;						ItemCount oNameIndex;						char namestring[512];							/* best effort here.  If the name lookup or the name tests							fail, then we simply don't include the item in the list */ 						err = ATSUFindFontName( fontIDList[ i ], kFontFullName,								kFontMacintoshPlatform, kFontRomanScript, kFontEnglishLanguage,								sizeof(namestring), namestring, &namelen, &oNameIndex );						if (err == noErr) {							namestring[namelen] = 0;							if ( namelen > 0 && namestring[0] != '.' && namestring[0] != '%' && namestring[0] != '#' ) {								char* p;								p = (char*) malloc(strlen(namestring) + 1);								if (p == NULL) {									err = memFullErr;									break;								} else {									strcpy(p, namestring);									fiv[fivlen].theFontID = fontIDList[ i ];									fiv[fivlen++].name = p;								}							}						}					}								}			}			free(fontIDList);		}	}		/* sort the resulting list */	if (err == noErr) {		if (fivlen > 0) {			qsort(fiv, fivlen, sizeof(FontNameIDRec), FIDRECCMP);		}		*fontList = fiv;		*listLength = fivlen;	} else {		if (fiv != NULL) {			for (i=0; i<fivlen; i++)				free(fiv[i].name);			free(fiv);		}	}	return err;}						OSStatus GetNamedFontID(char* fontName, ATSUFontID *theFontID) {	return ATSUFindFontFromName(				fontName,				strlen(fontName),				kFontFullName,				kFontMacintoshPlatform,				kFontRomanScript,				kFontEnglishLanguage, theFontID);}	/* MakeSimpleATSUIStyle creates a simple ATSUI style record that	can be used in calls to the RenderCFString routine. */ OSStatus MakeSimpleATSUIStyle(ATSUFontID theFontID, short fontSize, short qdStyle, RGBColor *fontColor, ATSUStyle *theStyle) {	ATSUFontID atsuFont;	OSStatus err;		/* Three parrallel arrays for setting up attributes. */#define kTagMax 32	ItemCount tagCount;	ATSUAttributeTag theTags[kTagMax];	ByteCount theSizes[kTagMax];	ATSUAttributeValuePtr theValues[kTagMax];		Fixed atsuSize;	short atsuOrientation;	RGBColor defaultColor = { 0, 0, 0};	Boolean trueVar = true, falseVar = false;	ATSUStyle localStyle;		/* initial tag count */	tagCount = 0;		/* the font */	atsuFont = theFontID;	theTags[tagCount] = kATSUFontTag;	theSizes[tagCount] = sizeof(ATSUFontID);	theValues[tagCount++] = &atsuFont;		/* the size */	atsuSize = FixRatio(fontSize, 1);	theTags[tagCount] = kATSUSizeTag;	theSizes[tagCount] = sizeof(Fixed);	theValues[tagCount++] = &atsuSize;		/* the orientation */	atsuOrientation = kATSUStronglyHorizontal;	theTags[tagCount] = kATSUVerticalCharacterTag;	theSizes[tagCount] = sizeof(UInt16);	theValues[tagCount++] = &atsuOrientation;		/* font color */	theTags[tagCount] = kATSUColorTag;	theSizes[tagCount] = sizeof(RGBColor);	theValues[tagCount++] = ((fontColor != NULL) ? fontColor : &defaultColor);		/* bold */	theTags[tagCount] = kATSUQDBoldfaceTag;	theSizes[tagCount] = sizeof(Boolean);	theValues[tagCount++] = ((qdStyle & bold) != 0 ? &trueVar : &falseVar);		/* italic */	theTags[tagCount] = kATSUQDItalicTag;	theSizes[tagCount] = sizeof(Boolean);	theValues[tagCount++] = ((qdStyle & italic) != 0 ? &trueVar : &falseVar);		/* underline */	theTags[tagCount] = kATSUQDUnderlineTag;	theSizes[tagCount] = sizeof(Boolean);	theValues[tagCount++] = ((qdStyle & underline) != 0 ? &trueVar : &falseVar);		/* condensed */	theTags[tagCount] = kATSUQDCondensedTag;	theSizes[tagCount] = sizeof(Boolean);	theValues[tagCount++] = ((qdStyle & condense) != 0 ? &trueVar : &falseVar);		/* extended */	theTags[tagCount] = kATSUQDExtendedTag;	theSizes[tagCount] = sizeof(Boolean);	theValues[tagCount++] = ((qdStyle & extend) != 0 ? &trueVar : &falseVar);	err = ATSUCreateStyle(&localStyle);	if (err == noErr) {		err = ATSUSetAttributes( localStyle, tagCount, theTags, theSizes, theValues );		if (err == noErr) {			*theStyle = localStyle;		} else {			ATSUDisposeStyle(localStyle);		}	}		/* done, return */		return err;}	/* MeasureUnicodeString returns a rectangle where the CFStringRef would	be drawn if it were drawn at location (0,0) -- just like QDTextBounds. */OSStatus MeasureUnicodeString(ConstUniCharArrayPtr iText, UniCharCount iTextLength,		ATSUStyle theStyle, Rect *textbounds, Rect *imagebounds) {	OSStatus err;		/* set up our locals, verify parameters... */	if (iText == NULL || theStyle == NULL) {		err = paramErr;	} else {		ATSUTextLayout theLayout;			/* create the ATSUI layout */		err = ATSUCreateTextLayoutWithTextPtr( iText, 0,			iTextLength, iTextLength, 1, 			(unsigned long *) &iTextLength, &theStyle, &theLayout);		if (err == noErr) {			ATSLineLayoutOptions layoutOptions;			ATSUAttributeTag theTags[1];			ByteCount theSizes[1];			ATSUAttributeValuePtr theValues[1];				/* ask atsui to ignore the default leading value for the font */			layoutOptions = kATSLineIgnoreFontLeading;			theTags[0] = kATSULineLayoutOptionsTag;			theSizes[0] = sizeof(ATSLineLayoutOptions);			theValues[0] = &layoutOptions;			err = ATSUSetLayoutControls (theLayout, 1, theTags, theSizes, theValues);			if (err == noErr) {				ATSUTextMeasurement oTextBefore, oTextAfter, oAscent, oDescent;					/* measure the text */				err = ATSUMeasureText( theLayout, 0, iTextLength,					&oTextBefore, &oTextAfter, &oAscent, &oDescent);				if (err == noErr) {					SetRect(textbounds, - FixRound(oTextBefore), - FixRound(oAscent), FixRound(oTextAfter), FixRound(oDescent)+1);					err = ATSUMeasureTextImage(theLayout, 0, iTextLength, 0, 0, imagebounds);				}			}			ATSUDisposeTextLayout(theLayout);		}	}		/* done */	return err;}	/* RenderCFString renders a CFString at (h, v) in the current grafport	using ATSUI using the style specified in the atsui style record. */OSStatus RenderUnicodeString(			ConstUniCharArrayPtr iText,	/* text to draw */			UniCharCount iTextLength,	/* number of characters */			ATSUStyle theStyle,			/* the style */			short h,					/* horizontal position to draw at */			short v,					/* vertical position to draw at */			CGContextRef theContext) {	/* cg context, can be NULL */	OSStatus err;		/* set up our locals, verify parameters... */	if (iText == NULL || theStyle == NULL) {		err = paramErr;	} else {		ATSUTextLayout theLayout;			/* create the ATSUI layout */		err = ATSUCreateTextLayoutWithTextPtr( iText, 0,			iTextLength, iTextLength, 1, 			(unsigned long *) &iTextLength, &theStyle, &theLayout);		if (err == noErr) {			ATSLineLayoutOptions layoutOptions;			ATSUAttributeTag theTags[10];			ByteCount theSizes[10];			ATSUAttributeValuePtr theValues[10];			ItemCount numcontrols = 0;				/* ask atsui to ignore the default leading value for the font */			layoutOptions = kATSLineIgnoreFontLeading;			theTags[numcontrols] = kATSULineLayoutOptionsTag;			theSizes[numcontrols] = sizeof(ATSLineLayoutOptions);			theValues[numcontrols++] = &layoutOptions;			if (theContext != NULL) {				theTags[numcontrols] = kATSUCGContextTag;				theSizes[numcontrols] = sizeof(theContext);				theValues[numcontrols++] = &theContext;			}			err = ATSUSetLayoutControls (theLayout, numcontrols, theTags, theSizes, theValues);			if (err == noErr) {					/* draw the text */				err = ATSUDrawText(theLayout, 0, iTextLength, FixRatio(h, 1), FixRatio(v, 1));			}			ATSUDisposeTextLayout(theLayout);		}	}		/* done */	return err;}