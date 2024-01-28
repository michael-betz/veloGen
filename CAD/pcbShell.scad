include<common.scad>
$fn=120;
case_poly = 120;

// PCB mockup
module pcb(){
    color("green") translate([-177.5, 95, -tPcb / 2 + 0.05]) import("vg2.stl");
    translate([0, 13.3, l_spacer - 0.35]) color("red") oled_spacer();

    translate([-27.7 / 2, -0.7, l_spacer + 0.7]) union() {
        color("blue") cube(size=[27.7, 27.8, 1.2], center=false);
        translate([0, 4.5, 1.2]) color("black") cube(size=[27.7, 19.2, 2.7 - 1.2], center=false);
    }
}

// to be placed between velogen PCB and oled PCB
l_spacer = 2.2;
module oled_spacer() {
    d_spacer = 4.5;
    l_x = 23.3;
    l_y = 23.7;
    module spacer() {
        // difference() {
        union() {
            cylinder(h=l_spacer, r=d_spacer / 2, center=true);
            translate([0, 0, l_spacer / 2]) cylinder(h=l_spacer, r=1.9 / 2, center=true);
        }
    }
    translate([-l_x / 2,  -l_y / 2, 0]) spacer();
    translate([-l_x / 2, l_y / 2, 0]) spacer();
    translate([ l_x / 2,  -l_y / 2, 0]) spacer();
    translate([ l_x / 2, l_y / 2, 0]) spacer();
    offs = 4;
    difference() {
        translate([0, -offs / 2, 0]) cube(size=[l_x + d_spacer, l_y + d_spacer - offs, l_spacer], center=true);
        translate([0, -offs / 2, 0]) cube(size=[l_x + 2.1, l_y + 2.1 - offs, l_spacer + 0.2], center=true);
    }
}


cubeX = 35.2;
cubeY = 70.5;
tPcb = 1.7;
smoothRad = 5;
nTeethX = 3;
nTeethY = 6;
rPipe = 32.1 / 2;
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
            sphere(r=smoothRad, $fn=case_poly);
        }
        // Cut out the inner cube
        translate([0, 0, -cubeZ/2+1]) cube(size=[cubeX, cubeY, cubeZ+2], center=true);
        // Cut off the top flat
        translate([0, 0, cubeZ]) cube(size=[cubeX+50, cubeY+50, cubeZ*2], center=true);
        // Teeth holes
        translate([-cubeX/2-1.75, -3, 0]) rotate([0, 0, 90]) scale([1, 1, 1.2]) teeth(1, nTeethY);
        translate([ cubeX/2+1.75,  3, 0]) rotate([0, 0, 90]) scale([1, 1, 1.2]) teeth(1, nTeethY);
        // translate([-3, -cubeY/2-1.8, 0]) scale([1, 1, 1.2]) teeth(1, nTeethX);
        translate([ 3,  cubeY/2+1.8, 0]) scale([1, 1, 1.2]) teeth(1, nTeethX);
    }
    // Holding struts
    // if(isMirror){
    //     mirror([1, 0, 0]) struts();
    // } else {
    //     struts();
    // }
    // Teeths
    translate([-cubeX/2-1.75, 3, 0]) rotate([0, 0, 90]) teeth(0.94, nTeethY);
    translate([ cubeX/2+1.75,-3, 0]) rotate([0, 0, 90]) teeth(0.94, nTeethY);
    // translate([ 3, -cubeY/2-1.8, 0]) teeth(0.94, nTeethX);
    translate([-3,  cubeY/2+1.8, 0]) teeth(0.94, nTeethX);
}

module lowerConCutout() {
    translate([-32.5 / 2, -39, -7]) cube(size=[32.5, 10, 8], center=false);
}

module upperConCutout() {
    translate([-11.7, 34, -3.9]) cube(size=[8, 10, 6.2], center=true);
    translate([11.7, 34, -3.9]) cube(size=[8, 10, 6.2], center=true);
}

lowerShellDepth = 7;
module pcbLowerShell(){
    // Lower shell
    difference(){
        union(){
            pcbHalfShell(lowerShellDepth);
            translate([0, 0, -15]) holder(rPipe + 1);
        }
        translate([0, 0, -rPipe-lowerShellDepth-1]) rotate([90, 90, 0]) cylinder(r=rPipe, h=90, center=true);
        // Clip the holder sticking out the top
        translate([0, 0, 1.5-lowerShellDepth]) cube(size=[20, cubeY, 3], center=true);

        // connector cut-outs
        lowerConCutout();
        upperConCutout();

        // Antenna cut-out
        translate([22, -14.2, 1]) cube(size=[10, 18.5, 6.2], center=true);

    }
}

module zip_tie_cutter() {
    union() {
        translate([0, 0, 6.0]) cube(size=[60, 5.5, 1.6], center=true);
        translate([0, 0, -20]) rotate([90, 0, 0]) difference(){
            cylinder(h=5.5, r=40, center=true);
            cylinder(h=5.6, r=30, center=true);
        }
    }
}

module touchy(a=7) {
    touchy_pos() {
        cube(size=[a, a, 10], center=true);
        cube(size=[a, a, 10], center=true);
        cube(size=[a, a, 10], center=true);
        cube(size=[a, a, 10], center=true);
    }
}

module touchy_pos() {
    translate([-14.3, -2.15, 0]) children(0);
    translate([-4.78, -2.15, 0]) children(1);
    translate([ 4.78, -2.15, 0]) children(2);
    translate([ 14.3, -2.15, 0]) children(3);
}

module lbl(t=">") {
    translate([0, 0, -5])
        linear_extrude(10)
            text(t, size=7, font="Noto Sans Symbols2", halign="center", valign="center");
}

module pcbUpperShell(){
    // Upper shell
    translate([0, 0, 0]) union() {
        difference() {
            rotate([0, 180, 0]) pcbHalfShell(4, isMirror=true);
            // Display cut-out
            translate([0, 13.4 + 3 / 2, 3.5]) cube(size=[29, 22 + 3, 4], center=true);
            translate([0, 15.25, 6]) cube(size=[25, 13.5, 4], center=true);

            // Antenna cut-out
            translate([22, -14.2, -1]) cube(size=[10, 18.5, 2], center=true);

            // Make lower pins shorter
            upperConCutout();

            // Zip tie notches
            translate([0,  32, 0]) zip_tie_cutter();
            translate([0, -32, 0]) zip_tie_cutter();

            // Buttons
            translate([0, 0, 0.8]) touchy(7);
            difference() {
                translate([0, 0, 11.3]) touchy(8.5);
                // translate([0, 0, 5]) touchy_pos() {
                //     translate([0, -.8, 0]) scale(1.5) lbl("🠸");
                //     lbl("✓");
                //     lbl("✗");
                //     translate([0, -.8, 0]) scale(1.5) lbl("🠺");
                // }
            }
        }
        // Antenna gap closer
        tmp = 0.85;
        translate([16.35 + 3.25, -14.2, 0.15 + tmp/2]) cube(size=[3.5, 18.5, 1.7 + tmp], center=true);
        // lower con gap closer
        translate([-32.25 / 2, -38.97, -0.74]) cube(size=[32.25, 3.75, 1.5], center=false);
    }
}

// PCB mockup
// translate([0, 0, 0]) pcb();

// Cutting plane view
// intersection(){
//     pcbUpperShell();
//     translate([0, -60, 0]) cube(size=[50, 100, 50], center=true);
// }

// export upper shell
pcbUpperShell();

// export lower shell
// translate([0, 0, -10]) pcbLowerShell();

// export spacer
// oled_spacer();
