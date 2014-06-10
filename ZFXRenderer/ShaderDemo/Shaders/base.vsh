vs.1.1
dcl_position	v0
dcl_normal		v3
dcl_texcoord	v6
dcl_tangent		v8

;transform position - we multiply row by row, assuming the transform matrix has been transposed.
dp4 oPos.x, v0, c0
dp4 oPos.y, v0, c1
dp4 oPos.z, v0, c2
dp4 oPos.w, v0, c3

;ambient light as diffuse color output
mov oD0, c4
;dot product normals and directional light vector
dp3 oD1, v3, -c20

;texture coords go to output
mov oT0, v6
mov oT1, v6