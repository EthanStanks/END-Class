#include "dev_app.h"
#include "debug_renderer.h"
#include <iostream>
//TODO include debug_renderer.h and pools.h and anything else you might need here
#include "pools.h"
#include <DirectXMath.h>
#include "input.h"

#define WM_KEYDOWN 0x0100

#define RAND_FLOAT(min, max) ((float(rand()) / RAND_MAX) * (max - min) + min)
#define EnableParticles false
#define EnableGrid true
#define AmountOfAABB 3

namespace end
{
	double dt = 0.0;
	double color_timer = 0.0;
	double sorted_spawn_timer = 0.0;
	double free_spawn_timer = 0.0;

	// needed for changing line colors
	float4 gridColor = { 0, 0, 1, 1 };
	float redValue = 0;
	float blueValue = 1;
	float greenValue = 0;
	bool isRCycle = true;
	bool isGCycle = false;
	bool isBCycle = false;



	struct Particle
	{
		float3 pos;
		float3 prev_pos;
		float4 color;
		float3 velocity;
		float life;
	};
	const int16_t sortPoolSize = 256;
	sorted_pool_t<Particle, sortPoolSize> sp;

	pool_t<Particle, 1024> shared_pool;
	struct Emitter
	{
		float3 spawn_pos;
		float4 spawn_color;
		// indices into the shared_pool 
		sorted_pool_t<int16_t, 256> indices;
	};
	const int16_t freePoolSize = 1024;
	pool_t<Particle, freePoolSize> fp;
	Emitter emitters[4];

	struct AABB
	{
		float3 min, max;
		bool isSeen;
	};
	void DrawAABB(AABB drawMe)
	{
		float3 BBR = { drawMe.max.x, drawMe.min.y, drawMe.min.z };
		float3 BTR = { drawMe.max.x, drawMe.min.y, drawMe.max.z };
		float3 BTL = { drawMe.min.x, drawMe.min.y, drawMe.max.z };

		float3 TTL = { drawMe.min.x, drawMe.max.y, drawMe.max.z };
		float3 TBL = { drawMe.min.x, drawMe.max.y, drawMe.min.z };
		float3 TBR = { drawMe.max.x, drawMe.max.y, drawMe.min.z };

		float4 color;
		if (drawMe.isSeen)
			color = float4(1, 1, 1, 1);
		else color = float4(0, 0, 1, 1);

		end::debug_renderer::add_line(drawMe.min, BBR, color);
		end::debug_renderer::add_line(BBR, BTR, color);
		end::debug_renderer::add_line(BTR, BTL, color);
		end::debug_renderer::add_line(BTL, drawMe.min, color);

		end::debug_renderer::add_line(BTL, TTL, color);
		end::debug_renderer::add_line(BTR, drawMe.max, color);
		end::debug_renderer::add_line(BBR, TBR, color);
		end::debug_renderer::add_line(drawMe.min, TBL, color);

		end::debug_renderer::add_line(drawMe.max, TBR, color);
		end::debug_renderer::add_line(TBR, TBL, color);
		end::debug_renderer::add_line(TBL, TTL, color);
		end::debug_renderer::add_line(TTL, drawMe.max, color);
	}
	AABB InitializeAABB(float3 min, float3 max)
	{
		AABB returnMe;
		returnMe.min = min;
		returnMe.max = max;
		returnMe.isSeen = false;
		return returnMe;
	}
	AABB box1 = InitializeAABB({ -12, 0, -2 }, { -8, 3, -1 });
	AABB box2 = InitializeAABB({ -6, 0, 5 }, { -2, 3, 8 });
	AABB box3 = InitializeAABB({ 7.5f, 0, -7.5f }, { 8.5f, 1, -6.5f });
	AABB AABB_Array[AmountOfAABB] = { box1, box2, box3 };

