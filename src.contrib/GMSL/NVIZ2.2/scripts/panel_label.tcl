#!../glnviz.new/nvwish -f
# 4/4/95
# M. Astley
# USACERL, blah blah blah

##########################################################################
# 
# Panel to facilitate label placement for finishing images produced
# by nviz.
#
##########################################################################

# Changes
#

# Panel specific globals
global Nv_

# Font Type: Times, Helvetica, Courier
set Nv_(labelFontType) times

# Font Weight: Italic, Bold
set Nv_(labelFontWeight1) 0
set Nv_(labelFontWeight2) 0

# Font Point Size: varies
set Nv_(labelFontSize) 12
set Nv_(labelFontColor) #FF0000

# Legend section
set Nv_(catval) 1
set Nv_(catlabel) 0
set Nv_(leg_invert) 0
set Nv_(leg_userange) 0
set Nv_(leg_discat) 0
set Nv_(leg_uselist) 0
set Nv_(cat_list) [list]
set Nv_(cat_list_select) 0

# Label Sites section
set Nv_(labvalues) 1
set Nv_(lablabels) 1
set Nv_(labinbox) 1

global clr

##########################################################################

proc mklabelPanel { BASE } {
    global Nv_ 

    set panel [St_create {window name size priority} $BASE "Label" 2 5]
    frame $BASE -relief groove -borderwidth 2
    Nv_mkPanelname $BASE "Label Panel"
    
    ##########################################################################
    # This section contains widgets for setting font type, size and color
    frame $BASE.text_char -relief groove -bd 5
    set rbase $BASE.text_char
    
    frame $rbase.font_size -relief raised
    label $rbase.font_size.label -text "Font Size:"
    entry $rbase.font_size.entry -relief sunken -width 3 \
	-textvariable Nv_(labelFontSize)
    button $rbase.font_size.color -text "Color" \
	-bg #FF0000 -width 8 \
	-command "change_label_color $rbase.font_size.color"
    pack $rbase.font_size.label $rbase.font_size.entry \
	$rbase.font_size.color -side left \
	-padx 2 -anchor n -expand no 

    frame $rbase.font_type -relief raised
    radiobutton $rbase.font_type.times -text "Times-Roman" \
	-value times -variable Nv_(labelFontType) -anchor w \
	-width 15
    radiobutton $rbase.font_type.helv  -text "Helvetica" \
	-value helvetica -variable Nv_(labelFontType) -anchor w \
	-width 15
    radiobutton $rbase.font_type.cour  -text "Courier" \
	-value courier -variable Nv_(labelFontType) -anchor w \
	-width 15
    pack $rbase.font_type.times $rbase.font_type.helv \
	$rbase.font_type.cour -side top \
	-expand yes

    frame $rbase.font_weight -relief raised
    checkbutton $rbase.font_weight.italic -text "Italic" \
	-variable Nv_(labelFontWeight1) -anchor w
    checkbutton $rbase.font_weight.bold   -text "Bold" \
	-variable Nv_(labelFontWeight2) -anchor w
    pack $rbase.font_weight.italic $rbase.font_weight.bold \
	-side top -expand yes -anchor w

    pack $rbase.font_type $rbase.font_weight -side left \
	-fill x -expand yes -padx 2 -pady 2 -anchor n
    pack $rbase.font_size -side top -expand yes \
	-before $rbase.font_type -anchor n -padx 2 -pady 2

    ##########################################################################
    # This section contains widgets for specifying the label text and a button
    # which actually places the label
    frame $BASE.text_place -relief groove -bd 5
    set rbase $BASE.text_place

    frame $rbase.buttons -relief raised
    button $rbase.buttons.place -text "Place Label" -command "place_label"
    button $rbase.buttons.undo -text "Undo" -command ""
    pack $rbase.buttons.place $rbase.buttons.undo -side left \
	-padx 2 -expand no

    entry $rbase.text -relief sunken -width 30 -textvariable Nv_(label_text)
    label $rbase.label -text "Label Text"
    pack $rbase.buttons -side top -expand yes\
	-padx 2 -pady 2 -anchor n
    pack $rbase.text $rbase.label -side top -expand no \
	-padx 2 -pady 2 -anchor n


    ##########################################################################
    # Separator
    Nv_makeSeparator $BASE.sep1

    ##########################################################################
    # This section contains widgets for specifying a legend
    frame $BASE.legends -relief groove -bd 5
    set rbase $BASE.legends

    # Legend button, invert checkbutton and category checkbuttons
    frame $rbase.leg_inv
   button $rbase.leg_inv.legend -text "Legend" -command "place_legend"

    checkbutton $rbase.leg_inv.invert -text "Invert" -anchor w \
	-variable Nv_(leg_invert) -onvalue 1 -offvalue 0
    pack $rbase.leg_inv.legend $rbase.leg_inv.invert \
	-fill x -side top -expand no -padx 2 -pady 1

    frame $rbase.cats
    checkbutton $rbase.cats.values -text "Category Values" \
	-anchor w -variable Nv_(catval) -onvalue 1 -offvalue 0
    checkbutton $rbase.cats.labels -text "Category Labels" \
	-anchor w -variable Nv_(catlabel) -onvalue 1 -offvalue 0
    pack $rbase.cats.values $rbase.cats.labels -side top \
	-expand no -padx 2 -pady 1

    # Use-range portion of panel
    frame $rbase.ranges -relief sunken
    checkbutton $rbase.ranges.useit -text "Use Range" -anchor w \
	-variable Nv_(leg_userange) -onvalue 1 -offvalue 0

    frame $rbase.ranges.bound_low
    entry $rbase.ranges.bound_low.entry -relief sunken -width 8 \
        -textvariable Nv_(leg_lorange)
    label $rbase.ranges.bound_low.label -text "Low:" \
	-width 5 -anchor e
    pack $rbase.ranges.bound_low.entry \
	$rbase.ranges.bound_low.label -side right \
	-padx 2 -pady 1 -fill x -expand no

    frame $rbase.ranges.bound_hi
    entry $rbase.ranges.bound_hi.entry  -relief sunken -width 8 \
         -textvariable Nv_(leg_hirange)
    label $rbase.ranges.bound_hi.label -text "Hi:" \
	-width 5 -anchor e
    pack $rbase.ranges.bound_hi.entry \
	$rbase.ranges.bound_hi.label -side right \
	-padx 2 -pady 1 -fill x -expand no

    # Return bindings for "use range" entries
    bind $rbase.ranges.bound_low.entry <Return> "$rbase.ranges.useit select"
    bind $rbase.ranges.bound_hi.entry <Return> "$rbase.ranges.useit select"

    pack $rbase.ranges.bound_low $rbase.ranges.bound_hi \
	-side top -padx 2 -pady 1 -expand no
    pack $rbase.ranges.useit -side left -anchor n \
	-padx 2 -pady 2 -expand no \
	-before $rbase.ranges.bound_low

    # Discrete categories and use-list portion
    checkbutton $rbase.disc_cat -text "Discrete Categories" \
	-anchor w -width 18 -variable Nv_(leg_discat) \
	-onvalue 1 -offvalue 0
    
    # Some special handling for the "Use List" entry
    frame $rbase.use_list
    checkbutton $rbase.use_list.cb -text "Use List" \
	-anchor w -width 18 -variable Nv_(leg_uselist) \
	-onvalue 1 -offvalue 0 -command "make_cat_list $rbase.use_list.curr.m" \
        -state disabled
    menubutton $rbase.use_list.curr -text "Current List" \
	-menu $rbase.use_list.curr.m -relief raised \
        -state disabled
    menu $rbase.use_list.curr.m -disabledforeground black
    pack $rbase.use_list.cb $rbase.use_list.curr -side left \
	-padx 2 -expand no
    $rbase.use_list.curr.m add command -label "None" -state disabled

    # Pack all portions
    pack $rbase.leg_inv $rbase.cats -side left -fill x -expand yes
    pack $rbase.use_list \
	$rbase.disc_cat \
	$rbase.ranges \
	-side bottom -expand no -before $rbase.leg_inv \
	-padx 2 -pady 1

    ##########################################################################
    # Separator
    Nv_makeSeparator $BASE.sep2

    ##########################################################################
    # This section contains widgets for specifying label sites
    frame $BASE.lab_sites -relief groove -bd 5
    set rbase $BASE.lab_sites

    frame $rbase.left
    button $rbase.left.lab_sites -text "  Label Sites  "
    checkbutton $rbase.left.in_box -text "In Box" -anchor w \
	-variable Nv_(labinbox) -onvalue 1 -offvalue 0
    pack $rbase.left.lab_sites $rbase.left.in_box \
	-side top -fill x -expand no -padx 2 -pady 1

    frame $rbase.right
    checkbutton $rbase.right.values -text "Values" -anchor w \
	-variable Nv_(labvalues) -onvalue 1 -offvalue 0
    checkbutton $rbase.right.labels -text "Labels" -anchor w \
	-variable Nv_(lablabels) -onvalue 1 -offvalue 0
    pack $rbase.right.values $rbase.right.labels \
	-side top -fill x -expand no -padx 2 -pady 1

    pack $rbase.left $rbase.right -side left -expand yes \
	-padx 2 -pady 1

    ##########################################################################
    # Pack all frames
    pack $BASE.text_char \
	$BASE.text_place \
	$BASE.sep1 \
	$BASE.legends \
	$BASE.sep2 \
	$BASE.lab_sites \
	-padx 2 -pady 2 -fill x -expand no

    return $panel
}

