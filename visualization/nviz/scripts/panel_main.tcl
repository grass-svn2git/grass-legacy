#########################################################################
# Default Priority for this panel
#
# priority is from 0 to 10
#  the lower the number, the quicker it will be bumped
#  10 cannot be bumped
#  Panels will be loaded by the greater of 5 or their current priority

#*** ACS_MODIFY 1.0 BEGIN ******************************************************
# flags to enable(1)/disable(0) added FlyThrough Functions
	set Nv_(FlyThrough) 1
	if {$Nv_(FlyThrough)} {source $src_boot/etc/nviz2.2/scripts/flythrough.tcl}

###########################################################################
# procedure to make main control area
###########################################################################

proc mkmainPanel { BASE } {
    global Nv_
    global XY
#Globals for draw features
    global surface vector sites volume
    global legend labels n_arrow fringe
    global n_arrow_x n_arrow_y n_arrow_z
    global fringe_nw fringe_ne fringe_sw fringe_se

    catch {destroy $BASE}

    #  Initialize panel info
    if [catch {set Nv_($BASE)}] {
	set panel [St_create {window name size priority} $BASE "Main" 1 10]
    } else {
	set panel $Nv_($BASE)
    }

    frame $BASE  -relief groove -borderwidth 2
    set Nv_(main_BASE) $BASE

    # make redraw button area
    pack [frame $BASE.redrawf]		-side top -fill x -expand 1
    pack [frame $BASE.redrawf.f1] 	-side top -fill x
    pack [frame $BASE.redrawf.f11]       -side top -fill x
    pack [frame $BASE.redrawf.f2] 	-side top -fill x

    set labl1 [label $BASE.redrawf.f1.label1  -text Auto: ]
    set auto [checkbutton $BASE.redrawf.f1.autoclear -text "Clear" \
		  -variable autoc ]
    help $BASE.redrawf.f1.autoclear balloon "Automatically clear screen before drawing"
    $auto select
    set auto_d [checkbutton $BASE.redrawf.f1.autodraw -text "Draw" \
                  -onvalue 1 -offvalue 0 -variable auto_draw ]
    help $BASE.redrawf.f1.autodraw balloon "Automatically draw selected features"
    $auto_d select
    pack $labl1 $auto $auto_d -side left -expand 1 -fill x

#checkbuttons for features to draw
    set labl2 [label $BASE.redrawf.f11.label1  -text Feature: ]

    menubutton $BASE.redrawf.f11.m1 -menu $BASE.redrawf.f11.m1.m \
	-relief raised -text "Main Features" -underline 0 -indicatoron 1
    help $BASE.redrawf.f11.m1 balloon "Select main draw features"

   menubutton $BASE.redrawf.f11.m2 -menu $BASE.redrawf.f11.m2.m \
        -relief raised -text "Decorations" -underline 0 -indicatoron 1
    help $BASE.redrawf.f11.m2 balloon "Select misc. draw features"

    menu $BASE.redrawf.f11.m1.m

    $BASE.redrawf.f11.m1.m add checkbutton -label "Surface" \
         -onvalue 1 -offvalue 0 -variable surface

    $BASE.redrawf.f11.m1.m add checkbutton -label "Vectors" \
         -onvalue 1 -offvalue 0 -variable vector

    $BASE.redrawf.f11.m1.m add checkbutton -label "Sites" \
         -onvalue 1 -offvalue 0 -variable sites

    $BASE.redrawf.f11.m1.m add checkbutton -label "Volumes" \
         -onvalue 1 -offvalue 0 -variable volume

    menu $BASE.redrawf.f11.m2.m
    
    $BASE.redrawf.f11.m2.m add checkbutton -label "Legend" \
         -onvalue 1 -offvalue 0 -variable legend
    $BASE.redrawf.f11.m2.m add checkbutton -label "Labels" \
         -onvalue 1 -offvalue 0 -variable labels
    $BASE.redrawf.f11.m2.m add checkbutton -label "North Arrow" \
         -onvalue 1 -offvalue 0 -variable n_arrow
    $BASE.redrawf.f11.m2.m add checkbutton -label "Fringe" \
         -onvalue 1 -offvalue 0 -variable fringe    

    pack $labl2  $BASE.redrawf.f11.m1 $BASE.redrawf.f11.m2 -side left \
    -expand 1 -fill x

#Set defaults
    set surface 1
    set vector 1
    set sites 1
    set volume 1

    set legend 0
    set labels 0
    set n_arrow 0
    set fringe 0

#Set North Arrow defaults
    set n_arrow_x 999
    set n_arrow_y 999
    set n_arrow_z 999

#Execute buttons

   button $BASE.redrawf.f2.exec -text DRAW 
   bind $BASE.redrawf.f2.exec <1> "Nset_cancel 0"
   bind $BASE.redrawf.f2.exec <B1-ButtonRelease> {Ndraw_all}
   help $BASE.redrawf.f2.exec balloon "Draw selected features"

   button $BASE.redrawf.f2.clear -text Clear -command {do_clear}
   help $BASE.redrawf.f2.clear balloon "Clear NVIZ display"

   button $BASE.redrawf.f2.cancel -text Cancel -command {Nset_cancel 1}
   help $BASE.redrawf.f2.cancel balloon "Cancel current draw"

   pack $BASE.redrawf.f2.exec  $BASE.redrawf.f2.clear $BASE.redrawf.f2.cancel \
    -side left -expand 1 -fill x


#pack frames
    pack [frame $BASE.midt ] -side top -expand 1 -fill x
    pack [frame $BASE.midf -relief raised -borderwidth 1 ] -side left -expand 1

    set draw_lab [label $BASE.midt.lab -text "View:" \
                 -relief flat]
    set draw_var1 [radiobutton $BASE.midt.b1 -text "eye" \
                 -variable draw_option -value 0 \
		-command "change_display 1" ]

    set draw_var2 [radiobutton $BASE.midt.b2 -text "center" \
                 -variable draw_option -value 1 \
                 -command "change_display 0" ]
	$draw_var1 select
   
    help $BASE.midt.b1 balloon "Change view by moving eye position"
    help $BASE.midt.b2 balloon "Change view by moving scene center position"


#*** ACS_MODIFY 1.0 BEGIN ******************************************************
	if {$Nv_(FlyThrough)} {
		mkFlyButtons $BASE "midt" $draw_lab $draw_var1 $draw_var2
	} else {
		# original code
	    pack $draw_lab $draw_var1 $draw_var2 -side left -expand 0
	}
#*** ACS_MODIFY 1.0 END ********************************************************

    help $BASE.midt.b3 balloon "Change view interactively in display"

    # make  position "widget"
    set XY [Nv_mkXYScale $BASE.midf.pos puck XY_POS 125 125 105 105 update_eye_position]
    set H [mk_hgt_slider $BASE.midf]
    set E [mk_exag_slider $BASE.midf]
    help $E.scale balloon "Set vertical exaggeration"
    help $E.entry balloon "Set vertical exaggeration"
    help $H.scale balloon "Set eye height"
    help $H.entry balloon "Set eye height"
    pack $XY $H $E -side left -expand y

    # make lookat buttons
    frame $BASE.midf.lookat -relief sunken -borderwidth 1

    label $BASE.midf.lookat.l -text LOOK

    button $BASE.midf.lookat.here -text here \
	-command {bind $Nv_(TOP).canvas <Button> {look_here %W %x %y
	if {[Nauto_draw] == 1} {Ndraw_all}
	}}

    button $BASE.midf.lookat.center -text center -command { look_center
	if {[Nauto_draw] == 1} {Ndraw_all} }
    button $BASE.midf.lookat.top -text top -command {
# Nv_itemDrag $Nv_(main_BASE).midf.pos $Nv_(XY_POS) 62.5 62.5
# note: below value is somewhat strange, but with 0.5 0.5 the map rotates:
#	update_eye_position 0.496802 0.50100
	set val2 [$Nv_(main_BASE).midf.height.f.entry get]
	Nset_focus_top $val2
	change_display 1
	update

        if {[Nauto_draw] == 1} {Ndraw_all}
}
    button $BASE.midf.lookat.cancel -text cancel -command no_focus

    frame $BASE.bframe
    frame $BASE.bframe.cframe -relief sunken -borderwidth 1
    set P [Nv_mkScale $BASE.bframe.cframe.pers h perspective 120 3 40 Nchange_persp 0]
    set T [Nv_mkScale $BASE.bframe.cframe.tw h twist 180 -180 0 Nchange_twist 0]
    help $BASE.bframe.cframe.pers balloon "Set field of view size (degrees)"
    help $BASE.bframe.cframe.tw balloon "Set twist angle (degrees)"

    button $BASE.midf.lookat.reset -text RESET -command "do_reset $XY $H $E $P"

    pack $BASE.midf.lookat.l $BASE.midf.lookat.here \
	$BASE.midf.lookat.center $BASE.midf.lookat.top \
	$BASE.midf.lookat.cancel $BASE.midf.lookat.reset \
	-side top -fill x -expand 1

    pack $BASE.midf.lookat -side left -expand 1
    pack $BASE.midf -side top -fill x -expand 1

    pack $BASE.bframe.cframe.pers -side left
    pack $BASE.bframe.cframe.tw -side right
    pack $BASE.bframe -side top -fill x -expand 1
    pack $BASE.bframe.cframe -side top -fill x -expand 1

#*** ACS_MODIFY 1.0 BEGIN ******************************************************
	if {$Nv_(FlyThrough)} {
		set Nv_(TWIST_SLIDER) $T
		set Nv_(EXAG_SLIDER) $E
	}
#*** ACS_MODIFY 1.0 END ******************************************************


# According to the documentation, the Main panel can never be closed
#	button $BASE.close -text Close -command "Nv_closePanel $BASE" -anchor s
#	pack $BASE.close -side right

    return $panel
}