	struct Frustrum
	{
		float3 FTL, FTR, FBL, FBR, NTL, NTR, NBL, NBR;
	};
	float Float3Magnitude(float3 v) // the length of the vector
	{
		return sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
	}
	float3 Float3Normalize(float3 v)
	{
		float magnitude = Float3Magnitude(v);
		//if (magnitude == 0) return float3(0, 0, 0);
		float3 vec;
		vec.x = v.x / magnitude;
		vec.y = v.y / magnitude;
		vec.z = v.z / magnitude;
		return vec;
	}
	Frustrum CalculateFrustrumPoints(float4x4 matrix, float FOV, float AspectRatio, float nearPlane, float farPlane)
	{
		float PIdiv360 = 0.00872222f;
		float y = tanf(FOV * PIdiv360);
		float x = y * AspectRatio;

		float3 forward = matrix[2].xyz;
		float3 right = matrix[0].xyz;
		float3 up = matrix[1].xyz;
		float3 position = matrix[3].xyz;

		Frustrum yodie;
		yodie.FTL = position + forward * float3(farPlane, farPlane, farPlane) - float3(farPlane, farPlane, farPlane) * right * float3(x, x, x) * float3(farPlane, farPlane, farPlane) + up * float3(y, y, y) * float3(farPlane, farPlane, farPlane);
		yodie.FTR = position + forward * float3(farPlane, farPlane, farPlane) + float3(farPlane, farPlane, farPlane) * right * float3(x, x, x) * float3(farPlane, farPlane, farPlane) + up * float3(y, y, y) * float3(farPlane, farPlane, farPlane);
		yodie.FBL = position + forward * float3(farPlane, farPlane, farPlane) - float3(farPlane, farPlane, farPlane) * right * float3(x, x, x) * float3(farPlane, farPlane, farPlane) - up * float3(y, y, y) * float3(farPlane, farPlane, farPlane);
		yodie.FBR = position + forward * float3(farPlane, farPlane, farPlane) + float3(farPlane, farPlane, farPlane) * right * float3(x, x, x) * float3(farPlane, farPlane, farPlane) - up * float3(y, y, y) * float3(farPlane, farPlane, farPlane);
		yodie.NTL = position + forward * float3(nearPlane, nearPlane, nearPlane) - float3(nearPlane, nearPlane, nearPlane) * right * float3(x, x, x) * float3(nearPlane, nearPlane, nearPlane) + up * float3(y, y, y) * float3(nearPlane, nearPlane, nearPlane);
		yodie.NTR = position + forward * float3(nearPlane, nearPlane, nearPlane) + float3(nearPlane, nearPlane, nearPlane) * right * float3(x, x, x) * float3(nearPlane, nearPlane, nearPlane) + up * float3(y, y, y) * float3(nearPlane, nearPlane, nearPlane);
		yodie.NBL = position + forward * float3(nearPlane, nearPlane, nearPlane) - float3(nearPlane, nearPlane, nearPlane) * right * float3(x, x, x) * float3(nearPlane, nearPlane, nearPlane) - up * float3(y, y, y) * float3(nearPlane, nearPlane, nearPlane);
		yodie.NBR = position + forward * float3(nearPlane, nearPlane, nearPlane) + float3(nearPlane, nearPlane, nearPlane) * right * float3(x, x, x) * float3(nearPlane, nearPlane, nearPlane) - up * float3(y, y, y) * float3(nearPlane, nearPlane, nearPlane);
		return yodie;
	}
	float Float3DotProduct(float3 v1, float3 v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}
	float3 Float3CrossProduct(float3 v1, float3 v2)
	{
		float3 crossed;
		crossed.x = v1.y * v2.z - v1.z * v2.y;
		crossed.y = v1.z * v2.x - v1.x * v2.z;
		crossed.z = v1.x * v2.y - v1.y * v2.x;
		return crossed;
	}
	float4 CalculatePlane(float3 a, float3 b, float3 c)
	{
		float3 ba = b - a;
		float3 cb = c - b; 
		float3 normXYZ = Float3CrossProduct(ba, cb);
		normXYZ = Float3Normalize(normXYZ);
		float offset = Float3DotProduct(a, normXYZ);
		return float4(normXYZ.x, normXYZ.y, normXYZ.z, offset);
	}
	void PrintPlane(float4 plane, float3 a, float3 b, float3 c, float3 d)
	{
		float planeLength = 0.50f;
		float4 color = float4(1, 1, 0, 1);
		float x = (a.x + b.x + c.x + d.x) / 4;
		float y = (a.y + b.y + c.y + d.y) / 4;
		float z = (a.z + b.z + c.z + d.z) / 4;
		float3 vLineStart = float3(x, y, z);
		float3 vLineEnd = vLineStart + plane.xyz * planeLength;
		end::debug_renderer::add_line(vLineStart, vLineEnd, color);
	}
	void DrawFrustrum(Frustrum drawMe)
	{
		float4 color = float4(1, 0, 0, 1);
		end::debug_renderer::add_line(drawMe.NBL, drawMe.NBR, color);
		end::debug_renderer::add_line(drawMe.NBR, drawMe.NTR, color);
		end::debug_renderer::add_line(drawMe.NTR, drawMe.NTL, color);
		end::debug_renderer::add_line(drawMe.NTL, drawMe.NBL, color);

		end::debug_renderer::add_line(drawMe.FBL, drawMe.FBR, color);
		end::debug_renderer::add_line(drawMe.FBR, drawMe.FTR, color);
		end::debug_renderer::add_line(drawMe.FTR, drawMe.FTL, color);
		end::debug_renderer::add_line(drawMe.FTL, drawMe.FBL, color);

		end::debug_renderer::add_line(drawMe.NBL, drawMe.FBL, color);
		end::debug_renderer::add_line(drawMe.NTL, drawMe.FTL, color);
		end::debug_renderer::add_line(drawMe.NBR, drawMe.FBR, color);
		end::debug_renderer::add_line(drawMe.NTR, drawMe.FTR, color);
	}
	Frustrum playerFrustrum;