# Simple routine to change the color of fonts
proc change_label_color { me } {
global Nv_

    set clr [lindex [$me configure -bg] 4]
    set clr [mkColorPopup .colorpop LabelColor $clr 1]
    set Nv_(labelFontColor) $clr
    $me configure -bg $clr
}

# Routine to popup a list selector for selecting a discrete list of values
proc make_cat_list {MENU} {
    global Nv_

    # Check to see if we are turning this check button on
    if {$Nv_(leg_uselist) == 0} return

    # Reinitalize list values
    set Nv_(cat_list) [list]
    set Nv_(cat_list_select) 0
    $MENU delete 0 last

    # Create the "individual" subpanel
    set BASE ".cat_list"
    set pname $BASE
    toplevel $pname -relief raised -bd 3
    list_type1 $pname.list 3c 3c
    $pname.list.t configure -text "Category Values"
    entry $pname.level -relief sunken -width 10
    bind $pname.level <Return> "make_cat_list_add $BASE"
    button $pname.addb -text "Add"    -command "make_cat_list_add $BASE"
    button $pname.delb -text "Delete" -command "make_cat_list_delete $BASE"
    button $pname.done -text "Done"   -command "set Nv_(cat_list_select) 1"
    pack $pname.list $pname.level $pname.addb $pname.delb $pname.done\
	-fill x -padx 2 -pady 2

    tkwait variable Nv_(cat_list_select)
    for {set i 0} {$i < [$pname.list.l size]} {incr i} {
	set temp [$pname.list.l get $i]
	lappend Nv_(cat_list) $temp
	$MENU add command -label "$temp" -state disabled
    }

    if {[llength $Nv_(cat_list)]==0} {
	$MENU add command -label None -state disabled
    }

    destroy $BASE
}

