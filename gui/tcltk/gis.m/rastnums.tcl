##########################################################################
# rastnums.tcl - cell value display layer options file for GRASS GIS Manager
# March 2006 Michael Barton, Arizona State University
# COPYRIGHT:	(C) 1999 - 2006 by the GRASS Development Team
#
#		This program is free software under the GNU General Public
#		License (>=v2). Read the file COPYING that comes with GRASS
#		for details.
#
##########################################################################

namespace eval GmRnums {
    variable array opt # rnums current options
    variable count 1
    variable array tree # mon
    variable array lfile # raster
    variable array lfilemask # raster
    variable optlist
    variable array dup # vector
}

###############################################################################
proc GmRnums::create { tree parent } {
    variable optlist
    variable lfile
    variable lfilemask
    variable opt
    variable count
    variable dup
    global gmpath
    global iconpath
    global mon

    set node "rnums:$count"

    set frm [ frame .rnumsicon$count]
    set check [checkbutton $frm.check \
		-variable GmRnums::opt($count,1,_check) \
		-height 1 -padx 0 -width 0]

    image create photo nico -file "$iconpath/module-d.rast.num.gif"
    set ico [label $frm.ico -image nico -bd 1 -relief raised]
    
    pack $check $ico -side left
        
	#insert new layer
	if {[$tree selection get] != "" } {
		set sellayer [$tree index [$tree selection get]]
    } else { 
    	set sellayer "end" 
    }

    $tree insert $sellayer $parent $node \
	-text  "cell values $count"\
	-window    $frm \
	-drawcross auto  
    
    set opt($count,1,_check) 1 
    set dup($count) 0

    set opt($count,1,map) "" 
	set opt($count,1,opacity) 1.0
    set opt($count,1,grid_color) "grey" 
    set opt($count,1,text_color) "black" 
    set opt($count,1,cellcolor) 0 
    set opt($count,1,mod) 1

	set optlist {_check map grid_color text_color cellcolor}

    foreach key $optlist {
		set opt($count,0,$key) $opt($count,1,$key)
    } 
    
	# create files in tmp diretory for layer output
	set mappid [pid]
	set lfile($count) [eval exec "g.tempfile pid=$mappid"]
	set lfilemask($count) $lfile($count)
	append lfile($count) ".ppm"
	append lfilemask($count) ".pgm"
    
    incr count
    return $node
}

###############################################################################
proc GmRnums::set_option { node key value } {
    variable opt
 
    set id [GmTree::node_id $node]
    set opt($id,1,$key) $value
}

proc GmRnums::select_map { id } {
    variable tree
    variable node
    global mon
    
    set m [GSelect cell]
    if { $m != "" } { 
        set GmRnums::opt($id,1,map) $m
        GmTree::autonamel "cell values for $m"
    }
}

###############################################################################
# Options for displaying cats in raster cells
proc GmRnums::options { id frm } {
    variable opt
    global gmpath
    global bgcolor
    global iconpath

    # Panel heading
    set row [ frame $frm.heading1 ]
    Label $row.a -text "Display cell values from raster map or image" \
    	-fg MediumBlue
    pack $row.a -side left
    pack $row -side top -fill both -expand yes

    set row [ frame $frm.heading2 ]
    Label $row.a -text "  (resolution must be 100x100 or less)"
    pack $row.a -side left
    pack $row -side top -fill both -expand yes

	#opacity
	set row [ frame $frm.opc]
	Label $row.a -text [G_msg "Opaque "]
	scale $row.b -from 1.0 -to 0.0 -showvalue 1  \
		-orient horizontal -length 300 -resolution 0.01 -fg "#656565"\
		-variable GmRnums::opt($id,1,opacity) 
	Label $row.c -text [G_msg " Transparent"]
    pack $row.a $row.b $row.c -side left
    pack $row -side top -fill both -expand yes	
	
    # raster name for histogram
    set row [ frame $frm.name ]
    Label $row.a -text "Raster to display: "
    Button $row.b -image [image create photo -file "$iconpath/element-cell.gif"] \
        -highlightthickness 0 -takefocus 0 -relief raised -borderwidth 1  \
		-command "GmRnums::select_map $id"
    Entry $row.c -width 35 -text " $opt($id,1,map)" \
          -textvariable GmRnums::opt($id,1,map)
    Label $row.d -text "   "
    Button $row.e -text [G_msg "Help"] \
		-image [image create photo -file "$iconpath/gui-help.gif"] \
		-command "run g.manual d.rast.num" \
		-background $bgcolor -helptext [G_msg "Help"]
    pack $row.a $row.b $row.c $row.d $row.e -side left
    pack $row -side top -fill both -expand yes

    # grid color
    set row [ frame $frm.grid ]
    Label $row.a -text "Color for cell grid:       "
    ComboBox $row.b -padx 2 -width 10 -textvariable GmRnums::opt($id,1,grid_color) \
		-values {"white" "grey" "gray" "black" "brown" "red" "orange" \
		"yellow" "green" "aqua" "cyan" "indigo" "blue" "purple" "violet" \
		"magenta"}
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes

    # text color
    set row [ frame $frm.text ]
    Label $row.a -text "Color for cell values:   "
    ComboBox $row.b -padx 2 -width 10 -textvariable GmRnums::opt($id,1,text_color) \
		-values {"white" "grey" "gray" "black" "brown" "red" "orange" \
		"yellow" "green" "aqua" "cyan" "indigo" "blue" "purple" "violet" \
		"magenta"}
    checkbutton $row.c -text [G_msg "use raster colors for cell values"] \
        -variable GmRnums::opt($id,1,cellcolor) 
    pack $row.a $row.b $row.c -side left
    pack $row -side top -fill both -expand yes
}