# Procedure to reset the main panel
proc Nviz_main_reset {} {
    global Nv_

    # Simple, just invoke the reset button
    $Nv_(main_BASE).midf.lookat.reset invoke
}

# Procedure to save camera parameters
proc Nviz_main_save { file_hook } {
    global Nv_

    set BASE $Nv_(main_BASE)

    #Get canvas size
    set width [lindex [$Nv_(TOP).canvas configure -width] 4]
    set height [lindex [$Nv_(TOP).canvas configure -height] 4]


    # Need to make this accurate
    # Also need to save "look here" information
    # TODO prob. need focus indication AND realto (if focused)
    puts $file_hook "$width $height"
    puts $file_hook "[$BASE.bframe.cframe.pers.f.entry get]"
    puts $file_hook "[$BASE.midf.zexag.f.entry get]"
    puts $file_hook "[$BASE.midf.height.f.entry get]"
    puts $file_hook "[Nv_getXYPos  XY_POS]"
    puts $file_hook "[Nhas_focus]"
    puts $file_hook "[Nget_focus]"
    # if not focused, should use view_to
}

# Procedure to load camera parameters
proc Nviz_main_load { file_hook } {
    global Nv_

    # window size
    gets $file_hook data
    set win_width [lindex $data 0]
    set win_height [lindex $data 1]
    $Nv_(TOP).canvas configure -width $win_width -height $win_height
    pack $Nv_(TOP).canvas -side top -expand 0

    # perspective
    gets $file_hook data
    Nv_setEntry $Nv_(main_BASE).bframe.cframe.pers.f.entry [expr int($data)]
    Nv_scaleCallback $Nv_(main_BASE).bframe.cframe.pers e 0 null [expr int($data)]
    update

    # zexag
    gets $file_hook data
    Nv_setEntry $Nv_(main_BASE).midf.zexag.f.entry $data
    Nv_floatscaleCallback $Nv_(main_BASE).midf.zexag e 2 null $data
    update

    # height
    gets $file_hook data
    Nv_setEntry $Nv_(main_BASE).midf.height.f.entry $data
    Nv_floatscaleCallback $Nv_(main_BASE).midf.height e 2 null $data
    update

    # XY position
    gets $file_hook data
    set data [split "$data"]
    Nv_itemDrag $Nv_(main_BASE).midf.pos $Nv_(XY_POS) \
	[expr int([lindex $data 0] * 125)]  [expr int([lindex $data 1] * 125)]
    update_eye_position [lindex $data 0] [lindex $data 1]
    update

    # focus
    gets $file_hook data
    set data [split "$data"]
    if {"[lindex $data 0]" == "1"} then {
	gets $file_hook data
	set data [split "$data"]
	Nset_focus [lindex $data 0] [lindex $data 1] [lindex $data 2]
    } else {
	# insert code to set view_to here
	Nset_focus_state 0
    }
    update

}

