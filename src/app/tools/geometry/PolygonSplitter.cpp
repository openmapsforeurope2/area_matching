#include <app/tools/geometry/PolygonSplitter.h>

//SOCLE
#include <ign/data/all.h>
#include <ign/geometry/graph/detail/NextEdge.h>


namespace app{
namespace tools{
namespace geometry{

	///
	///
	///
	PolygonSplitter::PolygonSplitter( ign::geometry::Polygon const& poly, double scale ):
		_scale( scale ),
		_count(0),
		_numRings(0)
	{
		_planarizer.reset( new ign::geometry::graph::tools::SnapRoundPlanarizer< detail::PolygonSplitterGraphType >( _graph, _scale ) );

        for( size_t i = 0 ; i < poly.numRings() ; ++i )
			_addGeometry( poly.ringN(i) );

		_numRings = _count;
	}

	///
	///
	///
	PolygonSplitter::~PolygonSplitter()
	{
	}

	///
	///
	///
	void PolygonSplitter::addCuttingGeometry( ign::geometry::Geometry const& geom )
	{
		switch( geom.getGeometryType() )
		{
			case ign::geometry::Geometry::GeometryTypeLineString :{
				_addGeometry( geom.asLineString() );
				break;}
			case ign::geometry::Geometry::GeometryTypePolygon :{
				_addGeometry( geom.asPolygon() );
				break;}
			case ign::geometry::Geometry::GeometryTypeMultiLineString :{
				_addGeometry( geom.asMultiLineString() );
				break;}
			case ign::geometry::Geometry::GeometryTypeMultiPolygon :{
				_addGeometry( geom.asMultiPolygon() );
				break;}
			default :{
				IGN_THROW_EXCEPTION( "epg::tools::geometry::PolygonSplitter:addCuttingGeometry : geometry type "+geom.getGeometryTypeName()+" not allowed" );
				break;
			}
		}
	}

	///
	///
	///
	void PolygonSplitter::_addGeometry( ign::geometry::LineString const& ls )
	{
		_planarizer->addEdge( ls, ign::data::Integer( _count++ ).toString() );
	}

	///
	///
	///
	void PolygonSplitter::_addGeometry( ign::geometry::Polygon const& poly )
	{
		for( size_t i = 0 ; i < poly.numRings() ; ++i )
			_addGeometry( poly.ringN(i) );
	}

	///
	///
	///
	void PolygonSplitter::_addGeometry( ign::geometry::MultiLineString const& mls )
	{
		for( size_t i = 0 ; i < mls.numGeometries() ; ++i )
			_addGeometry( mls.lineStringN(i) );
	}

	///
	///
	///
	void PolygonSplitter::_addGeometry( ign::geometry::MultiPolygon const& mp )
	{
		for( size_t i = 0 ; i < mp.numGeometries() ; ++i )
			_addGeometry( mp.polygonN(i) );
	}

	///
	///
	///
	void PolygonSplitter::split( std::vector< ign::geometry::Polygon >& vPolygons )
	{
		_split();

		std::set<face_descriptor> sHoleFaces = _getHoleFaces();

		face_iterator fit, fend;
		for( _graph.faces( fit, fend ) ; fit != fend ; ++fit )
		{
			IGN_ASSERT( _graph[ *fit ].isFinite );
			if( *_graph[ *fit ].isFinite )
			{
				ign::geometry::GeometryPtr geom( _graph.getGeometry( *fit ).clone() );

				if (sHoleFaces.find(*fit) != sHoleFaces.end()) continue;

				_getPolygons( *geom, vPolygons );
			}
		}
	}

	///
	///
	///
	std::set<PolygonSplitter::face_descriptor> PolygonSplitter::_getHoleFaces() const 
	{
		std::set<face_descriptor> sHoleFaces;

		std::set<face_descriptor> sFaceTouchingExtBound;
		std::set<face_descriptor> sTreatedFaces;
		std::vector<std::set<face_descriptor>> vsGroups;

		face_iterator fit, fend;
		for( _graph.faces( fit, fend ) ; fit != fend ; ++fit ) {
			sTreatedFaces.insert(*fit);

			//DEBUG
			// ign::geometry::Polygon faceG = _graph.getGeometry( *fit );
			// if (faceG.intersects(ign::geometry::Point(4017319.3,3084558.2))) {
			// 	bool test = true;
			// }
			// if (faceG.intersects(ign::geometry::Point(4017264.2803,3084799.8512))) {
			// 	bool test = true;
			// }
			// if (faceG.intersects(ign::geometry::Point(4017264.13,3084790.79))) {
			// 	bool test = true;
			// }
			// if (faceG.intersects(ign::geometry::Point(4017259.3258,3084743.6678))) {
			// 	bool test = true;
			// }

			oriented_edge_descriptor startEdge = _graph.getIncidentEdge( *fit );
			oriented_edge_descriptor nextEdge = startEdge;
			do{
				std::vector< std::string > vOrigins = _graph.origins( nextEdge.descriptor );

				bool isCuttingEdge = true;
				for( size_t i = 0 ; i < vOrigins.size() ; ++i ) 
				{
					int origin = ign::data::String( vOrigins[i] ).toInteger();
					if( origin == 0 ) sFaceTouchingExtBound.insert(*fit);
					if( origin < _numRings ) isCuttingEdge = false;
				}

				if (isCuttingEdge) {
					std::pair< bool, face_descriptor > foundRf = _graph.rightFace( nextEdge );
					

					if (foundRf.first) {
						if (sTreatedFaces.find(foundRf.second) == sTreatedFaces.end()) {
							vsGroups.push_back(std::set<face_descriptor>());
							vsGroups.back().insert(*fit);
							vsGroups.back().insert(foundRf.second);
						}						
					}
				}

				nextEdge = ign::geometry::graph::detail::nextEdge( nextEdge, _graph );
			}while( nextEdge != startEdge );
		}

		//--
		_gatherGroups(vsGroups);

		//--
		for (std::vector<std::set<face_descriptor>>::const_iterator vit = vsGroups.begin() ; vit != vsGroups.end() ; ++vit ) {
			bool isHole = true;
			for (std::set<face_descriptor>::const_iterator sit = vit->begin() ; sit != vit->end() ; ++sit ) {
				if (sFaceTouchingExtBound.find(*sit) != sFaceTouchingExtBound.end()) {
					isHole = false;
					break;
				}
			}

			if(isHole)
				sHoleFaces.insert(vit->begin(), vit->end());
		}

		return sHoleFaces;		
	}

