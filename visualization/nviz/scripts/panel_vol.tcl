##########################################################################
# Routines for volume control panel for Nviz program.
# Written Winter 2003
# Tomas Paudits
##########################################################################

##########################################################################
# procedure to make main control area
##########################################################################
proc mkvolPanel { BASE } {
    global Nv_

    catch {destroy $BASE}
    if {[catch {set Nv_(VolVisStyle)}]} {set Nv_(VolVisStyle) isosurfs}

    #  Initialize panel info
    if [catch {set Nv_($BASE)}] {
    set panel [St_create {window name size priority} $BASE "Volumes" 2 5]
    } else {
    set panel $Nv_($BASE)
    }

    frame $BASE  -relief groove -borderwidth 2
    Nv_mkPanelname $BASE "Volume Panel"

    ###### make widgets that control which is current volume #############
    set tmp [frame $BASE.volcontrol]

    label $tmp.current -text "Current:" -anchor nw
    mkMapList $tmp.list vol
    button $tmp.new -text New -anchor ne -command new_vol
    button $tmp.delete -text Delete -anchor ne -command delete_vol

    pack $tmp.current $tmp.list -side left
    pack $tmp.delete $tmp.new -side right -expand 1

    set tmp [frame $BASE.vistype]
    ######## controls for controling volume visualization ###############
     label $tmp.title -text "Visualization Type: " -relief flat
    menubutton $tmp.list -menu $tmp.list.m -relief flat
    set pname [menu $tmp.list.m]
    $pname add command -label "isosurfaces" -command "shuffle_vistype $BASE isosurfs"
    $pname add command -label "slices" -command "shuffle_vistype $BASE slices"
    pack $tmp.title $tmp.list -side left

    ######## subpanels for control volume visualization #################
    set tmp [frame $BASE.subpanel]
    create_isosurfs_subpanel $tmp
    create_slices_subpanel $tmp
    # show current visualization type
    shuffle_vistype $BASE $Nv_(VolVisStyle)

    ############# manage  frames #########################################
    pack $BASE.volcontrol -side top -fill both -expand 1
    pack $BASE.vistype -side top -fill both -expand 1
    pack $BASE.subpanel -side top -fill both -expand 1

    ########## make button to close panel ################################
    button $BASE.close -text Close -command "Nv_closePanel $BASE"
    button $BASE.draw_current -text "Draw Current"
	bind $BASE.draw_current <1> "Nset_cancel 1"
	bind $BASE.draw_current <B1-ButtonRelease> "Nvol_draw_one [Nget_current vol]"

    pack $BASE.close -side right -fill y
    pack $BASE.draw_current -side left -fill y

    return $panel
}

# Reset procedure for this panel
proc Nviz_vol_reset {} {
	# Need to nuke all the volumes here to reset
    set vol_list [Nget_vol_list]

    foreach i $vol_list {
	Nvol$i delete
    }

    set_new_curr vol 0
}

# Save procedure for saving state of Nviz
proc Nviz_vol_save {file_hook} {

}

# Load procedure for loading state of Nviz
proc Nviz_vol_load { file_hook } {

}

# Add new volume
proc new_vol {} {
    global Nv_
	set new [create_map_browser .fbrowse vol 1]

    # Let user know that we are busy
    appBusy

    if {$new != "-1"} {
        set temp [Nnew_map_obj vol]
        $temp set_att map $new

        # Make sure we call the focus routine if this is the first surface loaded
        if {$Nv_(CALLED_SET_FOCUS) == "no"} then {
            Nset_focus_map
            set Nv_(CALLED_SET_FOCUS) yes
        }

        set_new_curr vol [string range $temp 4 end]
    }

    # Let user know that he may proceed
    appNotBusy
}

# Delete volume
proc delete_vol {} {
    set curr [Nget_current vol]

    if {0 != $curr} {
        Nvol$curr delete

        set name 0
        set new_list [Nget_vol_list]

        if {[llength $new_list] != 0} then {
            set name [lindex $new_list 0]
        } else {
            set name 0
        }

        set_new_curr vol $name
    }
}

######################## subpanel change #################################

# A quick routine for shuffling the current visualization type
proc shuffle_vistype { BASE new } {
    global Nv_

    set tmp $BASE.subpanel

    catch {pack forget $tmp.isosurfs}
    catch {pack forget $tmp.slices}

    pack $tmp.$new -side bottom -fill both -expand yes

    switch $new {
        isosurfs {
			$BASE.vistype.list configure -text "isosurfaces"
			update_isosurfs_subpanel $tmp.isosurfs
		}
        slices {
			$BASE.vistype.list configure -text "slices"
			update_slices_subpanel $tmp.slices
		}
    }

	set Nv_(VolVisStyle) $new
}

