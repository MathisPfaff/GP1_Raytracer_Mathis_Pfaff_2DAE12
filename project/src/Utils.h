#pragma once
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			Vector3 vectorL = sphere.origin - ray.origin;

			float od2 = dae::Square(Vector3::Reject(vectorL, ray.direction).Magnitude());


			if (od2 > dae::Square(sphere.radius))
			{
				if (!ignoreHitRecord) hitRecord.didHit = false;
			}
			else
			{
				float t = Vector3::Dot(vectorL, ray.direction) - sqrt(dae::Square(sphere.radius) - od2);
				
				if (t < ray.min || t >= ray.max)
				{
					if (!ignoreHitRecord) hitRecord.didHit = false;
				}
				else
				{
					if (!ignoreHitRecord)
					{
						hitRecord.didHit = true;
						hitRecord.t = t;
						hitRecord.origin = ray.origin + ray.direction * hitRecord.t;
						hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
						hitRecord.materialIndex = sphere.materialIndex;
					}

					return true;
				}
				
			}

			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			Vector3 Vectorl = plane.origin - ray.origin;

			float t = Vector3::Dot(Vectorl, plane.normal) / Vector3::Dot(ray.direction, plane.normal);
			
			if(t < ray.min || t >= ray.max)
			{
				if (!ignoreHitRecord) hitRecord.didHit = false;
			}
			else
			{
				if (!ignoreHitRecord)
				{
					hitRecord.didHit = true;
					hitRecord.t = t;
					hitRecord.origin = ray.origin + ray.direction * t;
					hitRecord.normal = plane.normal;
					hitRecord.materialIndex = plane.materialIndex;
				}

				return true;
			}
			
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const float dot{ Vector3::Dot(triangle.normal, ray.direction) };

			if (triangle.cullMode == TriangleCullMode::BackFaceCulling && dot > 0) return false;
			if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && dot < 0) return false;
			if (AreEqual(dot, 0)) return false;

			const Vector3& l{ triangle.v0 - ray.origin };
			const float t{ Vector3::Dot(l, triangle.normal) / dot };
			if (t < ray.min || t > ray.max) return false;

			const Vector3& hitPoint = ray.origin + ray.direction * t;

			Vector3 e{ triangle.v1 - triangle.v0 };
			Vector3 p{ hitPoint - triangle.v0 };
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0)
			{
				return false;
			}

			e = triangle.v2 - triangle.v1;
			p = hitPoint - triangle.v1;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0)
			{
				return false;
			}

			e = triangle.v0 - triangle.v2;
			p = hitPoint - triangle.v2;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0)
			{
				return false;
			}

			hitRecord.didHit = true;
			hitRecord.t = t;
			hitRecord.materialIndex = triangle.materialIndex;
			hitRecord.origin = ray.origin + ray.direction * t;
			hitRecord.normal = triangle.normal;

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float t1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float t2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(t1, t2);
			float tmax = std::max(t1, t2);

			t1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			t2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(t1, t2));
			tmax = std::min(tmax, std::max(t1, t2));

			t1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			t2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(t1, t2));
			tmax = std::min(tmax, std::max(t1, t2));

			return tmax > 0 && tmax >= tmin;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}
			
			Triangle tempTriangle{};

			if (ignoreHitRecord) // for light rays, invert culling
			{
				switch (mesh.cullMode)
				{
				case TriangleCullMode::BackFaceCulling:
					tempTriangle.cullMode = TriangleCullMode::FrontFaceCulling;
					break;
				case TriangleCullMode::FrontFaceCulling:
					tempTriangle.cullMode = TriangleCullMode::BackFaceCulling;
					break;
				case TriangleCullMode::NoCulling:
					tempTriangle.cullMode = mesh.cullMode;
					break;
				}

			}
			else
			{
				tempTriangle.cullMode = mesh.cullMode;
			}

			for (int idx{}; idx < mesh.indices.size(); idx += 3)
			{
				HitRecord tempHR{};
				tempTriangle.v0 = mesh.transformedPositions[mesh.indices[idx]];
				tempTriangle.v1 = mesh.transformedPositions[mesh.indices[idx + 1]];
				tempTriangle.v2 = mesh.transformedPositions[mesh.indices[idx + 2]];
				tempTriangle.normal = mesh.transformedNormals[idx / 3];
				if (HitTest_Triangle(tempTriangle, ray, tempHR, ignoreHitRecord))
				{
					if (tempHR.t < hitRecord.t)
					{
						hitRecord = tempHR;
						hitRecord.materialIndex = mesh.materialIndex;
					}

				}
			}

			return hitRecord.didHit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			return light.origin - origin;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			return light.color * (light.intensity / (light.origin - target).SqrMagnitude());
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}