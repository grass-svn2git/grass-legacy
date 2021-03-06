
#include "orthophoto.h"

FILE *I_fopen_cam_file_old();
FILE *I_fopen_cam_file_new();

#define CAMERA_FILE "CAMERA"

I_read_cam_info (fd, cam_info)
    FILE *fd;
    struct Ortho_Camera_File_Ref *cam_info;
{   
    int n;
    char buf[100];
    char cam_name[30];
    char cam_id[30];
    double Xp,Yp,CFL;
    int num_fid;
    char fid_id[30];
    double Xf,Yf;

    G_getl (buf, sizeof buf, fd); 
    G_strip(buf);
    if (sscanf(buf,"CAMERA NAME   %s \n",cam_name) == 1)
       strcpy(cam_info->cam_name,cam_name);

    G_getl (buf, sizeof buf, fd); 
    G_strip(buf);
    if (sscanf(buf,"CAMERA ID     %s \n",cam_id) == 1)
       strcpy(cam_info->cam_id,cam_id);

    G_getl (buf, sizeof buf, fd); 
    G_strip(buf);
    if (sscanf(buf,"CAMERA XP     %lf \n",&Xp) == 1)
       cam_info->Xp = Xp;

    G_getl (buf, sizeof buf, fd); 
    G_strip(buf);
    if (sscanf(buf,"CAMERA YP     %lf \n",&Yp) == 1)
       cam_info->Yp = Yp;

    G_getl (buf, sizeof buf, fd); 
    G_strip(buf);
    if (sscanf(buf,"CAMERA CFL    %lf \n",&CFL) == 1)
       cam_info->CFL = CFL;

    G_getl (buf, sizeof buf, fd); 
    G_strip(buf);
    if (sscanf(buf,"NUM FID       %d \n",&num_fid) == 1)
       cam_info->num_fid = num_fid;

    for (n=0; n<cam_info->num_fid; n++) 
    {
        G_getl (buf, sizeof buf, fd);
	G_strip(buf);
	if (sscanf (buf, "%s %lf %lf", fid_id, &Xf, &Yf) == 3)
	{  strcpy(cam_info->fiducials[n].fid_id, fid_id);
           cam_info->fiducials[n].Xf = Xf;
           cam_info->fiducials[n].Yf = Yf; 
        }     
    }

    return 1;
}

I_new_fid_point (cam_info, fid_id, Xf, Yf)
    struct Ortho_Camera_File_Ref *cam_info;
    char fid_id[30];
    double Xf, Yf;
{
    int i;
    unsigned int size;

}

I_write_cam_info (fd, cam_info)
    FILE *fd;
    struct Ortho_Camera_File_Ref *cam_info;
{
    int i;

    fprintf (fd,"CAMERA NAME   %s \n",cam_info->cam_name);
    fprintf (fd,"CAMERA ID     %s \n",cam_info->cam_id);
    fprintf (fd,"CAMERA XP     %lf \n",cam_info->Xp);
    fprintf (fd,"CAMERA YP     %lf \n",cam_info->Yp);
    fprintf (fd,"CAMERA CFL    %lf \n",cam_info->CFL);
    fprintf (fd,"NUM FID       %d \n",cam_info->num_fid);
    for (i = 0; i < cam_info->num_fid; i++)
	fprintf (fd, "  %5s %15lf %15lf \n",
		cam_info->fiducials[i].fid_id,
                cam_info->fiducials[i].Xf,
                cam_info->fiducials[i].Yf);
}

I_get_cam_info (camera, cam_info)
    char *camera;
    struct Ortho_Camera_File_Ref *cam_info;
{
    FILE *fd;
    char msg[100];
    int stat;

    fd = I_fopen_cam_file_old (camera);
    if (fd == NULL)
    {
	sprintf (msg, "unable to open camera file %s in %s",
		camera, G_mapset());
	G_warning (msg);
	return 0;
    }

    stat = I_read_cam_info (fd, cam_info);
    fclose (fd);
    if (stat < 0)
    {
	sprintf (msg, "bad format in camera file %s in %s",
		camera, G_mapset());
	G_warning (msg);
	return 0;
    }
    return 1;
}

I_put_cam_info (camera, cam_info)
    char *camera;
    struct Ortho_Camera_File_Ref *cam_info;
{
    FILE *fd;
    char msg[100];

    fd = I_fopen_cam_file_new (camera);
    if (fd == NULL)
    {
	sprintf (msg, "unable to open camera file %s in %s",
		camera, G_mapset());
	G_warning (msg);
	return 0;
    }

    I_write_cam_info (fd, cam_info);
    fclose (fd);
    return 1;
}

