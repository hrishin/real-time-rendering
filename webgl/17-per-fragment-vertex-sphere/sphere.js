const VDG_PI = 3.14159265358979323846;
function makeSphere(sphereMesh, fRadius, iSlices, iStacks)
{
    // code
    drho = parseFloat(VDG_PI) / parseFloat(iStacks);
    dtheta = 2.0 * parseFloat(VDG_PI) / parseFloat(iSlices);
    ds = 1.0 / parseFloat(iSlices);
    dt = 1.0 / parseFloat(iStacks);
    t = 1.0;
    s = 0.0;
    i=0;
    j=0;
    
    sphereMesh.allocate(iSlices * iStacks * 6);
    
    for (i = 0; i < iStacks; i++)
    {
        var rho = parseFloat(i * drho);
        var srho = parseFloat(Math.sin(rho));
        var crho = parseFloat(Math.cos(rho));
        var srhodrho = parseFloat(Math.sin(rho + drho));
        var crhodrho = parseFloat(Math.cos(rho + drho));
        
        // Many sources of OpenGL sphere drawing code uses a triangle fan
        // for the caps of the sphere. This however introduces texturing
        // artifacts at the poles on some OpenGL implementations
        s = 0.0;
        
        // initialization of three 2-D arrays, two are 4 x 3 and one is 4 x 2
        var vertex=new Array(4); // 4 rows
        for(var a=0;a<4;a++)
            vertex[a]=new Array(3); // 3 columns
        var normal=new Array(4); // 4 rows
        for(var a=0;a<4;a++)
            normal[a]=new Array(3); // 3 columns
        var texture=new Array(4);
        for(var a=0;a<4;a++)
            texture[a]=new Array(2); // 3 columns

        for ( j = 0; j < iSlices; j++)
        {
            var theta = (j == iSlices) ? 0.0 : j * dtheta;
            var stheta = parseFloat(-Math.sin(theta));
            var ctheta = parseFloat(Math.cos(theta));
            
            var x = stheta * srho;
            var y = ctheta * srho;
            var z = crho;
            
            texture[0][0] = s;
            texture[0][1] = t;
            normal[0][0] = x;
            normal[0][1] = y;
            normal[0][2] = z;
            vertex[0][0] = x * fRadius;
            vertex[0][1] = y * fRadius;
            vertex[0][2] = z * fRadius;
            
            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;
            
            texture[1][0] = s;
            texture[1][1] = t - dt;
            normal[1][0] = x;
            normal[1][1] = y;
            normal[1][2] = z;
            vertex[1][0] = x * fRadius;
            vertex[1][1] = y * fRadius;
            vertex[1][2] = z * fRadius;
            
            theta = ((j+1) == iSlices) ? 0.0 : (j+1) * dtheta;
            stheta = parseFloat(-Math.sin(theta));
            ctheta = parseFloat(Math.cos(theta));
            
            x = stheta * srho;
            y = ctheta * srho;
            z = crho;
            
            s += ds;
            texture[2][0] = s;
            texture[2][1] = t;
            normal[2][0] = x;
            normal[2][1] = y;
            normal[2][2] = z;
            vertex[2][0] = x * fRadius;
            vertex[2][1] = y * fRadius;
            vertex[2][2] = z * fRadius;
            
            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;
            
            texture[3][0] = s;
            texture[3][1] = t - dt;
            normal[3][0] = x;
            normal[3][1] = y;
            normal[3][2] = z;
            vertex[3][0] = x * fRadius;
            vertex[3][1] = y * fRadius;
            vertex[3][2] = z * fRadius;
            
            sphereMesh.addTriangle(vertex, normal, texture);
            
            // Rearrange for next triangle
            vertex[0][0]=vertex[1][0];
            vertex[0][1]=vertex[1][1];
            vertex[0][2]=vertex[1][2];
            normal[0][0]=normal[1][0];
            normal[0][1]=normal[1][1];
            normal[0][2]=normal[1][2];
            texture[0][0]=texture[1][0];
            texture[0][1]=texture[1][1];
            
            vertex[1][0]=vertex[3][0];
            vertex[1][1]=vertex[3][1];
            vertex[1][2]=vertex[3][2];
            normal[1][0]=normal[3][0];
            normal[1][1]=normal[3][1];
            normal[1][2]=normal[3][2];
            texture[1][0]=texture[3][0];
            texture[1][1]=texture[3][1];
            
            sphereMesh.addTriangle(vertex, normal, texture);
        }
        t -= dt;
    }

    sphereMesh.prepareToDraw();
}
