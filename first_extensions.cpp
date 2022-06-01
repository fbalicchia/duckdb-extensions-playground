#define DUCKDB_BUILD_LOADABLE_EXTENSION
#include "duckdb.hpp"
#include "h3api.h"

using namespace duckdb;


#define R 6371.0088
#define STRING_LENGTH 32



template<typename TYPE>
inline void udf_vectorized(DataChunk &input, ExpressionState &state, Vector &result) {
	// set the result vector type
	result.SetVectorType(VectorType::FLAT_VECTOR);
	// get a raw array from the result
	auto result_data = FlatVector::GetData<TYPE>(result);
	auto ldata = FlatVector::GetData<TYPE>(input.data[0]);
	auto &mask = FlatVector::Validity(input.data[0]);
	FlatVector::SetValidity(result, mask);
	// handling the data
    for (idx_t i = 0; i < input.size(); i++) {
			if (!mask.RowIsValid(i)) {
				continue;
			}
			result_data[i] = ldata[i];
	}
    
}


double haversineDistance(double th1, double ph1, double th2, double ph2) {
    double dx, dy, dz;
    ph1 -= ph2;

    dz = sin(th1) - sin(th2);
    dx = cos(ph1) * cos(th1) - cos(th2);
    dy = sin(ph1) * cos(th1);
    return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

/**
     * Returns the distance between <code>a</code> and <code>b</code>. This is the grid distance, or
     * distance expressed in number of H3 cells.
     *
     * <p>In some cases H3 cannot compute the distance between two indexes. This can happen because:
     *
     * <ul>
     *   <li>The indexes are not comparable (difference resolutions, etc)
     *   <li>The distance is greater than the H3 core library supports
     *   <li>The H3 library does not support finding the distance between the two cells, because of
     *       pentagonal distortion.
     * </ul>
     *
     * @param a An H3 index.
     * @param b Another H3 index.
     * @return Distance between the two in grid cells.
     * @throws RuntimeException H3 cannot compute the distance because the two cells have different
     *     resolutions.
     */

inline string_t h3_distance(string_t origin, string_t destination) {
    
    H3Index h3HQ1;
    H3Index h3HQ2;
    stringToH3(origin.GetString().c_str(), &h3HQ1);
    stringToH3(destination.GetString().c_str(), &h3HQ2);
    LatLng geoHQ1, geoHQ2;
    cellToLatLng(h3HQ1, &geoHQ1);
    cellToLatLng(h3HQ2, &geoHQ2);

    int64_t distance;
    assert(gridDistance(h3HQ1, h3HQ2, &distance) == E_SUCCESS);

    char result[200];
    snprintf(result,200,"%f",haversineDistance(geoHQ1.lat, geoHQ1.lng, geoHQ2.lat, geoHQ2.lng));
    return result;

}





//Directed edge functions

//cellsToDirectedEdge done

//isValidDirectedEdge done

// getDirectedEdgeOrigin done

//getDirectedEdgeDestination done

//directedEdgeToCells to understand how manage 

inline bool h3_is_valid_cell(string_t index) {
    H3Index h3Index;
    stringToH3(index.GetString().c_str(), &h3Index);
    return  isValidCell(h3Index) == 1;
}


inline bool h3_is_valid_directed_edge(string_t index) {
    H3Index h3Index;
    stringToH3(index.GetString().c_str(), &h3Index);
    return  isValidDirectedEdge(h3Index) == 1;
}

inline int h3_get_base_cell_number(string_t index) {
        H3Index h3Index;
        stringToH3(index.GetString().c_str(), &h3Index);
        return getBaseCellNumber(h3Index);
}


inline string_t h3_cells_to_directed_edge(string_t origin, string_t destination) {
        H3Index h3HQ1;
        H3Index h3HQ2;
        stringToH3(origin.GetString().c_str(), &h3HQ1);
        stringToH3(destination.GetString().c_str(), &h3HQ2);
        H3Index h3Index;
        assert(cellsToDirectedEdge(h3HQ1, h3HQ2, &h3Index) == E_SUCCESS);
        char result[STRING_LENGTH];
        h3ToString(h3Index, result, STRING_LENGTH);
        return result;
}


inline string_t h3_get_directed_edge_origin(string_t origin) {
        H3Index h3HQ1;       
        stringToH3(origin.GetString().c_str(), &h3HQ1);
        H3Index h3Index;
        assert(getDirectedEdgeOrigin(h3HQ1, &h3Index) == E_SUCCESS);
        char result[STRING_LENGTH];
        h3ToString(h3Index, result, STRING_LENGTH);
        return result;
}

inline string_t h3_get_directed_edge_destination(string_t origin) {
        H3Index h3HQ1;       
        stringToH3(origin.GetString().c_str(), &h3HQ1);
        H3Index h3Index;
        assert(getDirectedEdgeDestination(h3HQ1, &h3Index) == E_SUCCESS);
        char result[STRING_LENGTH];
        h3ToString(h3Index, result, STRING_LENGTH);
        return result;
}





inline string_t hello_fun(string_t what) {
	return "Hello, " + what.GetString();
}

bool bigger_than_four(int value) {
    return value > 4;
}



extern "C" {
DUCKDB_EXTENSION_API void first_extensions_init(duckdb::DatabaseInstance &db) {
	Connection con(db);
	con.BeginTransaction();
	con.CreateScalarFunction<string_t, string_t>("hello", {LogicalType(LogicalTypeId::VARCHAR)},
	                                             LogicalType(LogicalTypeId::VARCHAR), &hello_fun);
    con.CreateScalarFunction<string_t, string_t,string_t>("h3_distance", {LogicalType(LogicalTypeId::VARCHAR), LogicalType(LogicalTypeId::VARCHAR)},
	                                             LogicalType(LogicalTypeId::VARCHAR), &h3_distance);
    con.CreateScalarFunction<bool, int>("bigger_than_four", &bigger_than_four);
    con.CreateScalarFunction<bool, string_t>("h3_is_valid_cell", &h3_is_valid_cell);
    con.CreateScalarFunction<int, string_t>("h3_get_base_cell_number", &h3_get_base_cell_number);
    con.CreateScalarFunction<string_t, string_t,string_t>("h3_cells_to_directed_edge", &h3_cells_to_directed_edge);
    con.CreateScalarFunction<string_t, string_t>("h3_get_directed_edge_origin", &h3_get_directed_edge_origin);
    con.CreateScalarFunction<string_t, string_t>("h3_get_directed_edge_destination", &h3_get_directed_edge_destination);
    con.CreateVectorizedFunction<int, int>("udf_vectorized_int", &udf_vectorized<int>);
	con.Commit();
}

DUCKDB_EXTENSION_API const char *first_extensions_version() {
	return DuckDB::LibraryVersion();
}
}