proc do_clear {} {

# TEST    Nset_draw both
    Nset_draw front

    Nready_draw
    Nclear
    Ndone_draw
    Nset_draw back
}

# TODO - if started with view file, use these params for reset

proc do_reset {XY H E P} {
    global Nv_

    appBusy

    Nset_focus_map
    Nv_itemDrag $XY $Nv_(XY_POS) 105 105
    Nv_xyCallback Nchange_position 125 125 105 105

    set exag [Nget_first_exag]
    set val $exag
    Nv_floatscaleCallback $E b 2 Nchange_exag $val

    set list [Nget_height]
    set val [lindex $list 0]
    Nv_floatscaleCallback $H b 2 Nchange_height $val

    Nv_scaleCallback $P b 0 Nchange_persp 40

    appNotBusy
}


proc mk_exag_slider {W} {

    set exag [Nget_first_exag]
    set val $exag
    set exag [expr $val * 10]
    set min 0
#	if {$val < 1} then {
#		set min $val
#	} else {
#		set min 0
#	}

    Nv_mkFloatScale $W.zexag v zexag $exag $min $val update_exag 2

    return $W.zexag
}
proc mk_hgt_slider {W} {
    global Nv_

    set list [Nget_height]
    set val [lindex $list 0]
    set min [lindex $list 1]
    set max [lindex $list 2]

    # make sliders
    set Nv_(HEIGHT_SLIDER) $W.height
    Nv_mkFloatScale $W.height v height $max $min $val update_height 2

    return $W.height
}

