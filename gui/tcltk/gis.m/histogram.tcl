##########################################################################
# histogram.tcl - raster histogram display layer options file for GRASS GIS Manager
# COPYRIGHT:	(C) 1999 - 2006 by the GRASS Development Team
#
#		This program is free software under the GNU General Public
#		License (>=v2). Read the file COPYING that comes with GRASS
#		for details.
#
##########################################################################

namespace eval GmHist {
    variable array opt # hist current options
    variable count 1
    variable array tree # mon
    variable array lfile # histogram
    variable array lfilemask # histogram
    variable optlist
    variable first
    variable array dup # layer
}

proc GmHist::create { tree parent } {
    variable opt
    variable count
    variable lfile
    variable lfilemask
    variable optlist
    variable first
	variable dup
    global mon
    global iconpath

    set node "histogram:$count"

    set frm [ frame .histicon$count]
    set check [checkbutton $frm.check \
		-variable GmHist::opt($count,1,_check) \
		-height 1 -padx 0 -width 0]

    image create photo hico -file "$iconpath/module-d.histogram.gif"
    set ico [label $frm.ico -image hico -bd 1 -relief raised]
    
    pack $check $ico -side left
        
	#insert new layer
	if {[$tree selection get] != "" } {
		set sellayer [$tree index [$tree selection get]]
    } else { 
    	set sellayer "end" 
    }

    $tree insert $sellayer $parent $node \
	-text  "histogram $count"\
	-window    $frm \
	-drawcross auto  
    
    set opt($count,1,_check) 1 
    set dup($count) 0
    
    set opt($count,1,map) "" 
	set opt($count,1,opacity) 1.0
    set opt($count,1,color) "black" 
    set opt($count,1,style) "bar" 
    set opt($count,1,nsteps) 255 
    set opt($count,1,nulls) 0 
    
    set opt($count,1,mod) 1
    set first 1

	set optlist {_check map color style nsteps nulls}

    foreach key $optlist {
		set opt($count,0,$key) $opt($count,1,$key)
    } 
    
	# create files in tmp diretory for layer output
	set mappid [pid]
	set lfile($count) [exec g.tempfile pid=$mappid]
	set lfilemask($count) $lfile($count)
	append lfile($count) ".ppm"
	append lfilemask($count) ".pgm"

    incr count
    return $node
}

proc GmHist::set_option { node key value } {
    variable opt
 
    set id [GmTree::node_id $node]
    set opt($id,1,$key) $value
}

proc GmHist::select_map { id } {
    variable tree
    variable node
    global mon
    
    set m [GSelect cell]
    if { $m != "" } { 
        set GmHist::opt($id,1,map) $m
        GmTree::autonamel "histogram of $m"
    }
}

# display histogram options
proc GmHist::options { id frm } {
    variable opt
    global bgcolor
    global iconpath

    # Panel heading
    set row [ frame $frm.heading ]
    Label $row.a -text "Draw histogram of values from raster map or image" \
    	-fg MediumBlue
    pack $row.a -side left
    pack $row -side top -fill both -expand yes

	#opacity
	set row [ frame $frm.opc]
	Label $row.a -text [G_msg "Opaque "]
	scale $row.b -from 1.0 -to 0.0 -showvalue 1  \
		-orient horizontal -length 300 -resolution 0.01 -fg "#656565"\
		-variable GmHist::opt($id,1,opacity) 
	Label $row.c -text [G_msg " Transparent"]
    pack $row.a $row.b $row.c -side left
    pack $row -side top -fill both -expand yes	
	
    # raster name for histogram
    set row [ frame $frm.name ]
    Label $row.a -text "Raster to histogram: "
    Button $row.b -image [image create photo -file "$iconpath/element-cell.gif"] \
        -highlightthickness 0 -takefocus 0 -relief raised -borderwidth 1  \
		-command "GmHist::select_map $id"
    Entry $row.c -width 35 -text " $opt($id,1,map)" \
          -textvariable GmHist::opt($id,1,map)
    Label $row.d -text "   "
    Button $row.e -text [G_msg "Help"] \
		-image [image create photo -file "$iconpath/gui-help.gif"] \
		-command "run g.manual d.histogram" \
		-background $bgcolor -helptext [G_msg "Help"]
    pack $row.a $row.b $row.c $row.d $row.e -side left
    pack $row -side top -fill both -expand yes

    # graph style and color
    set row [ frame $frm.style ]
    Label $row.a -text "Graph style"
    ComboBox $row.b -padx 2 -width 4 -textvariable GmHist::opt($id,1,style) \
		-values {"bar" "pie"} 
    # histogram color
    Label $row.c -text "Histogram frame and text color"
    ComboBox $row.d -padx 2 -width 10 -textvariable GmHist::opt($id,1,color) \
		-values {"white" "grey" "gray" "black" "brown" "red" "orange" \
		"yellow" "green" "aqua" "cyan" "indigo" "blue" "purple" "violet" \
		"magenta"} 
    pack $row.a $row.b $row.c $row.d -side left
    pack $row -side top -fill both -expand yes
    
    # steps for fp maps and nulls
    set row [ frame $frm.steps ]
    Label $row.a -text "Steps/bins for values (fp maps only)" 
    SpinBox $row.b -range {2 255 1} -textvariable GmHist::opt($id,1,nsteps) \
		-width 4 -helptext "steps/bins"  
    Label $row.c -text "   "
    checkbutton $row.d -text [G_msg "include null values"] \
        -variable GmHist::opt($id,1,nulls) 
    pack $row.a $row.b $row.c $row.d -side left
    pack $row -side top -fill both -expand yes
}

