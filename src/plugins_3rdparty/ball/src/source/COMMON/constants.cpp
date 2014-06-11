// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// $Id: constants.C,v 1.6 2002/02/27 12:20:33 sturm Exp $

#include <BALL/COMMON/constants.h>

namespace BALL
{
	namespace Constants
	{
		// EPSILON (used for comparisons)
		double EPSILON = 1e-6;

		// PI
		const double  PI = 3.14159265358979323846L;

		// Euler's number - base of the natural logarithm
		const double  E  = 2.718281828459045235L;
		
		//	Elementary charge.
		const double	ELEMENTARY_CHARGE = 1.60217738E-19L;  	 // C     
	
		/// Elementary charge (alias)
		const double	e0								=	ELEMENTARY_CHARGE;

		// Electron mass.
		const double	ELECTRON_MASS   	= 9.1093897E-31L;   	 // kg

		// Proton mass.
		const double	PROTON_MASS     	= 1.6726230E-27L;   	 // kg

		// Neutron mass.
		const double	NEUTRON_MASS    	= 1.6749286E-27L;   	 // kg

		// Avogadro constant.
		const double	AVOGADRO        	= 6.0221367E+23L;   	 // 1 / mol

		// Avogadro constant (alias)
		const double	NA								= AVOGADRO;

		// Avogadro constant (alias)
		const double	MOL             	= AVOGADRO;

		// Boltzmann constant.
		const double	BOLTZMANN       	= 1.380657E-23L;  	   // J / K

		// Boltzmann constant (alias)
		const double	k	        				= BOLTZMANN;
		
		// Planck constant.
		const double	PLANCK 	         	= 6.6260754E-34L;      // J * sec

		// Planck constant (alias)
		const double	h       	   			= PLANCK;

		// Gas constant (= NA * k)	
		const double	GAS_CONSTANT 	  	= NA * k;

		// Gas constant (alias)
		const double R 								= GAS_CONSTANT;

		// Faraday constant (= NA * e0)
		const double	FARADAY         	= NA * e0;

		// Faraday constant (alias)
		const double	F    							= FARADAY;

		// Bohr radius.
		const double	BOHR_RADIUS     	= 5.29177249E-11L;     // m

		// Bohr radius (alias)
		const double	a0     						= BOHR_RADIUS;

		//  the following values from: 
		//  P.W.Atkins: Physical Chemistry, 5th ed., Oxford University Press, 1995

		// Vacuum permittivity.
		const double	VACUUM_PERMITTIVITY    	= 8.85419E-12L;     // C^2 / (J * m)

		// Vacuum permeability.
		const double	VACUUM_PERMEABILITY     = (4 * PI * 1E-7L);	// J s^2 / (C^2 * m)

		// Speed of light.
		const double	SPEED_OF_LIGHT          = 2.99792458E+8L;	  // m / s

		// Speed of Light (alias)
		const double	c												= SPEED_OF_LIGHT;

		// Gravitational constant.
		const double	GRAVITATIONAL_CONSTANT  = 6.67259E-11L;    	// N m^2 / kg^2

		// Fine structure constant.
		const double	FINE_STRUCTURE_CONSTANT = 7.29735E-3L;   		// 1      
			
		// Degree per rad.
		const double	DEG_PER_RAD				= 57.2957795130823209L;

		// Rad per degree.
		const double	RAD_PER_DEG			 	= 0.0174532925199432957L;

		// mm per inch.
		const double	MM_PER_INCH 			= 25.4L;

		// m per foot.
		const double	M_PER_FOOT  			= 3.048L;

		// Joule per calorie
		const double	JOULE_PER_CAL     = 4.184;

		// Calories per Joule.
		const double	CAL_PER_JOULE     = (1 / 4.184);

	}
}
