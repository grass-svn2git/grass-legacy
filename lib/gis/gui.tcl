# Read README.GUI !

lappend auto_path $env(GISBASE)/bwidget
package require -exact BWidget 1.2.1
source $env(GISBASE)/etc/gtcltk/options.tcl
source $env(GISBASE)/etc/gtcltk/select.tcl
source $env(GISBASE)/etc/gtcltk/gronsole.tcl

set env(GISDBASE) [exec g.gisenv get=GISDBASE]
set env(LOCATION_NAME) [exec g.gisenv get=LOCATION_NAME]
set env(MAPSET) [exec g.gisenv get=MAPSET]

set dlg 0
set path {}
set iconpath $env(GISBASE)/etc/gui/icons/


################################################################################
# Miscellanious

proc icon {class member} {
	global iconpath
	set name "::img::icon-$class-$member"
	if {! [catch {image type $name}]} {
		return $name
	}
	if {! [catch {image create photo $name -file "$iconpath/$class-$member.gif"}]} {
		return $name
	}
	if {$class == "module" && $member != ""} {
		set memberparts [split $member "."]
		# tcl/tk8.0: Can't use end-1
		set memberparts [lrange $memberparts 0 [expr [llength $memberparts] - 2]]
		set member [join $memberparts "."]
		return [icon $class $member]
	}
			
	return 0
}

# Frame scrolling that works:
proc handle_scroll {window ammount} {
	if {[winfo exists $window] && [winfo ismapped $window]} {
		$window yview scroll [expr {-$ammount/120}] units
	}
}

################################################################################

proc mkcmd {dlg} {
	global opt
	set pgm_name $opt($dlg,pgm_name)
	set nopt $opt($dlg,nopt)

	set cmd [list $pgm_name]
	for {set i 1} {$i <= $nopt} {incr i} {
		switch -- $opt($dlg,$i,class) {
		multi {
			set nmulti $opt($dlg,$i,nmulti)
			set opts {}
			for {set j 1} {$j <= $nmulti} {incr j} {
				if {$opt($dlg,$i,val,$j) == 1} {
					lappend opts $opt($dlg,$i,valname,$j)
				}
			}
			if {$opts != {}} {
				lappend cmd "$opt($dlg,$i,name)=[join $opts ,]"
			}
		}
		opt {
			# Tempting, but buggy: && [string compare $opt($dlg,$i,val) $opt($dlg,$i,answer) ] != 0
			if {[string length $opt($dlg,$i,val)] > 0} {
				lappend cmd "$opt($dlg,$i,name)=$opt($dlg,$i,val)"
			}
		}
		flag {
			if {$opt($dlg,$i,val) == 1} {
				lappend cmd "-$opt($dlg,$i,name)"
			}
		}
		}
	}

	return $cmd
}

proc mkcmd_string {dlg} {
	set cmd [mkcmd $dlg]
	set cmd_string {}
	foreach word $cmd {
		if {[llength $word] > 1} {
			regsub -all -- {'} $word {'\''} newword
			append cmd_string {'} $newword {' }
		} {
			append cmd_string $word { }
		}
	}
	return $cmd_string
}

# Display the current command text in the label
proc show_cmd {dlg} {
	global opt
	set opt($dlg,cmd_string) [mkcmd_string $dlg]
}

proc get_file {dlg optn} {
	global opt
	set filename [tk_getOpenFile -title {Load File}]
	if {$filename != ""} {
		if {$opt($dlg,$optn,multi) && $opt($dlg,$optn,val) != ""} {
			append opt($dlg,$optn,val) "," $filename
		} {
			set opt($dlg,$optn,val) $filename
		}
	}
	show_cmd $dlg
}

proc get_map {dlg optn elem} {
	global opt
	set val [GSelect_::create $elem]
	if {$val != ""} {
		if {$opt($dlg,$optn,multi) && $opt($dlg,$optn,val) != ""} {
			append opt($dlg,$optn,val) "," $val
		} {
			set opt($dlg,$optn,val) $val
		}
	}
	show_cmd $dlg
}

