$fn=30;

// Bike frame mock-up
rPipe = 35.4/2;
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
module teeth( scaleF, n=20, raster=12 ){
  for (i = [0:n-1]){
    xTooth = (i - (n - 1) / 2) * raster;
    translate([xTooth,0,0]) scale(scaleF) union(){
        translate([1.5, 0, 0]) cylinder(h=2, r=1, center=true);
        cube(size=[3,2,2], center=true);
        translate([-1.5, 0, 0]) cylinder(h=2, r=1, center=true);
    }
  }
}

module pin(){
    difference(){
        cylinder(r=2.5, h=4.5);
        // Donut
        translate([0, 0, 3.2]) rotate_extrude() translate([3, 0, 0]) circle(r=1);
    }
}