############################## slices ####################################

# Create subpanel for slices
proc create_slices_subpanel { BASE } {
    global Nv_

	if {[catch {set Nv_(ShadeStyle)}]}    {set Nv_(ShadeStyle) gouraud}
	if {[catch {set Nv_(SliceAxis)}]}     {set Nv_(SliceAxis) 2}
	if {[catch {set Nv_(SliceSelected)}]} {set Nv_(SliceSelected) -1}

	set pname [frame $BASE.slices -relief raised]

	# resolution, shading and position widgets
    frame $pname.t1 -relief groove -borderwidth 2

    set tmp [frame $pname.t1.l]
    # polygon resolution
    Nv_mkArrows $tmp.polyarrows "Polygon  Resolution" slice_set_res 8
    set Nv_(SlicePolyResWidget) $tmp.polyarrows.f2.entry
    pack $tmp.polyarrows -side right -expand 1 -fill y
    bind $tmp.polyarrows.f2.entry <Return> "+ slice_set_res"

    set tmp [frame $pname.t1.r]
    # shading menu
    menubutton $tmp.shading -text "Shading:" -menu $tmp.shading.m \
    -relief raised -underline 0 -indicatoron 1
    menu $tmp.shading.m
    $tmp.shading.m add radiobutton -label Flat -value flat \
    -variable Nv_(ShadeStyle) -command "slice_set_drawmode"
    $tmp.shading.m add radiobutton -label Gouraud -value gouraud \
    -variable Nv_(ShadeStyle) -command "slice_set_drawmode"
    # position button

	button $tmp.position -text "Position" -command "mkVolPositionPanel .pos_vol"
    pack $tmp.shading $tmp.position -pady 1 -expand 1 -fill both

	# slice manipulation widgets
	frame $pname.t2 -relief groove -borderwidth 2

	# slice axes
	set tmp [frame $pname.t2.t -borderwidth 1 -relief raised]
	radiobutton $tmp.xaxis -text "X-axis" -state disabled -anchor nw -value 0 \
	 -variable Nv_(SliceAxis) -command "slice_set_pos $pname"
    radiobutton $tmp.yaxis -text "Y-axis" -state disabled -anchor nw -value 1 \
	-variable Nv_(SliceAxis) -command "slice_set_pos $pname"
	radiobutton $tmp.zaxis -text "Z-axis" -state disabled -anchor nw -value 2 \
	 -variable Nv_(SliceAxis) -command "slice_set_pos $pname"
	pack $tmp.xaxis $tmp.yaxis $tmp.zaxis -expand 1 -side left

	button $tmp.transp -text "Transparency" -command "slice_set_transp" -state disabled
	pack $tmp.transp -pady 1 -fill both -side right

	# slice position
	set tmp [frame $pname.t2.b1 -borderwidth 1 -relief raised]
	mkSlicePosScale $tmp.s_x1 X1 "X1 :" $pname
	mkSlicePosScale $tmp.s_x2 X2 "X2 :" $pname
	pack $tmp.s_x1 $tmp.s_x2 -fill both -side left -expand 1

	set tmp [frame $pname.t2.b2 -borderwidth 1 -relief raised]
	mkSlicePosScale $tmp.s_y1 Y1 "Y1 :" $pname
	mkSlicePosScale $tmp.s_y2 Y2 "Y2 :" $pname
	pack $tmp.s_y1 $tmp.s_y2 -fill both -side left -expand 1

	set tmp [frame $pname.t2.b3 -borderwidth 1 -relief raised]
	mkSlicePosScale $tmp.s_z1 Z1 "Z1 :" $pname
	mkSlicePosScale $tmp.s_z2 Z2 "Z2 :" $pname
	pack $tmp.s_z1 $tmp.s_z2 -fill both -side left -expand 1

	frame $pname.t3 -relief groove -borderwidth 2
	# list
	set tmp [frame $pname.t3.l]
    list_type_vscroll $tmp.list
    bind $tmp.list.l <<ListboxSelect>> "+ slice_update_att $pname"
    pack $tmp.list -side right -expand 1 -fill both

	# buttons
	set tmp [frame $pname.t3.r]
	button $tmp.add -text "Add" -command "slice_add $pname"
    button $tmp.delete -text "Delete" -command "slice_del $pname"
	label $tmp.lorder
	button $tmp.up -text "Move Up" -command "slice_up $pname"
    button $tmp.down -text "Move Down" -command "slice_down $pname"
    pack $tmp.add $tmp.delete $tmp.lorder $tmp.up $tmp.down -pady 2 -expand 1 -fill x -side top

	pack $pname.t1.l $pname.t1.r -side left -expand 1 -fill both
    pack $pname.t1 -expand 1 -fill x -side top
	pack $pname.t2.t $pname.t2.b1 $pname.t2.b2 $pname.t2.b3 -side top -expand 1 -fill both
    pack $pname.t2 -expand 1 -fill x -side top
	pack $pname.t3.l -side left -expand 1 -fill both
	pack $pname.t3.r -side right -expand 0 -fill both
	pack $pname.t3 -expand 1 -fill x -side top

	# update subpanel info
    update_slices_subpanel $pname
}