	dev_app_t::dev_app_t()
	{
		std::cout << "Log whatever you need here.\n"; // Don’t forget to include <iostream>
	}
	double dev_app_t::get_delta_time()const
	{
		return dt;
	}
	double calc_delta_time()
	{
		static std::chrono::time_point<std::chrono::high_resolution_clock> last_time = std::chrono::high_resolution_clock::now();

		std::chrono::time_point<std::chrono::high_resolution_clock> new_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_seconds = new_time - last_time;
		last_time = new_time;

		return min(1.0 / 15.0, elapsed_seconds.count());
	}
	float dev_app_t::ClampColor(float c)
	{
		if (c < 0) return 0;
		if (c > 1) return 1;
		return c;
	}
	float4 dev_app_t::CycleColors(float& r, float& g, float& b)
	{
		if (isRCycle)
		{
			if (r >= 0.9f)
			{
				isRCycle = false;
				isGCycle = true;
			}
			r += 0.1f;
			b -= 0.1f;
		}
		else if (isGCycle)
		{
			if (g >= 0.9f)
			{
				isGCycle = false;
				isBCycle = true;
			}
			g += 0.1f;
			r -= 0.1f;
		}
		else if (isBCycle)
		{
			if (blueValue >= 0.9f)
			{
				isBCycle = false;
				isRCycle = true;
			}
			b += 0.1f;
			g -= 0.1f;
		}
		r = ClampColor(r);
		g = ClampColor(g);
		b = ClampColor(b);
		return float4(r, g, b, 1);
	}
	void dev_app_t::EnableDebugGrid(bool isEnabled)
	{
		if (isEnabled)
		{
			float x = -10;
			float z = 10;
			float normalPosition = 10;
			float lineSpace = 1;
			for (int i = 0; i < 21; ++i)
			{
				end::debug_renderer::add_line(float3(x, -0.2f, normalPosition), float3(x, -0.2f, -normalPosition), gridColor);
				x += lineSpace;
				end::debug_renderer::add_line(float3(-normalPosition, -0.2f, z), float3(normalPosition, -0.2f, z), gridColor);
				z -= lineSpace;
			}
		}
	}
	void UpdateParticlesSorted(sorted_pool_t<Particle, sortPoolSize>& pool)
	{
		for (int i = 0; i < sp.size(); ++i)
		{
			Particle& p = sp[i];
			if (p.life <= 0 || p.pos.y < 0)
			{
				pool.free(i);
				--i;
			}
			else
			{
				// update particle
				p.prev_pos = p.pos - (p.velocity * dt);
				p.pos += (p.velocity * dt);
				p.velocity += (float3(0, -9.81f, 0) * dt);
				p.life -= dt;
			}
		}
	}
	void InsertParticleSorted(sorted_pool_t<Particle, sortPoolSize>& pool)
	{
		for (int i = 0; i < 3; ++i)
		{
			int16_t index = sp.alloc();
			if (index != -1)
			{
				Particle p;
				p.pos = { 0, 1, 0 };
				p.color = { 1 , 1 , 1 , 1 };
				p.life = RAND_FLOAT(3, 5);
				p.velocity = { RAND_FLOAT(-1,1), 8, RAND_FLOAT(-1,1) };
				pool[index] = p;
			}
		}
	}
	void InsertParticlesFree(pool_t<Particle, freePoolSize>& pool, Emitter& emitter)
	{
		for (int i = 0; i < 3; ++i)
		{
			int16_t pool_index = pool.alloc();
			if (-1 != pool_index)
			{
				int16_t emitter_index = emitter.indices.alloc();
				if (-1 != emitter_index)
				{
					Particle p;
					p.pos = emitter.spawn_pos;
					p.color = emitter.spawn_color;
					p.life = RAND_FLOAT(3, 5);
					p.velocity = { RAND_FLOAT(-1,1), 8, RAND_FLOAT(-1,1) };
					pool[pool_index] = p;
					emitter.indices[emitter_index] = pool_index;
				}
				else
				{
					pool.free(pool_index);
				}
			}
		}
	}
	void UpdateParticlesFree(pool_t<Particle, freePoolSize>& pool, Emitter& emitter)
	{
		for (int i = 0; i < emitter.indices.size(); ++i)
		{
			Particle& p = pool[emitter.indices[i]];
			if (p.life <= 0 || p.pos.y < 0)
			{
				pool.free(emitter.indices[i]);
				emitter.indices.free(i);
				--i;
			}
			else
			{
				// update particle
				p.prev_pos = p.pos - (p.velocity * dt);
				p.pos += (p.velocity * dt);
				p.velocity += (float3(0, -9.81f, 0) * dt);
				p.life -= dt;
			}
		}
	}
	float4x4 InitializeIdentity()
	{
		float4x4 wm;
		for (int i = 0; i < 4; ++i)
		{
			wm[i].x = 0;
			wm[i].y = 0;
			wm[i].z = 0;
			wm[i].w = 0;
		}
		wm[0].x = 1;
		wm[1].y = 1;
		wm[2].z = 1;
		wm[3].w = 1;
		return wm;
	}
	float4x4 InitializeAtPos(float x, float y, float z, float w)
	{
		float4x4 wm = InitializeIdentity();
		wm[3].x = x;
		wm[3].y = y;
		wm[3].z = z;
		wm[3].w = w;
		return wm;
	}
	void PrintFloat4x4(float4x4 m)
	{
		std::cout << "(\t" << m[0].x << "\t" << m[0].y << "\t" << m[0].z << "\t" << m[0].w << "\t)" << std::endl;
		std::cout << "(\t" << m[1].x << "\t" << m[1].y << "\t" << m[1].z << "\t" << m[1].w << "\t)" << std::endl;
		std::cout << "(\t" << m[2].x << "\t" << m[2].y << "\t" << m[2].z << "\t" << m[2].w << "\t)" << std::endl;
		std::cout << "(\t" << m[3].x << "\t" << m[3].y << "\t" << m[3].z << "\t" << m[3].w << "\t)" << std::endl;
		std::cout << "\n" << std::endl;
	}
	float DegreeToRadian(float degree)
	{
		return degree * (3.14 / 180);
	}
	void DrawMatrix(float4x4 m)
	{
		float lineScale = 1;
		float3 directionX = m[3].xyz + m[0].xyz * lineScale;
		float3 directionY = m[3].xyz + m[1].xyz * lineScale;
		float3 directionZ = m[3].xyz + m[2].xyz * lineScale;
		end::debug_renderer::add_line(m[3].xyz, directionX, float4(1, 0, 0, 1));
		end::debug_renderer::add_line(m[3].xyz, directionY, float4(0, 1, 0, 1));
		end::debug_renderer::add_line(m[3].xyz, directionZ, float4(0, 0, 1, 1));
	}
	bool MoveInDirection(char direction, bool isForward, float4x4 & worldMatrix, double deltaTime, float moveSpeed)
	{
		if ((direction == 'x' || direction == 'X') && isForward)
		{
			worldMatrix[3].xyz += worldMatrix[0].xyz * deltaTime * moveSpeed;
			return true;
		}
		else if ((direction == 'x' || direction == 'X') && !isForward)
		{
			worldMatrix[3].xyz += (float3(-1, -1, -1) * worldMatrix[0].xyz) * deltaTime * moveSpeed;
			return true;
		}

		if ((direction == 'y' || direction == 'Y') && isForward)
		{
			worldMatrix[3].xyz += worldMatrix[1].xyz * deltaTime * moveSpeed;
			return true;
		}
		else if ((direction == 'y' || direction == 'Y') && !isForward)
		{
			worldMatrix[3].xyz += (float3(-1, -1, -1) * worldMatrix[1].xyz) * deltaTime * moveSpeed;
			return true;
		}

		if ((direction == 'z' || direction == 'Z') && isForward)
		{
			worldMatrix[3].xyz += worldMatrix[2].xyz * deltaTime * moveSpeed;
			return true;
		}
		else if ((direction == 'z' || direction == 'Z') && !isForward)
		{
			worldMatrix[3].xyz += (float3(-1, -1, -1) * worldMatrix[2].xyz) * deltaTime * moveSpeed;
			return true;
		}

		return false;
	}
	
