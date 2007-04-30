##########################################################################
#
# gm.tcl
#
# Primary tcltk script for GIS Manager: GUI for GRASS 6
# Author: Michael Barton (Arizona State University)
# Based in part on Display Manager for GRASS 5.7 by Radim Blazek (ITC-IRST)
# and tcltkgrass for GRASS 5.7 by Michael Barton (Arizona State University)--
# with contributions by Glynn Clements, Markus Neteler, Lorenzo Moretti,
# Florian Goessmann, and others
#
# March 2006
#
# COPYRIGHT:	(C) 1999 - 2006 by the GRASS Development Team
#
#		This program is free software under the GNU General Public
#		License (>=v2). Read the file COPYING that comes with GRASS
#		for details.
#
##########################################################################

lappend auto_path $env(GISBASE)/bwidget

package require -exact BWidget 1.2.1

# Load up all the gis.m layers and things.
# pkgIndex.tcl only loads the files when they are first called.
lappend auto_path $env(GISBASE)/etc/gm
package require -exact GisM 1.0

set env(GISDBASE) [exec g.gisenv get=GISDBASE]
set env(LOCATION_NAME) [exec g.gisenv get=LOCATION_NAME]
set env(MAPSET) [exec g.gisenv get=MAPSET]

set gisdbase [exec g.gisenv get=GISDBASE]
set location_name [exec g.gisenv get=LOCATION_NAME]
set mapset [exec g.gisenv get=MAPSET]

# path to GIS Manager files
set gmpath $env(GISBASE)/etc/gm

# path to icons for GIS Manager
set iconpath $env(GISBASE)/etc/gui/icons

# path to X11 TrueType fonts
set xfontdir "/usr/X11R6/lib/X11/fonts/TTF/"

global iconpath
global gmpath
global xfontdir

set keycontrol "Control"
set tmenu "1"
set keyctrl "Ctrl"
set execom "execute"
set msg 0
set mon 1

if {[info exists env(HOSTTYPE)]} {
	set HOSTTYPE $env(HOSTTYPE)
} else {
	set HOSTTYPE ""
}

# add for OSX aqua
if {[info exists env(osxaqua)]} {
    set osxaqua $env(osxaqua)
} else {
    set osxaqua "0"
}

if { $osxaqua == "1"} {
    set keycontrol "Command"
    set tmenu "0"
    set keyctrl "Command"
    set execom "spawn"
}

if {[info exists env(OS)] && $env(OS) == "Windows_NT"} {
	set mingw "1"
	set devnull "NUL:"
} else {
	set mingw "0"
	set devnull "/dev/null"
}


#fetch GRASS Version number:
catch {set fp [open $env(GISBASE)/etc/VERSIONNUMBER r]}
set GRASSVERSION [read -nonewline $fp]

if {[catch {close $fp} error]} {
	puts $error
}


source $env(GISBASE)/etc/gui.tcl
# gui.tcl also sources these:
# $env(GISBASE)/etc/gtcltk/gmsg.tcl
# $env(GISBASE)/etc/gtcltk/options.tcl
# $env(GISBASE)/etc/gtcltk/select.tcl
# $env(GISBASE)/etc/gtcltk/gronsole.tcl

# Load a console user interface
source $gmpath/runandoutput.tcl

namespace eval Gm {
    variable gm_mainframe
    variable status
    variable array tree # mon
    variable rcfile
    variable moncount
    variable prgtext
    variable mainwindow
    variable dfont
    variable fonttype
	global array filename # mon

}


set Gm::prgtext ""
global prgindic
global max_prgindic

set max_prgindic 20


###############################################################################

append regexp .* $env(GISBASE) {[^:]*}
regsub -- $regexp $env(PATH) "&:$env(GISBASE)/etc/gm/script" env(PATH)


###############################################################################