# Update slices subpanel info
proc update_slices_subpanel { BASE {select -1}} {
    global Nv_

    set curr [Nget_current vol]
    if {0 == $curr} return

    # set slices resolution and drawmode
    set L [Nvol$curr slice get_res]
	set res [lindex $L 1]
	Nv_setEntry $Nv_(SlicePolyResWidget) [lindex $L 1]
    set Nv_(ShadeStyle) [Nvol$curr slice get_drawmode]

    # fill list with slices
    set n_slices [Nvol$curr slice num_slices]
    set listw $BASE.t3.l.list
    $listw.l delete 0 $n_slices
    for {set i 0} {$i < $n_slices} {incr i} {
		set Lpos [Nvol$curr slice get_pos $i]

		if { [lindex $Lpos 6] == "0"} { set dir X }
		if { [lindex $Lpos 6] == "1"} { set dir Y }
		if { [lindex $Lpos 6] == "2"} { set dir Z }

		set x1 [lindex $Lpos 0]
		set x2 [lindex $Lpos 1]
		set y1 [lindex $Lpos 2]
		set y2 [lindex $Lpos 3]
		set z1 [lindex $Lpos 4]
		set z2 [lindex $Lpos 5]

		set crnt [expr $i + 1]

		$listw.l insert end "$dir-Axis, $x1, $y1, $x2, $y2"
    }

	# select slice in list
    if {$select != -1} then {
        $listw.l selection set $select $select
    }

    slice_update_att $BASE
}

# Make slice position scale widget
proc mkSlicePosScale { S name title BASE } {
	global Nv_

	frame $S

	label $S.l -text $title -anchor nw -state disabled

	scale $S.s -orient horizontal -showvalue 0 -state disabled \
	-variable Nv_(SlicePos$name) -from 0.0 -to 1.0 -resolution 0

	bind $S.s <B1-ButtonRelease> "slice_set_pos $BASE"
	bind $S.s <B1-ButtonRelease> "+ if {[Nauto_draw] == 1} {Ndraw_all}"
	bind $S.s <B1-Motion> "slice_set_pos $BASE; Nquick_draw"

	pack $S.l $S.s -fill both -side left -expand 1

	return $S
}

# Enable slice position scale widget
proc enableSlicePosScale { S name value} {
	global Nv_

	$S.l configure -state normal
	$S.s configure -state normal

	set Nv_(SlicePos$name) $value
}

# Disable slice position scale widget
proc disableSlicePosScale { S name } {
	global Nv_

	$S.l configure -state disabled
	$S.s configure -state disabled

	set Nv_(SlicePos$name) 0.0
}

# Get index of selected slice
proc slice_get_selected { BASE } {
    set curr [Nget_current vol]
    if {0 == $curr} then {
        return -1
    }

    set listw $BASE.t3.l.list
    set is_selected [llength [$listw.l curselection]]

    if {$is_selected == 1} then {
        set range [$listw.l curselection]
        return [lindex $range 0]
    } else {
        return -1
    }
}

# Set slice transparency
proc slice_set_transp {} {
	global Nv_

	set curr [Nget_current vol]
    if {0 == $curr} return

	set id $Nv_(SliceSelected)
	if {$id == -1} return

	set value [create_slideconstant_popup .new_const 1]
	Nvol$curr slice set_transp $id $value
}

