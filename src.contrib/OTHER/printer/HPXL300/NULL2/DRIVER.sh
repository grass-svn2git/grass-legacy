:

HRES=150.0
VRES=150.0
NCHARS=120

TEXTSCALE=.85
TEXTFUDGE=2
TEXTSPACE=2
BLOCKSIZE=50
BLOCKSPACE=30
NBLOCKS=15

export HRES VRES NCHARS
export TEXTSCALE TEXTSPACE TEXTFUDGE BLOCKSIZE BLOCKSPACE NBLOCKS

exec ${PAINT_DRIVER?}
