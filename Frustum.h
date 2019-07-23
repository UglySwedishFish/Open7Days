#pragma once

#include "Dependencies/DependenciesMath.h"

//Frustum, for frustum culling
//uses a geometric aproach 
//based on http://www.lighthouse3d.com/tutorials/view-frustum-culling/

namespace Open7Days {

	enum FrustumPlanes {
		Top, Bottom, Left,
		Right, Near, Far
	};

	struct FrustumAABB {
		Vector3f Min, Max; 

		FrustumAABB(Vector3f Min, Vector3f Max) : Min(Min), Max(Max) {}
	
		Vector3f GetMinPointFromNormal(Vector3f Normal) {
			
			Vector3f Result = Max; 

			for (int i = 0; i < 3; i++)
				if (Normal[i] >= 0)
					Result[i] = Min[i]; 

			return Result; 

		}

		Vector3f GetMaxPointFromNormal(Vector3f Normal) {

			Vector3f Result = Min; 

			for (int i = 0; i < 3; i++)
				if (Normal[i] >= 0)
					Result[i] = Max[i];

			return Result;


		}

	};

	struct FrustumPlane {

		float DistToOrig; 
		Vector3f Normal; 

		FrustumPlane(float DistToOrig = 0.0, Vector3f Normal = Vector3f(0.)):
			DistToOrig(DistToOrig), Normal(Normal) {}

		float DistanceToPoint(Vector3f Point) {
			return glm::dot(Normal, Point) + DistToOrig; 
		}

	};

	struct Frustum {

		FrustumPlane Planes[6] = { FrustumPlane(),FrustumPlane(),FrustumPlane(),FrustumPlane(),FrustumPlane(),FrustumPlane() };

		void Update(Matrix4f Matrix); 
		bool InFrustum(FrustumAABB Box);

	};
}
