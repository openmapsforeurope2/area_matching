#ifndef _APP_TOOLS_ZTOOLS_H_
#define _APP_TOOLS_ZTOOLS_H_

// SOCLE
#include <ign/geometry/Geometry.h>

namespace app{
namespace tools{

    /// @brief Fonction utilitaire pour le remplissage de la coordonnée Z
    /// @param geom Géométrie à laquelle on veut affecter un Z
    /// @param value Valeur à affecter à la coordonnée Z
    void zFiller( ign::geometry::Geometry & geom, double value );

    /// @brief Fonction utilitaire pour supprimer les points dont la coordonnée Z 
    /// a une valeur spécifique. Cette fonction est utile pour supprimer les points
    /// intermédiaires inutiles créés artificiellement lors d'opération de traitement 
    /// comme la découpe.
    /// @param geom Géométrie à nettoyer
    /// @param value Valeur de la coordonnée Z pour laquelle les points doivent être supprimés
    void removePointWithZ( ign::geometry::Geometry & geom, double value );

}
}

#endif