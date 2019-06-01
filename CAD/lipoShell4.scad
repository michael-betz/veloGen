include<common.scad>
cylOffset=7;
skin_height = 64;

// Lipo battery mockup
module lipo() {
  union() {
    color("green") cylinder(h=65.75, r=18.5/2, center=true);
    color("silver") translate([0,0,65.75/2]) cylinder(h=1, r=4, center=true);
  }
}

module outerSkin(isMirror){
  intersection(){
    minkowski(){
        translate([0, cylOffset, 0]) scale([1, 0.5, 1]) cylinder(h=skin_height, r=15, center=true);
        sphere(7.5);
    }
    union(){
      translate([0, cylOffset, 0]) union(){
        translate([-9.5, 0, 0]) scale([1.5, 1.2, 1.5]) cylinder(h=65.75, r=18.5/2, center=true);
        translate([ 9.5, 0, 0]) scale([1.5, 1.2, 1.5]) cylinder(h=65.75, r=18.5/2, center=true);
      }
      translate([0, 20, 0]) cube(size=[20, 20, 100], center=true);
    }
    if (isMirror)
      union() {
        strap_height = 25.4;
        translate([0, cylOffset, 0]) scale([1, 0.5, 1]) cylinder(h=strap_height, r=23, center=true);
        translate([0, 0 ,-50 - strap_height / 2]) cube(size=[100, 100, 100], center=true);
        translate([0, 0 ,50 + strap_height / 2]) cube(size=[100, 100, 100], center=true);
      }
  }
}

// module outerSkin(){
//     minkowski(){
//       translate([0, cylOffset, 0]) union(){
//         translate([-9.5, 0, 0]) scale(0.9) cylinder(h=65.75, r=18.5/2, center=true);
//         translate([ 9.5, 0, 0]) scale(0.9) cylinder(h=65.75, r=18.5/2, center=true);
//       }
//       sphere(12);
//     }
// }

module lipoHalfShell(cylOffset=0, isMirror=false, isFilled=false){
  mirInv = isMirror ? -1 : 1;
  teethOffset = mirInv * -3;
  lipo_z_offset = 0.9;
  union(){
    difference(){
        // Outer shell
        translate([0, cylOffset, 0]) outerSkin(isMirror);
      // Cut in half
      if(isMirror)
        translate([0, 50, 0]) cube(size=[100, 100, 100], center=true);
      else
        translate([0, -50, 0]) cube(size=[100, 100, 100], center=true);
      // Inner spaces
      if (!isFilled) {
        translate([-9.5, 0, lipo_z_offset]) scale(1.02) lipo();
        translate([ 9.5, 0, -lipo_z_offset]) rotate([0, 180, 0]) scale(1.02) lipo();
      }
      if (!isFilled) {
        // Spring spaces
        translate([-7, 0, 33.7 + lipo_z_offset]) cube(size=[14, 12, 2], center=true);
        translate([ 8, 0,-33.7 - lipo_z_offset]) cube(size=[16, 12, 2], center=true);
        translate([ 9.5, 0, 35 - lipo_z_offset]) cylinder(h=3, r=15/2, center=true);
        translate([-9.5, 0, -35 + lipo_z_offset]) cylinder(h=3, r=15/2, center=true);
        // Wire tubes
        if(isMirror){
          cylinder(h=68, r=3, center=true);
        } else {
          translate([0, mirInv * 6, 2]) cylinder(h=75.5, r=1.5, center=true);
          translate([0, mirInv * 2, 0]) cylinder(h=71.2, r=3.5, center=true);
        }
      }
      // Teeth holes
      translate([ 20.5, 0, -teethOffset]) rotate([90, 90, 0]) scale([1, 1, 1.2]) teeth(n=5);
      translate([-20.5, 0, -teethOffset]) rotate([90, 90, 0]) scale([1, 1, 1.2]) teeth(n=5);
      translate([0, 0, 37.5]) rotate([90, 0, 0]) scale([1, 1, 1.2]) teeth(n=isMirror ? 3 : 2);
      translate([0, 0, -37.5]) rotate([90, 0, 0]) scale([1, 1, 1.2]) teeth(n=isMirror ? 3 : 2);
    }
    // Teeth
    translate([ 20.5, 0, teethOffset]) rotate([90, 90, 0]) teeth(0.98, n=5);
    translate([-20.5, 0, teethOffset]) rotate([90, 90, 0]) teeth(0.98, n=5);
    translate([0, 0, 37.5]) rotate([90, 0, 0]) teeth(0.98, n=isMirror ? 2 : 3);
    translate([0, 0, -37.5]) rotate([90, 0, 0]) teeth(0.98, n=isMirror ? 2 : 3);
  }
}

// D shaped, shall conform to bike frame
module holder() {
    difference() {
        translate([0, 0, -9]) rotate([90, 0, 0]) cylinder(r=rPipe + 5, h=72, center=true);
        translate([0, 0, -25]) rotate([0,  15, 0]) cube(size=[61, 100, 50], center=true);
        translate([0, 0, -25]) rotate([0, -15, 0]) cube(size=[61, 100, 50], center=true);
    }
}

module lipoShell(){
  difference(){
    union() {
      lipoHalfShell(-7);
      difference() {
        translate([0, 18, 0]) rotate([90, 0, 0]) holder();
        lipoHalfShell(-7, isFilled=true);
      }
    }
    // Bike frame cutout
    translate([0, 27, 0]) cylinder(h=200, r=rPipe, center=true);
  }
  // translate([0, 0, 0]) lipoHalfShell(-7, isMirror=true);
}

// Cut cross sections
intersection(){
  union(){
    lipoShell();
  }
  // translate([0, 0, 59.5-30]) cube(size=[200, 200, 50], center=true);
  // translate([0, 99, 0]) cube(size=[200, 200, 200], center=true);
}
