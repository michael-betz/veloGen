include<common.scad>
include<pcbShell.scad>
include<lipoShell.scad>

bikeFrame();

translate([25, 0, -10]) rotate([90, 0, 90]) pcbShell();
translate([110, 0, -99]) rotate([0, -60, 0]) rotate([0, 0, 90]) lipoShell();