proc GmHist::save { tree depth node } {
    variable opt
    variable optlist
    global mon
    
    set id [GmTree::node_id $node]

    foreach key $optlist {
         GmTree::rc_write $depth "$key $opt($id,1,$key)"
    } 
}

proc GmHist::display { node mod } {
    global mon
    variable optlist
    variable lfile 
    variable lfilemask
    variable opt
    variable tree
    variable dup
    variable count
    variable first

    set rasttype ""
    set tree($mon) $GmTree::tree($mon)
    set id [GmTree::node_id $node]

    # If we are told dirty (for zoom) force dirty
    # Don't remove a dirty from a previous unrendered zoom
    if {$mod} {set opt($id,1,mod) 1}

    if { $opt($id,1,map) == "" } { return } 

    set cmd "d.histogram -q map=$opt($id,1,map) style=$opt($id,1,style) \
    	color=$opt($id,1,color)"

    # include nulls
    if { $opt($id,1,nulls) } { 
        append cmd " -n"
    }

    # set steps
    if { $opt($id,1,nsteps) != "" } { 
		set rt [open "|r.info map=$opt($id,1,map) -t" r]
		set rasttype [read $rt]
		close $rt
		if {[regexp -nocase ".=FCELL" $rasttype] || [regexp -nocase ".=DCELL" $rasttype]} {
            append cmd " nsteps=$opt($id,1,nsteps)"
        }
	}

	# Decide whether to run, run command, and copy files to temp
	GmCommonLayer::display_command [namespace current] $id $cmd
}
    
proc GmHist::mapname { node } {
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

proc GmHist::duplicate { tree parent node id } {
    variable optlist
    variable lfile
    variable lfilemask
    variable opt
    variable count
	variable dup
	variable first
	global iconpath

    set node "hist:$count"
	set dup($count) 1
    set first 1

    set frm [ frame .histicon$count]
    set check [checkbutton $frm.check \
		-variable GmHist::opt($count,1,_check) \
		-height 1 -padx 0 -width 0]

    image create photo hico -file "$iconpath/module-d.histogram.gif"
    set ico [label $frm.ico -image hico -bd 1 -relief raised]
    
    pack $check $ico -side left

	#insert new layer
	if {[$tree selection get] != "" } {
		set sellayer [$tree index [$tree selection get]]
    } else { 
    	set sellayer "end" 
    }

	if { $opt($id,1,map) == ""} {
 	   $tree insert $sellayer $parent $node \
		-text      "histogram $count" \
		-window    $frm \
		-drawcross auto
	} else {
	    $tree insert $sellayer $parent $node \
		-text      "histogram for $opt($id,1,map)" \
		-window    $frm \
		-drawcross auto
	}

	set opt($count,1,opacity) $opt($id,1,opacity)

	set optlist {_check map color style nsteps nulls}

    foreach key $optlist {
    	set opt($count,1,$key) $opt($id,1,$key)
		set opt($count,0,$key) $opt($count,1,$key)
    } 
	
	set id $count
	
	# create files in tmp directory for layer output
	set mappid [pid]
	set lfile($count) [exec g.tempfile pid=$mappid]
	set lfilemask($count) $lfile($count)
	append lfile($count) ".ppm"
	append lfilemask($count) ".pgm"

    incr count
    return $node
}
