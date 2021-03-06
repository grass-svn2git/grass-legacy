#define String 257
#define Integer 258
#define Real 259
#define Logical 260
#define Program 261
#define Title 262
#define Description 263
#define Help 264
#define HelpFile 265
#define HelpWidgetRef 266
#define Capture 267
#define ErrorCodes 268
#define CommandString 269
#define Dialog 270
#define Flag 271
#define Parameter 272
#define Key 273
#define Requires 274
#define Precludes 275
#define Optional 276
#define Input 277
#define Multiple 278
#define Type 279
#define TypeEnumerate 280
#define TypeFileName 281
#define TypeDatabaseElement 282
#define TypeCharacter 283
#define TypeInteger 284
#define TypeFloat 285
#define TypeDouble 286
#define TypeLogical 287
#define DatabaseElementRaster 288
#define DatabaseElementAsciiDlg 289
#define DatabaseElementDlg 290
#define DatabaseElementAsciiVector 291
#define DatabaseElementVector 292
#define DatabaseElementSites 293
#define DatabaseElementRegion 294
#define DatabaseElementIcon 295
#define DatabaseElementLabel 296
#define DatabaseElementGroup 297
#define DatabaseElementSubGroup 298
#define StatusOld 299
#define StatusNew 300
#define SelectType 301
#define Default 302
typedef union {
    char *  cval;
    int ival;
    double dval;
    Boolean bval;
} YYSTYPE;
extern YYSTYPE yylval;
