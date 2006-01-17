# 1 Sept 2005
# panel for d.legend
# Michael Barton, Arizona State University

namespace eval GmLegend {
    variable array opt # legend options
    variable count 1
}


proc GmLegend::create { tree parent } {
    variable opt
    variable count
    global gmpath

    set node "legend:$count"

    set frm [ frame .legendicon$count]
    set fon [font create -size 10] 
    set check [checkbutton $frm.check -font $fon \
                           -variable GmLegend::opt($count,_check) \
                           -height 1 -padx 0 -width 0]

    image create photo legico -file "$gmpath/legend.gif"
    set ico [label $frm.ico -image legico -bd 1 -relief raised]
    
    pack $check $ico -side left
    
    $tree insert end $parent $node \
	-text  "legend $count"\
	-window    $frm \
	-drawcross auto  
        
    set opt($count,_check) 1 
    set opt($count,map) "" 
    set opt($count,erase) 1 
    set opt($count,color) "black" 
    set opt($count,lines) 0 
    set opt($count,thin) 1 
    set opt($count,labelnum) 5 
    set opt($count,at) "5,95,5,10" 
    set opt($count,use) "" 
    set opt($count,range) "" 
    set opt($count,nolbl) 0 
    set opt($count,noval) 0 
    set opt($count,skip) 0 
    set opt($count,smooth) 0 
    set opt($count,mouse) 0 
    set opt($count,flip) 0 
    
    incr count
    return $node
}

proc GmLegend::set_option { node key value } {
    variable opt
 
    set id [GmTree::node_id $node]
    set opt($id,$key) $value
}

proc GmLegend::select_map { id } {
    variable tree
    variable node
    set m [GSelect cell]
    if { $m != "" } { 
        set GmLegend::opt($id,map) $m
        GmTree::autonamel "legend for $m"
    }
}

# legend options
proc GmLegend::options { id frm } {
    variable opt
    global gmpath
    global bgcolor

    # raster name
    set row [ frame $frm.map ]
    Button $row.a -text [G_msg "Raster map for legend:"] \
           -command "GmLegend::select_map $id"
    Entry $row.b -width 40 -text " $opt($id,map)" \
          -textvariable GmLegend::opt($id,map) \
          -background white
    Button $row.c -text [G_msg "Help"] \
            -image [image create photo -file "$gmpath/grass.gif"] \
            -command "run g.manual d.legend" \
            -background $bgcolor \
            -helptext [G_msg "Help"]
    pack $row.a $row.b $row.c -side left
    pack $row -side top -fill both -expand yes

    # monitor for legend
    set row [ frame $frm.monitor ]
    checkbutton $row.a -text [G_msg " erase monitor before drawing legend"] -variable \
        GmLegend::opt($id,erase) 
    pack $row.a -side left
    pack $row -side top -fill both -expand yes

    # text color
    set row [ frame $frm.color ]
    Label $row.a -text [G_msg "Text color: "] 
    ComboBox $row.b -padx 0 -width 10 -textvariable GmLegend::opt($id,color) \
                    -values {"white" "grey" "gray" "black" "brown" "red" "orange" \
                    "yellow" "green" "aqua" "cyan" "indigo" "blue" "purple" "violet" "magenta"} \
                    -entrybg white
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes

    # display lines
    set row [ frame $frm.lines ]
    Label $row.a -text "Number of lines to display (0=display all):" 
    SpinBox $row.b -range {0 1000 1} -textvariable GmLegend::opt($id,lines) \
                   -entrybg white -width 5 -helptext "Lines to display" 
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes
    
    # thin
    set row [ frame $frm.thin ]
    Label $row.a -text "Interval between categories (thinning interval) of integer maps:" 
    SpinBox $row.b -range {1 1000 1} -textvariable GmLegend::opt($id,thin) \
                   -entrybg white -width 5 -helptext "Thinning interval" 
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes
    
    # labelnum
    set row [ frame $frm.labelnum ]
    Label $row.a -text "Maximum number of labels for smooth gradients" 
    SpinBox $row.b -range {2 100 1} -textvariable GmLegend::opt($id,labelnum) \
                   -entrybg white -width 4 -helptext "Maximum labels for gradient" 
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes
    
    # at
    set row [ frame $frm.at ]
    Label $row.a -text "Place legend at 0-100% from bottom left (bottom,top,left,right)"
    LabelEntry $row.b -textvariable GmLegend::opt($id,at) -width 15 \
            -entrybg white
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes
        
    # use cats
    set row [ frame $frm.use ]
    Label $row.a -text "Display only these categories"
    LabelEntry $row.b -textvariable GmLegend::opt($id,use) -width 42 \
            -entrybg white
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes
    
    # range
    set row [ frame $frm.range ]
    Label $row.a -text "Display only this range of values"
    LabelEntry $row.b -textvariable GmLegend::opt($id,range) -width 39 \
            -entrybg white
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes
    
    # no category labels or numbers
    set row [ frame $frm.cats ]
    checkbutton $row.a -text [G_msg "do not show cat labels"] -variable \
        GmLegend::opt($id,nolbl) 
    checkbutton $row.b -text [G_msg "do not show cat numbers"] -variable \
        GmLegend::opt($id,noval) 
    pack $row.a $row.b -side left
    pack $row -side top -fill both -expand yes

    # skip, gradient, and flip
    set row [ frame $frm.opts ]
    checkbutton $row.a -text [G_msg "skip cats with no labels"] -variable \
        GmLegend::opt($id,skip) 
    checkbutton $row.b -text [G_msg "draw smooth gradient"] -variable \
        GmLegend::opt($id,smooth) 
    checkbutton $row.c -text [G_msg "flip legend"] -variable \
        GmLegend::opt($id,flip) 
    pack $row.a $row.b $row.c -side left
    pack $row -side top -fill both -expand yes

    # mouse
    set row [ frame $frm.mouse ]
    checkbutton $row.a -text \
        [G_msg "place with mouse (cannot save placement with group)"] \
        -variable GmLegend::opt($id,mouse) 
    pack $row.a -side left
    pack $row -side top -fill both -expand yes
}