# Update menu for current slice
proc slice_update_att { BASE } {
    global Nv_

    set attw $BASE.t2
    set id [slice_get_selected $BASE]

    if {$id != -1} then {
        set Lpos [Nvol[Nget_current vol] slice get_pos $id]

		set Nv_(SliceAxis)   [lindex $Lpos 6]
		set Nv_(SliceSelected) $id

        # enable attribute widgets
        $attw.t.xaxis configure -state normal
        $attw.t.yaxis configure -state normal
        $attw.t.zaxis configure -state normal
        $attw.t.transp configure -state normal

		# slice position scales
		enableSlicePosScale $attw.b1.s_x1 X1 [lindex $Lpos 0]
		enableSlicePosScale $attw.b1.s_x2 X2 [lindex $Lpos 1]
		enableSlicePosScale $attw.b2.s_y1 Y1 [lindex $Lpos 2]
		enableSlicePosScale $attw.b2.s_y2 Y2 [lindex $Lpos 3]
		enableSlicePosScale $attw.b3.s_z1 Z1 [lindex $Lpos 4]
		enableSlicePosScale $attw.b3.s_z2 Z2 [lindex $Lpos 5]

	} else {
		# set undefined values
		set Nv_(SliceAxis) 2
		set Nv_(SliceSelected) -1

        # disable attribute widgets
        $attw.t.xaxis configure -state disabled
        $attw.t.yaxis configure -state disabled
        $attw.t.zaxis configure -state disabled
        $attw.t.transp configure -state disabled

		# x1, y1
		disableSlicePosScale $attw.b1.s_x1 X1
		disableSlicePosScale $attw.b1.s_x2 X2
		disableSlicePosScale $attw.b2.s_y1 Y1
		disableSlicePosScale $attw.b2.s_y2 Y2
		disableSlicePosScale $attw.b3.s_z1 Z1
		disableSlicePosScale $attw.b3.s_z2 Z2
    }
}

# Set slices polygon resolution
proc slice_set_res {{args null}} {
    global Nv_

    set curr [Nget_current vol]

    if {0 != $curr} {
        set res [$Nv_(SlicePolyResWidget) get]
		puts "setres: $res"
        Nvol$curr slice set_res $res $res $res
    }
}

# Set slices drawmode
proc slice_set_drawmode {} {
    global Nv_

    set curr [Nget_current vol]

    if {0 != $curr} {
        Nvol$curr slice set_drawmode $Nv_(ShadeStyle)
    }
}

# Set slice position
proc slice_set_pos { BASE } {
	global Nv_

	set curr [Nget_current vol]
    if {0 == $curr} return

    set id $Nv_(SliceSelected)
	if {$id == -1} return

	set x1 $Nv_(SlicePosX1)
	set x2 $Nv_(SlicePosX2)
	set y1 $Nv_(SlicePosY1)
	set y2 $Nv_(SlicePosY2)
	set z1 $Nv_(SlicePosZ1)
	set z2 $Nv_(SlicePosZ2)

	set dir $Nv_(SliceAxis)

	Nvol$curr slice set_pos $id $x1 $x2 $y1 $y2 $z1 $z2 $dir

	update_slices_subpanel $BASE $id
}

# Add slice
proc slice_add { BASE } {
    set curr [Nget_current vol]
    if {0 == $curr} return

    set id [Nvol$curr slice num_slices]
    Nvol$curr slice add

	# set defaults
	Nvol$curr slice set_pos $id 0 1 0 1 0 1 0

    update_slices_subpanel $BASE $id
}

# Delete slice
proc slice_del { BASE } {
    set curr [Nget_current vol]
    if {0 == $curr} return

    set id [slice_get_selected $BASE]

    if {$id != -1} then {
        Nvol$curr slice del $id
    }

	update_slices_subpanel $BASE [expr $id - 1]
}

# Move up slice
proc slice_up { BASE } {
    set curr [Nget_current vol]
    if {0 == $curr} return

    set id [slice_get_selected $BASE]

    if {$id != -1} then {
        Nvol$curr slice move_up $id
    }

	if {$id != 0} then {
		set select [expr $id - 1]
	} else {
		set select $id
	}

	update_slices_subpanel $BASE $select
}

# Move down slice
proc slice_down { BASE } {
    set curr [Nget_current vol]
    if {0 == $curr} return

    set id [slice_get_selected $BASE]

    if {$id != -1} then {
        Nvol$curr slice move_down $id
    }

	set n [Nvol$curr slice num_slices]

	if {$id != [expr $n - 1]} then {
		set select [expr $id + 1]
	} else {
		set select $id
	}

	update_slices_subpanel $BASE $select
}

