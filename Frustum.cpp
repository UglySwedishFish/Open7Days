#include "Frustum.h"


//https://github.com/Hopson97/MineCraft-One-Week-Challenge/blob/a17cb344ebd61f0349f375a247d559e607d0dc06/Source/Maths/Frustum.cpp
void Open7Days::Frustum::Update(Matrix4f Matrix)
{

	Planes[FrustumPlanes::Left].Normal.x = Matrix[0][3] + Matrix[0][0];
	Planes[FrustumPlanes::Left].Normal.y = Matrix[1][3] + Matrix[1][0];
	Planes[FrustumPlanes::Left].Normal.z = Matrix[2][3] + Matrix[2][0];
	Planes[FrustumPlanes::Left].DistToOrig = Matrix[3][3] + Matrix[3][0];

	// right
	Planes[FrustumPlanes::Right].Normal.x = Matrix[0][3] - Matrix[0][0];
	Planes[FrustumPlanes::Right].Normal.y = Matrix[1][3] - Matrix[1][0];
	Planes[FrustumPlanes::Right].Normal.z = Matrix[2][3] - Matrix[2][0];
	Planes[FrustumPlanes::Right].DistToOrig = Matrix[3][3] - Matrix[3][0];

	// bottom
	Planes[FrustumPlanes::Bottom].Normal.x = Matrix[0][3] + Matrix[0][1];
	Planes[FrustumPlanes::Bottom].Normal.y = Matrix[1][3] + Matrix[1][1];
	Planes[FrustumPlanes::Bottom].Normal.z = Matrix[2][3] + Matrix[2][1];
	Planes[FrustumPlanes::Bottom].DistToOrig = Matrix[3][3] + Matrix[3][1];

	// top
	Planes[FrustumPlanes::Top].Normal.x = Matrix[0][3] - Matrix[0][1];
	Planes[FrustumPlanes::Top].Normal.y = Matrix[1][3] - Matrix[1][1];
	Planes[FrustumPlanes::Top].Normal.z = Matrix[2][3] - Matrix[2][1];
	Planes[FrustumPlanes::Top].DistToOrig = Matrix[3][3] - Matrix[3][1];

	// near
	Planes[FrustumPlanes::Near].Normal.x = Matrix[0][3] + Matrix[0][2];
	Planes[FrustumPlanes::Near].Normal.y = Matrix[1][3] + Matrix[1][2];
	Planes[FrustumPlanes::Near].Normal.z = Matrix[2][3] + Matrix[2][2];
	Planes[FrustumPlanes::Near].DistToOrig = Matrix[3][3] + Matrix[3][2];

	// far
	Planes[FrustumPlanes::Far].Normal.x = Matrix[0][3] - Matrix[0][2];
	Planes[FrustumPlanes::Far].Normal.y = Matrix[1][3] - Matrix[1][2];
	Planes[FrustumPlanes::Far].Normal.z = Matrix[2][3] - Matrix[2][2];
	Planes[FrustumPlanes::Far].DistToOrig = Matrix[3][3] - Matrix[3][2];

	for (auto& plane : Planes)
	{
		float length = glm::length(plane.Normal);
		plane.Normal /= length;
		plane.DistToOrig /= length;
	}


}

bool Open7Days::Frustum::InFrustum(FrustumAABB Box)
{
	bool result = true;
	for (auto& plane : Planes)
	{
		if (plane.DistanceToPoint(Box.GetMaxPointFromNormal(plane.Normal)) < 0)
		{
			return false;
		}
		else if (plane.DistanceToPoint(Box.GetMinPointFromNormal(plane.Normal)) < 0)
		{
			result = true;
		}
	}
	return result;

}
