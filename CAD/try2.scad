$fn=50;

// Lipo battery mockup
module lipo() {
  union() {
    color("green") cylinder(h=65.75, r=18.5/2, center=true);
    color("silver") translate([0,0,65.75/2]) cylinder(h=1, r=6/2, center=true);
  }
}

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

// PCB mockup
module pcb(){
  translate([0,-24.2/2,-57.8/2]) union(){
    cube(size=[5.5,24.2,57.8]);
    translate([2,(24.2-16.5)/2,7]) cube(size=[2.6,16.5,57.8]);
    translate([-6,6.5,12.8]) cube(size=[6.1,12.5,12.5]);
    translate([-5,3,48.0]) cube(size=[5.1,16,9]);
  }
}

module lipoCase(){
  #translate([-5,0,-105]) cylinder(h=140, r=15, center=false, $fn=20);
  lipo();
  translate([0,0,-67]) lipo();
}

bikeFrame();

translate([55, 0, -65]) rotate([0, -60, 0]) lipoCase();