########################### isosurfaces ##################################

# Create subpanel for isosurfaces
proc create_isosurfs_subpanel { BASE } {
    global Nv_

    if {[catch {set Nv_(ShadeStyle)}]}         {set Nv_(ShadeStyle) gouraud}

    set pname [frame $BASE.isosurfs -relief raised]

    # resolution, shading and position widgets
    frame $pname.t1 -relief groove -borderwidth 2

    set tmp [frame $pname.t1.l]
    # polygon resolution
    Nv_mkArrows $tmp.polyarrows "Polygon  Resolution" isosurf_set_res 8
    set Nv_(IsoPolyResWidget) $tmp.polyarrows.f2.entry
    pack $tmp.polyarrows -side right -expand 1 -fill y
    #bind $tmp.polyarrows.f2.entry <Motion> "+ isosurf_set_res"
    bind $tmp.polyarrows.f2.entry <Return> "+ isosurf_set_res"

    set tmp [frame $pname.t1.r]
    # shading menu
    menubutton $tmp.shading -text "Shading:" -menu $tmp.shading.m \
    -relief raised -underline 0 -indicatoron 1
    menu $tmp.shading.m
    $tmp.shading.m add radiobutton -label Flat -value flat \
    -variable Nv_(ShadeStyle) -command "isosurf_set_drawmode"
    $tmp.shading.m add radiobutton -label Gouraud -value gouraud \
    -variable Nv_(ShadeStyle) -command "isosurf_set_drawmode"
    # position button
    button $tmp.position -text "Position" -command "mkVolPositionPanel .pos_vol"
    pack $tmp.shading $tmp.position -pady 1 -expand 1 -fill both

    pack $pname.t1.l $pname.t1.r -side left -expand 1 -fill both
    pack $pname.t1 -expand 1 -fill x -side top

    # isosurface attributes and flags
    set tmp [frame $pname.t2 -relief groove -borderwidth 2]

    # isosurface attributes
    menubutton $tmp.menu1 -menu $tmp.menu1.m -text "Isosurface Attribute" -relief raised \
    -indicatoron 1 -bd 2 -state disabled
    pack $tmp.menu1 -side left -fill y -padx 2
    menu $tmp.menu1.m
    foreach i {threshold color mask transparency shininess emission} {
        $tmp.menu1.m add command
    }

    # inout flag
    checkbutton $tmp.inout -text "toggle normal direction" -state disabled \
    -variable Nv_(InoutFlag) -command "isosurf_set_flags $pname"

    pack $tmp.inout -side left -padx 2 -expand 1 -fill x

    pack $pname.t2 -expand 1 -fill x -side top

    # isosurfaces list and manipulation buttons
    frame $pname.t3 -relief groove -borderwidth 2

    # list
    set tmp [frame $pname.t3.l]
    list_type_vscroll $tmp.list
    bind $tmp.list.l <<ListboxSelect>> "+ isosurf_update_att $pname"
    pack $tmp.list -side right -expand 1 -fill both

    # add, modify, delete buttons
    set tmp [frame $pname.t3.r]
    button $tmp.add -text "Add" -command "isosurf_add $pname"
    button $tmp.delete -text "Delete" -command "isosurf_del $pname"
	label $tmp.lorder
	button $tmp.up -text "Move Up" -command "isosurf_up $pname"
    button $tmp.down -text "Move Down" -command "isosurf_down $pname"

    pack $tmp.add $tmp.delete $tmp.lorder $tmp.up $tmp.down -pady 2 -expand 1 -fill x -side top

	pack $pname.t3.l -side left -expand 1 -fill both
	pack $pname.t3.r -side right -expand 0 -fill both
    pack $pname.t3 -expand 1 -fill x -side top

    # update subpanel info
    update_isosurfs_subpanel $pname
}

# Update isosurfaces subpanel info
proc update_isosurfs_subpanel { BASE {select null}} {
    global Nv_

    set curr [Nget_current vol]
    if {0 == $curr} return

    # set isosurface resolution and drawmode
    set L [Nvol$curr isosurf get_res]
    Nv_setEntry $Nv_(IsoPolyResWidget) [lindex $L 1]
    set Nv_(ShadeStyle) [Nvol$curr isosurf get_drawmode]

    # fill list isosurfaces level
    set n_isosurfs [Nvol$curr isosurf num_isosurfs]
    set listw $BASE.t3.l.list
    $listw.l delete 0 $n_isosurfs
    for {set i 0} {$i < $n_isosurfs} {incr i} {
        set thresh [isosurf_get_att_status $i threshold]
        $listw.l insert end "level: $thresh"
    }

    # select isosurface in list
    if {$select != "null"} then {
        $listw.l selection set $select $select
    }

    isosurf_update_att $BASE
}

