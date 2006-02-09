/*
 *	img_seek, img_write, img_read, img_optseek -
 *
 *				Paul Haeberli - 1984
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <grass/config.h>
#include "image.h"
#include "local_proto.h"


long img_seek(IMAGE *image, unsigned int y, unsigned int z)
{
    if(img_badrow(image,y,z)) {
	i_errhdlr("img_seek: row number out of range\n");
	return EOF;
    }
    image->x = 0;
    image->y = y;
    image->z = z;
    if(ISVERBATIM(image->type)) {
	switch(image->dim) {
	    case 1:
		return img_optseek(image, 512L);
	    case 2: 
		return img_optseek(image,512L+(y*image->xsize)*BPP(image->type));
	    case 3: 
		return img_optseek(image,
		    512L+(y*image->xsize+z*image->xsize*image->ysize)*
							BPP(image->type));
	    default:
		i_errhdlr("img_seek: weird dim\n");
		break;
	}
    } else if(ISRLE(image->type)) {
	switch(image->dim) {
	    case 1:
		return img_optseek(image, image->rowstart[0]);
	    case 2: 
		return img_optseek(image, image->rowstart[y]);
	    case 3: 
		return img_optseek(image, image->rowstart[y+z*image->ysize]);
	    default:
		i_errhdlr("img_seek: weird dim\n");
		break;
	}
    } else 
	i_errhdlr("img_seek: weird image type\n");

    return -1;
}

int img_badrow(IMAGE *image, int y, int z)
{
    if(y>=image->ysize || z>=image->zsize)
	return 1;
    else
        return 0;
}

long img_write(IMAGE *image, char *buffer, long count)
{
    long retval;

    retval =  write(image->file, buffer, count * sizeof(char));
    if(retval == count) 
	image->offset += count;
    else
	image->offset = -1;
    return retval;
}

long img_read(IMAGE *image, char *buffer, long count)
{
    long retval;

    retval =  read(image->file, buffer, count * sizeof(char));
    if(retval == count) 
	image->offset += count;
    else
	image->offset = -1;
    return retval;
}

off_t img_optseek(IMAGE *image, off_t offset)
{
    if(image->offset != offset) {
       image->offset = offset;
       return lseek(image->file,offset,0);
   }
   return offset;
}
