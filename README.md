tinydb
======

queryOpt tinydb WS1415


<b> Build </b><br/>
build by calling 
 ```
make
 ```

<b> load database </b><br/>
load uni database with
 ```
cd data
./loaduni
 ```
or load tpch database with
 ```
cd data/tpch/
sudo ./loadtpch-cpp
 ```
 
<b> execute programs </b><br/>
Binaries can be found in the bin-folder.<br/>
Examples: Execute a query q1 on the uni database with
 ```
./bin/isql data/uni q1.sql
```
Execute a query q2 on the tpch database with
 ```
sudo ./bin/isql data/tpch/tpch q2.sql
```
