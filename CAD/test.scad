$fn=50;

// Lipo battery mockup
module lipo() {
  union() {
    color("green") cylinder(h=65.75, r=18.5/2, center=true);
    color("silver") translate([0,0,65.75/2]) cylinder(h=1, r=6/2, center=true);
  }
}

// Actual battery case (before beeing split in two)
dBatt = 25;
rCase = 15.5;
module basicCase() {
  difference() {
    // Basic case shape
    union() {
      translate([27,-dBatt/2+2,-12]) cylinder(h=77, r=rCase, center=true);
      translate([27, dBatt/2-2,-12]) cylinder(h=77, r=rCase, center=true);
      translate([27+rCase/2,0,-12]) cube(size=[rCase,dBatt,77], center=true);
    }
    // Subtract bicycle frame
    translate([0,0,-12]) cylinder(h=80, r=rPipe+0.5, center=true);
    // Subtract battery space
    translate([27,-dBatt/2,-12]) scale([1.02,1.02,1.01]) lipo();
    translate([27, dBatt/2,-12]) scale([1.02,1.02,1.01]) lipo();
  }
}

// Bike frame mock-up
rPipe = 35.4/2;
color("DarkGray") union() {
  // Frame
  cylinder(h=250, r=rPipe, center=true);
  translate([0,0,-(50+rPipe)]) rotate([0,120,0]) cylinder(h=300, r=rPipe);
  translate([0,0, (50+rPipe)]) rotate([0, 70,0]) cylinder(h=300, r=rPipe);
  // Nipple
  translate([0,0, 50-15]) rotate([0, 90,0]) cylinder(h=20, r=5);
}

// half a battery case
intersection() {
  basicCase();
  translate([27-rCase,0,0]) cube(size=[2*rCase,100,100], center=true);
}

// half a battery case
translate([15,0,0]) intersection() {
  basicCase();
  translate([27+rCase,0,0]) cube(size=[2*rCase,100,100], center=true);
}

// Insert batteries
translate([27,-dBatt/2,-12]) lipo();
translate([27, dBatt/2,-12]) lipo();
