#ifndef MYSQL_WRAPPER
#define MYSQL_WRAPPER
#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <ctime>
#include <time.h>

using namespace std;

class mysqlResult {
public:

    mysqlResult() {
    }

    ~mysqlResult() {
        if( res ) {
            res->close();
            delete res;
            res = NULL;
        }

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
 
    void get( sql::PreparedStatement* & prep_stmt ) {

        try {
            if( prep_stmt ) {
                res = prep_stmt->executeQuery();
                prep_stmt->close();
            }

        } catch (sql::SQLException &e) {
            manageException(e);
        }
    }

    int count() {
        return res?res->rowsCount():0;
    }

    bool fetch() {
        return res?res->next():false;
    }

    bool next() {
        return res?res->next():false;
    }
    
    int getInt( const std::string& field ) {
        return res?res->getInt( field ):0;
    }

    std::string getString( const std::string& field ) {
        return res?res->getString( field ):"";
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
        };

        ~mysqlWrapper() {
            con->close();

            delete prep_stmt;
            delete con;
            con = NULL;
            prep_stmt = NULL;
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
        
    private :
        std::string _host;
        std::string _user;
        std::string _password;
        std::string _db;
 
    public :
        mysqlWrapper& connect( const std::string& host, const std::string& user, const std::string& password = ""  ) {
            _host = host;
            _user = user;
            _password = password;

            try {
                driver = get_driver_instance();

                con = driver->connect(host, user, password);

                _is_connected = true;
            } catch (sql::SQLException &e){
                manageException(e);
            }


            return *this;
        };
        
        mysqlWrapper& connect_uc( const std::string& host, const std::string& user, const std::string& password = "" ) {
            while( true ) {
                connect( host, user, password );
                if( _is_connected ) break;
                sleep(1);
            }

            return *this;
        }

        void switchDb(const std::string& db_name) {
            _db = db_name;

            try {
                if( _is_connected ) {
                    con->setSchema(db_name);              
                }

            } catch (sql::SQLException &e) {
                manageException(e);
            }
        }
        
        void reconnect() {
            connect_uc( _host, _user, _password ).switchDb( _db );
        }


        mysqlWrapper& prepare(const std::string& query) {

            delete prep_stmt;
            prep_stmt = NULL;
            _wilcard = 0;

            try{
                prep_stmt = con->prepareStatement(query);
            } catch ( sql::SQLException &e ){
                manageException(e);
            }
            
            if( ! prep_stmt ) {
                _is_connected = false;
                reconnect();
                prepare( query );

            }

            return *this;
        }
        
        mysqlWrapper& setInt( const int& data) {
            if( prep_stmt )
                prep_stmt->setInt(++_wilcard, data);

            return *this;
        }
        
        mysqlWrapper& setString( const std::string& data) {
            if( prep_stmt )
                prep_stmt->setString(++_wilcard, data);
            return *this;
        }
        
        mysqlResult execute() {
            mysqlResult res;
            res.get( prep_stmt );

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
        sql::PreparedStatement* prep_stmt;
};

#endif