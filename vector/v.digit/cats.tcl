# Categories

# remove all cats from list
proc clear_cats { } {
    global Cats CatsFr GVariable GWidget GBgcmd

    foreach f [ winfo children $CatsFr] {
	destroy $f
    }
}

# add category to list
proc add_cat { line field cat } {
    global Cats CatsFr GVariable GWidget GBgcmd

    set row [ frame $CatsFr.${field}_$cat -relief sunken -borderwidth 2 ]

    Label $row.field -anchor w -width 10 -text "Field: $field"
    Label $row.cat -anchor w -width 10 -text "Cat: $cat"
    Button $row.del -anchor w -text [G_msg "Delete"] -command "c_del_cat $line $field $cat" 

    pack $row.field $row.cat $row.del -side left
    pack $row -side top -fill x -expand yes -anchor n
}

# create cats window
proc mk_cats { } {
    global Cats CatsFr GVariable GWidget GBgcmd

    # Already opened
    if { [winfo exists .cats] } {
        puts "Categories already opened"
        wm deiconify .cats
        raise .cats
        return
    } 

    # Window
    set Cats [toplevel .cats]
    wm title $Cats [G_msg "Categories"]
 
    # Scrolling window
    set CatsSw [ScrolledWindow $Cats.sw -relief sunken -borderwidth 2]
    set CatsSf [ScrollableFrame $Cats.sf -width 300 -height 100]
    $CatsSw setwidget $CatsSf
    pack $CatsSw $CatsSf -fill both -expand yes
    set CatsFr [$CatsSf getframe]

    set row1 [ frame $Cats.row1 ]
    Label $row1.flab -anchor w -width 7 -text [G_msg "Field:"]
    Entry $row1.fentry -width 5 -textvariable GVariable(new_cat_field)
    Label $row1.clab -anchor w -width 10 -text [G_msg "Category:"]
    Entry $row1.centry -width 8 -textvariable GVariable(new_cat_cat)
    pack $row1.flab $row1.fentry $row1.clab $row1.centry -side left
    pack $row1 -side top -fill x -expand yes -anchor n

    set row2 [ frame $Cats.row2 ]
    checkbutton $row2.newrec -variable GVariable(new_cat_newrec) -height 1
    Label $row2.newreclab -anchor w -width 30 -text [G_msg "Insert new record to table"]
    Button $row2.new -text [G_msg "Add new"] \
        -command { c_add_cat $GVariable(new_cat_field) $GVariable(new_cat_cat) $GVariable(new_cat_newrec) }
    pack $row2.newrec $row2.newreclab $row2.new -side left
    pack $row2 -side top -fill x -expand yes -anchor n

    pack $Cats -fill both -expand yes -padx 1 -pady 1
    $Cats raise
    tkwait visibility $Cats
}



# destroy cats window
proc destroy_cats {} {
    global Cats GVariable GWidget GBgcmd

    puts "Destroy Cats"
    destroy $Cats
}
