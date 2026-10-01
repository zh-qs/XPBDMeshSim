#pragma once

#include "mat3.h"
#include "polar_decomposition/polar_decomposition_3x3.hpp"

#define EPS 1e-9

void atomicAdd3(FVector3f& x, const FVector3f& y)
{
	x[0] += y[0];
	x[1] += y[1];
	x[2] += y[2];
}

int getTetrahedronMeshIndex(int* binding, int meshIdx, size_t meshVerticesCount)
{
	if (meshIdx >= meshVerticesCount)
		return -1; // TODO probably may be deleted for optimization purposes
	return binding[meshIdx];
}

float getTetSignedVolume(const TArray<FVector3f>& tetVertices, const FIndex4i& tet)
{
	const FVector3f& x1 = tetVertices[tet[0]],
		& x2 = tetVertices[tet[1]],
		& x3 = tetVertices[tet[2]],
		& x4 = tetVertices[tet[3]];
	const mat3 X{
		x1[0] - x4[0], x2[0] - x4[0], x3[0] - x4[0],
		x1[1] - x4[1], x2[1] - x4[1], x3[1] - x4[1],
		x1[2] - x4[2], x2[2] - x4[2], x3[2] - x4[2]
	};
	return X.det();
}

float getTetVolume(const TArray<FVector3f>& tetVertices, const FIndex4i& tet)
{
	return FMath::Abs(getTetSignedVolume(tetVertices, tet)) / 6.0f;
}

void invertMasses(TArray<float>& vertMasses, size_t tetVerticesCount, int idx)
{
	if (idx >= tetVerticesCount)
		return;

	if (vertMasses[idx] != 0)
		vertMasses[idx] = 1 / vertMasses[idx];
}

void initMasses(TArray<FVector3f>& tetVertices, size_t tetVerticesCount, TArray<FIndex4i>& tetIndices, size_t tetIndicesCount, TArray<float>& vertMasses, int idx)
{
	if (idx >= tetIndicesCount)
		return;

	const FIndex4i& tet = tetIndices[idx];

	const float vol = 0.25f * getTetVolume(tetVertices, tet);
	vertMasses[tet[0]] += vol;
	vertMasses[tet[1]] += vol;
	vertMasses[tet[2]] += vol;
	vertMasses[tet[3]] += vol;
}

mat3 getVolumetricConstraintGradient(const mat3& F, const TArray<FVector3f>& initialTetVertices, const FIndex4i& tet, const mat3& Fp, const bool log)
{
	const FVector3f& ix1 = initialTetVertices[tet[0]],
		& ix2 = initialTetVertices[tet[1]],
		& ix3 = initialTetVertices[tet[2]],
		& ix4 = initialTetVertices[tet[3]];

	const mat3 IX{
		ix1[0] - ix4[0], ix2[0] - ix4[0], ix3[0] - ix4[0],
		ix1[1] - ix4[1], ix2[1] - ix4[1], ix3[1] - ix4[1],
		ix1[2] - ix4[2], ix2[2] - ix4[2], ix3[2] - ix4[2]
	};

	if (log)
	{
		const auto mat = F.adjoint().transpose() * Fp.transpose() * IX.inverse().transpose();
		UE_LOG(LogTemp, Log, TEXT("\ngetVolumetricConstraintGradient: gradV=[%.2f, %.2f, %.2f]\n"
			"                                       [%.2f, %.2f, %.2f]\n"
			"                                       [%.2f, %.2f, %.2f]"),
			mat.m[0][0], mat.m[1][0], mat.m[2][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2]);
	}
	return F.adjoint().transpose() * Fp.transpose() * IX.inverse().transpose();
}