proc GmLegend::save { tree depth node } {
    variable opt
    
    set id [GmTree::node_id $node]

    foreach key { _check map color lines thin labelnum at use range \
             nolbl noval skip smooth mouse flip } {
        GmTree::rc_write $depth "$key $opt($id,$key)"
    } 
}


proc GmLegend::display { node } {
    variable opt
    variable tree
    global mon
    global gmpath

    set line ""
    set input ""
    set cmd ""

    set tree($mon) $GmTree::tree($mon)
    set id [GmTree::node_id $node]


    if { ! ( $opt($id,_check) ) } { return } 

    if { $opt($id,map) == "" } { return } 
    set cmd "d.legend map=$opt($id,map) color=$opt($id,color) \
            lines=$opt($id,lines) thin=$opt($id,thin) \
            labelnum=$opt($id,labelnum) at=$opt($id,at)"

    # use cats
    if { $opt($id,use) != "" } { 
        append cmd " use=$opt($id,use)"
    }

    # range
    if { $opt($id,range) != "" } { 
        append cmd " range=$opt($id,range)"
    }

    # nolbl
    if { $opt($id,nolbl) != 0 } { 
        append cmd " -v"
    }

    # noval
    if { $opt($id,noval) != 0 } { 
        append cmd " -c"
    }

    # skip
    if { $opt($id,skip) != 0} { 
        append cmd " -n"
    }

    # smooth
    if { $opt($id,smooth) != 0 } { 
        append cmd " -s"
    }
    
    # mouse
    if { $opt($id,mouse) != 0 } { 
        append cmd " -m"
    }
    
    # flip
    if { $opt($id,flip) != 0 } { 
        append cmd " -f"
    }

    #display legend
	if { $opt($id,erase) == 1 } {run "d.erase white"}
	run_panel $cmd
    
}


proc GmLegend::duplicate { tree parent node id } {
    variable opt
    variable count 
    global gmpath

    set node "legend:$count"

    set frm [ frame .legendicon$count]
    set fon [font create -size 10] 
    set check [checkbutton $frm.check -font $fon \
                           -variable GmLegend::opt($count,_check) \
                           -height 1 -padx 0 -width 0]

    image create photo legico -file "$gmpath/legend.gif"
    set ico [label $frm.ico -image legico -bd 1 -relief raised]
    
    pack $check $ico -side left

	if { $opt($id,map) == ""} {
    	$tree insert end $parent $node \
		-text      "legend $count" \
		-window    $frm \
		-drawcross auto
	} else {
	    $tree insert end $parent $node \
		-text      "legend for $opt($id,map)" \
		-window    $frm \
		-drawcross auto
	}

    set opt($count,_check) $opt($id,_check)

    set opt($count,map) "$opt($id,map)" 
    set opt($count,color) "$opt($id,color)" 
    set opt($count,lines) "$opt($id,lines)" 
    set opt($count,thin) "$opt($id,thin)" 
    set opt($count,labelnum) "$opt($id,labelnum)"
    set opt($count,at) "$opt($id,at)"
    set opt($count,use) "$opt($id,use)"
    set opt($count,range) "$opt($id,range)"
    set opt($count,nolbl) "$opt($id,nolbl)" 
    set opt($count,noval) "$opt($id,noval)" 
    set opt($count,skip) "$opt($id,skip)" 
    set opt($count,smooth) "$opt($id,smooth)"
    set opt($count,mouse) "$opt($id,mouse)" 
    set opt($count,flip) "$opt($id,flip)" 

    incr count
    return $node
}