# Two quick routines to add or delete isosurface levels for
# selecting them individually
proc make_cat_list_add { BASE } {
    # For this routine we just use the value stored in the
    # entry widget
    # Get the value from the entry widget
    set level [$BASE.level get]

    # Now just append it to the list
    $BASE.list.l insert end $level
}

proc make_cat_list_delete { BASE } {
    # For this procedure we require that the user has selected
    # a range of values in the list which we delete
    # Get the range of selections
    set range [$BASE.list.l curselection]
    
    # Now delete the entries
    foreach i $range {
	$BASE.list.l delete $i
    }
}

# Routine to do_legend
proc do_legend {W x y flag } {
    global Nv_
    global x1 y1 x2 y2

if {$flag == 1} {
#pick first corner
set y [expr $Nv_(height) - $y]

#set first corner of box
      set x1 $x
      set y1 $y

  } else {
set y [expr $Nv_(height) - $y]
#set last corner of box and reset binding
      #Get name of current map 
      set name [Nget_current surf]
      if { [lindex [Nsurf$name get_att color] 0] == "const"} {
	puts "Colortable constant -- no legend available"
#reset everything
        bind $W <Button-1> {}
        bind $W <Button-3> {}
        unset x1
        unset y1
        update
	return
      } 

      set name [lindex [Nsurf$name get_att color] 1]

      set range_low -9999
      set range_high -9999
      if {$Nv_(leg_userange)} {
         set range_low $Nv_(leg_lorange)
         set range_high $Nv_(leg_hirange)
          if { $range_low == ""} {set range_low -9999}
          if { $range_high == ""} {set range_high -9999}
      }
#make sure corner 1 is picked
      if {[info exists x1]} {

      if {$x1 > $x} {
	set x2 $x1
	set x1 $x
      } else {
	set x2 $x
      }
      if {$y1 > $y} {
        set y2 $y1
        set y1 $y
      } else {
        set y2 $y
      }
#get font description
	if {$Nv_(labelFontWeight2) == 1} {
		set weight "bold"
	} else {
		set weight "medium"
	}
	if {$Nv_(labelFontWeight1) == 1} {
		set slant "i"
	} else {
		set slant "r"
	}	
	set font "*-$Nv_(labelFontType)-$weight-$slant-normal--$Nv_(labelFontSize)-*-*-*-*-*-*-*"

#Ndraw_legend Args -- filename use_vals use_labels invert use_range 
# low_range high_range discrete colors corner_coords

      Ndraw_legend $name $font $Nv_(catval) $Nv_(catlabel) $Nv_(leg_invert) $Nv_(leg_discat) \
	$Nv_(leg_userange) $range_low $range_high $x1 $x2 $y1 $y2

#reset bindings
      bind $W <Button-1> {}
      bind $W <Button-3> {}
      unset x1
      unset x2
      unset y1
      unset y2
      update
      }
}

}