proc run_cmd {dlg} {
	global opt
	set gronsole $opt($dlg,gronsole)

	set title "Output"
	layout_raise_special_frame $dlg [list $title] $title]

	set cmd [mkcmd $dlg]

	catch {$opt($dlg,run_button) configure -state disabled}

	$gronsole run $cmd {} "catch {$opt($dlg,run_button) configure -state active}"
}

proc help_cmd {dlg} {
	global opt env
	set pgm_name $opt($dlg,pgm_name)

	exec $env(GRASS_HTML_BROWSER) $env(GISBASE)/docs/html/$pgm_name.html &
}

proc clear_cmd {dlg} {
	global opt
	set gronsole $opt($dlg,gronsole)

	$gronsole clear
}

proc close_cmd {dlg} {
	global opt
	set root $opt($dlg,root)

	destroy $root
}

proc progress {dlg percent} {
	global opt
	
	set opt($dlg,percent) $percent
	
	# it seems that there is a bug in ProgressBar and it is not always updated ->
	$opt($dlg,progress) _modify
}

################################################################################
# Default layout rule:
# Section based notebook layout

# Make a frame for part of the layout tree
proc layout_make_frame {dlg guisection optn} {
	global opt

	if {$guisection == {}} {set guisection {{}}}
	if {[llength $guisection] == 1} {
		# A frame for a toplevel section
		# This uses a scrolled frame in a notebook tab
		set label [lindex $guisection 0]
		# Ungrouped options go under Options
		if {$label == {}} {
			set label "Options"
		}
		set path $opt($dlg,path)
		set optpane [$path.nb insert end $label -text $label]
		# Specials don't get scrolling frames:
		if {$optn == -1} {
			$path.nb raise $label
			return $optpane
		}
		# And the frames and scrollers:
		set optwin [ScrolledWindow $optpane.optwin -relief sunken -borderwidth 2]
		set optfra [ScrollableFrame $optwin.fra -height 200]
		$optwin setwidget $optfra
		pack $optwin -fill both -expand yes

		# Bindings for scrolling the frame
		bind all <MouseWheel> "+handle_scroll $optfra %D"
		bind all <Button-4> "+handle_scroll $optfra 120"
		bind all <Button-5> "+handle_scroll $optfra -120"

		set suf [$optfra getframe]
		# Binding magic to make the whole program start at an appropriate size
		bind $suf <Configure> {+[winfo parent %W] configure -width [winfo reqwidth %W]}
		$path.nb raise $label

		return $suf
	} else {
		# Make a frame for things in this guisection
		# We could add labels, but I fear it would just make a clutter
		# tcl/tk8.0: Can't use end-1
		set parent_section [lrange $guisection 0 [expr [llength $guisection]-2]]
		set parent_frame [layout_get_frame $dlg $parent_section $optn]
		set id [llength [winfo children $parent_frame]]
		set suf [frame $parent_frame.fra$id]
		pack $suf -side top -fill x
		return $suf
	}
}

# Get the frame for an option, or make it if it doesn't exist yet
proc layout_get_frame {dlg guisection optn} {
	global opt
	if {! [info exists opt($dlg,layout_frame,$guisection)] } {
		set frame [layout_make_frame $dlg $guisection $optn]
		set opt($dlg,layout_frame,$guisection) $frame
	}
	return $opt($dlg,layout_frame,$guisection)
}

proc layout_get_special_frame {dlg guisection key} {
	return [layout_get_frame $dlg $guisection -1]
}

proc layout_raise_frame {dlg guisection optn} {
	global opt
	set path $opt($dlg,path)
	if {$guisection == {}} {set guisection {{}}}
	set label [lindex $guisection 0]
	if {$label == {}} {
		set label "Options"
	}
	$path.nb raise $label
}

proc layout_raise_special_frame {dlg guisection key} {
	layout_raise_frame $dlg $guisection -1
}

# Make the layout:
proc make_layout {dlg path root} {
	# Make the tabs (notebook)
	set pw [NoteBook $path.nb -side top]
	pack $pw -fill both -expand yes
}

################################################################################
# Make widgets

