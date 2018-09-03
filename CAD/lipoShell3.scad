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
  intersection(){
    minkowski(){
        translate([0, cylOffset, 0]) scale([1, 0.5, 1]) cylinder(h=60, r=15, center=true);
        sphere(8);
    }
    union(){
      translate([0, cylOffset, 0]) union(){
        translate([-9.5, 0, 0]) scale(1.4) cylinder(h=65.75, r=18.5/2, center=true);
        translate([ 9.5, 0, 0]) scale(1.4) cylinder(h=65.75, r=18.5/2, center=true);
      }
      translate([0, 20, 0]) cube(size=[20, 20, 100], center=true);
    }
  }
}

module lipoHalfShell(cylOffset=0, isMirror=false){
  teethOffset = isMirror ? 3 : -3;
  union(){
    difference(){
        // Outer shell
        translate([0, cylOffset, 0]) outerSkin();
      // Cut in half
      if(isMirror)
        translate([0, 50, 0]) cube(size=[100, 100, 100], center=true);
      else
        translate([0, -50, 0]) cube(size=[100, 100, 100], center=true);
      // Inner spaces
      translate([-9.5, 0, 0]) scale(1.02) lipo();
      translate([ 9.5, 0, 0]) scale(1.02) lipo();
      // Teeth holes
      translate([ 20.5, 0, -teethOffset]) rotate([90, 90, 0]) scale([1, 1, 1.2]) teeth(n=5);
      translate([-20.5, 0, -teethOffset]) rotate([90, 90, 0]) scale([1, 1, 1.2]) teeth(n=5);
    }
    // Teeth
    translate([ 20.5, 0, teethOffset]) rotate([90, 90, 0]) teeth(0.98, n=5);
    translate([-20.5, 0, teethOffset]) rotate([90, 90, 0]) teeth(0.98, n=5);
  }
}

module lipoShell(){
  difference(){
    union(){
      lipoHalfShell(-7);
      translate([0, 19, 0]) rotate([90, 0, 0]) holder();
    }
    // Bike frame cutout
    translate([0, 27, 0]) cylinder(h=200, r=rPipe, center=true);
  }
  translate([0, -20, 0]) lipoHalfShell(-7, isMirror=true);
}

intersection(){
  union(){
    lipoShell();
  }
  translate([0, 0, 30]) cube(size=[200, 200, 50], center=true);
}
