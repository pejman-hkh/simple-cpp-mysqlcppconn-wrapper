#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

class mysqlResult {
public:
	~mysqlResult() {
		delete res;
	}

    void manageException(sql::SQLException& e) {
		if ( e.getErrorCode() != 0 ) {
			std::cout << "# ERR: SQLException in " << __FILE__;
			std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
			std::cout << "# ERR: " << e.what();
			std::cout << " (MySQL error code: " << e.getErrorCode();
			std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		}        	
    }
 
	void get( const std::string& query, sql::Statement** stmt, sql::PreparedStatement** prep_stmt) {

	    try {
	    	if (query != "") {
	    		res = (*stmt)->executeQuery(query);
	    	} else {
	    	
	    		res = (*prep_stmt)->executeQuery();
	    		(*prep_stmt)->close();
	    	}
	    } catch (sql::SQLException &e) {
	        manageException(e);
	    }	
	}

    int count() {
    	return res->rowsCount();
    }

    bool fetch() {
    	return res->next();
    }

    bool next() {
    	return res->next();
    }
    
    int getInt( const std::string& field ) {
    	return res->getInt( field );
    }

    std::string getString( const std::string& field ) {
    	return res->getString( field );
    }

private:
	sql::ResultSet* res;
};


class mysqlWrapper
{
          
    public:

        mysqlWrapper()
        {
        	con = 0;
        	prep_stmt = 0;
        	stmt = 0;
        };


        ~mysqlWrapper() {
	
		    delete prep_stmt;
		    delete stmt;
		    delete con;
        }

        void manageException(sql::SQLException& e) {
			if ( e.getErrorCode() != 0 ) {
				std::cout << "# ERR: SQLException in " << __FILE__;
				std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
				std::cout << "# ERR: " << e.what();
				std::cout << " (MySQL error code: " << e.getErrorCode();
				std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
			}        	
        }
        
        mysqlWrapper& connect( const std::string& host, const std::string& user, const std::string& password = ""  ) {

		    try {
		        driver = get_driver_instance();

		        con = driver->connect(host, user, password);

		        _is_connected = true;
		    } catch (sql::SQLException &e){

		        manageException(e);
		    }


		    return *this;
        };
        
        void switchDb(const std::string& db_name) {
		    try {
		    	if( _is_connected ) {

		    		con->setSchema(db_name);
		    		stmt = con->createStatement();		    		
		    	}

		    } catch (sql::SQLException &e) {
		    	manageException(e);
		    }
        }
        
        mysqlWrapper& prepare(const std::string& query) {
        	_wilcard = 0;
			try{
				prep_stmt = con->prepareStatement(query);
			} catch (sql::SQLException &e){
				manageException(e);
			}
			return *this;
        }
        
        mysqlWrapper& setInt( const int& data) {
        	prep_stmt->setInt(++_wilcard, data);
        	return *this;
        }
        
        mysqlWrapper& setString( const std::string& data) {
        	prep_stmt->setString(++_wilcard, data);
        	return *this;
        }
        
        mysqlResult execute(const std::string& query = "") {
        	mysqlResult res;
        	res.get( query, &stmt, &prep_stmt );

        	return res;

        }


        bool is_connected() {
        	return _is_connected;
        }

    private:        
        int _wilcard = 0;
 		bool _is_connected = false;
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::PreparedStatement* prep_stmt;
};


int main() {


	mysqlWrapper db;
	db.connect( "tcp://127.0.0.1:3306", "root", "123456" ).switchDb( "pa" );

	if( db.is_connected() ) {

		mysqlResult res = db.prepare(" SELECT * FROM db1 ORDER BY id ASC LIMIT 0,2 ").execute();

		std::cout << "count is " << res.count() << std::endl;

		while( res.fetch() ) {
			std::cout << "here" << res.getString("id") << std::endl;
			
			mysqlResult res1 = db.prepare(" SELECT * FROM db2 WHERE db1id = ? ORDER BY id DESC LIMIT 0,5 ").setInt( res.getInt("id") ).execute();
			
			while( res1.fetch() ) {
				std::cout << "detail" << res1.getString("id") << std::endl;
			}

		}

		db.prepare("UPDATE db1 SET title = ? WHERE id = ? ")\
		.setString("bbbb")\
		.setInt(2).execute();
		db.prepare("UPDATE db1 SET title = ? WHERE id = ? ").setString("aaa").setInt(3).execute();
	}

	


}