proc make_module_description {dlg path root} {
	global opt

	if {$opt($dlg,label) != {}} {
		set l1 $opt($dlg,label)
		set l2 $opt($dlg,desc)
	} else {
		set l1 $opt($dlg,desc)
		set l2 {}
	}
	frame $path.module
	set icon [icon module $opt($dlg,pgm_name)]
	if {$icon != 0} {
		button $path.module.icon -relief flat -image $icon -anchor n
		pack $path.module.icon -side left		
	}
	frame $path.module.r
	label $path.module.r.labdesc1 -text $l1 -anchor w -justify left
	label $path.module.r.labdesc2 -text $l2 -anchor w -justify left
	pack $path.module.r.labdesc1 $path.module.r.labdesc2 -side top -fill x
	pack $path.module.r -side left -fill x
	pack $path.module -side top -fill x
}

proc make_command_label {dlg path root} {
	# Widget for displaying current command
	frame $path.cmd
	set cmdlabel [label $path.cmd.label -textvariable opt($dlg,cmd_string) -anchor w -justify left]
	bind $cmdlabel <Configure> "$cmdlabel configure -wraplength \[winfo width $cmdlabel\]"
	button $path.cmd.copy -text "Copy" -anchor n -command "show_cmd $dlg\nclipboard clear -displayof $cmdlabel\nclipboard append -displayof $cmdlabel \$opt($dlg,cmd_string)"
	if {[set icon [icon edit copy]] != 0} {
		$path.cmd.copy configure -image $icon
	}
	pack $path.cmd.copy -side left
	pack $cmdlabel -fill x -side top
	pack $path.cmd -expand no -fill x

	# Bindings for updating command
	bind [winfo toplevel $root] <Button> "+show_cmd $dlg"
	bind [winfo toplevel $root] <Key> "+show_cmd $dlg"
	bind [winfo toplevel $root] <ButtonRelease> "+show_cmd $dlg"
	bind [winfo toplevel $root] <KeyRelease> "+show_cmd $dlg"
}

proc make_output {dlg path root} {
	global opt

	set title "Output"
	set outpane [layout_get_special_frame $dlg [list $title] $title]

	set gronsole [Gronsole $outpane.gronsole -height 5 -width 60 -bg white]
	pack $gronsole -expand yes -fill both
	set opt($dlg,gronsole) $gronsole
}

proc make_progress {dlg path root} {
	global opt

	# Progress bar
	set opt($dlg,percent) -1
        set progress [ProgressBar $path.progress -fg green -height 20 -relief raised -maximum 100 -variable opt($dlg,percent) ]
	pack $progress -expand no -fill x
	set opt($dlg,progress) $progress
}

proc make_buttons {dlg path root} {
	global opt env
	set pgm_name $opt($dlg,pgm_name)

	button $path.run   -text Run   -command "run_cmd $dlg"
	button $path.help  -text Help  -command "help_cmd $dlg"
	button $path.clear -text Clear -command "clear_cmd $dlg"
	button $path.close -text Close -command "close_cmd $dlg"

	set opt($dlg,run_button) $path.run 

	# Turn off help button if the help file doesn't exist
	if {! [file exists $env(GISBASE)/docs/html/$pgm_name.html]} {
		$path.help configure -state disabled
	}

	pack $path.run $path.help $path.clear $path.close \
		-side left -expand yes -padx 20 -pady 5
}

proc make_dialog {dlg path root} {
	make_module_description $dlg $path $root
	make_layout $dlg $path $root
	make_command_label $dlg $path $root
}

proc make_dialog_end {dlg path root} {
	make_output $dlg $path $root
	make_progress $dlg $path $root
	make_buttons $dlg $path $root
}

proc do_button_file {dlg optn suf} {
	global opt

	button $suf.val$optn.sel -text {>} -command [list get_file $dlg $optn]
	if {[set icon [icon file open]] != 0} {
		$suf.val$optn.sel configure -image $icon
	}
	pack $suf.val$optn.sel -side left -fill x
}

