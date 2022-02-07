
RWTexture2D<float4> _texture;

[numthreads(8, 8, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	_texture[groupThreadID.xy] = float4(1, 0, 0, 1);
}