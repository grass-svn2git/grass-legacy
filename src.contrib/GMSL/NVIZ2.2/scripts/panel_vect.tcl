##########################################################################
##########################################################################
# Default Priority for this panel
# 
# priority is from 0 to 10
#  the lower the number, the quicker it will be bumped
#  10 cannot be bumped
#  Panels will be loaded by the greater of 5 or their current priority

##########################################################################

############################################################################
# procedure to make main control area
###########################################################################


proc mkvectPanel { BASE } {
    global Nv_
    
    catch {destroy $BASE}

    set curr [Nget_current vect]
    
    if {0 != $curr}  {
	set width [Nvect$curr get_att width]
	set flat_state [Nvect$curr get_att flat]
	set height [expr [lindex [Nvect$curr get_trans] 2] * 10]
	set maplist [Nget_map_list surf]
    } else {
	set width 1
        set flat_state 0
	set height 0
	set maplist {}
    }
    
    #  Initialize panel info
    if [catch {set Nv_($BASE)}] {
	set panel [St_create {window name size priority} $BASE "Vectors" 1 5]
    } else {
	set panel $Nv_($BASE)
    }
    frame $BASE -relief groove -borderwidth 2
    Nv_mkPanelname $BASE "Vector Panel"
    
    #top frame
    set tmp [frame $BASE.top]
    label $tmp.current -text "Current:" -anchor nw

    mkMapList $tmp.list vect

    button $tmp.new -text New -anchor ne -command "add_map vect" 
    button $tmp.delete -text Delete -anchor ne -command "delete_map vect"

    pack $tmp.current -side left -expand 1
    pack $tmp.list -side left 
    pack $tmp.delete $tmp.new -side right -expand 1
    
    pack $tmp -side top -fill x -expand 1
    
    #bottom frame
    set tmp [frame $BASE.f]
    button $tmp.close -text Close -command "Nv_closePanel $BASE" -anchor s
    pack $tmp.close -side right
    button $tmp.draw_current -text {Draw Current} \
	-command {Nvect_draw_one [Nget_current vect]}
    pack $tmp.draw_current -side left
    pack $tmp -side bottom -fill x -expand 1

    #mid frame
    set tmp [frame $BASE.mid]
    set tmp1 [frame $tmp.left]
    set tmp2 [frame $tmp.mid]
    set tmp3 [frame $tmp.right]
    set tmp1a [frame $tmp1.b]

    Nv_mkArrows $tmp1.linewidth "Line Width" [concat set_width vect] $width
#	checkbutton $tmp.load -relief flat -text "Load to memory"
    button $tmp1a.color -text Color \
	-command "change_color vect $tmp1a.color"
    bind $tmp1a.color <Expose> \
	"$tmp1a.color configure -bg \[get_curr_sv_color vect\]"
    radiobutton $tmp2.label1 -text "Display on surface(s):" \
	-variable flat_state -value 0 \
        -command "check_list $tmp2.list"
    radiobutton $tmp2.label2 -text "Display Flat" \
	-variable flat_state -value 1 -command "check_list $tmp2.list"

    Nv_mkScale $tmp3.scale v "Vect. Z" 1000 0 $height set_ht 1

    pack $tmp1a.color -side left
    pack $tmp1a -side bottom -fill x -expand 1
    pack $tmp1.linewidth -anchor w \
	-padx 2 -pady 2 -side left -expand 1
    pack $tmp2.label2 $tmp2.label1 -expand 1
    pack $tmp3.scale -expand 1

# Let radiobutton state handle building list
# of available surfaces
    if {$flat_state == 0} {
	$tmp2.label1 select
	check_list $tmp2.list
    } else {
	$tmp2.label2 select
	check_list $tmp2.list
    }

    pack $tmp1 -side left -fill both -expand 1
    pack $tmp2 -side left -fill both -expand 1
    pack $tmp3 -side right -fill both -expand 1
    pack $tmp -side top  -fill both -expand 1

    return $panel
}

# Reset procedure for this panel
proc Nviz_vect_reset {} {
    set vect_list [Nget_vect_list]

    foreach i $vect_list {
	Nvect$i delete
    }

    set_new_curr vect 0
}

# Save procedure for saving state of Nviz vect files
proc Nviz_vect_save {file_hook} {
    # For each vector file we write out all of its attribute information. 
    # Vectors are referenced by logical name so that they are reloadable
    # (otherwise, they may be assigned different id's each time they are loaded
    # and scripts won't work correctly).

    # Get the list of vect files
    set vect_list [Nget_vect_list]

    # Get the list of surfaces for checking draping
    set surf_list [Nget_surf_list]

    # Write out the total number of vector files
    puts $file_hook "[llength $vect_list]"

    # For each vector file write out the following:
    # 1. Logical name
    # 2. map name
    # 3. color
    # 4. width
    # 5. list of logical names of surfaces displayed on
    foreach i $vect_list {

	# logical name
	puts $file_hook "[Nvect$i get_logical_name]"
	
	# map name
	puts $file_hook "[Nvect$i get_att map]"

	# color
	puts $file_hook "[Nvect$i get_att color]"

	# width
	puts $file_hook "[Nvect$i get_att width]"

	# logical names of surfaces displayed on
	set draped [list]
	foreach j $surf_list {
	    if {[Nvect$i surf_is_selected Nsurf$j]} then {
		lappend draped $j
	    }
	}
	puts $file_hook "[llength $draped]"
	foreach j $draped {
	    puts $file_hook "[Nlogical_from_literal Nsurf$j]"
	}

	flush $file_hook
    }

    # Done...
}

