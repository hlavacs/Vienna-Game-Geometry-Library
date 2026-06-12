//
// This file is part of the Terathon Math Library, by Eric Lengyel.
// Copyright 1999-2025, Terathon Software LLC
//
// This software is distributed under the MIT License.
// Separate proprietary licenses are available from Terathon Software.
//


#ifndef TSMath_h
#define TSMath_h


#ifndef TERATHON_NO_SIMD

	#include "TSSimd.h"

#else

	#include "TSPlatform.h"

#endif


#define TERATHON_MATH 1


/// \brief Terathon math and container namespace.

namespace Terathon
{
	inline real Fabs(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecAndc(VecLoadScalar(&x), VecFloatGetMinusZero()), &result);
			return (result);

		#else

			return ((x < 0.0F) ? -x : x);

		#endif
	}

	inline real Fnabs(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecLoadScalar(&x) | VecFloatGetMinusZero(), &result);
			return (result);

		#else

			return ((x > 0.0F) ? -x : x);

		#endif
	}

	inline real Fsgn(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecFsgn(VecLoadScalar(&x)), &result);
			return (result);

		#else

			return ((x < 0.0F) ? -1.0F : ((x > 0.0F) ? 1.0F : 0.0F));

		#endif
	}

	inline real Fnsgn(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecFnsgn(VecLoadScalar(&x)), &result);
			return (result);

		#else

			return ((x < 0.0F) ? 1.0F : ((x > 0.0F) ? -1.0F : 0.0F));

		#endif
	}

	inline real NonzeroFsgn(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecNonzeroFsgn(VecLoadScalar(&x)), &result);
			return (result);

		#else

			return ((x < 0.0F) ? -1.0F : 1.0F);

		#endif
	}

	inline real NonzeroFnsgn(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecNonzeroFnsgn(VecLoadScalar(&x)), &result);
			return (result);

		#else

			return ((x < 0.0F) ? 1.0F : -1.0F);

		#endif
	}

	inline real Fmin(const real& x, const real& y)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMinScalar(VecLoadScalar(&x), VecLoadScalar(&y)), &result);
			return (result);

		#else

			return ((x < y) ? x : y);

		#endif
	}

	inline real Fmin(const real& x, const real& y, const real& z)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMinScalar(VecMinScalar(VecLoadScalar(&x), VecLoadScalar(&y)), VecLoadScalar(&z)), &result);
			return (result);

		#else

			return (Fmin(Fmin(x, y), z));

		#endif
	}

	inline real Fmin(const real& x, const real& y, const real& z, const real& w)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMinScalar(VecMinScalar(VecLoadScalar(&x), VecLoadScalar(&y)), VecMinScalar(VecLoadScalar(&z), VecLoadScalar(&w))), &result);
			return (result);

		#else

			return (Fmin(Fmin(x, y), Fmin(z, w)));

		#endif
	}

	inline real Fmax(const real& x, const real& y)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMaxScalar(VecLoadScalar(&x), VecLoadScalar(&y)), &result);
			return (result);

		#else

			return ((x < y) ? y : x);

		#endif
	}

	inline real Fmax(const real& x, const real& y, const real& z)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMaxScalar(VecMaxScalar(VecLoadScalar(&x), VecLoadScalar(&y)), VecLoadScalar(&z)), &result);
			return (result);

		#else

			return (Fmax(Fmax(x, y), z));

		#endif
	}

	inline real Fmax(const real& x, const real& y, const real& z, const real& w)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMaxScalar(VecMaxScalar(VecLoadScalar(&x), VecLoadScalar(&y)), VecMaxScalar(VecLoadScalar(&z), VecLoadScalar(&w))), &result);
			return (result);

		#else

			return (Fmax(Fmax(x, y), Fmax(z, w)));

		#endif
	}

	inline real FminZero(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMinScalar(VecLoadScalar(&x), VecFloatGetZero()), &result);
			return (result);

		#else

			return ((x < 0.0F) ? x : 0.0F);

		#endif
	}

	inline real FmaxZero(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMaxScalar(VecLoadScalar(&x), VecFloatGetZero()), &result);
			return (result);

		#else

			return ((x < 0.0F) ? 0.0F : x);

		#endif
	}

	inline real Saturate(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMinScalar(VecMaxScalar(VecLoadScalar(&x), VecFloatGetZero()), VecLoadVectorConstant<0x3F800000>()), &result);
			return (result);

		#else

			real f = (x < 0.0F) ? 0.0F : x;
			return ((f < 1.0F) ? f : 1.0F);

		#endif
	}

	inline real Clamp(const real& x, const real& y, const real& z)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecMinScalar(VecMaxScalar(VecLoadScalar(&x), VecLoadScalar(&y)), VecLoadScalar(&z)), &result);
			return (result);

		#else

			real f = (x < y) ? y : x;
			return ((f < z) ? f : z);

		#endif
	}

	#ifndef TERATHON_NO_SIMD

		inline real Floor(const real& x)
		{
			real	result;

			VecStoreX(VecFloorScalar(VecLoadScalar(&x)), &result);
			return (result);
		}

		inline real Ceil(const real& x)
		{
			real	result;

			VecStoreX(VecCeilScalar(VecLoadScalar(&x)), &result);
			return (result);
		}

	#else

		TERATHON_API real Floor(const real& x);
		TERATHON_API real Ceil(const real& x);

	#endif

	inline real PositiveFloor(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecPositiveFloorScalar(VecLoadScalar(&x)), &result);
			return (result);

		#else

			return (Floor(x));

		#endif
	}

	inline real NegativeFloor(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecNegativeFloorScalar(VecLoadScalar(&x)), &result);
			return (result);

		#else

			return (Floor(x));

		#endif
	}

	inline real PositiveCeil(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecPositiveCeilScalar(VecLoadScalar(&x)), &result);
			return (result);

		#else

			return (Ceil(x));

		#endif
	}

	inline real NegativeCeil(const real& x)
	{
		#ifndef TERATHON_NO_SIMD

			real	result;

			VecStoreX(VecNegativeCeilScalar(VecLoadScalar(&x)), &result);
			return (result);

		#else

			return (Ceil(x));

		#endif
	}

	inline void FloorCeil(const real& x, real *f, real *c)
	{
		#ifndef TERATHON_NO_SIMD

			vec_float		vf, vc;

			VecFloorCeilScalar(VecLoadScalar(&x), &vf, &vc);
			VecStoreX(vf, f);
			VecStoreX(vc, c);

		#else

			*f = Floor(x);
			*c = Ceil(x);

		#endif
	}

	inline void PositiveFloorCeil(const real& x, real *f, real *c)
	{
		#ifndef TERATHON_NO_SIMD

			vec_float		vf, vc;

			VecPositiveFloorCeilScalar(VecLoadScalar(&x), &vf, &vc);
			VecStoreX(vf, f);
			VecStoreX(vc, c);

		#else

			*f = Floor(x);
			*c = Ceil(x);

		#endif
	}

	inline void NegativeFloorCeil(const real& x, real *f, real *c)
	{
		#ifndef TERATHON_NO_SIMD

			vec_float		vf, vc;

			VecNegativeFloorCeilScalar(VecLoadScalar(&x), &vf, &vc);
			VecStoreX(vf, f);
			VecStoreX(vc, c);

		#else

			*f = Floor(x);
			*c = Ceil(x);

		#endif
	}

	inline real Frac(const real& x)
	{
		return (x - Floor(x));
	}

	inline real PositiveFrac(const real& x)
	{
		return (x - PositiveFloor(x));
	}

	inline real NegativeFrac(const real& x)
	{
		return (x - NegativeFloor(x));
	}


	TERATHON_API real Sqrt(real x);
	TERATHON_API real InverseSqrt(real x);

	TERATHON_API real Sin(real x);
	TERATHON_API real Cos(real x);
	TERATHON_API real Tan(real x);
	TERATHON_API void CosSin(real x, real *c, real *s);

	TERATHON_API real Arcsin(real y);
	TERATHON_API real Arccos(real x);
	TERATHON_API real Arctan(real x);
	TERATHON_API real Arctan(real y, real x);

	TERATHON_API real Exp(real x);
	TERATHON_API real Log(real x);

	inline real Pow(real base, real exponent)
	{
		return (Exp(Log(base) * exponent));
	}


	namespace Math
	{
		TERATHON_API extern const real infinity;
		TERATHON_API extern const real minus_infinity;
		TERATHON_API extern const real min_float;
		TERATHON_API extern const real max_float;

		TERATHON_API extern const real tau;
		TERATHON_API extern const real two_tau;
		TERATHON_API extern const real three_tau_over_4;
		TERATHON_API extern const real three_tau_over_8;
		TERATHON_API extern const real tau_over_2;
		TERATHON_API extern const real tau_over_3;
		TERATHON_API extern const real two_tau_over_3;
		TERATHON_API extern const real tau_over_4;
		TERATHON_API extern const real tau_over_6;
		TERATHON_API extern const real tau_over_8;
		TERATHON_API extern const real tau_over_12;
		TERATHON_API extern const real tau_over_16;
		TERATHON_API extern const real tau_over_24;
		TERATHON_API extern const real tau_over_40;
		TERATHON_API extern const real one_over_tau;
		TERATHON_API extern const real two_over_tau;
		TERATHON_API extern const real four_over_tau;
		TERATHON_API extern const real one_over_two_tau;

		TERATHON_API extern const real pi;
		TERATHON_API extern const real two_pi;
		TERATHON_API extern const real four_pi;
		TERATHON_API extern const real three_pi_over_2;
		TERATHON_API extern const real three_pi_over_4;
		TERATHON_API extern const real two_pi_over_3;
		TERATHON_API extern const real four_pi_over_3;
		TERATHON_API extern const real pi_over_2;
		TERATHON_API extern const real pi_over_3;
		TERATHON_API extern const real pi_over_4;
		TERATHON_API extern const real pi_over_6;
		TERATHON_API extern const real pi_over_8;
		TERATHON_API extern const real pi_over_12;
		TERATHON_API extern const real pi_over_20;
		TERATHON_API extern const real one_over_pi;
		TERATHON_API extern const real one_over_two_pi;
		TERATHON_API extern const real one_over_four_pi;

		TERATHON_API extern const real sqrt_2;
		TERATHON_API extern const real sqrt_2_over_2;
		TERATHON_API extern const real sqrt_2_over_3;
		TERATHON_API extern const real sqrt_3;
		TERATHON_API extern const real sqrt_3_over_2;
		TERATHON_API extern const real sqrt_3_over_3;

		TERATHON_API extern const real ln_2;
		TERATHON_API extern const real one_over_ln_2;
		TERATHON_API extern const real ln_10;
		TERATHON_API extern const real one_over_ln_10;
		TERATHON_API extern const real ln_256;


		TERATHON_API extern const uint32 trigTable[256][2];
	}
}


#endif
