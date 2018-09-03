include<common.scad>

// Lipo battery mockup
module lipo() {
  union() {
    color("green") cylinder(h=65.75, r=18.5/2, center=true);
    color("silver") translate([0,0,65.75/2]) cylinder(h=1, r=4, center=true);
  }
}


module lipoInnerSpace(){
  scale(1.02) translate([0, 0, 33]) lipo();
  scale(1.02) translate([0, 0, -33]) lipo();
}


module lipoHalfShell(cylOffset=0, isMirror=false){
  difference(){
    // Outer shell
    minkowski(){
      translate([0, cylOffset, 0]) cylinder(h=125, r=5, center=true);
      sphere(10);
    }
    // Cut in half
    translate([0, -15, 0]) cube(size=[30, 30, 200], center=true);
    // Inner spaces
    if(isMirror){
      mirror([0, 0, 1]) lipoInnerSpace();
    } else {
      lipoInnerSpace();
    }
    // Teeth holes
    translate([-12.5, 0, -3]) rotate([90, 90, 0]) scale([1, 1, 1.2]) teeth(n=11);
    translate([ 12.5, 0,  3]) rotate([90, 90, 0]) scale([1, 1, 1.2]) teeth(n=11);
  }
  // Teeth
  translate([-12.5, 0,  3]) rotate([90, 90, 0]) teeth(0.94, n=11);
  translate([ 12.5, 0, -3]) rotate([90, 90, 0]) teeth(0.94, n=11);
}

module lipoShell(){
  difference(){
    lipoHalfShell(4);
    // Bike frame cutout
    translate([0, 28, 0]) cylinder(h=200, r=rPipe, center=true);
  }
  %translate([0, 0, 0]) rotate([180, 0, 0]) lipoHalfShell(-4, isMirror=true);
  // Lipo Mockup
  // translate([0, 0, -33.1]) lipo();
  // translate([0, 0,  33.1]) lipo();
}

lipoShell();
