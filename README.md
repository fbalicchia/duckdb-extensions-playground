# DuckDB <> Extension Playground





To build, type 
```
make update; make
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