	///
	///
	///
	void PolygonSplitter::_gatherGroups( std::vector<std::set<face_descriptor>> & vsGroups ) const
	{
		size_t previousSize = vsGroups.size();

		do {
			previousSize = vsGroups.size();
			for(int i = vsGroups.size()-1 ; i > 0 ; --i) {
				bool found = false;
				for(size_t j = 0 ; j < i ; ++j) {
					for (std::set<face_descriptor>::const_iterator sit = vsGroups[i].begin() ; sit != vsGroups[i].end() ; ++sit ) {
						if (vsGroups[j].find(*sit) != vsGroups[j].end()) {
							found = true;
							break;
						}
					}
					if (found) {
						vsGroups[j].insert(vsGroups[i].begin(), vsGroups[i].end());
						break;
					}
				}
				if (found) vsGroups.erase(vsGroups.begin()+i);
			}
		} while (previousSize != vsGroups.size());
	}

	///
	///
	///
	void PolygonSplitter::_getPolygons( ign::geometry::Geometry const& geom, std::vector< ign::geometry::Polygon >& vPolygons )const
	{
		switch( geom.getGeometryType() )
		{
			case ign::geometry::Geometry::GeometryTypePolygon :
				{
					vPolygons.push_back( geom.asPolygon() );
					break;
				}
			case ign::geometry::Geometry::GeometryTypeMultiPolygon :
				{
					ign::geometry::MultiPolygon const& mp = geom.asMultiPolygon();
					for( size_t i = 0 ; i < mp.numGeometries() ; ++i )
						vPolygons.push_back( mp.polygonN(i) );
					break;
				}
			case ign::geometry::Geometry::GeometryTypeGeometryCollection :
				{
					ign::geometry::GeometryCollection const& gc = geom.asGeometryCollection();
					for( size_t i = 0 ; i < gc.numGeometries() ; ++i )
					{
						if( gc.geometryN(i).isPolygon() )
							vPolygons.push_back( gc.geometryN(i).asPolygon() );
						if( gc.geometryN(i).isMultiPolygon() )
						{
							ign::geometry::MultiPolygon const& mp = gc.geometryN(i).asMultiPolygon();
							for( size_t k = 0 ; k < mp.numGeometries() ; ++k )
								vPolygons.push_back( mp.polygonN(k) );
						}
					}
					break;
				}
		}
	}

	///
	///
	///
	void PolygonSplitter::_split()
	{
		_planarizer->planarize();
		ign::geometry::graph::detail::createFaces( _graph );

		std::list< face_descriptor > lFaces;
		face_iterator fit, fend;
		for( _graph.faces( fit, fend ) ; fit != fend ; ++fit )
			lFaces.push_back( *fit );

		while( !lFaces.empty() )
		{
			face_descriptor f = lFaces.back();
			lFaces.pop_back();

			oriented_edge_descriptor startEdge = _graph.getIncidentEdge( f );
			oriented_edge_descriptor nextEdge = startEdge;
			do{

				std::pair< bool, face_descriptor > foundRf = _graph.rightFace( nextEdge );
				if( !foundRf.first )
				{
					if( _isBoundary( nextEdge.descriptor ) ) _graph[ f ].isFinite = false;
					else _graph[ f ].isFinite = true;
					break;
				}
				else
				{
					if( _graph[ foundRf.second ].isFinite )
					{
						if( _isBoundary( nextEdge.descriptor ) ) _graph[ f ].isFinite = *_graph[ foundRf.second ].isFinite;
						else _graph[ f ].isFinite = !*_graph[ foundRf.second ].isFinite;
						break;
					}
				}

				nextEdge = ign::geometry::graph::detail::nextEdge( nextEdge, _graph );
			} while( nextEdge != startEdge );

			if( !_graph[ f ].isFinite ) lFaces.push_front( f );/*non traite*/
		}
	}

	///
	///
	///
	bool PolygonSplitter::_isBoundary( edge_descriptor e )const
	{
		std::vector< std::string > vOrigins = _graph.origins( e );

		int numBorders = 0;
		bool hasBoundary = false;
		for( size_t i = 0 ; i < vOrigins.size() ; ++i ) 
		{
			int origin = ign::data::String( vOrigins[i] ).toInteger();
			if( origin < _numRings ) ++numBorders;
			else hasBoundary = true;
		}
		
		if( !hasBoundary ) return false;
		if( numBorders % 2 != 0 ) return false;
		return true;
	}

}
}
}
