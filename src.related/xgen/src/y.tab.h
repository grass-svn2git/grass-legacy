#define String 257
#define Integer 258
#define Real 259
#define Logical 260
#define Environment 261
#define InitialShells 262
#define Menu 263
#define CommandBoard 264
#define Label 265
#define Message 266
#define List 267
#define PushButton 268
#define TextEntry 269
#define Table 270
#define Separator 271
#define Slider 272
#define Toggle 273
#define PullDown 274
#define MultiLine 275
#define Font 276
#define FixedFont 277
#define EditorFont 278
#define Background 279
#define Foreground 280
#define BackgroundPixmap 281
#define TopShadowColor 282
#define TopShadowPixmap 283
#define BottomShadowColor 284
#define BottomShadowPixmap 285
#define X 286
#define DX 287
#define Y 288
#define DY 289
#define Width 290
#define Height 291
#define ForceSize 292
#define MaxWidth 293
#define MaxHeight 294
#define Columns 295
#define Override 296
#define Functions 297
#define Decorations 298
#define Popup 299
#define Popdown 300
#define Destroy 301
#define Exit 302
#define Help 303
#define Eval 304
#define UpdateObject 305
#define PostNotice 306
#define RunForeground 307
#define RunBackground 308
#define CommandShell 309
#define InteractiveShell 310
#define InputFrom 311
#define CaptureOutput 312
#define NotifyComplete 313
#define UpdateFrom 314
#define Pane 315
#define PaneType 316
#define Store 317
#define Highlight 318
#define GetEnv 319
#define Clear 320
#define CommandArg 321
#define TableArg 322
#define Set 323
#define Alignment 324
#define ListElement 325
#define ListSeparator 326
#define ListType 327
#define VisibleItems 328
#define ScrollBar 329
#define ValueString 330
#define Scrolled 331
#define LabelPixmap 332
#define MaxLength 333
#define Minimum 334
#define Maximum 335
#define StartValue 336
#define SliderWidth 337
#define SliderHeight 338
#define Orientation 339
#define DecimalPoints 340
#define EntryFont 341
#define Rows 342
#define RowsDisplayed 343
#define FixedRows 344
#define FixedColumns 345
#define ColumnsDisplayed 346
#define ColumnHeadings 347
#define RowHeadings 348
#define RowValue 349
#define TableValue 350
#define RowHeight 351
#define ColumnWidth 352
#define Newline 353
#define TitleString 354
#define ToggleType 355
#define ToggleState 356
#define SeparatorType 357
#define Sensitive 358
#define Insensitive 359
typedef union {
    char *  cval;
    int ival;
    double dval;
    Boolean bval;
} YYSTYPE;
extern YYSTYPE yylval;