# Routine to place legend
proc place_legend { } {
    global Nv_
    global x1 y1 x2 y2

#do bindings
bind $Nv_(TOP).canvas <Button-1> {do_legend %W %x %y 1 }
bind $Nv_(TOP).canvas <Button-3> {do_legend %W %x %y 2 }
puts "Select Legend Corners in Window ..."
puts "Corner 1 = left button      Corner 2 = right button"
update

##Tried binding to draw rectangle outline but unsupported with togl ??
#bind $Nv_(TOP).canvas <Motion> {do_legend %W %x %y 3}
}


# Routines to allow user to place a label
proc place_label { } {
    global Nv_
    # We bind the canvas area so that the user can click to place the
    # label.  After the click is processed we unbind the canvas area

bind $Nv_(TOP).canvas <Button-1> {place_label_cb %x %y }

}


proc place_label_cb { sx sy } {
    global Nv_ 

set sy [expr $Nv_(height) - $sy]

#get font description
	if {$Nv_(labelFontWeight2) == 1} {
		set weight "bold"
	} else {
		set weight "medium"
	}
	if {$Nv_(labelFontWeight1) == 1} {
		set slant "i"
	} else {
		set slant "r"
	}	
	set font "*-$Nv_(labelFontType)-$weight-$slant-normal--$Nv_(labelFontSize)-*-*-*-*-*-*-*"

set clr $Nv_(labelFontColor)
puts "BOB -- $clr"

	Nplace_label $Nv_(label_text) $font $clr $sx $sy



#remove binding
bind $Nv_(TOP).canvas <Button-1> {}
}





