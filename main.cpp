#include <iostream>
#include "mysqlWrapper.h"
#include <signal.h>
#include "utils.h"

mysqlWrapper db;

int main() {

	db.connect_uc("localhost", "root", "123456" ).switchDb("db_name");


	mysqlResult res = db.prepare("SELECT * FROM test_table LIMIT 0,1").execute();
	res.next();


	std::cout << res.getString("title");


	//update query
	db.prepare("UPDATE test_table SET title = ? WHERE id = ? ")\
	.setString("test it")\
	.setInt(1)\
	.execute();



	//insert query
	db.prepare("INSERT INTO test_table(title) VALUES(?)")\
	.setString("ttttt")\
	.execute();


	//
	mysqlResult res1 = db.prepare("SELECT title FROM test_table LIMIT 0,10").execute();
	
	int i = 0;
	while( res1.next() ) {

		std::cout << "res1 result " << res1.getString("title") << std::endl;
	

		//test nested query		
		mysqlResult res2 = db.prepare("SELECT title FROM test_table LIMIT ?,1").setInt(i++).execute();
		res2.next();

		std::cout << "res2 result" << res2.getString("title") << std::endl;

	}


	return 0;
}