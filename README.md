# DuckDB <> Extension Playground

This is a simple project that permits me to play and understand Duckdb UDF function [API](https://duckdb.org/docs/api/cpp).
The main class is `first_extension` which provides two functions. One is the classic hello world the other one is a simple 
a function that uses [H3](https://github.com/uber/h3) to calculate the distance in KM between two points expressed in geohash


Load submodule
```
git submodule update --init --recursive 
```


To build, type 
```
make
```

To run, run the bundled `duckdb` shell:
```
 ./duckdb/build/release/duckdb 
```

Then, load the Postgres extension like so:
```SQL
LOAD 'build/release/first_extensions.duckdb_extension';
```

```SQL
 select hello('World from Italy');
```


```SQL
select distance_in_km('8f2830828052d25', '8f283082a30e623');
```