# Load procedure for loading state of Nviz vect files
proc Nviz_vect_load { file_hook } {
    # Read the number of surfaces saved in this state file
    gets $file_hook num_vects

    # For each vect file, create a new surface with the given logical
    # name and fill in the attributes as appropriate
    for {set i 0} {$i < $num_vects} {incr i} {
	# Read in the logical name for this new vect map
	gets $file_hook logical_name

	# Now create a new vect map with the given logical name
	set new_vect [Nnew_map_obj vect "name=$logical_name"]

	# Set all attributes as appropriate (i.e. as they are read from the state file)
	
	# map
	gets $file_hook att_data
	$new_vect set_att map $att_data

	# color 
	gets $file_hook att_data
	$new_vect set_att color $att_data

	# width
	gets $file_hook att_data
	$new_vect set_att width $att_data

	# Select all the appropriate surfaces to put this map on
	gets $file_hook num_selected_surfs
	for {set j 0} {$j < $num_selected_surfs} {incr j} {
	    gets $file_hook selected_surf

	    # Select this surf by translating from a logical name and selecting
	    $new_vect select_surf [Nliteral_from_logical $selected_surf]
	}

	Nset_current vect [string range $new_vect 5 end]
    }

}


proc change_color { type me } {
    set curr [Nget_current $type]
    switch $type {
	"vect" { set head Nvect }
	"site" { set head Nsite }
    }

    if {0 != $curr} {
	set clr [$head$curr get_att color]
	set clr [mkColorPopup .colorpop Color $clr 1]
	$head$curr set_att color $clr
    }

    $me configure -bg [get_curr_sv_color $type]
}

proc get_curr_sv_color { type } {
    set curr [Nget_current $type]
    switch $type {
	"vect" { set head Nvect }
	"site" { set head Nsite }
    }

    if {0 == $curr} then {
	return "gray90"
    }

    set color [$head$curr get_att color]

    set color [expr int([tcl_to_rgb $color])]
    set blue  [hexval [expr int($color & 0x0000ff)]]
    set green [hexval [expr int(($color & 0x00ff00)>>8)]]
    set red   [hexval [expr int(($color & 0xff0000)>>16)]]
    return "#$red$green$blue"

}

proc delete_map {type} {
    set curr [Nget_current $type]
    switch $type {
	"vect" { set head Nvect }
	"site" { set head Nsite }
    }
    
    if {0 != $curr} {
	$head$curr delete
	set name 0
	
	switch $type {
	    "vect" { set new_list [Nget_vect_list] }
	    "site" { set new_list [Nget_site_list] }
	}
	
	if {[llength $new_list] != 0} then {
	    set name [lindex $new_list 0]
	} else {
	    set name 0
	}
	
	set_new_curr $type $name
    }
    
}

# Use this routine when adding a vect or site in a script
proc script_add_map { type map_name } {

    set temp [Nnew_map_obj $type]
    $temp set_att map $map_name
    set_new_curr $type [string range $temp 5 end]
    
    return [string range $temp 5 end]
}

proc add_map {type} {
    set new [create_map_browser .fbrowse $type 1]

    # Let user know that we are busy
    appBusy

    if {$new != "-1"} {
	set temp [Nnew_map_obj $type]
	$temp set_att map $new
	set_new_curr $type [string range $temp 5 end]
    }

    # Let user know that he may proceed
    appNotBusy
}

proc set_width {type E} {
    set i [$E get]
    set curr [Nget_current $type]
    switch $type {
	"vect" { set head Nvect }
	"site" { set head Nsite }
    }
    
    if {0 != $curr} {
	$head$curr set_att width $i
    }
}

# Procedure to set vector elevation eith above surface
# or level height
proc set_ht {h} {
    global Nv_ vect_height

    set vect_height $h
    set curr [Nget_current vect]
    if {0 != $curr}  {
    Nvect$curr set_trans 0 0 $vect_height
    }
      
}

#Procedure to update vect atts from radiobutton
proc check_list {address} {
    global Nv_ flat_state curr vect_height

    set state [winfo exists $address]
    set curr [Nget_current vect]
    if {0 != $curr}  {
	set maplist [Nget_map_list surf]

        if {$state == 0 && $flat_state == 0 } {
        #display on surface
	catch {destroy $address}
        Nvect$curr set_att flat 0
	Nv_mkSurfacelist $address $maplist Nvect$curr vect
	pack $address
	} 
        if { $state == 1 && $flat_state == 1} {
        #display on flat
	catch {destroy $address}
	Nvect$curr set_att flat 1
	}
     }
	
}