proc read_moncap {} {
	global env moncap

	set moncap {}

	catch {set file [open [file join $env(GISBASE) etc monitorcap] r]}
	set data [read $file]
	
	if {[catch {close $file} error]} {
		puts $error
	}

	set data [subst -nocommands -novariables $data]
	foreach line [split $data \n] {
		if {[string match {\#*} $line]} continue
		if {![string match {*:*:*:*:*:*} $line]} continue
		set fields {}
		foreach field [split $line :] {
			lappend fields [string trim $field]
		}
		lappend moncap $fields
	}
}

###############################################################################

proc monitor_menu {op} {
	global moncap

	set submenu {}
	set last_driver {}
	foreach mon $moncap {
		set name [lindex $mon 0]
		set driver [lindex $mon 1]
		if {$last_driver != "" && $last_driver != $driver} {
			lappend submenu {separator}
		}
		set last_driver $driver
		lappend submenu [list command $name {} "" {} -command "run d.mon $op=$name"]	}

	return [list $submenu]
}

###############################################################################

read_moncap

proc Gm::color { color } {

    regexp -- {#(..)(..)(..)} $color x r g b

    set r [expr 0x$r ]
    set g [expr 0x$g ]
    set b [expr 0x$b ]

    return "$r:$g:$b"
}


###############################################################################
# Deprecated
# Use guarantee_xmon and any run command instead.

proc Gm::xmon { type cmd } {
	guarantee_xmon

	if { $type == "term" } {
		term_panel $cmd
	} else {
		run_panel $cmd
	}

	return
}

###############################################################################
# Determine if an element already exists

proc Gm::element_exists {elem name} {
	global devnull
	set exists 1
	
	set failure [catch {exec g.findfile element=$elem file=$name >& $devnull}]

	return [expr {! $failure}]
}

###############################################################################


proc Gm::create { } {
    variable mainwindow
    variable prgtext
    variable gm_mainframe
    variable tree
	variable moncount
    global gmpath
    global mon
    global tree_pane
    global options
    global pgs
    global prgindic
    global keycontrol
    global env

	set moncount 1

    set Gm::prgtext [G_msg "Loading GIS Manager"]
    set prgindic -1
    _create_intro
    update

	source $gmpath/gmmenu.tcl

    set Gm::prgtext [G_msg "Creating MainFrame..."]

    set gm_mainframe [MainFrame .mainframe \
		       -menu $descmenu \
		       -textvariable Gm::status \
		       -progressvar  Gm::prgindic ]

    set mainwindow [$gm_mainframe getframe]

    # toolbar 1 & 2 creation
    set tb1  [$gm_mainframe addtoolbar]
    GmToolBar1::create $tb1
    set tb2  [$gm_mainframe addtoolbar]
    GmToolBar2::create $tb2
    set pw1 [PanedWindow $mainwindow.pw1 -side left -pad 0 -width 10 ]

    # tree
    set treemon [expr {$mon + 1}]
    set tree_pane  [$pw1 add  -minsize 50 -weight 1]
	set pgs [PagesManager $tree_pane.pgs]


	pack $pgs -expand yes -fill both


    # options
    set options_pane  [$pw1 add -minsize 50 -weight 1]
    set options_sw [ScrolledWindow $options_pane.sw -relief flat -borderwidth 1]
    set options_sf [ScrollableFrame $options_sw.sf]
    $options_sf configure -height 145 -width 460
    $options_sw setwidget $options_sf
    set options [$options_sf getframe]
    pack $options_sw -fill both -expand yes

    # Scroll the options window with the mouse
    bind_scroll $options_sf

    pack $pw1 -side top -expand yes -fill both -anchor n

	# finish up
    set Gm::prgtext [G_msg "Done"]

    set Gm::status [G_msg "Welcome to GRASS GIS"]
    $gm_mainframe showstatusbar status

    pack $gm_mainframe -fill both -expand yes

	Gm::startmon

	bind .mainframe <Destroy> {
		if {"%W" == ".mainframe"} {
			Gm::cleanup}
	}



}


###############################################################################

# start new display monitor and increment canvas monitor number
proc Gm::startmon { } {
	variable mainwindow
	variable moncount
	variable tree
	global mon

	set mon $moncount
	incr moncount 1

	#create initial display canvas and layer tree
	MapCanvas::create
	GmTree::create $mon

	wm title .mapcan($mon) [format [G_msg "Map Display %d"] $mon]
	wm withdraw .mapcan($mon)
	wm deiconify .mapcan($mon)
}


###############################################################################

proc Gm::_create_intro { } {
	variable prgtext
    global gmpath
    global GRASSVERSION
    global location_name
    global max_prgindic
    global prg

    set top [toplevel .intro -relief raised -borderwidth 2]

    wm withdraw $top
    wm overrideredirect $top 1

    set ximg  [label $top.x -image [image create photo -file "$gmpath/intro.gif"] ]

    set frame [frame $ximg.f -background white]
    set lab1  [label $frame.lab1 \
		-text [format [G_msg "GRASS%s GIS Manager - %s"] $GRASSVERSION $location_name] \
		-background white -foreground black -font introfont]
    set lab2  [label $frame.lab2 -textvariable Gm::prgtext -background white]
    set prg   [ProgressBar $frame.prg -width 50 -height 15 -background white \
		   -variable Gm::prgindic -maximum $max_prgindic]
    pack $lab1 $prg -side left -fill both -expand yes
    pack $lab2 -side right -expand yes
    place $frame -x 0 -y 0 -anchor nw
    pack $ximg
    BWidget::place $top 0 0 center
    wm deiconify $top
}

###############################################################################

# nviz
proc Gm::nviz { } {
global osxaqua
global HOSTTYPE

    set cmd "nviz"
	if { $HOSTTYPE == "macintosh" || $HOSTTYPE == "powermac" || $HOSTTYPE == "powerpc" || $HOSTTYPE == "intel-pc"} {
		if { $osxaqua == "1"} {
			spawn $cmd
		} else {
			term $cmd
		}
	} else {
		spawn $cmd
	}

}

###############################################################################

# d.nviz: set up NVIZ flight path (not much use without a backdrop?)
proc Gm::fly { } {

    guarantee_xmon
    exec d.nviz -i --ui &

}

###############################################################################

# xganim
proc Gm::xganim { } {

    exec xganim --ui &

}

###############################################################################

# help
proc Gm::help { } {

    set cmd [list g.manual --ui]
    term $cmd

}

###############################################################################

#open dialog box
proc Gm::OpenFileBox { } {
    variable mainwindow
    global filename
    global mon

# thanks for brace tip to suchenwi from #tcl@freenode
    set types [list \
	    [list [G_msg "Map Resource File"] [list ".dm" ".dmrc" ".grc"]] \
	    [list [G_msg "All Files"] "*"] \
    ]

	set filename_new [tk_getOpenFile -parent $mainwindow -filetypes $types \
		-title [G_msg "Open File"] ]
	if { $filename_new == "" } { return}
	set filename($mon) $filename_new
	GmTree::load $filename($mon)

};

###############################################################################

#save dialog box
proc Gm::SaveFileBox { } {
    variable mainwindow
    global filename
    global mon

    catch {
	if {[ regexp -- {^Untitled_} $filename($mon) r]} {
		set filename($mon) ""
	}
    }

    if { $filename($mon) != "" } {
	GmTree::save $filename($mon)
    } else {
	set types [list \
	    [list [G_msg "Map Resource File"] {.grc}] \
	    [list [G_msg "DM Resource File"] [list {.dm} {.dmrc}]] \
	    [list [G_msg "All Files"] "*"] \
	]
	set filename($mon) [tk_getSaveFile -parent $mainwindow -filetypes $types \
		-title [G_msg "Save File"] -defaultextension .grc]
	if { $filename($mon) == "" } { return}
	GmTree::save $filename($mon)
    }
};

###############################################################################
# sets default display font
proc Gm:DefaultFont { } {
	global env iconpath
	variable dfont
	variable fonttype

	# create a dialog with selector for stroke font and true type font, and
	# text entry for character encoding
	
	set Gm::fonttype "grassfont"
	set dfont "romans"
	set encoding "ISO-8859-1"

    toplevel .dispfont
    wm title .dispfont [G_msg "Select default font for GRASS displays"]
    
    set row [ frame .dispfont.fontopt1 ]
    Label $row.a -text [G_msg "Font type: "] 
    set gfont [radiobutton $row.b -variable Gm::fonttype -value "grassfont" \
        -text [G_msg "GRASS stroke font"] -highlightthickness 0]
        DynamicHelp::register $gfont balloon [G_msg "GRASS stroke fonts"]
    set ofont [radiobutton $row.c -variable Gm::fonttype -value "truetype" \
        -text [G_msg "TrueType font"] -highlightthickness 0]
        DynamicHelp::register $ofont balloon [G_msg "Custom font path"]
    pack $row.c $row.b $row.a -side right -anchor e
    pack $row -side top -fill both -expand yes -pady 3 -padx 5

	set row [ frame .dispfont.fontopt2 ]
    Label $row.a -text [G_msg "Font: "] 
    Button $row.b -image [image create photo -file "$iconpath/gui-font.gif"] \
        -highlightthickness 0 -takefocus 0 -relief raised -borderwidth 1  \
        -helptext [G_msg "Select font"] \
	    -command "Gm::SelectFont"
    Entry $row.c -width 30 -text "$dfont" \
	    -textvariable Gm::dfont
    pack $row.c $row.b $row.a -side right -anchor e
    pack $row -side top -fill both -expand yes -pady 3 -padx 5
    
 	set row [ frame .dispfont.fontopt3 ]
    Label $row.a -text [G_msg "Character encoding: "] 
    Entry $row.b -width 30 -text $encoding \
	    -textvariable encoding
    pack $row.b $row.a -side right -anchor e
    pack $row -side top -fill both -expand yes -pady 3 -padx 5
    
	set row [ frame .dispfont.buttons ]
    Button $row.ok -text [G_msg "OK"] -width 8 -bd 1 \
    	-command "Gm::SetFont $dfont $encoding; destroy .dispfont"
    pack $row.ok -side left -fill x -expand 0
    button $row.cancel -text [G_msg "Cancel"] -width 8 -bd 1 \
    	-command "destroy .dispfont"
    pack $row.cancel -side right -fill x -expand 0
    pack $row -side bottom -pady 3  -padx 5 -expand 0 -fill x 
    
    #tkwait window .dispfont
}


proc Gm::SelectFont { } {
	global env
	variable dfont
	variable fonttype
	
	set systemtype [exec uname -s]
	set systemtype [string trim $systemtype]

	if {$Gm::fonttype == "grassfont"} {
		set fontpath  [file join "$env(GISBASE)" "fonts"]
	} elseif {$Gm::fonttype == "truetype"} {
		if {$systemtype == "Darwin"} {
			set fontpath [file join "/Library" "Fonts"]
		} elseif {$systemtype == "MINGW"} {
			set fontpath [file joint $WINDIR "Fonts"]
		} elseif {$systemtype == "CYGWIN"} {
			set fontpath [file joint $WINDIR "Fonts"]
		} else {
			set fontpath [file join "/usr" "lib" "X11" "fonts"]
		}
	}
	
	if {![file exists $fontpath]} {set fontpath ""}

	set fontname [tk_getOpenFile -initialdir $fontpath \
		-title [G_msg "Select font"] ]

	set Gm::dfont $fontname

};

proc Gm::SetFont { dfont encoding } {
	# Set GRASS environmental variables for default display font and
	# character encoding

    if { $encoding != "" && $encoding != "ISO-8859-1"} {
    	set env(GRASS_FT_ENCODING) $encoding
    }
    
    if {$dfont != ""} {set env(GRASS_FONT) $dfont}

};

###############################################################################

proc Gm::cleanup { } {
	global mon
	global tmpdir
	global legfile
	variable moncount
	
	# delete temporary local region files (no longer needed)
	#for {set x 1} {$x<$moncount} {incr x} {
	#	exec g.remove region=map_$x
	#}

	set mappid $MapCanvas::mappid
	
	# delete all map display ppm files
	cd $tmpdir
	set deletefile $mappid
	append deletefile ".*"
	foreach file [glob -nocomplain $deletefile] {
		catch {file delete $file}
	}

	if {[file exists $legfile]} {catch {file delete -force $legfile}}

	unset mon

}

###############################################################################

proc main {argc argv} {
    variable gm_mainframe
    global auto_path
    global GRASSVERSION
    global location_name
    global mapset
    global keycontrol
    global filename
    global mon

    wm withdraw .
    wm title . [format [G_msg "GRASS%s GIS Manager - %s %s"] $GRASSVERSION $location_name $mapset]

    bind . <$keycontrol-Key-o> {
		Gm::OpenFileBox
    }
    bind . <$keycontrol-Key-n> {
		GmTree::new
    }
    bind . <$keycontrol-Key-s> {
		Gm::SaveFileBox
    }
    bind . <$keycontrol-Key-q> {
		exit
	}
    bind . <$keycontrol-Key-w> {
		GmTree::FileClose {}
    }

    Gm::create
    BWidget::place . 0 0 at 400 100

    wm deiconify .
    raise .mainframe
    focus -force .
    destroy .intro
    
    if { $argc > 1 } {
    	foreach i $argv {
    		if { [regexp -- {\.grc$} $i] || [regexp -- {\.dmrc$} $i] || [regexp -- {\.dm$} $i] } { 
			set filename($mon) [lindex $argv 0]
			GmTree::load $filename($mon)
    		}
    	}
    }
}


main $argc $argv
wm geom . [wm geom .]

