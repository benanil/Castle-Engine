

RWStructuredBuffer<int2> TestRW;

[numthreads(8,8, 1)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	TestRW[dispatchThreadID.x * 16 + dispatchThreadID.y] += 1;
}