# Get index of selected isosurface
proc isosurf_get_selected { BASE } {
    set curr [Nget_current vol]
    if {0 == $curr} then {
        return -1
    }

    set listw $BASE.t3.l.list
    set is_selected [llength [$listw.l curselection]]

    if {$is_selected == 1} then {
        set range [$listw.l curselection]
        return [lindex $range 0]
    } else {
        return -1
    }
}

# Update menu for current isosurface
proc isosurf_update_att { BASE } {
    global Nv_

    set attw $BASE.t2
    set id [isosurf_get_selected $BASE]

    if {$id != -1} then {
        # update attribute menu
        set ndx 1
        foreach i {threshold color mask transparency shininess emission} {
        $attw.menu1.m entryconfigure $ndx \
        -label "$i: [isosurf_get_att_status $id $i]" \
        -command "mkIsosurfAttPopup .pop $id $i 1; update_isosurfs_subpanel $BASE $id"
        incr ndx
        }

        # set isosurface flags
        set Nv_(InoutFlag) [isosurf_get_flag_status $id "inout"]

        # enable attribute widgets
        $attw.menu1 configure -state normal
        $attw.inout configure -state normal
    } else {
        # set isosurface flags
        set Nv_(InoutFlag) 0

        # disable attribute widgets
        $attw.menu1 configure -state disabled
        $attw.inout configure -state disabled
    }
}

# Set isosurfaces polygon resolution
proc isosurf_set_res {{args null}} {
    global Nv_

    set curr [Nget_current vol]

    if {0 != $curr} {
        set res [$Nv_(IsoPolyResWidget) get]
        Nvol$curr isosurf set_res $res $res $res
    }
}

# Set isosurfaces drawmode
proc isosurf_set_drawmode {} {
    global Nv_

    set curr [Nget_current vol]

    if {0 != $curr} {
        Nvol$curr isosurf set_drawmode $Nv_(ShadeStyle)
    }
}

# Add isosurface
proc isosurf_add { BASE } {
    global attIsoPopup_Status

    set curr [Nget_current vol]
    if {0 == $curr} return

    set id [Nvol$curr isosurf num_isosurfs]
    Nvol$curr isosurf add

    # Now automatically invoke the threshold popup
    mkIsosurfAttPopup .temporary_new_isosurf $id threshold 1

    if {"$attIsoPopup_Status" == "no_change"} then {
         Nvol$curr isosurf del $id
    }
	if {"$attIsoPopup_Status" == "threshold not set"} then {
         Nvol$curr isosurf del $id
    }

    update_isosurfs_subpanel $BASE $id
}

# Delete isosurface
proc isosurf_del { BASE } {
    set curr [Nget_current vol]
    if {0 == $curr} return

    set id [isosurf_get_selected $BASE]

    if {$id != -1} then {
        Nvol$curr isosurf del $id
    }

    update_isosurfs_subpanel $BASE
}

# Move up isosurf
proc isosurf_up { BASE } {
    set curr [Nget_current vol]
    if {0 == $curr} return

    set id [isosurf_get_selected $BASE]

    if {$id != -1} then {
        Nvol$curr isosurf move_up $id
    }

	if {$id != 0} then {
		set select [expr $id - 1]
	} else {
		set select $id
	}

	update_isosurfs_subpanel $BASE $select
}

# Move down isosurf
proc isosurf_down { BASE } {
    set curr [Nget_current vol]
    if {0 == $curr} return

    set id [isosurf_get_selected $BASE]

    if {$id != -1} then {
        Nvol$curr isosurf move_down $id
    }

	set n [Nvol$curr isosurf num_isosurfs]

	if {$id != [expr $n - 1]} then {
		set select [expr $id + 1]
	} else {
		set select $id
	}

	update_isosurfs_subpanel $BASE $select
}