proc do_button_old {dlg optn suf elem} {
	global opt
	
	button $suf.val$optn.sel -text {>} -command [list get_map $dlg $optn $elem]
	if {[set icon [icon element $elem]] != 0} {
		$suf.val$optn.sel configure -image $icon
	}
	pack $suf.val$optn.sel -side left -fill x
}

proc do_entry {dlg optn suf} {
	global opt

	Entry $suf.val$optn.val -textvariable opt($dlg,$optn,val)
	pack $suf.val$optn.val -side left -fill x -expand yes
}

proc do_label {dlg optn suf} {
	global opt

	set label $opt($dlg,$optn,label_text)
	set type $opt($dlg,$optn,type)
	set req $opt($dlg,$optn,required)

	set req [expr {$req ? "required" : "optional"}]
	set frame [frame $suf.lab$optn]
	label $frame.label -text "$label:" -anchor w -justify left
	label $frame.req -text "($type; $req)" -anchor e -justify right
	pack $frame.label -side left
	pack $frame.req -side right
	pack $frame -side top -fill x
	DynamicHelp::register $frame balloon $opt($dlg,$optn,help_text)
}

proc do_check {dlg optn suf i s} {
	global opt

	checkbutton $suf.val$optn.val$i -text $s -variable opt($dlg,$optn,val,$i) -onvalue 1 -offvalue 0
	pack $suf.val$optn.val$i -side left
	set opt($dlg,$optn,valname,$i) $s
}

proc do_combo {dlg optn suf vals} {
	global opt

	ComboBox $suf.val$optn.val -underline 0 -labelwidth 0 -width 25 -textvariable opt($dlg,$optn,val) -values $vals -helptext $opt($dlg,$optn,help_text)
	pack $suf.val$optn.val -side left
}

################################################################################
# Input clean-up and normalization

# Make guisections match up with different spacing near delimiters:
proc normalize_guisection {dlg optn} {
	global opt
	#TODO: Trim each part
	set trimmed {}
	foreach untrimmed [split $opt($dlg,$optn,guisection) ";"] {
		lappend trimmed [string trim $untrimmed]
	}
	set opt($dlg,$optn,guisection) $trimmed
}

# Pick the text to use for visible labels and balloon help.
proc choose_help_text {dlg optn} {
	global opt

	# Set label text and help text
	# Use description for label if label is absent
	set opt($dlg,$optn,label_text) $opt($dlg,$optn,label)
	set opt($dlg,$optn,help_text) $opt($dlg,$optn,desc)

	if {$opt($dlg,$optn,label_text) == {}} {
		set opt($dlg,$optn,label_text) $opt($dlg,$optn,help_text)
		set opt($dlg,$optn,help_text) {}
	}
}

################################################################################
# Options interface

proc dialog_set_command {dlg cmd} {
	global opt
	set pgm_name $opt($dlg,pgm_name)
	set nopt $opt($dlg,nopt)

	if {[lindex $cmd 0] != $pgm_name} {
		return -1
	}

	# "Parse" the command
	# Note that these commands shan't have quotes around them
	foreach argv [lrange $cmd 1 end] {
		if {[string length $argv] < 2} continue
		if {[string index $argv 0] == "-"} {
			foreach char [split [string range $argv 1 end] {}] {
				set args(-$char) 1
			}
		} else {
			set eq_idx [string first "=" $argv]
			set name [string range $argv 0 [expr $eq_idx - 1]]
			set value [string range $argv [expr $eq_idx + 1] end]
			set args($name) $value
		}
	}

	# Query the command for each part of every option
	for {set i 1} {$i <= $nopt} {incr i} {
		switch -- $opt($dlg,$i,class) {
		multi {
			set name $opt($dlg,$i,name)
			if {! [info exists args($name)] } continue
			set nmulti $opt($dlg,$i,nmulti)
			for {set j 1} {$j <= $nmulti} {incr j} {
				set opt($dlg,$i,valname,$j) [expr ([lsearch -exact $args($name) $opt($dlg,$i,valname,$j)] != -1) ? 1 : 0]
			}
		}
		opt {
			set name $opt($dlg,$i,name)
			if {! [info exists args($name)] } continue
			set opt($dlg,$i,val) $args($name)
		}
		flag {
			set name -$opt($dlg,$i,name)
			set opt($dlg,$i,val) [expr [info exists args($name)] ? 1 : 0]
		}
		}
	}

	show_cmd $dlg
	update
	return 0
}

