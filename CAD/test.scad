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

// Actual battery case (before beeing split in two)
dBatt = 32;
rCase = 21;
xPos = 24;
zPos = -12;
module basicCase() {
  difference() {
    // Basic case shape
    hull() {
      translate([xPos,-dBatt/2+2,zPos]) cylinder(h=77, r=rCase, center=true);
      translate([xPos, dBatt/2-2,zPos]) cylinder(h=77, r=rCase, center=true);
    }
    // Subtract bicycle frame
    translate([0,0,zPos]) cylinder(h=80, r=rPipe+0.5, center=true);
    // Subtract battery space
    translate([xPos,-dBatt/2,zPos]) scale([1.02,1.02,1.01]) lipo();
    translate([xPos, dBatt/2,zPos]) scale([1.02,1.02,1.01]) lipo();
  }
}

// End section
module endCap(){
  intersection() {
    basicCase();
    translate([start3,-50,-50+zPos]) cube(size=[30,100,100]);
  }
}

// Split box into 3 parts
width1 = rCase;
width2 = 10;

start1 = xPos-rCase;
start2 = start1 + width1;
start3 = start2 + width2;

bikeFrame();

// bike frame section
intersection() {
  basicCase();
  translate([start1,-50,-50+zPos]) cube(size=[width1,100,100]);
}

// Middle section
translate([15,0,0]) difference() {
  intersection() {
    basicCase();
    translate([start2,-50,-50+zPos]) cube(size=[width2,100,100]);
  }
  translate([start2-1,-25,-25+zPos]) cube(size=[width2+2,50,50]);
}

// Insert batteries
translate([xPos,-dBatt/2,-12]) lipo();
translate([xPos, dBatt/2,-12]) lipo();

translate([30,0,0]) difference(){
  endCap();
  translate([start3+4.5,0,zPos]) mirror([1,0,0]) pcb();
}
