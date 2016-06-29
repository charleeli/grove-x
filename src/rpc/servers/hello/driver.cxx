#include <memory>
#include <cassert>
#include <iostream>

#include <odb/database.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/session.hxx>
#include <odb/callback.hxx>
#include <odb/connection.hxx>
#include <odb/transaction.hxx>
#include <odb/mysql/database.hxx>
#include <odb/mysql/tracer.hxx>
#include <odb/mysql/statement.hxx>
#include <odb/mysql/connection-factory.hxx>
#include <odb/mysql/exceptions.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/optional.hpp>

#include "OdbConnectionPool.h"
#include "OdbWrapperPool.h"
#include "../../orm/person.hxx"
#include "./person/person-odb.hxx"

using namespace std;
using namespace boost::posix_time;
using namespace odb::core;

static void
print_ages (unsigned short age, odb::result<person> r){
    cout << "over " << age << ':';

    for (odb::result<person>::iterator i (r.begin ()); i != r.end (); ++i)
    cout << ' ' << i->age ();

    cout << endl;
}

int
main (int argc, char* argv[]){
    typedef odb::query<person> query;
    typedef odb::result<person> result;

    int pos = string(argv[0]).find_last_of('/');
    string program = string(argv[0]).substr(pos+1);
    GLogHelper gh(program,"");

    try{
        {
            auto_ptr<odb::mysql::connection_factory> f (
                    new odb::mysql::connection_pool_factory (0,8));

            std::shared_ptr<database> db (new odb::mysql::database (
                    "root", "","ecsite","127.0.0.1",3306,NULL,NULL,0,f));

            person john ("John", "Doe", 33,NULL,from_time_t(time(NULL)),from_time_t(time(NULL)));
            person jane ("Jane", "Doe", 32,NULL,from_time_t(time(NULL)),from_time_t(time(NULL)));
            person joe ("Joe", "Dirt", 30,NULL,from_time_t(time(NULL)),from_time_t(time(NULL)));

            session s;
            transaction t (db->begin ());

            db->persist (john);
            db->persist (jane);
            db->persist (joe);

            t.commit ();
        }

        {
            OdbWrapperPool * odbWrapperPool = OdbWrapperPool::GetInstance("127.0.0.1", 3306,"root","","ecsite",8);
            cout <<"odbWrapperPool zise:" <<odbWrapperPool->Size()<<endl;
            OdbWrapperGuard odbWrapperGuard(odbWrapperPool,odbWrapperPool->GetOdbWrapper());
            OdbWrapper* odbWrapper = odbWrapperGuard.getOdbWrapper();
            cout <<"odbWrapperPool zise:" <<odbWrapperPool->Size()<<" odbWrapper:"<<odbWrapper<<endl;

            session s;
            transaction t (odbWrapper->begin ());

            //result r (owrapper->query<person> (query::age > 30));
            result r(odbWrapper->query<person> ("age > 30"));

            for (person& i: r)
            {
                cout << "Hello ,  "<< i.first()<<" "<<i.middle_name()<<" "<<i.created_time()<<" "<<i.updated_time()<<"!" << endl;
            }

            t.commit();

            transaction t1 (odbWrapper->begin ());

            person_stat ps (odbWrapper->query_value<person_stat> ());

            cout << endl
               << "count  : " << ps.count << endl
               << "min age: " << ps.min_age << endl
               << "max age: " << ps.max_age << endl;

            t1.commit ();
        }

        {
            session s;

            //odb::connection_ptr conn (db->connection());
            odb::connection_ptr conn (OdbConnectionPool::GetInstance("127.0.0.1", 3306,"root","","ecsite",8)->GetConnection());

            unsigned short age;
            odb::query<person> q (query::age > query::_ref (age));
            odb::prepared_query<person> pq (conn->prepare_query<person>("person-age-query", q));

            cout << endl;
            for (age = 50; age > 10; age -= 10)
            {
                transaction t (conn->begin ());

                result r (pq.execute ());
                print_ages (age, r);

                t.commit ();
            }

            transaction t (conn->begin ());
            conn->execute ("CREATE TABLE test (n INT PRIMARY KEY)");
            t.commit ();
        }
    }catch (const odb::exception& e){
        cerr << e.what () << endl;
        return 1;
    }
}
