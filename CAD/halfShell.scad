$fn=12;

module teeth( scaleF, endLength=140 ){
  for ( xTooth = [0:12:endLength] ){
    translate([xTooth,0,0]) scale(scaleF) union(){
        translate([1.5, 0, 0]) cylinder(h=2, r=1, center=true);
        cube(size=[3,2,2], center=true);
        translate([-1.5, 0, 0]) cylinder(h=2, r=1, center=true);
    }
  }
}


module halfShell(cubeZ=10, isMirror=false){
    cubeX = 24.5;
    cubeY = 64.5;
    tPcb = 1.6;
    smoothRad = 5;
    module struts(){
        translate([cubeX/2, -cubeY/2+10+25, -tPcb/2])
            rotate([90, 180, 0]) linear_extrude(25) polygon([[0, 0], [2, 0], [0, 2]]);
        translate([-cubeX/2, -cubeY/2+25, -tPcb/2])
            rotate([90, 180, 180]) linear_extrude(14) polygon([[0, 0], [2, 0], [0, 2]]);
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
        translate([-cubeX/2-1.75, 6-cubeY/2+2, 0]) rotate([0, 0, 90]) scale([1, 1, 1.2]) teeth(1, 50);
        translate([+cubeX/2+1.75,  -cubeY/2+2, 0]) rotate([0, 0, 90]) scale([1, 1, 1.2]) teeth(1, 60);
        translate([6-cubeX/2+3.25, -cubeY/2-1.8, 0]) scale([1, 1, 1.2]) teeth(1, 20);
        translate([ -cubeX/2+3.25,  cubeY/2+1.8, 0]) scale([1, 1, 1.2]) teeth(1, 20);
    }
    // Holding struts
    if(isMirror){
        mirror([1, 0, 0]) struts();
    } else {
        struts();
    }
    // Teeths
    translate([-cubeX/2-1.75,  -cubeY/2+2, 0]) rotate([0, 0, 90]) teeth(0.94, 60);
    translate([ cubeX/2+1.75, 6-cubeY/2+2, 0]) rotate([0, 0, 90]) teeth(0.94, 50);
    translate([-cubeX/2+3.25, -cubeY/2-1.8, 0]) teeth(0.94, 20);
    translate([6-cubeX/2+3.25, cubeY/2+1.8, 0]) teeth(0.94, 20);
}

halfShell(6);
translate([0, 0, 10]) rotate([0, 180, 0]) halfShell(3.5, isMirror=true);

