##########################################################################
# legend.tcl - raster legend layer options file for GRASS GIS Manager
# March 2006 Michael Barton, Arizona State University
# COPYRIGHT:	(C) 1999 - 2006 by the GRASS Development Team
#
#		This program is free software under the GNU General Public
#		License (>=v2). Read the file COPYING that comes with GRASS
#		for details.
#
##########################################################################

namespace eval GmLegend {
    variable array opt # legend current options
    variable count 1
    variable array lfile # raster
    variable array lfilemask # raster
    variable optlist
    variable array dup # vector
    variable llcorner
}


###############################################################################
# create new legend layer
proc GmLegend::create { tree parent } {
    variable opt
    variable count
    variable lfile
    variable lfilemask
    variable optlist
    variable first
	variable dup
    

    set node "legend:$count"
	set dup($count) 1

    set frm [ frame .legendicon$count]
    set check [checkbutton $frm.check \
		-variable GmLegend::opt($count,1,_check) \
		-height 1 -padx 0 -width 0]

    set ico [label $frm.ico -bd 1 -relief raised -text "Leg"]
    icon_configure $ico module d.legend

    bind $ico <ButtonPress-1> "GmTree::selectn $tree $node"
    
    pack $check $ico -side left
    
	#insert new layer
	if {[$tree selection get] != "" } {
		set sellayer [$tree index [$tree selection get]]
    } else { 
    	set sellayer "end" 
    }

    $tree insert $sellayer $parent $node \
	-text  "legend $count"\
	-window    $frm \
	-drawcross auto  
        
    set opt($count,1,_check) 1 
    set dup($count) 0

    set opt($count,1,map) "" 
	set opt($count,1,opacity) 1.0
    set opt($count,1,color) "black" 
    set opt($count,1,lines) 0 
    set opt($count,1,thin) 1 
    set opt($count,1,mouseset) 0
    set opt($count,1,labelnum) 5 
    set opt($count,1,at) "5,90" 
    set opt($count,1,height) 80
    set opt($count,1,width) 5 
    set opt($count,1,use) "" 
    set opt($count,1,range) "" 
    set opt($count,1,nolbl) 0 
    set opt($count,1,noval) 0 
    set opt($count,1,skip) 0 
    set opt($count,1,smooth) 0 
    set opt($count,1,flip) 0 
    set opt($count,1,mod) 1
    
	set optlist { _check map opacity color lines thin labelnum at height width \
             mouseset use range nolbl noval skip smooth flip}
             
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

###############################################################################
proc GmLegend::set_option { node key value } {
    variable opt
 
    set id [GmTree::node_id $node]
    set opt($id,1,$key) $value
}

###############################################################################

proc GmLegend::mouseset { id } {
	# use mouse to set scalebar placement coordinates
	global mon pctentry
	variable llcorner 
	variable opt

	if { $GmLegend::opt($id,1,mouseset) == 1 } {
		set pctentry $GmLegend::llcorner
	} else {
		set pctentry ""
	}

}

###############################################################################
# select raster map
proc GmLegend::select_map { id } {
    variable tree
    variable node
    set m [GSelect cell title "Raster map for legend" parent [winfo containing [winfo pointerx .] [winfo pointery .]]]
    if { $m != "" } { 
        set GmLegend::opt($id,1,map) $m
        GmTree::autonamel "legend for $m"
    }
}

###############################################################################
# legend options
proc GmLegend::options { id frm } {
    variable opt
    variable llcorner
    global bgcolor

    # Panel heading
    set row [ frame $frm.heading1 ]
    Label $row.a -text "Display legend for raster map using cat values and labels" \
    	-fg MediumBlue
    pack $row.a -side left
    pack $row -side top -fill both -expand yes

	#opacity
	set row [ frame $frm.opc]
	Label $row.a -text [G_msg "Opaque "]
	scale $row.b -from 1.0 -to 0.0 -showvalue 1  \
		-orient horizontal -length 300 -resolution 0.01 -fg "#656565"\
		-variable GmLegend::opt($id,1,opacity) 
	Label $row.c -text [G_msg " Transparent"]
    pack $row.a $row.b $row.c -side left
    pack $row -side top -fill both -expand yes	
	
    # raster name
    set row [ frame $frm.map ]
    Label $row.a -text "Raster map: "
    Button $row.b -highlightthickness 0 -takefocus 0 -relief raised -borderwidth 1  \
		-command "GmLegend::select_map $id"
	icon_configure $row.b element cell
    Entry $row.c -width 35 -text " $opt($id,1,map)" \
          -textvariable GmLegend::opt($id,1,map) 
    Label $row.d -text "   "
    Button $row.e -text [G_msg "Help"] \
            -command "run g.manual d.legend" \
            -background $bgcolor \
            -helptext [G_msg "Help"]
	icon_configure $row.e gui help
    pack $row.a $row.b $row.c $row.d $row.e -side left
    pack $row -side top -fill both -expand yes

    # size and location
    set row [ frame $frm.at1 ]
    Label $row.a -text "Legend placement and size as 0-100% of display"
    pack $row.a -side left
    pack $row -side top -fill both -expand yes

    # at
    set row [ frame $frm.at2 ]
    Label $row.a -text "    x,y of lower left corner (in % from display top left)"
    set llcorner [LabelEntry $row.b -width 8 \
    	-textvariable GmLegend::opt($id,1,at)]
    checkbutton $row.c -text [G_msg "place with mouse"] \
    	-variable GmLegend::opt($id,1,mouseset) \
    	-command "GmLegend::mouseset $id"
    pack $row.a $row.b $row.c -side left
    pack $row -side top -fill both -expand yes
    
    # size
    set row [ frame $frm.size ]
    Label $row.a -text "    legend height "
    SpinBox $row.b -range {0 100 1} -textvariable GmLegend::opt($id,1,height) \
		 -width 5 -helptext "Legend height (% of display)" 
    Label $row.c -text "%  width" 
    SpinBox $row.d -range {0 100 1} -textvariable GmLegend::opt($id,1,width) \
		 -width 5 -helptext "Legend width (% of display)" 
    Label $row.e -text "%" 
    pack $row.a $row.b $row.c $row.d $row.e -side left
    pack $row -side top -fill both -expand yes

    # text color
    set row [ frame $frm.color ]
    Label $row.a -text [G_msg "Legend appearance: text color"] 
    ComboBox $row.b -padx 0 -width 10 -textvariable GmLegend::opt($id,1,color) \
		-values {"white" "grey" "gray" "black" "brown" "red" "orange" \
		"yellow" "green" "aqua" "cyan" "indigo" "blue" "purple" "violet" "magenta"}
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes
    
    # no category labels or numbers
    set row [ frame $frm.cats ]
    Label $row.a -text "    " 
    checkbutton $row.b -text [G_msg "do not display labels"] -variable \
        GmLegend::opt($id,1,nolbl) 
    checkbutton $row.c -text [G_msg "do not display values"] -variable \
        GmLegend::opt($id,1,noval) 
    pack $row.a $row.b $row.c -side left
    pack $row -side top -fill both -expand yes

    # display lines
    set row [ frame $frm.lines ]
    Label $row.a -text "    number of lines (0=display all):" 
    SpinBox $row.b -range {0 1000 1} -textvariable GmLegend::opt($id,1,lines) \
		 -width 5 -helptext "Lines to display" 
    Label $row.c -text "  " 
    checkbutton $row.d -text [G_msg "invert legend"] -variable \
        GmLegend::opt($id,1,flip) 
    pack $row.a $row.b $row.c $row.d -side left
    pack $row -side top -fill both -expand yes
    
    # thin
    set row [ frame $frm.thin ]
    Label $row.a -text "    interval between categories (integer maps)" 
    SpinBox $row.b -range {1 1000 1} -textvariable GmLegend::opt($id,1,thin) \
		 -width 5 -helptext "Thinning interval" 
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes
    
    # labelnum
    set row [ frame $frm.labelnum ]
    Label $row.a -text "  " 
    checkbutton $row.b -text [G_msg "draw smooth gradient (fp maps)"] -variable \
        GmLegend::opt($id,1,smooth) 
    Label $row.c -text "with maximum of" 
    SpinBox $row.d -range {2 100 1} -textvariable GmLegend::opt($id,1,labelnum) \
                    -width 4 -helptext "Maximum lines to display for gradient" 
    Label $row.e -text "lines" 
    pack $row.a $row.b $row.c $row.d $row.e -side left
    pack $row -side top -fill both -expand yes
            
	# display subset of values
    set row [ frame $frm.subset ]
    Label $row.a -text "Display legend for subset of raster values"
    pack $row.a -side left
    pack $row -side top -fill both -expand yes

    # skip
    set row [ frame $frm.opts ]
    Label $row.a -text "  " 
    checkbutton $row.b -text [G_msg "skip categories with no labels"] -variable \
        GmLegend::opt($id,1,skip) 
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes

    # use cats
    set row [ frame $frm.use ]
    Label $row.a -text "    legend for only these categories     "
    LabelEntry $row.b -textvariable GmLegend::opt($id,1,use) -width 28
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes
    
    # range
    set row [ frame $frm.range ]
    Label $row.a -text "    legend for only this range of values"
    LabelEntry $row.b -textvariable GmLegend::opt($id,1,range) -width 28
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes

	set opt($id,1,mod) "1"
}

###############################################################################
#save legend layer to  grc file
proc GmLegend::save { tree depth node } {
    variable opt
    variable optlist
    
    set id [GmTree::node_id $node]

    foreach key $optlist {
        GmTree::rc_write $depth "$key $opt($id,1,$key)"
    } 
}


###############################################################################
# render and composite legend layer
proc GmLegend::display { node mod } {
    global mon
    variable optlist
    variable lfile 
    variable lfilemask
    variable opt
    variable tree
    variable dup
    variable count

    set line ""
    set input ""
    set cmd ""

    set tree($mon) $GmTree::tree($mon)
    set id [GmTree::node_id $node]

    # If we are told dirty (for zoom) force dirty
    # Don't remove a dirty from a previous unrendered zoom
    if {$mod} {set opt($id,1,mod) 1}
    
	set atlist [split $opt($id,1,at) ","]
	set x1 [lindex $atlist 0]
	set y1 [expr 100 - [lindex $atlist 1]]
	
	set placement "$y1,[expr $y1+$opt($id,1,height)],$x1,[expr $x1+$opt($id,1,width)]"

    if { $opt($id,1,map) == "" } { return } 
    set cmd "d.legend map=$opt($id,1,map) color=$opt($id,1,color) \
            lines=$opt($id,1,lines) thin=$opt($id,1,thin) \
            labelnum=$opt($id,1,labelnum) at=$placement"

    # use cats
    if { $opt($id,1,use) != "" } { 
        append cmd " use=$opt($id,1,use)"
    }

    # range
    if { $opt($id,1,range) != "" } { 
        append cmd " range=$opt($id,1,range)"
    }

    # nolbl
    if { $opt($id,1,nolbl) != 0 } { 
        append cmd " -v"
    }

    # noval
    if { $opt($id,1,noval) != 0 } { 
        append cmd " -c"
    }

    # skip
    if { $opt($id,1,skip) != 0} { 
        append cmd " -n"
    }

    # smooth
    if { $opt($id,1,smooth) != 0 } { 
        append cmd " -s"
    }
        
    # flip
    if { $opt($id,1,flip) != 0 } { 
        append cmd " -f"
    }
    
	# Decide whether to run, run command, and copy files to temp
	GmCommonLayer::display_command [namespace current] $id $cmd
}


###############################################################################
# duplicate legend layer
proc GmLegend::duplicate { tree parent node id } {
    variable optlist
    variable lfile
    variable lfilemask
    variable opt
    variable count
	variable dup

    set node "legend:$count"

    set frm [ frame .legendicon$count]
    set check [checkbutton $frm.check \
		-variable GmLegend::opt($count,1,_check) \
		-height 1 -padx 0 -width 0]

    set ico [label $frm.ico -bd 1 -relief raised -text "Leg"]
	icon_configure $ico module d.legend
    
    pack $check $ico -side left

	# where to insert new layer
	if {[$tree selection get] != "" } {
		set sellayer [$tree index [$tree selection get]]
    } else { 
    	set sellayer "end" 
    }

	if { $opt($id,1,map) == ""} {
	    $tree insert $sellayer $parent $node \
		-text      "legend $count" \
		-window    $frm \
		-drawcross auto
	} else {
	    $tree insert $sellayer $parent $node \
		-text      "legend for $opt($id,1,map)" \
		-window    $frm \
		-drawcross auto
	}


	set opt($count,1,opacity) $opt($id,1,opacity)

	set optlist { _check map opacity color lines thin labelnum at height width \
             mouseset use range nolbl noval skip smooth flip}
             
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