# Get isosurface attribute status
proc isosurf_get_att_status {id att} {
    set curr [Nget_current vol]

    if {$curr == 0} return
    if {$id == -1} return

    set map_pair [Nvol$curr isosurf get_att $id $att]

    set att_type [lindex $map_pair 0]
    set att_value [lindex $map_pair 1]

    if {$att_type == "unset"} then {
        set txt "$att not set"
    } elseif {$att_type == "map"} then {
        set txt $att_value
    } else {
        if {"$att" == "color"} then {
            set tmp [expr int($att_value)]

            set red   [expr int($tmp & 0x0000ff)]
            set green [expr int(($tmp & 0x00ff00)>>8)]
            set blue  [expr int(($tmp & 0xff0000)>>16)]

            set txt "R$red, G$green, B$blue"
        } else {
            set txt $att_value
        }
    }

    return $txt
}

# Get isosurface flag
proc isosurf_get_flag_status {id flag} {
    set curr [Nget_current vol]

    if {$curr == 0} return
    if {$id == -1} return

    set flags [Nvol$curr isosurf get_flags $id]

    if {$flag == "inout"} then {
        return [lindex $flags 0]
    }

    return -1;
}

# Set isosurface flags
proc isosurf_set_flags { BASE } {
    global Nv_

    set curr [Nget_current vol]
    if {$curr == 0} return

    set id [isosurf_get_selected $BASE]
    if {$id == -1} return

    Nvol$curr isosurf set_flags $id $Nv_(InoutFlag)
}

##############################################################################

# Create popup for positioning the current surface
proc mkVolPositionPanel { w } {
    global Nv_

    # Create toplevel widget to hold everything
    toplevel $w -class Dialog
    wm title $w "Position Volume"
    wm iconname $w "Attribute"

    # Get current surface attributes
    set curr_vol [Nget_current vol]
    if {0 == $curr_vol} then {
	destroy $w
	return
    }
    set temp [Nvol$curr_vol get_trans]
    set cur_x [lindex $temp 0]
    set cur_y [lindex $temp 1]
    set cur_z [lindex $temp 2]

    # Make a position widget
    # Create the update routines unique for this widget
    set update_routine update
    append update_routine $w
    set ucmd "proc $update_routine \{x y\} \{ set z \[$w.zslide get\] ; psSetVolPos $w \[Nget_current vol\] \$x \$y \$z \}"
    uplevel #0 $ucmd
    set pos [Nv_mkXYScale $w.pos cross VOL_POS 126 126 63 63 $update_routine]
    pack $pos -side top -padx 3 -pady 3

    # Create frames to manage everything
    frame $w.left
    pack $w.left -side left -fill both -before $pos -padx 3 -pady 3 -anchor nw
    frame $w.bottom
    pack $w.bottom -side bottom -before $w.left -fill both -padx 3 -pady 3 -anchor sw

    # Create X, Y, and Z entry widgets along with Reset and Close buttons
    frame $w.coords
    label $w.coords.z_lbl -text "Z:"
    label $w.coords.x_lbl -text "X:"
    label $w.coords.y_lbl -text "Y:"
    entry $w.coords.z_ent -width 5 -relief sunken
    entry $w.coords.x_ent -width 5 -relief sunken
    entry $w.coords.y_ent -width 5 -relief sunken
    bind $w.coords.x_ent <Return> "psSetVolPosFromEntry $w $curr_vol"
    bind $w.coords.y_ent <Return> "psSetVolPosFromEntry $w $curr_vol"
    bind $w.coords.z_ent <Return> "psSetVolPosFromEntry $w $curr_vol"
    entry $w.coords.z_ent.old
    entry $w.coords.x_ent.old
    entry $w.coords.y_ent.old
    $w.coords.x_ent.old delete 0 end
    $w.coords.y_ent.old delete 0 end
    $w.coords.z_ent.old delete 0 end
    $w.coords.x_ent.old insert end $cur_x
    $w.coords.y_ent.old insert end $cur_y
    $w.coords.z_ent.old insert end $cur_z
    pack $w.coords.z_lbl $w.coords.z_ent \
	$w.coords.x_lbl $w.coords.x_ent \
	$w.coords.y_lbl $w.coords.y_ent -side left
    pack $w.coords -side top -in $w.bottom -fill both

    frame $w.commands
    button $w.commands.reset 	-text "Reset"	-command "$w.zslide set 0; Nv_itemDrag $w.pos $Nv_(VOL_POS) 63 63; Nv_xyCallback $update_routine 126 126 63 63"
    button $w.commands.close	-text "Close"	-command "destroy $w"
    pack $w.commands.reset $w.commands.close \
	-side left -fill both -padx 3 -pady 3 -expand yes
    pack $w.commands -in $w.bottom -fill both

    # Create Z slider
    set update_routine zupdate
    append update_routine $w
    set ucmd "proc $update_routine \{ z \} \{ psSetVolPos $w \[Nget_current vol\] \[lindex \[Nv_getXYPos VOL_POS\] 0\] \[lindex \[Nv_getXYPos VOL_POS\] 1\] \$z \}"
    uplevel #0 $ucmd
    scale $w.zslide -orient vertical -from 1000 -to -1000 -showvalue false \
	-activebackground gray80 -troughcolor gray90 -command $update_routine
    pack $w.zslide -side top -in $w.left

    # Initialize the interface and off we go
    psSetVolPos $w $curr_vol 0.5 0.5 0

    # Wait until the window is destroyed and exit
    tkwait window $w

    return
}

