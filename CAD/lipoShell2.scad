include<common.scad>
cylOffset=7;

// Lipo battery mockup
module lipo() {
  union() {
    color("green") cylinder(h=65.75, r=18.5/2, center=true);
    color("silver") translate([0,0,65.75/2]) cylinder(h=1, r=4, center=true);
  }
}

module outerSkin(){
  minkowski(){
      translate([0, cylOffset, 0]) cylinder(h=60, r=8, center=true);
      sphere(15);
  }
}

module lipoHalfShell(isMirror=false){
  difference(){
    // Outer shell
    outerSkin();
    // Cut in half
    // translate([0, -50, 0]) cube(size=[100, 100, 100], center=true);
    // Inner spaces
    scale(0.95) hull() case2();
    translate([-10, 0, 0]) lipo();
    translate([ 10, 0, 0]) lipo();
  }
}

module lipoShell(){
  difference(){
    lipoHalfShell(-7);
    // Bike frame cutout
    translate([0, 26, 0]) cylinder(h=200, r=rPipe, center=true);
  }
  // %translate([0, 0, 0]) rotate([180, 0, 0]) lipoHalfShell(-4, isMirror=true);
  // Lipo Mockup
  // translate([0, 0, -33.1]) lipo();
  // translate([0, 0,  33.1]) lipo();
}

module case1(){
  difference(){
    translate([11.1, 18, 53]) rotate([0, 90, 180]) import("./battery holder/files/18650_V2.STL");
    translate([0, 0, 53]) cube(size=[30, 40, 20], center=true);
  }
}

module case2(){
  intersection(){
    difference(){
      union() {
        translate([-10.5, 0, 0]) case1();
        translate([ 10.5, 0, 0]) case1();
      }
      translate([0, -15, 10]) cube(size=[0.5, 20, 50], center=false);
    }
    scale(0.95) outerSkin();
  }
}



intersection(){
  union(){
    case2();
    // lipoHalfShell();
  }
  // translate([0, 100, 30]) cube(size=[200, 200, 50], center=true);
}