mat3 getDeviatoricConstraintGradient(const float rS, const mat3& F, const TArray<FVector3f>& initialTetVertices, const FIndex4i& tet, const mat3& Fp, const bool log)
{
	const FVector3f& ix1 = initialTetVertices[tet[0]],
		& ix2 = initialTetVertices[tet[1]],
		& ix3 = initialTetVertices[tet[2]],
		& ix4 = initialTetVertices[tet[3]];

	const mat3 IX{
		ix1[0] - ix4[0], ix2[0] - ix4[0], ix3[0] - ix4[0],
		ix1[1] - ix4[1], ix2[1] - ix4[1], ix3[1] - ix4[1],
		ix1[2] - ix4[2], ix2[2] - ix4[2], ix3[2] - ix4[2]
	};

	if (log)
	{
		const auto mat = F * Fp.transpose() * IX.inverse().transpose() / rS;
		UE_LOG(LogTemp, Log, TEXT("\ngetDeviatoricConstraintGradient: gradD=[%.2f, %.2f, %.2f]\n"
			"                                       [%.2f, %.2f, %.2f]\n"
			"                                       [%.2f, %.2f, %.2f]"),
			mat.m[0][0], mat.m[1][0], mat.m[2][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2]);
	}
	return F * Fp.transpose() * IX.inverse().transpose() / rS;
}

float gradDot(const mat3& gradF, const TArray<FVector3f>& tetVertices, const TArray<FVector3f>& xprevs, const FIndex4i& tet, const bool log)
{
	const FVector3f grads[4] = {
		{gradF.m[0][0], gradF.m[0][1], gradF.m[0][2]},
		{gradF.m[1][0], gradF.m[1][1], gradF.m[1][2]},
		{gradF.m[2][0], gradF.m[2][1], gradF.m[2][2]},
		{(-gradF.m[0][0] - gradF.m[1][0] - gradF.m[2][0]), (-gradF.m[0][1] - gradF.m[1][1] - gradF.m[2][1]), (-gradF.m[0][2] - gradF.m[1][2] - gradF.m[2][2]) }
	};

	return grads[0].Dot(tetVertices[tet[0]] - xprevs[tet[0]])
		+ grads[1].Dot(tetVertices[tet[1]] - xprevs[tet[1]])
		+ grads[2].Dot(tetVertices[tet[2]] - xprevs[tet[2]])
		+ grads[3].Dot(tetVertices[tet[3]] - xprevs[tet[3]]);
}

mat3 getX(const TArray<FVector3f>& tetVertices, const TArray<FVector3f>& initialTetVertices, const FIndex4i& tet, const bool log)
{
	const FVector3f& x1 = tetVertices[tet[0]],
		& x2 = tetVertices[tet[1]],
		& x3 = tetVertices[tet[2]],
		& x4 = tetVertices[tet[3]];

	const mat3 X{
		x1[0] - x4[0], x2[0] - x4[0], x3[0] - x4[0],
		x1[1] - x4[1], x2[1] - x4[1], x3[1] - x4[1],
		x1[2] - x4[2], x2[2] - x4[2], x3[2] - x4[2]
	};

	if (log)
	{
		const auto mat = X;
		UE_LOG(LogTemp, Log, TEXT("\ngetX: X=[%.2f, %.2f, %.2f]\n"
			"        [%.2f, %.2f, %.2f]\n"
			"        [%.2f, %.2f, %.2f]"),
			mat.m[0][0], mat.m[1][0], mat.m[2][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2]);
	}
	return X;
}

mat3 getIX(const TArray<FVector3f>& tetVertices, const TArray<FVector3f>& initialTetVertices, const FIndex4i& tet, const bool log)
{
	const FVector3f& ix1 = initialTetVertices[tet[0]],
		& ix2 = initialTetVertices[tet[1]],
		& ix3 = initialTetVertices[tet[2]],
		& ix4 = initialTetVertices[tet[3]];

	const mat3 IX{
		ix1[0] - ix4[0], ix2[0] - ix4[0], ix3[0] - ix4[0],
		ix1[1] - ix4[1], ix2[1] - ix4[1], ix3[1] - ix4[1],
		ix1[2] - ix4[2], ix2[2] - ix4[2], ix3[2] - ix4[2]
	};

	if (log)
	{
		const auto mat = IX;
		UE_LOG(LogTemp, Log, TEXT("\ngetIX: IX=[%.2f, %.2f, %.2f]\n"
			"                [%.2f, %.2f, %.2f]\n"
			"                [%.2f, %.2f, %.2f]"),
			mat.m[0][0], mat.m[1][0], mat.m[2][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2]);
	}
	return IX;
}

