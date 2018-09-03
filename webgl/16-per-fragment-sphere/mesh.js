// functions use without 'this' prefix are private to class.
// functions with 'this' prefix are 'public' and hence when they use
function Mesh()
{
    var elements=[];
    var verts=[];
    var norms=[];
    var texCoords=[];
    
    var numElements=0;
    var maxElements=0;
    var numVertices=0;
    
    var vbo_position=0;
    var vbo_normal=0;
    var vbo_texture=0;
    var vbo_index=0;
    var vao=0;

    this.allocate=function(numIndices)
    {
        // code
        // first cleanup, if not initially empty
        cleanupMeshData();
        
        maxElements = numIndices;
        numElements = 0;
        numVertices = 0;
        
        var iNumIndices=numIndices/3;
        
        elements = new Uint16Array(iNumIndices * 3 * 2); // 3 is x,y,z and 2 is sizeof short
        verts = new Float32Array(iNumIndices * 3 * 4); // 3 is x,y,z and 4 is sizeof float
        norms = new Float32Array(iNumIndices * 3 * 4); // 3 is x,y,z and 4 is sizeof float
        texCoords = new Float32Array(iNumIndices * 2 * 4); // 2 is s,t and 4 is sizeof float
    }

    // Add 3 vertices, 3 normal and 2 texcoords i.e. one triangle to the geometry.
    // This searches the current list for identical vertices (exactly or nearly) and
    // if one is found, it is added to the index array.
    // if not, it is added to both the index array and the vertex array.
    this.addTriangle=function(single_vertex, single_normal, single_texture)
    {
        //variable declarations
        const diff = 0.00001;
        var i, j;
        // code
        // normals should be of unit length
        normalizeVector(single_normal[0]);
        normalizeVector(single_normal[1]);
        normalizeVector(single_normal[2]);
        
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < numVertices; j++) //for the first ever iteration of 'j', numVertices will be 0 because of it's initialization in the parameterized constructor
            {
                if (isFoundIdentical(verts[j * 3], single_vertex[i][0], diff) &&
                    isFoundIdentical(verts[(j * 3) + 1], single_vertex[i][1], diff) &&
                    isFoundIdentical(verts[(j * 3) + 2], single_vertex[i][2], diff) &&
                    
                    isFoundIdentical(norms[j * 3], single_normal[i][0], diff) &&
                    isFoundIdentical(norms[(j * 3) + 1], single_normal[i][1], diff) &&
                    isFoundIdentical(norms[(j * 3) + 2], single_normal[i][2], diff) &&
                    
                    isFoundIdentical(texCoords[j * 2], single_texture[i][0], diff) &&
                    isFoundIdentical(texCoords[(j * 2) + 1], single_texture[i][1], diff))
                {
                    elements[numElements] = j;
                    numElements++;
                    break;
                }
            }
            
            //If the single vertex, normal and texture do not match with the given, then add the corressponding triangle to the end of the list
            if (j == numVertices && numVertices < maxElements && numElements < maxElements)
            {
                verts[numVertices * 3] = single_vertex[i][0];
                verts[(numVertices * 3) + 1] = single_vertex[i][1];
                verts[(numVertices * 3) + 2] = single_vertex[i][2];
                
                norms[numVertices * 3] = single_normal[i][0];
                norms[(numVertices * 3) + 1] = single_normal[i][1];
                norms[(numVertices * 3) + 2] = single_normal[i][2];
                
                texCoords[numVertices * 2] = single_texture[i][0];
                texCoords[(numVertices * 2) + 1] = single_texture[i][1];
                
                elements[numElements] = numVertices; //adding the index to the end of the list of elements/indices
                numElements++; //incrementing the 'end' of the list
                numVertices++; //incrementing coun of vertices
            }
        }
    }
    
    this.prepareToDraw=function()
    {
        // vao
        vao=gl.createVertexArray();
        gl.bindVertexArray(vao);
        
        // vbo for position
        vbo_position=gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER,vbo_position);
        gl.bufferData(gl.ARRAY_BUFFER,verts,gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_VERTEX,
                               3,
                               gl.FLOAT,
                               false,0,0);
        gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_VERTEX);
        gl.bindBuffer(gl.ARRAY_BUFFER,null);
        
        // vbo for normals
        vbo_normal=gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER,vbo_normal);
        gl.bufferData(gl.ARRAY_BUFFER,
                      norms,
                      gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_NORMAL,
                               3,
                               gl.FLOAT,
                               false,0,0);
        gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_NORMAL);
        gl.bindBuffer(gl.ARRAY_BUFFER,null);
        
        // vbo for texture
        vbo_texture=gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER,vbo_texture);
        gl.bufferData(gl.ARRAY_BUFFER,
                      texCoords,
                      gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0,
                               2, // 2 is for S,T co-ordinates in our texCoords array
                               gl.FLOAT,
                               false,0,0);
        gl.enableVertexAttribArray(WebGLMacros.VDG_ATTRIBUTE_TEXTURE0);
        gl.bindBuffer(gl.ARRAY_BUFFER,null);
        
        // vbo for index
        vbo_index=gl.createBuffer();
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,vbo_index);
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,
                      elements,
                      gl.STATIC_DRAW);
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,null);
        
        gl.bindVertexArray(null);
        
        // after sending data to GPU, now we can free our arrays
        cleanupMeshData();
    }
    
    this.draw=function()
    {
        // code
        // bind vao
        gl.bindVertexArray(vao);

        // draw

        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vbo_index);
        gl.drawElements(gl.TRIANGLES, numElements, gl.UNSIGNED_SHORT, 0);

        // unbind vao
        gl.bindVertexArray(null);
    }
    
    this.getIndexCount=function()
    {
        // code
        return(numElements);
    }
    
    this.getVertexCount=function()
    {
        // code
        return(numVertices);
    }
    
    normalizeVector=function(v)
    {
        // code
        
        // square the vector length
        var squaredVectorLength=(v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
        
        // get square root of above 'squared vector length'
        var squareRootOfSquaredVectorLength=Math.sqrt(squaredVectorLength);
        
        // scale the vector with 1/squareRootOfSquaredVectorLength
        v[0] = v[0] * 1.0/squareRootOfSquaredVectorLength;
        v[1] = v[1] * 1.0/squareRootOfSquaredVectorLength;
        v[2] = v[2] * 1.0/squareRootOfSquaredVectorLength;
    }
    
    isFoundIdentical=function(val1, val2, diff)
    {
        // code
        if(Math.abs(val1 - val2) < diff)
            return(true);
        else
            return(false);
    }
    
    cleanupMeshData=function()
    {
        // code
        if(elements!=null)
        {
            elements=null;
        }
        
        if(verts!=null)
        {
            verts=null;
        }
        
        if(norms!=null)
        {
            norms=null;
        }
        
        if(texCoords!=null)
        {
            texCoords=null;
        }
    }
    
    this.deallocate=function()
    {
        // code
        if(vao)
        {
            gl.deleteVertexArray(vao);
            vao=null;
        }
        
        if(vbo_index)
        {
            gl.deleteBuffer(vbo_index);
            vbo_index=null;
        }
        
        if(vbo_texture)
        {
            gl.deleteBuffer(vbo_texture);
            vbo_texture=null;
        }
        
        if(vbo_normal)
        {
            gl.deleteBuffer(vbo_normal);
            vbo_normal=null;
        }
        
        if(vbo_position)
        {
            gl.deleteBuffer(vbo_position);
            vbo_position=null;
        }
    }
}
