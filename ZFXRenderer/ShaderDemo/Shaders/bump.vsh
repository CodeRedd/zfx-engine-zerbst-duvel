vs.1.1
dcl_position  v0
dcl_normal    v3
dcl_texcoord  v7
dcl_tangent   v8
m4x4 oPos, v0, c0

m3x3 r5.xyz, v8, c31	; rotate tangent (U) to world space
mov  r5.w, c30.w
m3x3 r7.xyz, v3, c31	; rotate vertex normal to world space
mov	 r7.w, c30.w

; build binormal vector (use crs instruction with vs.2.0+)
mul r0, r5.zxyw, -r7.yzxw;
mad r6, r5.yzxw, -r7.zxyw, -r0

; build vector from vertex to light source in model space and transform to tangent space
sub r2, c25, v0
dp3 r8.x, r5.xyz, r2
dp3 r8.y, r6.xyz, r2
dp3 r8.z, r7.xyz, r2

; normalize the vector
dp3 r8.w, r8, r8
rsq r8.w, r8.w
mul r8.xyz, r8, r8.w

;shift values to pixel shader's 0.0 to 1.0 model
mad 0D0.xyz, r8.xyz, c30.x, c30.x
mov oT0.xy, v7.xy
mov oT1.xy, v7.xy