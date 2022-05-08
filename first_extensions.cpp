#define DUCKDB_BUILD_LOADABLE_EXTENSION
#include "duckdb.hpp"
#include "h3api.h"

using namespace duckdb;


#define R 6371.0088


double haversineDistance(double th1, double ph1, double th2, double ph2) {
    double dx, dy, dz;
    ph1 -= ph2;

    dz = sin(th1) - sin(th2);
    dx = cos(ph1) * cos(th1) - cos(th2);
    dy = sin(ph1) * cos(th1);
    return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}


//this function is a H3 example porting get in input two geohash and 
// return distance expressed in variuos format  
inline string_t distance_fun(string_t origin, string_t destination) {
    
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


inline string_t hello_fun(string_t what) {
	return "Hello, " + what.GetString();
}


extern "C" {
DUCKDB_EXTENSION_API void first_extensions_init(duckdb::DatabaseInstance &db) {
	Connection con(db);
	con.BeginTransaction();
	con.CreateScalarFunction<string_t, string_t>("hello", {LogicalType(LogicalTypeId::VARCHAR)},
	                                             LogicalType(LogicalTypeId::VARCHAR), &hello_fun);

    con.CreateScalarFunction<string_t, string_t,string_t>("distance_in_km", {LogicalType(LogicalTypeId::VARCHAR), LogicalType(LogicalTypeId::VARCHAR)},
	                                             LogicalType(LogicalTypeId::VARCHAR), &distance_fun);


	con.Commit();
}

DUCKDB_EXTENSION_API const char *first_extensions_version() {
	return DuckDB::LibraryVersion();
}
}