	float4x4 XMMATRIXtoFloat4x4(DirectX::XMMATRIX m)
	{
		DirectX::XMFLOAT4X4 temp;
		DirectX::XMStoreFloat4x4(&temp, m);
		float4x4 wm;
		wm[0][0] = temp._11;
		wm[0][1] = temp._12;
		wm[0][2] = temp._13;
		wm[0][3] = temp._14;
		wm[1][0] = temp._21;
		wm[1][1] = temp._22;
		wm[1][2] = temp._23;
		wm[1][3] = temp._24;
		wm[2][0] = temp._31;
		wm[2][1] = temp._32;
		wm[2][2] = temp._33;
		wm[2][3] = temp._34;
		wm[3][0] = temp._41;
		wm[3][1] = temp._42;
		wm[3][2] = temp._43;
		wm[3][3] = temp._44;
		return wm;
	}
	DirectX::XMMATRIX Float4x4toXMMATRIX(float4x4 wm)
	{
		DirectX::XMFLOAT4X4 temp;
		temp._11 = wm[0][0];
		temp._12 = wm[0][1];
		temp._13 = wm[0][2];
		temp._14 = wm[0][3];
		temp._21 = wm[1][0];
		temp._22 = wm[1][1];
		temp._23 = wm[1][2];
		temp._24 = wm[1][3];
		temp._31 = wm[2][0];
		temp._32 = wm[2][1];
		temp._33 = wm[2][2];
		temp._34 = wm[2][3];
		temp._41 = wm[3][0];
		temp._42 = wm[3][1];
		temp._43 = wm[3][2];
		temp._44 = wm[3][3];
		DirectX::XMMATRIX m = DirectX::XMLoadFloat4x4(&temp);
		return m;
	}
	bool TurnInDirection(bool isGlobal, char axis, float4x4& worldMatrix, float angle, double deltaTime, float turnSpeed)
	{
		if (angle == 0)
			return true;
		DirectX::XMMATRIX rotationMatrix;
		bool isAxis = false;

		if (axis == 'x' || axis == 'X') // turns on the x axis
		{
			rotationMatrix = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(angle * deltaTime * turnSpeed));
			isAxis = true;
		}
		else if (axis == 'y' || axis == 'Y') // turns on the y axis
		{
			rotationMatrix = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(angle * deltaTime * turnSpeed));
			isAxis = true;
		}
		else if (axis == 'z' || axis == 'Z') // turns on the z axis
		{
			rotationMatrix = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(angle * deltaTime * turnSpeed));
			isAxis = true;
		}

		if (isAxis)
		{
			DirectX::XMMATRIX wm = Float4x4toXMMATRIX(worldMatrix);
			if (isGlobal) // global rotation
			{
				float3 pos = worldMatrix[3].xyz;
				float w = worldMatrix[3].w;
				worldMatrix = XMMATRIXtoFloat4x4(wm * rotationMatrix);
				worldMatrix[3].xyz = pos;
				worldMatrix[3].w = w;
			}
			else // local rotation
				worldMatrix = XMMATRIXtoFloat4x4(rotationMatrix * wm);
			return true;
		}

		return false;
	}
	void OrthoNormalize(float4x4& subjectMatrix, float3 worldUp)
	{
		float3 vecZ = subjectMatrix[2].xyz;
		float3 vecX = Float3CrossProduct(worldUp, vecZ);
		vecX = Float3Normalize(vecX);

		float3 vecY = Float3CrossProduct(vecZ, vecX);
		vecY = Float3Normalize(vecY);
		subjectMatrix[0].xyz = vecX;
		subjectMatrix[1].xyz = vecY;
	}
	float4x4 LookAtMatrix(float3 subjectPos, float3 targetPos, float3 worldUp)
	{
		float3 vecZ = targetPos - subjectPos;
		vecZ = Float3Normalize(vecZ);
		float4x4 newMatrix;
		newMatrix[2].xyz = vecZ;
		newMatrix[3].xyz = subjectPos;
		newMatrix[3].w = 1;
		OrthoNormalize(newMatrix, worldUp);
		return newMatrix;
		
	}
	
	float AngleBetweenTwoVectors(float3 v1, float3 v2)
	{
		float dot = Float3DotProduct(v1, v2);
		float magnitudeV1 = Float3Magnitude(v1);
		float magnitudeV2 = Float3Magnitude(v2);
		float lengths = magnitudeV1 * magnitudeV2;
		float acosMe = dot / lengths;
		return acos(acosMe);
	}
	float4x4 MatrixMulMatrix(float4x4 m1, float4x4 m2)
	{
		float4x4 m;
		m[0].x = (m1[0].x * m2[0].x) + (m1[0].y * m2[1].x) + (m1[0].z * m2[2].x) + (m1[0].w * m2[3].x);
		m[0].y = (m1[0].x * m2[0].y) + (m1[0].y * m2[1].y) + (m1[0].z * m2[2].y) + (m1[0].w * m2[3].y);
		m[0].z = (m1[0].x * m2[0].z) + (m1[0].y * m2[1].z) + (m1[0].z * m2[2].z) + (m1[0].w * m2[3].z);
		m[0].w = (m1[0].x * m2[0].w) + (m1[0].y * m2[1].w) + (m1[0].z * m2[2].w) + (m1[0].w * m2[3].w);
		m[1].x = (m1[1].x * m2[0].x) + (m1[1].y * m2[1].x) + (m1[1].z * m2[2].x) + (m1[1].w * m2[3].x);
		m[1].y = (m1[1].x * m2[0].y) + (m1[1].y * m2[1].y) + (m1[1].z * m2[2].y) + (m1[1].w * m2[3].y);
		m[1].z = (m1[1].x * m2[0].z) + (m1[1].y * m2[1].z) + (m1[1].z * m2[2].z) + (m1[1].w * m2[3].z);
		m[1].w = (m1[1].x * m2[0].w) + (m1[1].y * m2[1].w) + (m1[1].z * m2[2].w) + (m1[1].w * m2[3].w);
		m[2].x = (m1[2].x * m2[0].x) + (m1[2].y * m2[1].x) + (m1[2].z * m2[2].x) + (m1[2].w * m2[3].x);
		m[2].y = (m1[2].x * m2[0].y) + (m1[2].y * m2[1].y) + (m1[2].z * m2[2].y) + (m1[2].w * m2[3].y);
		m[2].z = (m1[2].x * m2[0].z) + (m1[2].y * m2[1].z) + (m1[2].z * m2[2].z) + (m1[2].w * m2[3].z);
		m[2].w = (m1[2].x * m2[0].w) + (m1[2].y * m2[1].w) + (m1[2].z * m2[2].w) + (m1[2].w * m2[3].w);
		m[3].x = (m1[3].x * m2[0].x) + (m1[3].y * m2[1].x) + (m1[3].z * m2[2].x) + (m1[3].w * m2[3].x);
		m[3].y = (m1[3].x * m2[0].y) + (m1[3].y * m2[1].y) + (m1[3].z * m2[2].y) + (m1[3].w * m2[3].y);
		m[3].z = (m1[3].x * m2[0].z) + (m1[3].y * m2[1].z) + (m1[3].z * m2[2].z) + (m1[3].w * m2[3].z);
		m[3].w = (m1[3].x * m2[0].w) + (m1[3].y * m2[1].w) + (m1[3].z * m2[2].w) + (m1[3].w * m2[3].w);
		return m;
	}
	void TurnToMatrix(float4x4& subjectMatrix, float4x4 targetMatrix, float deltaTime, float3 worldUp)
	{
		float3 subjectPos = subjectMatrix[3].xyz;
		float3 targetPos = targetMatrix[3].xyz;
		float3 targetDistance = Float3Normalize(targetPos - subjectMatrix[3].xyz);

		float dotY = Float3DotProduct(targetDistance, subjectMatrix[0].xyz);
		float4x4 rotY = InitializeIdentity();
		TurnInDirection(true, 'Y', rotY, dotY, deltaTime, 16);
		subjectMatrix = MatrixMulMatrix(rotY, subjectMatrix);
		
		float dotX = Float3DotProduct(targetDistance, subjectMatrix[1].xyz);
		float4x4 rotX = InitializeIdentity();
		TurnInDirection(true , 'X', rotX, -1 * dotX, deltaTime, 16);
		subjectMatrix = MatrixMulMatrix(rotX, subjectMatrix);

		OrthoNormalize(subjectMatrix, worldUp);

	}

	float4x4 wmPlayer = InitializeIdentity();
	float4x4 wmLookAt = InitializeAtPos(-5, 2, -5, 1);
	float4x4 wmTurnTo = InitializeAtPos(5, 2, 5, 1);
	
	float4x4 Float4x4AtoFloat4x4(float4x4_a f)
	{
		float4x4 m;
		m[0].xyz = f[0].xyz;
		m[0].w = f[0].w;
		m[1].xyz = f[1].xyz;
		m[1].w = f[1].w;
		m[2].xyz = f[2].xyz;
		m[2].w = f[2].w;
		m[3].xyz = f[3].xyz;
		m[3].w = f[3].w;
		return m;
	}
	void Float4x4toFloat4x4A(float4x4_a& f, float4x4 m)
	{
		f[0].xyz = m[0].xyz;
		f[0].w = m[0].w;
		f[1].xyz = m[1].xyz;
		f[1].w = m[1].w;
		f[2].xyz = m[2].xyz;
		f[2].w = m[2].w;
		f[3].xyz = m[3].xyz;
		f[3].w = m[3].w;
	}
	bool isMouseLocked = true;
	void CameraMovement(end::renderer_t* renderer)
	{
		isMouseLocked = true;
		if (end::Input::isMouseKeyPressed(Input::KEY_RIGHT_MOUSE_BUTTON))
			isMouseLocked = false;

		end::Input::MousePos current, prev;
		
		current = end::Input::RetrieveMousePos(true);
		prev = end::Input::RetrieveMousePos(false);
		if (!isMouseLocked)
		{
			int deltaX = prev.x - current.x;
			int deltaY = prev.y - current.y;
			float mouseSpeed = 0.01f;
			DirectX::XMMATRIX rx = DirectX::XMMatrixRotationX(-1 * mouseSpeed * deltaY);
			DirectX::XMMATRIX ry = DirectX::XMMatrixRotationY(-1 * mouseSpeed * deltaX);
			DirectX::XMMATRIX camera = (DirectX::XMMATRIX&)renderer->default_view.view_mat;
			camera = DirectX::XMMatrixMultiply(rx, camera);
			DirectX::XMVECTOR cameraPos = camera.r[3];
			camera.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			camera = DirectX::XMMatrixMultiply(camera, ry);
			camera.r[3] = cameraPos;
			renderer->default_view.view_mat = (float4x4_a&)camera;

			if (end::Input::isKeyboardKeyPressed((int)'W'))
			{
				float4x4 cam = Float4x4AtoFloat4x4(renderer->default_view.view_mat);
				MoveInDirection('Z', true, cam, dt, 2);
				Float4x4toFloat4x4A(renderer->default_view.view_mat, cam);

			}
			else if (end::Input::isKeyboardKeyPressed((int)'S'))
			{
				float4x4 cam = Float4x4AtoFloat4x4(renderer->default_view.view_mat);
				MoveInDirection('Z', false, cam, dt, 2);
				Float4x4toFloat4x4A(renderer->default_view.view_mat, cam);
			}
			else if (end::Input::isKeyboardKeyPressed((int)'D'))
			{
				float4x4 cam = Float4x4AtoFloat4x4(renderer->default_view.view_mat);
				MoveInDirection('X', true, cam, dt, 2);
				Float4x4toFloat4x4A(renderer->default_view.view_mat, cam);
			}
			else if (end::Input::isKeyboardKeyPressed((int)'A'))
			{
				float4x4 cam = Float4x4AtoFloat4x4(renderer->default_view.view_mat);
				MoveInDirection('X', false, cam, dt, 2);
				Float4x4toFloat4x4A(renderer->default_view.view_mat, cam);
			}
			else if (end::Input::isKeyboardKeyPressed(VK_SPACE))
			{
				float4x4 cam = Float4x4AtoFloat4x4(renderer->default_view.view_mat);
				MoveInDirection('Y', true, cam, dt, 2);
				Float4x4toFloat4x4A(renderer->default_view.view_mat, cam);
			}
			else if (end::Input::isKeyboardKeyPressed(VK_SHIFT))
			{
				float4x4 cam = Float4x4AtoFloat4x4(renderer->default_view.view_mat);
				MoveInDirection('Y', false, cam, dt, 2);
				Float4x4toFloat4x4A(renderer->default_view.view_mat, cam);
			}
		}
		end::Input::UpdateMousePos(false, current.x, current.y);
	}
	void CollectInput()
	{
		if (end::Input::isKeyboardKeyPressed(VK_UP))
			MoveInDirection('Z', true, wmPlayer, dt, 2);
		else if (end::Input::isKeyboardKeyPressed(VK_DOWN))
			MoveInDirection('Z', false, wmPlayer, dt, 2);


		if (end::Input::isKeyboardKeyPressed(VK_RIGHT))
			TurnInDirection(true, 'Y', wmPlayer, 10.0f, dt, 10);
		else if (end::Input::isKeyboardKeyPressed(VK_LEFT))
			TurnInDirection(true, 'Y', wmPlayer, -10.0f, dt, 10);
	}


	void dev_app_t::update(end::renderer_t* renderer)
	{
		dt = calc_delta_time();
		color_timer += dt;
		
		EnableDebugGrid(EnableGrid);

		if (color_timer > 0.2f)
		{
			color_timer = 0;
			// color update changes
			gridColor = CycleColors(redValue, greenValue, blueValue);
		}
		if (EnableParticles)
		{
			sorted_spawn_timer += dt;
			free_spawn_timer += dt;

			emitters[0].spawn_pos = { 5, 1, 5 };
			emitters[0].spawn_color = { 1, 0, 0, 1 };
			emitters[1].spawn_pos = { 5, 1, -5 };
			emitters[1].spawn_color = { 0, 1, 0, 1 };
			emitters[2].spawn_pos = { -5, 1, 5 };
			emitters[2].spawn_color = { 0, 0, 1, 1 };
			emitters[3].spawn_pos = { -5, 1, -5 };
			emitters[3].spawn_color = { 1, 0, 1, 1 };

			if (sorted_spawn_timer > 0.05f)
			{
				sorted_spawn_timer = 0;
				InsertParticleSorted(sp);
			}
			if (free_spawn_timer > 0.05f)
			{
				free_spawn_timer = 0;
				InsertParticlesFree(fp, emitters[0]);
				InsertParticlesFree(fp, emitters[1]);
				InsertParticlesFree(fp, emitters[2]);
				InsertParticlesFree(fp, emitters[3]);
			}
			UpdateParticlesSorted(sp);
			UpdateParticlesFree(fp, emitters[0]);
			UpdateParticlesFree(fp, emitters[1]);
			UpdateParticlesFree(fp, emitters[2]);
			UpdateParticlesFree(fp, emitters[3]);
			// spawn particles
			for (int i = 0; i < sp.size(); ++i)
			{
				end::debug_renderer::add_line(sp[i].pos, sp[i].prev_pos, sp[i].color);
			}
			for (int i = 0; i < freePoolSize; ++i)
			{
				end::debug_renderer::add_line(fp[i].pos, fp[i].prev_pos, fp[i].color);
			}
		}

		//Draw Matrices
		DrawMatrix(wmPlayer);
		DrawMatrix(wmLookAt);
		wmLookAt = LookAtMatrix(wmLookAt[3].xyz, wmPlayer[3].xyz, float3(0, 1, 0));
		DrawMatrix(wmTurnTo);
		TurnToMatrix(wmTurnTo, wmPlayer, dt, float3(0, 1, 0));


		for (int i = 0; i < AmountOfAABB; ++i)
		{
			DrawAABB(AABB_Array[i]);
		}
		
		playerFrustrum = CalculateFrustrumPoints(wmPlayer, 45.0f, 720.0 / 1280, 1.0f, 5.0f);

		float4 planeBack = CalculatePlane(playerFrustrum.NBL, playerFrustrum.NBR, playerFrustrum.NTL);
		PrintPlane(planeBack, playerFrustrum.NBL, playerFrustrum.NBR, playerFrustrum.NTL, playerFrustrum.NTR);

		float4 planeFront = CalculatePlane(playerFrustrum.FBR, playerFrustrum.FBL, playerFrustrum.FTL);
		PrintPlane(planeFront, playerFrustrum.FBR, playerFrustrum.FBL, playerFrustrum.FTL, playerFrustrum.FTR);

		float4 planeLeft= CalculatePlane(playerFrustrum.NBL, playerFrustrum.NTL, playerFrustrum.FBL);
		PrintPlane(planeLeft, playerFrustrum.NBL, playerFrustrum.NTL, playerFrustrum.FBL, playerFrustrum.FTL);

		float4 planeRight = CalculatePlane(playerFrustrum.FTR, playerFrustrum.NTR, playerFrustrum.NBR);
		PrintPlane(planeRight, playerFrustrum.FTR, playerFrustrum.NTR, playerFrustrum.NBR, playerFrustrum.FBR);

		float4 planeBottom = CalculatePlane(playerFrustrum.FBR, playerFrustrum.NBR, playerFrustrum.FBL);
		PrintPlane(planeBottom, playerFrustrum.FBR, playerFrustrum.NBR, playerFrustrum.FBL, playerFrustrum.NBL);

		float4 planeTop = CalculatePlane(playerFrustrum.NTR, playerFrustrum.FTR, playerFrustrum.FTL);
		PrintPlane(planeTop, playerFrustrum.NTR, playerFrustrum.FTR, playerFrustrum.FTL, playerFrustrum.NTL);


		DrawFrustrum(playerFrustrum);
		// moving camera
		CameraMovement(renderer);

		//// collecting input
		CollectInput();
			
	}
}
