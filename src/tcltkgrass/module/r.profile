interface_build {
    {r.profile} 0
    {Outputs the raster map layer values lying on user-defined line(s).}
    {entry input {Raster map to be queried:} 0 raster}
    {entry output {Text File to be written (optional):} 0 ""}
    {entry profile {Profile coordinates (east,north,east,north[,...]):} 0 +xy}
    {entry res {Resolution along profile (default = curr. reg. res.):} 0 ""}
    {entry null {Character string to represent no data value [*]:} 0 ""}
    {checkbox -i {Interactively select End-Points} "" -i}
    {checkbox -g {Output Geographic Coordinates} "" -g}
}