proc update_exag {exag} {
    global Nv_

    if {$exag == 0.} {
	set exag [lindex [$Nv_(main_BASE).midf.zexag.scale configure -resolution] 4]
	Nv_setEntry $Nv_(main_BASE).midf.zexag.f.entry $exag
	Nv_floatscaleCallback $Nv_(main_BASE).midf.zexag e 2 null $exag
    }
    Nchange_exag $exag
#*** ACS_MODIFY 1.0 BEGIN ******************************************************
	if {$Nv_(FlyThrough) == 0} {
		# original 2 lines
	    Nv_floatscaleCallback $Nv_(HEIGHT_SLIDER) b 2 update_height \
		[$Nv_(HEIGHT_SLIDER).f.entry get]
	}
#*** ACS_MODIFY 1.0 END ********************************************************
#    Nv_floatscaleCallback $Nv_(HEIGHT_SLIDER) b 2 update_height [lindex [Nget_height] 0]
#    Nquick_draw
}

proc update_eye_position {x y} {
    global Nv_

    Nset_focus_state 1
    Nchange_position $x $y

    if {$Nv_(FollowView)} {
	set_lgt_position $x $y
	set x [expr int($x*125)]
	set y [expr int($y*125)]
	Nv_itemDrag $Nv_(LIGHT_XY) $Nv_(LIGHT_POS) $x $y
    }
}

proc update_center_position {x y} {
    global Nv_

     Nset_focus_state 1
     Nset_focus_gui $x $y

    if {$Nv_(FollowView)} {
        set_lgt_position $x $y
        set x [expr int($x*125)]
        set y [expr int($y*125)]
        Nv_itemDrag $Nv_(LIGHT_XY) $Nv_(LIGHT_POS) $x $y
    }
}


proc change_display {flag} {
global XY Nv_

       set NAME $XY
       set NAME2 [winfo parent $NAME]
       catch "destroy $XY"

# *** ACS_MODIFY 1.0 - one line

if {$Nv_(FlyThrough)} {Nset_fly_mode -1}

if {$flag == 1} {
#draw eye position
inform "Set eye position"
set XY [Nv_mkXYScale $NAME puck XY_POS 125 125 105 105 update_eye_position]

move_position

} else {
#draw center position
inform "Set center of view position"
set XY [Nv_mkXYScale $NAME cross XY_POS 125 125 109 109 update_center_position]

#*** ACS_MODIFY 1.0 BEGIN ******************************************************
	if {$Nv_(FlyThrough) == 0} {
		# original line
		pack $XY -side left -before $NAME2.height
	}
#*** ACS_MODIFY 1.0 END ******************************************************

move_position
}

#*** ACS_MODIFY 1.0 BEGIN ******************************************************
	if {$Nv_(FlyThrough)} {
		pack_XY
	} else {
		# original line
       pack $XY -side left -before $NAME2.height
	}
#*** ACS_MODIFY 1.0 END ******************************************************
}

proc update_height {h} {
    global Nv_

    Nset_focus_state 1
    Nchange_height $h

# I don't think this is correct -
# Nget_height does the exag guess  BB

    if {$Nv_(FollowView)} {
	set list [Nget_height]
	set val [lindex $list 0]
	set min [lindex $list 1]
	set max [lindex $list 2]
	set h [expr int((100.0*($h -$min))/($max - $min))]
	Nv_floatscaleCallback $Nv_(LIGHT_HGT) b 2 set_lgt_hgt $h
    }
}


proc move_position {} {
global Nv_ draw_option

#Make sure in correct view mode
Nset_focus_state 1

if {$draw_option == 0} {
#Move position puck
	set E [lindex [Nget_position] 0]
	if {$E < 0.} {set $E 0.}
	if {$E > 1.} {set $E 1.}

	set N [lindex [Nget_position] 1]
	set N [expr 1. - $N]
	if {$N < 0.} {set $N 0.}
	if {$N > 1.} {set $N 1.}

	set E [expr $E * 125.]
	set N [expr $N * 125.]

	Nv_itemDrag $Nv_(main_BASE).midf.pos $Nv_(XY_POS) $E $N
	update
}

if {$draw_option == 1} {
#Move center of view cross hair

	set E [lindex [Nget_focus_gui] 0]
	if {$E > 1.} { set E 1.}
	if {$E < 0.} {set E 0.}

	set N [lindex [Nget_focus_gui] 1]
	if {$N > 1.} {set N 1.}
	if {$N < 0.} {set N 0.}

	set E [expr ($E * 125.)]
	#reverse northing for canvas
	set N [expr 125 - ($N * 125.)]

	Nv_itemDrag $Nv_(main_BASE).midf.pos $Nv_(XY_POS) $E $N
	update
}

}

