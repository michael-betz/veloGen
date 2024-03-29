$fn=75;

// Bike frame mock-up
rPipe = 35.4 / 2;  // vertical pipe
// rPipe = 32.1 / 2; // horizontal pipe

module bikeFrame() {
  color("DarkGray") union() {
    // Frame
    cylinder(h=250, r=rPipe, center=true);
    translate([0,0,-(50+rPipe)]) rotate([0,120,0]) cylinder(h=300, r=rPipe);
    translate([0,0, (50+rPipe)]) rotate([0, 70,0]) cylinder(h=300, r=rPipe);
    // Nipple
    translate([0,0, 50-15]) rotate([0, 90,0]) cylinder(h=20, r=5);
  }
}

// To generate the interlocking teeth between half shells
module teeth(scaleF=1.0, n=20, raster=12) {
  for (i = [0:n-1]){
    xTooth = (i - (n - 1) / 2) * raster;
    translate([xTooth,0,0]) scale(scaleF) union(){
        translate([1.5, 0, 0]) cylinder(h=3, r=1, center=true);
        cube(size=[3, 2, 3], center=true);
        translate([-1.5, 0, 0]) cylinder(h=3, r=1, center=true);
    }
  }
}

module pin(){
    difference(){
        cylinder(r=2.5, h=4.5);
        // Donut
        translate([0, 0, 3.2]) rotate_extrude() translate([3, 0, 0]) circle(r=1);
        // translate([-5, 0, 6]) rotate([0, 45, 0]) cube(size=[10, 10, 10], center=true);
    }
}

// D shaped, shall conform to bike frame
module holder(r_outer){
    difference(){
        union(){
            translate([0, 0, -9]) rotate([90, 0, 0]) cylinder(r=r_outer+2, h=64, center=true);
            // translate([-15.0,  25, 0]) rotate([0, -65, 0]) pin();
            // translate([-15.0, -25, 0]) rotate([0, -65, 0]) pin();
            // translate([ 15.0,  25, 0]) rotate([0,  65, 0]) pin();
            // translate([ 15.0, -25, 0]) rotate([0,  65, 0]) pin();
        }
        translate([0, 0, -25]) cube(size=[50, 100, 50], center=true);
    }
}
