include<common.scad>
// $fn=9;

// PCB mockup
module pcb(){
    color("green") translate([-177.5, 95, -tPcb / 2 + 0.05]) import("pcb.stl");
    // pcbX = 24.2;
    // pcbY = 57.8;
    // color("green") translate([0, 0, -0.8]) cube(size=[pcbX, pcbY, 1.6], center=true);
    // translate([0, pcbY/2 - 25.6/2 + 6.3, 0.85/2]) {
    //     color("black") cube(size=[18, 25.6, 0.85], center=true);
    //     color("gray") translate([0, -3, 1]) cube(size=[15.88, 17.8, 2], center=true);
    // }
    // color("black") translate([0, -pcbY/2+6+12.6, -3-1.6]) cube(size=[12, 12, 6], center=true);
    // color("white") translate([0, -pcbY/2-16.2/2+3, -4.5]) rotate([90, 0, 0]){
    //     translate([ pcbX/2-6.2/2-6.1, 0, 0]) cube(size=[6.2, 5.8, 16.2], center=true);
    //     translate([-pcbX/2+6.2/2+5.6, 0, 0]) cube(size=[6.2, 5.8, 16.2], center=true);
    // }
    // color("blue") cube(size=[26.5, 27.65, 1.25], center=true);
}

cubeX = 35.2;
cubeY = 70.5;
tPcb = 1.7;
smoothRad = 5;
nTeethX = 3;
nTeethY = 6;
module pcbHalfShell(cubeZ=10, isMirror=false){
    module struts(){
        translate([cubeX/2, -cubeY/2+10+25, -tPcb/2])
            rotate([90, 180, 0]) linear_extrude(25) polygon([[0, 0], [2, 0], [0, 4]]);
        translate([-cubeX/2, -cubeY/2+25, -tPcb/2])
            rotate([90, 180, 180]) linear_extrude(14) polygon([[0, 0], [2, 0], [0, 4]]);
    }
    difference(){
        // SHape the outer cube
        minkowski(){
            translate([0, 0, -(cubeZ-smoothRad/2)/2]) cube(size=[cubeX-smoothRad/2, cubeY-smoothRad/2, cubeZ-smoothRad/2], center=true);
            sphere(r=smoothRad, center=true, $fn=10);
        }
        // Cut out the inner cube
        translate([0, 0, -cubeZ/2+1]) cube(size=[cubeX, cubeY, cubeZ+2], center=true);
        // Cut off the top flat
        translate([0, 0, cubeZ]) cube(size=[cubeX+50, cubeY+50, cubeZ*2], center=true);
        // Teeth holes
        translate([-cubeX/2-1.75, -3, 0]) rotate([0, 0, 90]) scale([1, 1, 1.2]) teeth(1, nTeethY);
        translate([ cubeX/2+1.75,  3, 0]) rotate([0, 0, 90]) scale([1, 1, 1.2]) teeth(1, nTeethY);
        translate([-3, -cubeY/2-1.8, 0]) scale([1, 1, 1.2]) teeth(1, nTeethX);
        translate([ 3,  cubeY/2+1.8, 0]) scale([1, 1, 1.2]) teeth(1, nTeethX);
    }
    // Holding struts
    if(isMirror){
        mirror([1, 0, 0]) struts();
    } else {
        struts();
    }
    // Teeths
    translate([-cubeX/2-1.75, 3, 0]) rotate([0, 0, 90]) teeth(0.94, nTeethY);
    translate([ cubeX/2+1.75,-3, 0]) rotate([0, 0, 90]) teeth(0.94, nTeethY);
    translate([ 3, -cubeY/2-1.8, 0]) teeth(0.94, nTeethX);
    translate([-3,  cubeY/2+1.8, 0]) teeth(0.94, nTeethX);
}

lowerShellDepth = 7;
module pcbShell(){
    difference(){
        union(){
            pcbHalfShell(lowerShellDepth);
            translate([0, 0, -15]) holder();
        }
        translate([0, 0, -rPipe-lowerShellDepth-1]) rotate([90, 90, 0]) cylinder(r=rPipe, h=90, center=true);
        // Clip the holder sticking out the top
        translate([0, 0, 1.5-lowerShellDepth]) cube(size=[20, cubeY, 3], center=true);
        translate([-0.2, -34, -3.9]) cube(size=[32.0, 10, 6.2], center=true);
    }
    translate([0, 0, 20]) rotate([0, 180, 0]) pcbHalfShell(3.5, isMirror=true);
    // PCB mockup
    // translate([0, -3, 0.8]) pcb();
}

// holder();

// Full view
// pcbShell();

// Cutting plane view

#pcb();

intersection(){
    pcbShell();
    translate([27, 0, 0]) cube(size=[50, 100, 50], center=true);
}