mat3 getIXInv(const TArray<FVector3f>& tetVertices, const TArray<FVector3f>& initialTetVertices, const FIndex4i& tet, const bool log)
{
	const FVector3f& ix1 = initialTetVertices[tet[0]],
		& ix2 = initialTetVertices[tet[1]],
		& ix3 = initialTetVertices[tet[2]],
		& ix4 = initialTetVertices[tet[3]];

	const mat3 IX{
		ix1[0] - ix4[0], ix2[0] - ix4[0], ix3[0] - ix4[0],
		ix1[1] - ix4[1], ix2[1] - ix4[1], ix3[1] - ix4[1],
		ix1[2] - ix4[2], ix2[2] - ix4[2], ix3[2] - ix4[2]
	};

	if (log)
	{
		const auto mat = IX.inverse();
		UE_LOG(LogTemp, Log, TEXT("\ngetIXInv: IXinv=[%.2f, %.2f, %.2f]\n"
			"                [%.2f, %.2f, %.2f]\n"
			"                [%.2f, %.2f, %.2f]"),
			mat.m[0][0], mat.m[1][0], mat.m[2][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2]);
	}
	return IX.inverse();
}

mat3 getDeformationGradient(const TArray<FVector3f>& tetVertices, const TArray<FVector3f>& initialTetVertices, const FIndex4i& tet, const mat3& Fp, const bool log)
{
	const FVector3f& x1 = tetVertices[tet[0]],
		& x2 = tetVertices[tet[1]],
		& x3 = tetVertices[tet[2]],
		& x4 = tetVertices[tet[3]];

	const FVector3f& ix1 = initialTetVertices[tet[0]],
		& ix2 = initialTetVertices[tet[1]],
		& ix3 = initialTetVertices[tet[2]],
		& ix4 = initialTetVertices[tet[3]];

	const mat3 X{
		x1[0] - x4[0], x2[0] - x4[0], x3[0] - x4[0],
		x1[1] - x4[1], x2[1] - x4[1], x3[1] - x4[1],
		x1[2] - x4[2], x2[2] - x4[2], x3[2] - x4[2]
	};
	const mat3 IX{
		ix1[0] - ix4[0], ix2[0] - ix4[0], ix3[0] - ix4[0],
		ix1[1] - ix4[1], ix2[1] - ix4[1], ix3[1] - ix4[1],
		ix1[2] - ix4[2], ix2[2] - ix4[2], ix3[2] - ix4[2]
	};

	if (log)
	{
		const auto mat = X * IX.inverse() * Fp;
		UE_LOG(LogTemp, Log, TEXT("\ngetDeformationGradient: F=[%.2f, %.2f, %.2f]\n"
			"                          [%.2f, %.2f, %.2f]\n"
			"                          [%.2f, %.2f, %.2f]"),
			mat.m[0][0], mat.m[1][0], mat.m[2][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2]);
	}
	return X * IX.inverse() * Fp;
}

bool isNearlyZero(float x)
{
	return x < EPS && x > -EPS;
}

