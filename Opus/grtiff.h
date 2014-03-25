

/*------------------------------------------------------------------------
*
*
* Microsoft Word -- Graphics
*
* TIFF bitmap reading code
*
*-----------------------------------------------------------------------*/

#ifndef grtiff_h
#define grtiff_h

#define	AreaOfInterest				302
#define	BitsPerSample				258
#define	Compression					259
#define	GrayResponseCurve			291
#define	GrayResponseUnit			290
#define	ImageLength					257
#define	ImageWidth					256
#define OldfileType					255
#define	PhotometricInterpretation	262
#define	ResolutionUnit				296
#define	RowsPerStrip				278
#define	SamplesPerPixel				277
#define	StripByteCountMax			319
#define	StripByteCounts				279
#define	StripOffsets				273
#define	SubfileType					254
#define	TiffClass					253
#define	XResolution					282
#define	YResolution					283

#define	typeShort	3
#define	typeLong	4
#define Scheme1		1
#define Scheme32773	0x8005

#define wGrayUnitLim 5		/* gray units range from 1 to 5 */
#define wGrayDen	100		/* if wGrayUnitLim, div by this to get 1000ths */
#define wGrayUnitDef 2		/* default is hundredths */
#define crwMac		32000	/* default is hundredths */

#define	ibTIFFIfd	4

typedef struct _den {
	int wTag;
	int wType;
	long	cValues;
	union {
		struct {
			unsigned w1Value;
			unsigned w2Value;
			} S1;
		struct {
			char far * lpValue;
			} S2;
		struct {
			long lValue;
			} S3;
		} U1;
	} DEN;


#endif
