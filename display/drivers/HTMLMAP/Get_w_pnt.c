/*
 * Using mouse device, get a new screen coordinate and button number.
 * Button numbers must be the following values which correspond to the
 * following software meanings:
 *   1 - left button
 *   2 - middle button
 *   3 - right button
 *
 * This is called directly by the application programs.
 *
 * A pointer is used. This can be a crosshair, pointer, or cursor. 
 * It starts out at (*wx, *wy) and then tracks the mouse.
 * Upon button depression, the current coordinate is returned in (*wx, *wy) and
 * the button pressed in returned in *button.
 */

Get_location_with_pointer2(wx, wy, button)
	int *wx, *wy ;    /* new x,y coordinate       */
	int *button ;     /* button pressed to return */
{
}