void simpleXPBDSimulationStep(
	TArray<FVector3f>& tetVertices, TArray<FVector3f>& tetVelocities, const TArray<FVector3f>& initialTetVertices, const TArray<float>& invMasses, size_t tetVerticesCount,
	const TArray<FIndex4i>& tetIndices, size_t tetIndicesCount, TArray<FVector3f>& internalForces,
	const TArray<FMatrix>& actorMatrices, const TArray<int>& coupledActorIndices, const TArray<FVector3f>& couplingVectors,
	float dt, float youngModulus, float poissonRatio, float yieldStress, float posDampingStiffness, const FVector& gravity, int numSteps, int numPosIters, const bool log,
	TArray<FMatrix>& plasticMatrices, TArray<FMatrix>& plasticMatricesVelocities, FTwoWayConstraintVertexCoupling* ccoupl, int plasticityModel)
{
	// Lame parameters
	const float l = poissonRatio * youngModulus / ((1 + poissonRatio) * (1 - 2 * poissonRatio));
	const float mu = 0.5f * youngModulus / (1 + poissonRatio);

	if (log)
	{
		UE_LOG(LogTemp, Warning, TEXT("XBPD step started"));
	}

	TArray<FVector3f> xprevs{};
	TArray<FVector2f> lambdas{};
	TArray<FMatrix> Pprevs{};
	TArray<mat3> gradsDev{}, gradsVol{};
	xprevs.SetNum(tetVerticesCount);
	lambdas.SetNum(tetIndicesCount);
	Pprevs.SetNum(tetIndicesCount);
	gradsDev.SetNum(tetIndicesCount);
	gradsVol.SetNum(tetIndicesCount);
	const float h = dt / numSteps;

	for (int i = 0; i < numSteps; ++i)
	{

		// do actual step
		for (int idx = 0; idx < tetVerticesCount; ++idx)
		{
			FVector3f& xprev = xprevs[idx];
			xprev = tetVertices[idx];
			// update velocity with external forces
			tetVelocities[idx][0] += h * static_cast<float>(gravity[0]);
			tetVelocities[idx][1] += h * static_cast<float>(gravity[1]);
			tetVelocities[idx][2] += h * static_cast<float>(gravity[2]);

			// update positions with velocity
			tetVertices[idx][0] += h * tetVelocities[idx][0];
			tetVertices[idx][1] += h * tetVelocities[idx][1];
			tetVertices[idx][2] += h * tetVelocities[idx][2];

			// reset internal forces
			internalForces[idx] = FVector3f::ZeroVector;
		}

		for (int idx = 0; idx < tetIndicesCount; ++idx)
		{
			Pprevs[idx] = plasticMatrices[idx];
		}

		for (int j = 0; j < numPosIters; ++j)
		{
			for (int idx = 0; idx < tetIndicesCount; ++idx)
			{
				// for all constraints (tetrahedra) initialize two lambdas = 0
				float& lambdaVolume = lambdas[idx][0], & lambdaDeviatoric = lambdas[idx][1];
				lambdaVolume = 0;
				lambdaDeviatoric = 0;
				const auto& tet = tetIndices[idx];

				auto& P = plasticMatrices[idx];
				const mat3 Fp{
					static_cast<float>(P.M[0][0]), static_cast<float>(P.M[1][0]), static_cast<float>(P.M[2][0]),
					static_cast<float>(P.M[0][1]), static_cast<float>(P.M[1][1]), static_cast<float>(P.M[2][1]),
					static_cast<float>(P.M[0][2]), static_cast<float>(P.M[1][2]), static_cast<float>(P.M[2][2])
				};

				if (log)
				{
					UE_LOG(LogTemp, Log, TEXT("STEP %d, dt = %f, h = %f"), i, dt, h);
					UE_LOG(LogTemp, Log, TEXT("mu = %f, l = %f"), mu, l);
					UE_LOG(LogTemp, Log, TEXT("TET %d, Inv masses: %f, %f, %f, %f"), idx, invMasses[tet[0]], invMasses[tet[1]], invMasses[tet[2]], invMasses[tet[3]]);
				}

				// for all constraints do sth
				// deviatoric constraint
				{
					const auto X = getX(tetVertices, initialTetVertices, tet, log);
					const auto IXinv = getIXInv(tetVertices, initialTetVertices, tet, log);
					const auto F = getDeformationGradient(tetVertices, initialTetVertices, tet, Fp, log);
					const auto vol = getTetVolume(initialTetVertices, tet);

					const float C = FMath::Sqrt(F.columnSqNorm(0) + F.columnSqNorm(1) + F.columnSqNorm(2));
					const float alpha = 1.0f / (mu * vol * h * h);
					const float beta = posDampingStiffness * h;// * h but we divide by h in formula for gamma;
					const float gamma = alpha * beta;
					const auto gradF = getDeviatoricConstraintGradient(C, F, initialTetVertices, tet, Fp, log);
					float w = gradF.columnSqNorm(0) * invMasses[tet[0]]
						+ gradF.columnSqNorm(1) * invMasses[tet[1]]
						+ gradF.columnSqNorm(2) * invMasses[tet[2]]
						+ gradF.colSumNorm() * invMasses[tet[3]];
					float damping = gradDot(gradF, tetVertices, xprevs, tet, log);
					const float dld = isNearlyZero(w) ? 0.0f : ((-C - alpha * lambdaDeviatoric - gamma * damping) / (w * (1 + gamma) + alpha));

					lambdaDeviatoric += dld;

					if (log)
					{
						UE_LOG(LogTemp, Log, TEXT("DEV: C=%f, w=%f, alpha=%f, dld=%f, vol=%f"), C, w, alpha, dld, vol);
						UE_LOG(LogTemp, Log, TEXT("dx1D=[%f, %f, %f]"), gradF.m[0][0] * dld * invMasses[tet[0]], gradF.m[0][1] * dld * invMasses[tet[0]], gradF.m[0][2] * dld * invMasses[tet[0]]);
					}

					const FVector3f dx1{ gradF.m[0][0] * dld * invMasses[tet[0]], gradF.m[0][1] * dld * invMasses[tet[0]], gradF.m[0][2] * dld * invMasses[tet[0]] };
					atomicAdd3(tetVertices[tet[0]], dx1);

					const FVector3f dx2{ gradF.m[1][0] * dld * invMasses[tet[1]], gradF.m[1][1] * dld * invMasses[tet[1]], gradF.m[1][2] * dld * invMasses[tet[1]] };
					atomicAdd3(tetVertices[tet[1]], dx2);

					const FVector3f dx3{ gradF.m[2][0] * dld * invMasses[tet[2]], gradF.m[2][1] * dld * invMasses[tet[2]], gradF.m[2][2] * dld * invMasses[tet[2]] };
					atomicAdd3(tetVertices[tet[2]], dx3);

					const FVector3f dx4{ (-gradF.m[0][0] - gradF.m[1][0] - gradF.m[2][0]) * dld * invMasses[tet[3]], (-gradF.m[0][1] - gradF.m[1][1] - gradF.m[2][1]) * dld * invMasses[tet[3]], (-gradF.m[0][2] - gradF.m[1][2] - gradF.m[2][2]) * dld * invMasses[tet[3]] };
					atomicAdd3(tetVertices[tet[3]], dx4);

					gradsDev[idx] = gradF;
				}

				// volumetric constraint
				{
					const auto X = getX(tetVertices, initialTetVertices, tet, log);
					const auto IXinv = getIXInv(tetVertices, initialTetVertices, tet, log);
					const auto F = getDeformationGradient(tetVertices, initialTetVertices, tet, Fp, log);
					const auto detF = F.det();
					const auto vol = getTetVolume(initialTetVertices, tet);

					const float C = detF - 1.0f - mu / l;
					const auto gradF = getVolumetricConstraintGradient(F, initialTetVertices, tet, Fp, log);
					float w = gradF.columnSqNorm(0) * invMasses[tet[0]]
						+ gradF.columnSqNorm(1) * invMasses[tet[1]]
						+ gradF.columnSqNorm(2) * invMasses[tet[2]]
						+ gradF.colSumNorm() * invMasses[tet[3]];

					const float alpha = 1.0f / (l * vol * h * h);
					const float beta = posDampingStiffness * h;// * h but we divide by h in formula for gamma;
					const float gamma = alpha * beta;
					float damping = gradDot(gradF, tetVertices, xprevs, tet, log);
					const float dlv = isNearlyZero(w) ? 0.0f : ((-C - alpha * lambdaVolume - gamma * damping) / (w * (1 + gamma) + alpha));

					lambdaVolume += dlv;

					if (log)
					{
						UE_LOG(LogTemp, Log, TEXT("VOL: C=%f, w=%f, alpha=%f, dlv=%f"), C, w, alpha, dlv);
						UE_LOG(LogTemp, Log, TEXT("dx1V=[%f, %f, %f]"), gradF.m[0][0] * dlv * invMasses[tet[0]], gradF.m[0][1] * dlv * invMasses[tet[0]], gradF.m[0][2] * dlv * invMasses[tet[0]]);
					}

					const FVector3f dx1{ gradF.m[0][0] * dlv * invMasses[tet[0]], gradF.m[0][1] * dlv * invMasses[tet[0]], gradF.m[0][2] * dlv * invMasses[tet[0]] };
					atomicAdd3(tetVertices[tet[0]], dx1);

					const FVector3f dx2{ gradF.m[1][0] * dlv * invMasses[tet[1]], gradF.m[1][1] * dlv * invMasses[tet[1]], gradF.m[1][2] * dlv * invMasses[tet[1]] };
					atomicAdd3(tetVertices[tet[1]], dx2);

					const FVector3f dx3{ gradF.m[2][0] * dlv * invMasses[tet[2]], gradF.m[2][1] * dlv * invMasses[tet[2]], gradF.m[2][2] * dlv * invMasses[tet[2]] };
					atomicAdd3(tetVertices[tet[2]], dx3);

					const FVector3f dx4{ (-gradF.m[0][0] - gradF.m[1][0] - gradF.m[2][0]) * dlv * invMasses[tet[3]], (-gradF.m[0][1] - gradF.m[1][1] - gradF.m[2][1]) * dlv * invMasses[tet[3]], (-gradF.m[0][2] - gradF.m[1][2] - gradF.m[2][2]) * dlv * invMasses[tet[3]] };
					atomicAdd3(tetVertices[tet[3]], dx4);

					gradsVol[idx] = gradF;
				}
			}

			// skeleton
			if (ccoupl != nullptr)
				ccoupl->EnforceConstraints();

			for (int idx = 0; idx < tetVerticesCount; ++idx)
			{
				// couplings
				if (coupledActorIndices[idx] >= 0)
				{
					const auto& transform = actorMatrices[coupledActorIndices[idx]];
					auto target = transform.TransformPosition(FVector{ couplingVectors[idx] });
					tetVertices[idx][0] = target.X;
					tetVertices[idx][1] = target.Y;
					tetVertices[idx][2] = target.Z;

					if (log)
					{
						UE_LOG(LogTemp, Log, TEXT("Vertex position: (%.2f  %.2f  %.2f), Actor %d position: (%.2f  %.2f  %.2f)"), target.X, target.Y, target.Z, coupledActorIndices[idx],
							transform.M[3][0], transform.M[3][1], transform.M[3][2]);
					}
				}

				// collisions
				if (tetVertices[idx][2] < 0)
					tetVertices[idx][2] = 0;
			}

			if (plasticityModel == 0)
			{
				// plasticity -- second idea
				for (int idx = 0; idx < tetIndicesCount; ++idx)
				{
					const auto& tet = tetIndices[idx];

					auto& P = plasticMatrices[idx];
					const mat3 Fp{
						static_cast<float>(P.M[0][0]), static_cast<float>(P.M[1][0]), static_cast<float>(P.M[2][0]),
						static_cast<float>(P.M[0][1]), static_cast<float>(P.M[1][1]), static_cast<float>(P.M[2][1]),
						static_cast<float>(P.M[0][2]), static_cast<float>(P.M[1][2]), static_cast<float>(P.M[2][2])
					};

					const auto X = getX(tetVertices, initialTetVertices, tet, log);
					const auto IXinv = getIXInv(tetVertices, initialTetVertices, tet, log);
					const auto IX = getIX(tetVertices, initialTetVertices, tet, log);
					const auto F = getDeformationGradient(tetVertices, initialTetVertices, tet, Fp, log);
					const auto B = F * F.transpose();
					const auto I = mat3{};
					const auto stressDeviator = mu * (B - I * (B.trace() / 3)); // note that stressDeviator is symmetric

					const auto kSq = yieldStress * yieldStress;
					const auto invMassSum = 1 / (1 / invMasses[tet[0]] + 1 / invMasses[tet[1]] + 1 / invMasses[tet[2]] + 1 / invMasses[tet[3]]);

					const auto C = stressDeviator.frobeniusNorm() - yieldStress;
					auto Fp2 = Fp;
					float w = 0;
					float dlp = 0;

					if (yieldStress > 0 && C > 0)
					{
						const auto GradF = IXinv.transpose() * X.transpose() * stressDeviator * F;

						w = (GradF.frobeniusSqNorm());

						dlp = isNearlyZero(w) ? 0.0f : ((-C) / (w));
						// lambdaPlastic is unnecessary

						const auto dFp = GradF * dlp;
						Fp2 = Fp + dFp;

						const auto detFp2 = Fp2.det();
					}
					
					// constraint det Fp = 1
					{
						const auto C2 = Fp2.det() - 1;

						const auto gradF2 = Fp2.adjoint();

						const auto w2 = gradF2.frobeniusSqNorm() * invMassSum;

						const float dlp2 = isNearlyZero(w2) ? 0.0f : ((-C2) / (w2));
						// lambdaPlastic is unnecessary

						const auto dFp2 = gradF2 * dlp2 * invMassSum;
						Fp2 = Fp2 + dFp2;

						{
							P.M[0][0] = Fp2.m[0][0]; P.M[1][0] = Fp2.m[1][0]; P.M[2][0] = Fp2.m[2][0];
							P.M[0][1] = Fp2.m[0][1]; P.M[1][1] = Fp2.m[1][1]; P.M[2][1] = Fp2.m[2][1];
							P.M[0][2] = Fp2.m[0][2]; P.M[1][2] = Fp2.m[1][2]; P.M[2][2] = Fp2.m[2][2];
						}
					}
				}
			}
			else if (plasticityModel == 1)
			{
				// plasticity -- from shape matching
				for (int idx = 0; idx < tetIndicesCount; ++idx)
				{
					const auto& tet = tetIndices[idx];

					auto& P = plasticMatrices[idx];
					const mat3 Fp{
						static_cast<float>(P.M[0][0]), static_cast<float>(P.M[1][0]), static_cast<float>(P.M[2][0]),
						static_cast<float>(P.M[0][1]), static_cast<float>(P.M[1][1]), static_cast<float>(P.M[2][1]),
						static_cast<float>(P.M[0][2]), static_cast<float>(P.M[1][2]), static_cast<float>(P.M[2][2])
					};

					const auto F = getDeformationGradient(tetVertices, initialTetVertices, tet, Fp, log);

					auto Ft = F.transpose();
					polar::Polar_decomposition<true> p{ *(reinterpret_cast<const polar::Mat3*>(&Ft)) };

					const mat3 I{};
					const auto& S = reinterpret_cast<const mat3*>(&p.matrix_S())->transpose();
					const auto stress = (S - I).frobeniusNorm();

					if (stress > yieldStress && yieldStress > 0)
					{
						auto Fp2 = Fp * (I + (h * 10) * (S - I)).inverse();
						Fp2 /= FMath::Pow(Fp2.det(), 1.0f / 3.0f);

						{
							P.M[0][0] = Fp2.m[0][0]; P.M[1][0] = Fp2.m[1][0]; P.M[2][0] = Fp2.m[2][0];
							P.M[0][1] = Fp2.m[0][1]; P.M[1][1] = Fp2.m[1][1]; P.M[2][1] = Fp2.m[2][1];
							P.M[0][2] = Fp2.m[0][2]; P.M[1][2] = Fp2.m[1][2]; P.M[2][2] = Fp2.m[2][2];
						}
					}
				}
			}
		}

		for (int idx = 0; idx < tetVerticesCount; ++idx)
		{
			// update velocities
			FVector3f& xprev = xprevs[idx];
			tetVelocities[idx][0] = (tetVertices[idx][0] - xprev[0]) / h;
			tetVelocities[idx][1] = (tetVertices[idx][1] - xprev[1]) / h;
			tetVelocities[idx][2] = (tetVertices[idx][2] - xprev[2]) / h;
		}

		for (int idx = 0; idx < tetIndicesCount; ++idx)
		{
			plasticMatricesVelocities[idx] = (plasticMatrices[idx] + Pprevs[idx] * (-1)) * (1 / h);
		}
	}
}