# Routine to set the position of the current volume and update the volume position popup
proc psSetVolPos {w curr_vol x y z} {

    # Grab old coordinates
    set x_old [$w.coords.x_ent.old get]
    set y_old [$w.coords.y_ent.old get]
    set z_old [$w.coords.z_ent.old get]

    # First update the interface
    $w.coords.x_ent delete 0 end
    $w.coords.y_ent delete 0 end
    $w.coords.z_ent delete 0 end

    # Set the Z slider appropriately
    set z [expr $z + $z_old]

    # Set the XY position appropriately.  These are given
    # as ratios so we convert the ratios to actual x-y shifts
    set range [Nget_xyrange]
    set x [expr ($x - 0.5) * $range + $x_old]
    set y [expr ($y - 0.5) * $range + $y_old]

    # Make the call to the underlying library to set
    # surface position
    Nvol$curr_vol set_trans $x $y $z
	Nquick_draw

	$w.coords.x_ent insert 0 "$x"
    $w.coords.y_ent insert 0 "$y"
    $w.coords.z_ent insert 0 "$z"
}


# Routine to set the position of the current volume and update the volume position popup
proc psSetVolPosFromEntry {w curr_vol} {
    global Nv_

    # Grab coordinates directly from entry widgets
    set x [$w.coords.x_ent get]
    set y [$w.coords.y_ent get]
    set z [$w.coords.z_ent get]
    set x_old [$w.coords.x_ent.old get]
    set y_old [$w.coords.y_ent.old get]
    set z_old [$w.coords.z_ent.old get]

    # Figure out appropriate positions
    set xdiff [expr $x - $x_old]
    set ydiff [expr $y - $y_old]
    set zdiff [expr $z - $z_old]
    if {$xdiff < -500} 	then { set xdiff -500 }
    if {$xdiff > 500} 	then { set xdiff 500 }
    if {$ydiff < -500} 	then { set ydiff -500 }
    if {$ydiff > 500} 	then { set ydiff 500 }
    if {$zdiff < [lindex [$w.zslide configure -to] 4]} then {
	$w.zslide configure -to $zdiff
    }
    if {$zdiff > [lindex [$w.zslide configure -from] 4]} 	then {
	$w.zslide configure -from $zdiff
    }

    # Update Z slider position
    $w.zslide set $zdiff
    update

    # Update crosshair interface
    set xrat [expr int(($xdiff / 1000.0 + 0.5) * 126)]
    set yrat [expr int(($ydiff / 1000.0 + 0.5) * 126)]
    Nv_itemDrag $w.pos $Nv_(VOL_POS) $xrat $yrat

    # Make the call to the underlying library to set
    # surface position
    Nvol$curr_vol set_trans [expr $xdiff + $x_old] \
	[expr $ydiff + $y_old] [expr $zdiff + $z_old]

	Nquick_draw

    $w.coords.x_ent delete 0 end
    $w.coords.y_ent delete 0 end
    $w.coords.z_ent delete 0 end
    $w.coords.x_ent insert end [expr $xdiff + $x_old]
    $w.coords.y_ent insert end [expr $ydiff + $y_old]
    $w.coords.z_ent insert end [expr $zdiff + $z_old]
}

# Create a single select listbox widget with a vertical scrollbar.
proc list_type_vscroll {window} {
    frame $window -relief raised
    listbox $window.l -relief flat -yscrollcommand "$window.sr set" \
        -exportselection 0 -selectmode single
    scrollbar $window.sr -command "$window.l yview"
    pack $window -side bottom -expand yes -fill both
    pack $window.l -side left -expand yes -fill both
    pack $window.sr -side right -fill y
}