###############################################################################
proc GmRnums::save { tree depth node } {
    variable opt
    variable optlist
    global mon
    
    set id [GmTree::node_id $node]

    foreach key $optlist {
         GmTree::rc_write $depth "$key $opt($id,1,$key)"
    } 
}

###############################################################################
proc GmRnums::display { node mod } {
    global mapfile
    global maskfile
    global complist
    global opclist
    global masklist
    global gmpath
    global mon
	global mapdispwd
	global mapdispht
	global env
    variable optlist
    variable lfile 
    variable lfilemask
    variable opt
    variable rasttype
    variable tree
    variable dup
    variable count

    set currmon ""
    global gmpath
    global mon

    set tree($mon) $GmTree::tree($mon)
    set id [GmTree::node_id $node]

    if { $opt($id,1,map) == "" } { return } 

    set cmd "d.rast.num map=$opt($id,1,map) grid_color=$opt($id,1,grid_color) \
    	text_color=$opt($id,1,text_color)"

    # include nulls
    if { $opt($id,1,cellcolor) } { 
        append cmd " -f"
    }

    
	# only run if less than 100x100 cells
	set string ""
	set cells 0
	set rest ""
	set rc [open "|g.region -gu" r]
	set rowscolumns [read $rc]
	close $rc
	regexp {rows=(\d*)} $rowscolumns string rows
	regexp {cols=(\d*)} $rowscolumns string cols
	set cells [expr $rows * $cols]

	# can only display if 10K cells or less in region
	if { $cells <= 10000} {
		# Decide whether to run, run command, and copy files to temp
		GmCommonLayer::display_command [namespace current] $id $cmd
		set mapdispht $env(GRASS_HEIGHT)
		set mapdispwd $env(GRASS_WIDTH)
	} elseif {$opt($id,1,_check)} {
		set msgtxt "Cell values can only be displayed\nfor regions of < 10,000 cells" 
		set answer [tk_messageBox -message $msgtxt -type ok -parent .mapcan($mon)]
		if { $answer == "ok" } {return}
    }

}
    
###############################################################################
proc GmRnums::mapname { node } {
    variable opt
    variable tree
    global mon
    global mapname
    
    set tree($mon) $GmTree::tree($mon)
    set id [GmTree::node_id $node]

    if { ! ( $opt($id,1,_check) ) } { return } 

    if { $opt($id,1,map) == "" } { return } 
    
    set mapname $opt($id,1,map)
    return $mapname
}

###############################################################################
proc GmRnums::duplicate { tree parent node id } {
    variable optlist
    variable lfile
    variable lfilemask
    variable opt
    variable count
    variable dup
    global gmpath
    global iconpath
    global mon

    set node "rnums:$count"
	set dup($count) 1

    set frm [ frame .rnumsicon$count]
    set check [checkbutton $frm.check \
		-variable GmRnums::opt($count,1,_check) \
		-height 1 -padx 0 -width 0]

    image create photo nico -file "$iconpath/module-d.rast.num.gif"
    set ico [label $frm.ico -image nico -bd 1 -relief raised]
    
    pack $check $ico -side left

	# where to insert new layer
	if {[$tree selection get] != "" } {
		set sellayer [$tree index [$tree selection get]]
    } else { 
    	set sellayer "end" 
    }

	if { $opt($id,1,map) == ""} {
	    $tree insert $sellayer $parent $node \
		-text      "cell values $count" \
		-window    $frm \
		-drawcross auto
	} else {
	    $tree insert $sellayer $parent $node \
		-text      "cell values for $opt($id,1,map)" \
		-window    $frm \
		-drawcross auto
	}

	set opt($count,1,opacity) $opt($id,1,opacity)

	set optlist {_check map grid_color text_color cellcolor}

    foreach key $optlist {
    	set opt($count,1,$key) $opt($id,1,$key)
		set opt($count,0,$key) $opt($count,1,$key)
    } 
    
	set id $count

	# create files in tmp directory for layer output
	set mappid [pid]
	set lfile($count) [eval exec "g.tempfile pid=$mappid"]
	set lfilemask($count) $lfile($count)
	append lfile($count) ".ppm"
	append lfilemask($count) ".pgm"

    incr count
    return $node
}