proc dialog_get_command {dlg} {
	return [mkcmd $dlg]
}

################################################################################

proc begin_dialog {pgm optlist} {
	global opt dlg path
	incr dlg
	
	array set opts $optlist

	foreach key {label desc} {
		set opt($dlg,$key) $opts($key)
	}

	set root [expr {$path == "" ? "." : $path}]
	set opt($dlg,path) $path
	set opt($dlg,root) $root

	set opt($dlg,pgm_name) $pgm
	if {[winfo toplevel $root] == $root} {
		wm title $root $pgm
	}

	make_dialog $dlg $path $root
}

proc end_dialog {n} {
	global opt dlg

	set opt($dlg,nopt) $n

	set path $opt($dlg,path)
	set root $opt($dlg,root)

	make_dialog_end $dlg $path $root

	if {$n > 0} {
		layout_raise_frame $dlg $opt($dlg,1,guisection) 1
	}

	update

	show_cmd $dlg
}

proc add_option {optn optlist} {
	global opt dlg

	array set opts $optlist

	set opts(class) [expr {$opts(multi) && $opts(options) != {} ? "multi" : "opt"}]

	foreach key {class name type multi desc required options answer prompt label guisection} {
		set opt($dlg,$optn,$key) $opts($key)
	}

	set opt($dlg,optn_index,$opts(name)) $optn

	choose_help_text $dlg $optn

	normalize_guisection $dlg $optn

	set suf [layout_get_frame $dlg $opt($dlg,$optn,guisection) $optn]

	do_label $dlg $optn $suf
	frame $suf.val$optn

	if {$opts(options) != {}} {
		set vals [split $opts(options) ,]
		set answers [split $opts(answer) ,]
		set opt($dlg,$optn,nmulti) [llength $vals]
		if {$opts(multi)} {
			set i 1
			foreach x $vals {
				do_check $dlg $optn $suf $i $x
				if { [lsearch $answers $x] >= 0 } {
					set opt($dlg,$optn,val,$i) 1
				}
				incr i
			}
		} else {
			do_combo $dlg $optn $suf $vals
			set opt($dlg,$optn,val) $opts(answer)
		}
	} else {
		set prompt $opts(prompt)
		if {$prompt != {}} {
			if {[string match file* $prompt]} {
				do_button_file $dlg $optn $suf
			}
			if {[string match old* $prompt]} {
				set p [split $prompt ,]
				do_button_old $dlg $optn $suf [lindex $p 1]
			}
		}
		do_entry $dlg $optn $suf
		if {$opts(answer) != {}} {
			set opt($dlg,$optn,val) $opts(answer)
		}
	}

	pack $suf.val$optn -side top -fill x
	DynamicHelp::register $suf.val$optn balloon $opt($dlg,$optn,help_text)
}

proc add_flag {optn optlist} {
	global opt dlg
	
	array set opts $optlist

	set opt($dlg,$optn,class) flag

	foreach key {name desc label guisection} {
		set opt($dlg,$optn,$key) $opts($key)
	}
	set opt($dlg,$optn,val) $opts(answer)

	set opt($dlg,optn_index,-$opts(name)) $optn

	choose_help_text $dlg $optn

	normalize_guisection $dlg $optn

	set suf [layout_get_frame $dlg $opt($dlg,$optn,guisection) $optn]

	frame $suf.val$optn
	checkbutton $suf.val$optn.chk -text $opt($dlg,$optn,label_text) -variable opt($dlg,$optn,val) -onvalue 1 -offvalue 0 -anchor w
	pack $suf.val$optn.chk -side left
	pack $suf.val$optn -side top -fill x
	DynamicHelp::register $suf.val$optn balloon $opt($dlg,$optn,help_text)
}

################################################################################