void getTotalVolumes(const TArray<FVector3f>& tetVertices, const TArray<FVector3f>& initialTetVertices, const TArray<FIndex4i>& tetIndices, float& initialVolume, float& currentVolume)
{
	initialVolume = 0;
	currentVolume = 0;
	for (const auto& tet : tetIndices)
	{
		initialVolume += getTetVolume(initialTetVertices, tet);
		currentVolume += getTetVolume(tetVertices, tet);
	}
}

struct ValueStatistics {
	float max = 0, min = INFINITY, average = 0, stddev = 0;
};

ValueStatistics getVolumeRatioStatistics(const TArray<FVector3f>& tetVertices, const TArray<FVector3f>& initialTetVertices, const TArray<FIndex4i>& tetIndices)
{
	ValueStatistics result;
	float avgsq = 0;
	for (const auto& tet : tetIndices)
	{
		const auto initialVolume = getTetVolume(initialTetVertices, tet),
			currentVolume = getTetVolume(tetVertices, tet),
			ratio = currentVolume / initialVolume;

		result.max = FMath::Max(result.max, ratio);
		result.min = FMath::Min(result.min, ratio);
		result.average += ratio;
		avgsq += ratio * ratio;
	}
	result.average /= tetIndices.Num();
	avgsq /= tetIndices.Num();
	result.stddev = FMath::Sqrt(avgsq - result.average * result.average);
	return result;
}

