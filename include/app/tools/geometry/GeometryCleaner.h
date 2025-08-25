#ifndef _APP_TOOLS_GEOMETRY_GEOMETRYCLEANER_H_
#define _APP_TOOLS_GEOMETRY_GEOMETRYCLEANER_H_

#include <set>

//SOCLE
#include <ign/geometry/MultiPolygon.h>

namespace app{
namespace tools{
namespace geometry{



	/// @brief Classe utilitaire pour supprimer les artefacts résultants 
	/// d'opérations géométriques
	class GeometryCleaner{

	public:

		/// @brief Constructeur
		/// @param angleTreshold  Angle minimum autorisé entre deux segment
		GeometryCleaner( double angleTreshold = 0.001 );

		/// \brief Destructeur
		~GeometryCleaner();

		/// @brief Ajoute une géométrie de découpe
		/// @param geom Géométrie à nettoyer
		static void Compute( ign::geometry::Geometry & geom, double angleTreshold = 0.001 );

	private:

		//--
		double       _angleTreshold;


	private:

		//--
		void _compute( ign::geometry::Geometry & geom ) const;
		//--
		void _compute( ign::geometry::LineString & ls ) const;
		//--
		void _compute( ign::geometry::Polygon & poly ) const;
		//--
		void _compute( ign::geometry::MultiLineString & mls ) const;
		//--
		void _compute( ign::geometry::MultiPolygon & mp ) const;
		
	};

}
}
}

#endif