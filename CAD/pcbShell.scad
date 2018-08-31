include<common.scad>
// $fn=9;

// PCB mockup
module pcb(){
  translate([0,-24.2/2,-57.8/2]) union(){
    cube(size=[5.5,24.2,57.8]);
    translate([2,(24.2-16.5)/2,7]) cube(size=[2.6,16.5,57.8]);
    translate([-6,6.5,12.8]) cube(size=[6.1,12.5,12.5]);
    translate([-5,3,48.0]) cube(size=[5.1,16,9]);
  }
}

cubeX = 24.5;
cubeY = 64.5;
tPcb = 1.6;
smoothRad = 5;
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
            sphere(r=smoothRad, center=true);
        }
        // Cut out the inner cube
        translate([0, 0, -cubeZ/2+1]) cube(size=[cubeX, cubeY, cubeZ+2], center=true);
        // Cut off the top flat
        translate([0, 0, cubeZ]) cube(size=[cubeX+50, cubeY+50, cubeZ*2], center=true);
        // Teeth holes
        translate([-cubeX/2-1.75, -3, 0]) rotate([0, 0, 90]) scale([1, 1, 1.2]) teeth(1, 5);
        translate([ cubeX/2+1.75,  3, 0]) rotate([0, 0, 90]) scale([1, 1, 1.2]) teeth(1, 5);
        translate([-3, -cubeY/2-1.8, 0]) scale([1, 1, 1.2]) teeth(1, 2);
        translate([ 3,  cubeY/2+1.8, 0]) scale([1, 1, 1.2]) teeth(1, 2);
    }
    // Holding struts
    if(isMirror){
        mirror([1, 0, 0]) struts();
    } else {
        struts();
    }
    // Teeths
    translate([-cubeX/2-1.75, 3, 0]) rotate([0, 0, 90]) teeth(0.94, 5);
    translate([ cubeX/2+1.75,-3, 0]) rotate([0, 0, 90]) teeth(0.94, 5);
    translate([ 3, -cubeY/2-1.8, 0]) teeth(0.94, 2);
    translate([-3,  cubeY/2+1.8, 0]) teeth(0.94, 2);
}

// D shaped
module holder(){
    difference(){
        union(){
            translate([0, 0, -9]) rotate([90, 0, 0]) cylinder(r=rPipe+2, h=64, center=true);
            translate([-14,  25, 1.5]) rotate([0, -55, 0]) pin();
            translate([-14, -25, 1.5]) rotate([0, -55, 0]) pin();
            translate([ 14,  25, 1.5]) rotate([0,  55, 0]) pin();
            translate([ 14, -25, 1.5]) rotate([0,  55, 0]) pin();
        }
        translate([0, 0, -25]) cube(size=[50, 100, 50], center=true);
    }
}


module pcbShell(){
    difference(){
        union(){
            pcbHalfShell(6);
            translate([0, 0, -15]) holder();
        }
        translate([0, 0, -rPipe-6.5]) rotate([90, 90, 0]) cylinder(r=rPipe, h=90, center=true);
        // Clip of the holder sticking out the top
        translate([0, 0, 5-6]) cube(size=[16, cubeY, 10], center=true);
    }


    // translate([0, 0, 10]) rotate([0, 180, 0]) pcbHalfShell(3.5, isMirror=true);
    // PCB mockup (not very good)
    // translate([0, -3.5, 0]) rotate([0, -90, -90]) pcb();
}

// holder();
// pcbShell();