ValueStatistics getStressStatistics(const TArray<FVector3f>& tetVertices, const TArray<FVector3f>& initialTetVertices, const TArray<FIndex4i>& tetIndices, const TArray<FMatrix>& plasticMatrices, float youngModulus, float poissonRatio, int plasticityModel)
{
	ValueStatistics result;
	const float mu = 0.5f * youngModulus / (1 + poissonRatio);
	mat3 maxF, minF;
	for (int i = 0; i < tetIndices.Num(); ++i)
	{
		const auto& tet = tetIndices[i];
		const auto P = plasticMatrices[i];
		const mat3 Fp{
			static_cast<float>(P.M[0][0]), static_cast<float>(P.M[1][0]), static_cast<float>(P.M[2][0]),
					static_cast<float>(P.M[0][1]), static_cast<float>(P.M[1][1]), static_cast<float>(P.M[2][1]),
					static_cast<float>(P.M[0][2]), static_cast<float>(P.M[1][2]), static_cast<float>(P.M[2][2])
		};

		const auto F = getDeformationGradient(tetVertices, initialTetVertices, tet, Fp, false);
		float stress = 0;
		if (plasticityModel == 0)
		{
			const auto B = F * F.transpose();
			const auto I = mat3{};
			const auto stressDeviator = mu * (B - I * (B.trace() / 3));

			stress = stressDeviator.frobeniusNorm();
		}
		else if (plasticityModel == 1)
		{
			auto Ft = F.transpose();
			polar::Polar_decomposition<true> p{ *(reinterpret_cast<const polar::Mat3*>(&Ft)) };

			const mat3 I{};
			const auto& S = reinterpret_cast<const mat3*>(&p.matrix_S())->transpose();
			stress = (S - I).frobeniusNorm();
		}
		
		if (result.max > stress)
			maxF = F;
		if (result.min < stress)
			minF = F;

		result.max = FMath::Max(result.max, stress);
		result.min = FMath::Min(result.min, stress);
	}

	return result;
}